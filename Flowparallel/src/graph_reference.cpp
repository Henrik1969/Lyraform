#include <flowcontracts/artifacts.hpp>
#include <flowparallel/bounded_input.hpp>
#include <flowparallel/diagnostics.hpp>

#include <cstddef>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <new>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace {
constexpr std::string_view version = "0.1.0";

class InputError final : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

void write_structured_failure(std::string_view code, std::string_view stage, std::string_view message) noexcept {
    std::fputs("{\"status\":\"failed\",\"code\":\"", stderr);
    flowparallel::write_json_string(stderr, code);
    std::fputs("\",\"stage\":\"", stderr);
    flowparallel::write_json_string(stderr, stage);
    std::fputs("\",\"message\":\"", stderr);
    flowparallel::write_json_string(stderr, message);
    std::fputs("\",\"disposition\":\"no_artifact\"}\n", stderr);
}

std::string read_bounded_input(std::istream& input) {
    try {
        return flowparallel::read_bounded(input, "semantic report");
    } catch (const std::bad_alloc&) {
        throw;
    } catch (const std::exception& error) {
        throw InputError(error.what());
    }
}

std::string read_input(int argc, char** argv) {
    if (argc > 2 && !(argc == 3 && std::string_view(argv[1]) == "--diagnostics" && std::string_view(argv[2]) == "json"))
        throw InputError("usage: flowparallel_graph_reference [semantic-report.json]");
    if (argc == 2) { std::ifstream file(argv[1]); if (!file) throw InputError("cannot open semantic report"); return read_bounded_input(file); }
    return read_bounded_input(std::cin);
}

std::string quote(std::string_view value) {
    std::string result = "\"";
    for (char character : value) { if (character == '\\' || character == '"') result.push_back('\\'); result.push_back(character); }
    result.push_back('"');
    return result;
}

int run(std::string_view report) {
#ifdef FLOWPARALLEL_GRAPH_REFERENCE_TEST_ALLOCATION_FAILURE
    (void)report;
    throw std::bad_alloc();
#endif
    const auto semantic = flowcontracts::semantic_report(flowcontracts::json::parse(report));
    if (semantic.artifact.status != "ok") {
        std::cout << "{\n  \"format\": \"flowparallel.graph_analysis\",\n  \"version\": 1,\n  \"status\": \"blocked\",\n  \"reason\": \"semantic report is not accepted\"\n}\n";
        return 2;
    }
    const auto& matrix = semantic.dependency_matrix;
    if (matrix.rows <= 0 || matrix.rows != matrix.columns || matrix.rows > 4096)
        throw flowcontracts::json::Error("$.analysis_graph", "unsupported graph matrix dimensions");
    const auto rows = static_cast<std::size_t>(matrix.rows);
    const auto columns = static_cast<std::size_t>(matrix.columns);
    std::vector<unsigned char> reach(rows * columns, 0);
    for (const auto& entry : matrix.entries)
        if (entry.value) reach[static_cast<std::size_t>(entry.row) * columns + static_cast<std::size_t>(entry.column)] = 1;
    for (std::size_t pivot = 0; pivot < rows; ++pivot)
        for (std::size_t row = 0; row < rows; ++row)
            if (reach[row * columns + pivot])
                for (std::size_t column = 0; column < columns; ++column)
                    reach[row * columns + column] = static_cast<unsigned char>(reach[row * columns + column] || reach[pivot * columns + column]);
    std::size_t reachable_pairs = 0;
    for (unsigned char value : reach) reachable_pairs += value != 0;
    std::cout << "{\n  \"format\": \"flowparallel.graph_analysis\",\n  \"version\": 1,\n  \"status\": \"verified\",\n  \"source\": {\"path\": " << quote(semantic.source_path) << "},\n  \"operation\": \"reachability\",\n  \"input\": {\"format\": \"flowanalyst.analysis_graph\", \"matrix\": \"region_dependency\", \"rows\": " << rows << ", \"columns\": " << columns << "},\n  \"semantics\": {\"semiring\": \"boolean\", \"paths\": \"one-or-more-edges\"},\n  \"reachable_pairs\": " << reachable_pairs << ",\n  \"provider\": \"cpu.reference\"\n}\n";
    return 0;
}
}

int main(int argc, char** argv) {
    bool structured_diagnostics = false;
    try {
        for (int index = 1; index + 1 < argc; ++index)
            if (std::strcmp(argv[index], "--diagnostics") == 0 && std::strcmp(argv[index + 1], "json") == 0)
                structured_diagnostics = true;
        if (argc == 2 && (std::string(argv[1]) == "-h" || std::string(argv[1]) == "-?" || std::string(argv[1]) == "--help")) { std::cout << "flowparallel_graph_reference - CPU reference graph reachability\n\nOptions: -h, -?, --help  show help\n         -a, --about    show about information\n         -v, --version  print the raw version number\n"; return 0; }
        if (argc == 2 && (std::string(argv[1]) == "-a" || std::string(argv[1]) == "--about")) { std::cout << "Flowparallel computes verified Boolean graph reachability as the CPU reference provider.\n"; return 0; }
        if (argc == 2 && (std::string(argv[1]) == "-v" || std::string(argv[1]) == "--version")) { std::cout << version << '\n'; return 0; }
        return run(read_input(argc, argv));
    } catch (const std::bad_alloc&) {
        if (structured_diagnostics) write_structured_failure("FLOWPARALLEL_GRAPH_REFERENCE_RESOURCE_EXHAUSTED", "runtime", "allocation failed");
        else std::cerr << "flowparallel_graph_reference error: allocation failed\n";
        return 1;
    } catch (const flowcontracts::json::Error& error) {
        if (structured_diagnostics) write_structured_failure("FLOWPARALLEL_GRAPH_REFERENCE_CONTRACT_FAILURE", "contract", error.what());
        else std::cerr << "flowparallel_graph_reference contract error: " << error.what() << '\n';
        return 1;
    } catch (const InputError& error) {
        if (structured_diagnostics) write_structured_failure("FLOWPARALLEL_GRAPH_REFERENCE_INPUT_INVALID", "input", error.what());
        else std::cerr << "flowparallel_graph_reference input error: " << error.what() << '\n';
        return 1;
    } catch (const std::exception& error) {
        if (structured_diagnostics) write_structured_failure("FLOWPARALLEL_GRAPH_REFERENCE_FAILURE", "runtime", error.what());
        else std::cerr << "flowparallel_graph_reference runtime error: " << error.what() << '\n';
        return 1;
    } catch (...) {
        if (structured_diagnostics) write_structured_failure("FLOWPARALLEL_GRAPH_REFERENCE_UNKNOWN_FAILURE", "runtime", "unknown non-standard failure");
        else std::cerr << "flowparallel_graph_reference error: unknown non-standard failure\n";
        return 1;
    }
}
