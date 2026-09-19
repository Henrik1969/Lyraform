#include <cerrno>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <new>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <sys/file.h>
#include <unistd.h>

#include <flowcontracts/validate.hpp>
#include <flowcontracts/bounded_input.hpp>
#include <flowcontracts/diagnostics.hpp>

#include "structured_plan.hpp"

namespace {

constexpr std::string_view VERSION = "0.1.0";

struct Options { std::string optimization_path, binding_path, llvm_path, target_name; bool structured_diagnostics = false; };
struct OutputError : std::runtime_error { using std::runtime_error::runtime_error; };
struct OutputUncertain : std::runtime_error { using std::runtime_error::runtime_error; };

Options parse_options(int argc, char** argv) {
    Options options;
    for (int i = 1; i < argc; ++i) {
        const std::string argument = argv[i];
        if (argument == "--diagnostics") { if (++i >= argc || std::string(argv[i]) != "json") throw std::runtime_error("--diagnostics requires json"); options.structured_diagnostics = true; }
        else if (argument == "--emit-llvm") { if (++i >= argc) throw std::runtime_error("--emit-llvm requires a path"); options.llvm_path = argv[i]; }
        else if (argument == "--binding-report") { if (++i >= argc) throw std::runtime_error("--binding-report requires a path"); options.binding_path = argv[i]; }
        else if (argument == "--target") { if (++i >= argc) throw std::runtime_error("--target requires a name"); options.target_name = argv[i]; }
        else if (!argument.empty() && argument.front() == '-') throw std::runtime_error("unknown option '" + argument + "'");
        else if (options.optimization_path.empty()) options.optimization_path = argument;
        else throw std::runtime_error("too many input paths");
    }
    return options;
}

std::string read_file_or_stdin(const std::string& path) {
    if (!path.empty()) { std::ifstream file(path); if (!file) throw std::runtime_error("cannot open report"); return flowcontracts::read_bounded(file, "report"); }
    return flowcontracts::read_bounded(std::cin, "report");
}

std::string quote(std::string_view value) {
    return flowcontracts::json::serialize(std::string(value));
}

void write_structured_failure(std::string_view code, std::string_view stage, std::string_view message,
                              std::string_view disposition = "no_artifact") noexcept {
    std::fputs("{\"status\":\"failed\",\"code\":\"", stderr);
    flowcontracts::write_json_string(stderr, code);
    std::fputs("\",\"stage\":\"", stderr);
    flowcontracts::write_json_string(stderr, stage);
    std::fputs("\",\"message\":\"", stderr);
    flowcontracts::write_json_string(stderr, message);
    std::fputs("\",\"disposition\":\"", stderr);
    flowcontracts::write_json_string(stderr, disposition);
    std::fputs("\"}\n", stderr);
}

void write_atomic_file(const std::string& path, std::string_view contents) {
    const auto slash = path.find_last_of('/');
    const std::string parent = slash == std::string::npos ? "." :
                               slash == 0 ? "/" : path.substr(0, slash);
    const std::string leaf = slash == std::string::npos ? path : path.substr(slash + 1);
    if (leaf.empty()) throw OutputError("LLVM output path has no file name");
    const std::string temporary = leaf + ".tmp.lyraform-v1";
    const int directory = open(parent.c_str(), O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    if (directory < 0) throw OutputError("cannot open LLVM output directory");
    if (flock(directory, LOCK_EX) != 0) {
        close(directory);
        throw OutputError("cannot lock LLVM output directory");
    }
    if (unlinkat(directory, temporary.c_str(), 0) != 0 && errno != ENOENT) {
        close(directory);
        throw OutputError("cannot remove stale private LLVM output");
    }
    const int descriptor = openat(directory, temporary.c_str(),
                                  O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC | O_NOFOLLOW, 0600);
    if (descriptor < 0) {
        close(directory);
        throw OutputError("cannot create private LLVM output");
    }
    FILE* file = fdopen(descriptor, "wb");
    bool published = false;
    if (file) {
        bool complete = std::fwrite(contents.data(), 1, contents.size(), file) == contents.size();
        if (complete && std::fflush(file) != 0) complete = false;
        if (complete && fsync(descriptor) != 0) complete = false;
        if (std::fclose(file) != 0) complete = false;
        if (complete && renameat(directory, temporary.c_str(), directory, leaf.c_str()) == 0) published = true;
    } else {
        close(descriptor);
    }
    if (!published) {
        unlinkat(directory, temporary.c_str(), 0);
        close(directory);
        throw OutputError("cannot publish LLVM output");
    }
    bool durable = fsync(directory) == 0;
    if (close(directory) != 0) durable = false;
    if (!durable)
        throw OutputUncertain("LLVM output is published but parent directory durability is uncertain");
}

int lower(std::string_view report, const Options& options, std::string_view binding_report) {
#ifdef FLOWLOWER_TEST_ALLOCATION_FAILURE
    (void)report;
    (void)options;
    (void)binding_report;
    throw std::bad_alloc();
#endif
    using namespace flowlower::structured;
    const auto root = Parser{std::string(report)}.parse();
    if (const auto* plan = field(root, "lowering_plan")) flowcontracts::require_executable_targets(*plan);
    if (const auto* plan = field(root, "lowering_plan"))
        if (const auto* graph = field(*plan, "source_graph")) {
            if (!flowcontracts::source_graph(*graph, "$.lowering_plan.source_graph").executable)
                throw std::runtime_error("source graph execution is not admitted");
        }
    const auto input_format = text(field(root, "format"));
    if (input_format != "flowoptimize.optimization_report" && input_format != "flowcore.backend_lowering_artifact")
        throw std::runtime_error("input is not a backend lowering artifact");
    const auto input_version = integer(field(root, "version"), "version");
    if (input_version != 1 && !(input_format == "flowcore.backend_lowering_artifact" && input_version == 2))
        throw std::runtime_error("unsupported Flowoptimize report version");
    if (input_format == "flowcore.backend_lowering_artifact") {
        flowcontracts::validate_backend_lowering_artifact(root);
        if (!binding_report.empty()) throw std::runtime_error("backend lowering artifacts already contain authorization evidence");
        if (input_version == 2) {
            const auto* policy = field(root, "target_policy");
            const auto* backend = field(*policy, "backend");
            const auto* architecture = field(*policy, "architecture");
            const auto* abi = field(*policy, "abi");
            if (text(field(*backend, "name")) != "llvm" || integer(field(*backend, "artifact_version"), "artifact_version") != 1 ||
                text(field(*architecture, "name")) != "host" || integer(field(*architecture, "word_bits"), "word_bits") != 64 ||
                text(field(*abi, "name")) != "flowcore-host-c" || integer(field(*abi, "version"), "abi.version") != 1) {
                std::cout << "{\"backend\":\"llvm\",\"format\":\"flowlower.lowering_report\",\"reason\":\"target policy is incompatible with the LLVM backend\",\"status\":\"unsupported\",\"version\":1}\n";
                return 2;
            }
            const auto* capabilities = field(*policy, "capabilities");
            for (const auto& required : array(field(*capabilities, "required"), "target_policy.capabilities.required")) {
                if (text(&required) != "llvm-host-toolchain") {
                    std::cout << "{\"backend\":\"llvm\",\"format\":\"flowlower.lowering_report\",\"reason\":\"target policy requires an unavailable provider capability\",\"status\":\"unsupported\",\"version\":1}\n";
                    return 2;
                }
            }
        }
    }
    if (text(field(root, "status")) != "ready") {
        std::cout << "{\n  \"format\": \"flowlower.lowering_report\",\n  \"version\": 1,\n  \"status\": \"blocked\",\n  \"backend\": \"llvm\",\n  \"reason\": \"optimization stage is not ready\"\n}\n";
        return 2;
    }
    if (input_format == "flowoptimize.optimization_report") {
        flowcontracts::validate_optimization_report(root);
        const auto* plan = field(root, "lowering_plan");
        flowcontracts::validate_lowering_authority(*plan);
        if (text(field(*plan, "status")) != "ready")
            throw flowcontracts::json::Error("$.lowering_plan.status", "lowering plan is not ready");
    }

    const auto* targets_value = field(root, "targets");
    Array targets;
    if (targets_value) targets = array(targets_value, "targets");
    std::string selected_target = options.target_name.empty() ? "main" : options.target_name;
    if (input_format == "flowcore.backend_lowering_artifact") {
        if (!options.target_name.empty()) throw std::runtime_error("target is already fixed by the backend lowering artifact");
        if (const auto* target = field(root, "target")) selected_target = text(field(*target, "name"));
        targets.clear();
    }
    if (!targets.empty()) {
        if (options.target_name.empty()) {
            std::cout << "{\n  \"format\": \"flowlower.lowering_report\",\n  \"version\": 1,\n  \"status\": \"blocked\",\n  \"backend\": \"llvm\",\n  \"reason\": \"multiple targets require explicit --target selection\"\n}\n";
            return 2;
        }
        bool found = false;
        for (const auto& target : targets) if (text(field(target, "name")) == selected_target) found = true;
        if (!found) throw std::runtime_error("requested target is not present in the optimization report");
    } else if (!options.target_name.empty()) {
        throw std::runtime_error("requested target is not present in the optimization report");
    }

    if (!options.llvm_path.empty()) {
        const auto llvm_body = emit(report, binding_report);
        if (!llvm_body) throw std::runtime_error("LLVM emission requires a supported typed lowering plan");
        const auto artifact = "; Flowcore target artifact: " + selected_target + '\n' + *llvm_body;
        write_atomic_file(options.llvm_path, artifact);
    }

    const bool native_graph = field(*field(root, "lowering_plan"), "source_graph") != nullptr;
    std::string source_path;
    if (const auto* source = field(root, "source")) source_path = text(field(*source, "path"));
    std::cout << "{\n  \"format\": \"flowlower.lowering_report\",\n"
                 "  \"version\": 1,\n"
                 "  \"status\": \"ready\",\n"
                 "  \"source\": {\"path\": " << quote(source_path) << "},\n"
                 "  \"target\": {\"name\": " << quote(selected_target) << ", \"selection\": \"explicit-or-default\"},\n"
                 "  \"artifact\": {\"backend\": \"llvm\", \"target_specific\": true, \"status\": \"" << (options.llvm_path.empty() ? "not-emitted" : "emitted") << "\"},\n"
                 "  \"backend\": {\"name\": \"llvm\", \"provider_status\": \"available\"},\n"
                 "  \"ir\": {\"format\": \"llvm-ir\", \"status\": \"" << (options.llvm_path.empty() ? "not-emitted" : "emitted") << "\"},\n"
                 "  \"runtime\": {\"library\": " << quote(native_graph ? "flowgraph_runtime" : "none") << ", \"required\": " << (native_graph ? "true" : "false") << "},\n"
                 "  \"message\": \"LLVM lowering boundary reached for the accepted lowering plan\"\n"
                 "}\n";
    return 0;
}

} // namespace

int main(int argc, char** argv) {
    bool structured_diagnostics = false;
    try {
        for (int index = 1; index + 1 < argc; ++index)
            if (std::strcmp(argv[index], "--diagnostics") == 0 && std::strcmp(argv[index + 1], "json") == 0)
                structured_diagnostics = true;
        if (argc == 2) {
            const std::string option = argv[1];
            if (option == "-h" || option == "--help" || option == "-?") {
                std::cout << "flowlower - target lowering boundary\n\n"
                             "Usage: flowlower [--target name] [--binding-report report.json] [optimization-report.json]\n"
                             "       flowlower --emit-llvm output.ll [--target name] [--binding-report report.json] < optimization-report.json\n"
                             "       flowmini ... | flowanalyst | flowoptimize | flowlower\n\n"
                             "Options: -h, -?, --help  show help\n"
                             "         -a, --about    show about information\n"
                             "         -v, --version  print the raw version number\n";
                return 0;
            }
            if (option == "-a" || option == "--about") { std::cout << "Flowlower projects optimized Flowcore state onto target backends.\n"; return 0; }
            if (option == "-v" || option == "--version") { std::cout << VERSION << '\n'; return 0; }
        }
        const auto options = parse_options(argc, argv);
        structured_diagnostics = options.structured_diagnostics;
        const auto optimization_report = read_file_or_stdin(options.optimization_path);
        const auto binding_report = options.binding_path.empty() ? std::string{} : read_file_or_stdin(options.binding_path);
        return lower(optimization_report, options, binding_report);
    } catch (const flowcontracts::json::Error& error) {
        if (structured_diagnostics) { write_structured_failure("FLOWLOWER_CONTRACT_FAILURE", "contract", error.what()); return 1; }
        std::cout << "{\"format\":\"flowlower.lowering_report\",\"version\":1,\"status\":\"blocked\","
                     "\"backend\":\"llvm\",\"diagnostic\":{\"code\":\"FLOWLOWER_CONTRACT\",\"path\":"
                  << quote(error.path()) << ",\"reason\":" << quote(error.reason()) << "}}\n";
        std::cerr << "flowlower error: " << error.what() << '\n'; return 1;
    } catch (const std::bad_alloc&) {
        if (structured_diagnostics) { write_structured_failure("FLOWLOWER_RESOURCE_EXHAUSTED", "runtime", "allocation failed"); return 1; }
        std::cerr << "flowlower error: allocation failed\n"; return 1;
    } catch (const OutputError& error) {
        if (structured_diagnostics) { write_structured_failure("FLOWLOWER_OUTPUT_FAILURE", "output", error.what()); return 1; }
        std::cerr << "flowlower output error: " << error.what() << '\n'; return 1;
    } catch (const OutputUncertain& error) {
        if (structured_diagnostics) {
            write_structured_failure("FLOWLOWER_OUTPUT_DURABILITY_UNCERTAIN", "output", error.what(),
                                     "artifact_published_durability_uncertain");
            return 1;
        }
        std::cerr << "flowlower output durability uncertain: " << error.what() << '\n'; return 1;
    } catch (const std::exception& error) {
        if (structured_diagnostics) { write_structured_failure("FLOWLOWER_INPUT_INVALID", "input", error.what()); return 1; }
        std::cout << "{\"format\":\"flowlower.lowering_report\",\"version\":1,\"status\":\"unsupported\","
                     "\"backend\":\"llvm\",\"diagnostic\":{\"code\":\"FLOWLOWER_REFUSAL\",\"reason\":"
                  << quote(error.what()) << "}}\n";
        std::cerr << "flowlower error: " << error.what() << '\n'; return 1;
    } catch (...) {
        if (structured_diagnostics) { write_structured_failure("FLOWLOWER_UNKNOWN_FAILURE", "runtime", "unknown non-standard failure"); return 1; }
        std::cout << "{\"format\":\"flowlower.lowering_report\",\"version\":1,\"status\":\"unsupported\","
                     "\"backend\":\"llvm\",\"diagnostic\":{\"code\":\"FLOWLOWER_UNKNOWN_FAILURE\",\"reason\":\"unknown non-standard failure\"}}\n";
        std::cerr << "flowlower error: unknown non-standard failure\n"; return 1;
    }
}
