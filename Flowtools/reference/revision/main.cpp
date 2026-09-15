#include "../diagnostics.hpp"

#include "frankencore/provenance.hpp"

#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {

constexpr std::string_view VERSION = "0.1.0";

void write_structured_failure(std::string_view code, std::string_view message) {
    std::fputs("{\"status\":\"failed\",\"code\":\"", stderr);
    flowtools::reference::write_json_string(stderr, code);
    std::fputs("\",\"message\":\"", stderr);
    flowtools::reference::write_json_string(stderr, message);
    std::fputs("\",\"disposition\":\"no_artifact\"}\n", stderr);
}

std::uint64_t parse_revision(std::string_view text) {
    if (text.empty()) throw std::runtime_error("revision must not be empty");
    std::uint64_t value = 0;
    for (const char character : text) {
        if (character < '0' || character > '9')
            throw std::runtime_error("revision must be a non-negative integer");
        const auto digit = static_cast<std::uint64_t>(character - '0');
        if (value > (std::numeric_limits<std::uint64_t>::max() - digit) / 10)
            throw std::runtime_error("revision is too large");
        value = value * 10 + digit;
    }
    return value;
}

int run(std::uint64_t old_revision, std::uint64_t new_revision,
        std::string_view old_value, std::string_view new_value) {
    using namespace frankencore::provenance;
    const auto attempt_id = generate_ulid();
    const auto correlation_id = generate_ulid();
    MutationRecord record{
        .attempt = {
            .attempt_id = attempt_id,
            .correlation_id = correlation_id,
            .entity_identity = "reference:revision-probe",
            .actor_identity = "reference.probe",
            .provider_identity = "reference.revision-probe",
            .authorizing_policy = "reference-default",
            .operation = "replace-observation",
            .causes = {"reference-input"},
        },
        .event_id = generate_ulid(),
        .old_revision = old_revision,
        .new_revision = new_revision,
        .before_state_reference = "reference:revision-probe@" + std::to_string(old_revision),
        .after_state_reference = "reference:revision-probe@" + std::to_string(new_revision),
        .atomicity = "single-publication",
        .recoverability = "old-revision-addressable",
        .rollback_reference = std::nullopt,
        .derived_entities = {},
        .before = {std::string(old_value)},
        .after = {std::string(new_value)},
    };
    std::cout << to_json(record) << '\n';
    return 0;
}

} // namespace

int main(int argc, char** argv) {
    bool structured_diagnostics = false;
    try {
        std::string old_revision = "1";
        std::string new_revision = "2";
        std::string old_value = "before";
        std::string new_value = "after";
        for (int index = 1; index < argc; ++index) {
            const std::string argument = argv[index];
            auto value_for = [&](std::string_view option) -> std::string {
                if (++index >= argc) throw std::runtime_error(std::string(option) + " requires a value");
                return argv[index];
            };
            if (argument == "--old-revision") old_revision = value_for(argument);
            else if (argument == "--new-revision") new_revision = value_for(argument);
            else if (argument == "--old-value") old_value = value_for(argument);
            else if (argument == "--new-value") new_value = value_for(argument);
            else if (argument == "--diagnostics") {
                if (value_for(argument) != "json") throw std::runtime_error("--diagnostics requires json");
                structured_diagnostics = true;
            }
            else if (argument == "-h" || argument == "-?" || argument == "--help") {
                std::cout << "frankencore_revision_probe - emit mutation provenance evidence\n\n"
                             "Usage: frankencore_revision_probe [--old-revision N --new-revision N]\n"
                             "       [--old-value TEXT --new-value TEXT]\n"
                             "More help: Flowtools/reference/revision/README.md\n";
                return 0;
            } else if (argument == "-a" || argument == "--about") {
                std::cout << "Reference producer for the Frankencore mutation-provenance contract.\n"
                             "More help: Flowtools/reference/revision/README.md\n";
                return 0;
            } else if (argument == "-v" || argument == "--version") {
                std::cout << VERSION << '\n';
                return 0;
            } else {
                throw std::runtime_error("unknown option '" + argument + "'");
            }
        }
        return run(parse_revision(old_revision), parse_revision(new_revision), old_value, new_value);
    } catch (const std::exception& error) {
        if (structured_diagnostics) write_structured_failure("FRANKENCORE_REVISION_FAILURE", error.what());
        else std::cerr << "frankencore_revision_probe error: " << error.what() << '\n';
        return 1;
    } catch (...) {
        if (structured_diagnostics) write_structured_failure("FRANKENCORE_REVISION_UNKNOWN_FAILURE", "unknown non-standard failure");
        else std::cerr << "frankencore_revision_probe error: unknown non-standard failure\n";
        return 1;
    }
}
