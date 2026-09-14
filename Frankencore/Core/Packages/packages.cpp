#include "frankencore/packages.hpp"

#include <fstream>
#include <exception>
#include <filesystem>
#include <algorithm>
#include <cstdio>
#include <sstream>
#include <new>
#include <optional>
#include <sys/wait.h>
#include <utility>

namespace frankencore::packages {
namespace {

constexpr std::size_t MAX_DPKG_RECORD_BYTES = 1024U * 1024U;
constexpr std::size_t MAX_APT_METADATA_BYTES = 4U * 1024U * 1024U;

std::string json_escape(const std::string& value) {
    std::string result;
    result.reserve(value.size() + 2);
    for (const char character : value) {
        switch (character) {
        case '\\': result += "\\\\"; break;
        case '"': result += "\\\""; break;
        case '\n': result += "\\n"; break;
        case '\r': result += "\\r"; break;
        case '\t': result += "\\t"; break;
        default: result += character; break;
        }
    }
    return result;
}

std::string shell_quote(const std::string& value) {
    std::string result = "'";
    for (const char character : value) {
        if (character == '\'') result += "'\\''";
        else result += character;
    }
    result += '\'';
    return result;
}

std::string field(const std::string& fields, const std::string& wanted) {
    std::istringstream input(fields);
    std::string line;
    const std::string prefix = wanted + ":";
    while (std::getline(input, line)) {
        if (line.rfind(prefix, 0) == 0) {
            std::string value = line.substr(prefix.size());
            if (!value.empty() && value.front() == ' ') value.erase(0, 1);
            return value;
        }
    }
    return {};
}

std::vector<std::string> split_words(const std::string& value) {
    std::istringstream input(value);
    std::vector<std::string> result;
    std::string word;
    while (input >> word) result.push_back(word);
    return result;
}

void add_diagnostic(Inventory& inventory, std::string code,
                    std::string message, std::size_t line) {
    inventory.diagnostics.push_back(
        Diagnostic{std::move(code), std::move(message), line});
}

void parse_deb822_source(Inventory& inventory, const std::string& text,
                         const std::string& path) {
    const auto types = split_words(field(text, "Types"));
    const auto uris = split_words(field(text, "URIs"));
    const auto suites = field(text, "Suites");
    const auto components = field(text, "Components");
    const auto signed_by = field(text, "Signed-By");
    const auto enabled = field(text, "Enabled");
    const bool is_enabled = enabled.empty() || enabled != "no";
    if (types.empty() || uris.empty()) {
        add_diagnostic(inventory, "malformed-source",
                       "Deb822 source lacks Types or URIs", 0);
        return;
    }
    for (const auto& type : types) {
        for (const auto& uri : uris) {
            inventory.apt_sources.push_back(
                AptSourceFact{path, type, uri, suites, components, signed_by,
                              is_enabled, false});
        }
    }
}

void parse_legacy_source(Inventory& inventory, const std::string& line,
                         const std::string& path) {
    std::istringstream input(line);
    std::string type;
    std::string token;
    if (!(input >> type >> token)) return;
    if (type != "deb" && type != "deb-src") return;

    AptSourceFact source{path, type, {}, {}, {}, {}, true, false};
    if (token.front() == '[') {
        std::string options = token;
        while (options.find(']') == std::string::npos && input >> token) {
            options += token;
        }
        const auto begin = options.find('[');
        const auto end = options.find(']');
        if (begin != std::string::npos && end != std::string::npos && end > begin) {
            std::istringstream option_stream(
                options.substr(begin + 1, end - begin - 1));
            std::string option;
            while (std::getline(option_stream, option, ',')) {
                const auto equals = option.find('=');
                const auto key = option.substr(0, equals);
                const auto value = equals == std::string::npos
                                       ? std::string{}
                                       : option.substr(equals + 1);
                if (key == "signed-by") source.signed_by = value;
                if (key == "trusted" && value == "yes") {
                    source.trusted_override = true;
                }
            }
        }
        if (!(input >> token)) return;
    }
    source.uri = token;
    if (!(input >> source.suites)) return;
    std::string component;
    while (input >> component) {
        if (!source.components.empty()) source.components += ' ';
        source.components += component;
    }
    inventory.apt_sources.push_back(std::move(source));
}

std::optional<std::string> read_bounded_text(const std::filesystem::path& path) {
    std::ifstream input(path);
    if (!input) return std::nullopt;
    std::string text;
    text.reserve(4096);
    char character = '\0';
    while (input.get(character)) {
        if (text.size() == MAX_APT_METADATA_BYTES) return std::nullopt;
        text.push_back(character);
    }
    return text;
}

std::optional<RepositoryFact> repository_from_metadata(
    const std::filesystem::path& path, const std::string& authentication) {
    const auto text = read_bounded_text(path);
    if (!text) return std::nullopt;
    RepositoryFact repository{
        path.filename().string(),
        field(*text, "Origin"),
        field(*text, "Suite"),
        field(*text, "Codename"),
        field(*text, "Components"),
        field(*text, "Architectures"),
        path.string(),
        authentication};
    return repository;
}

} // namespace

Inventory read_dpkg_status(const std::string& path) {
    Inventory inventory;
    inventory.source_path = path;

    std::ifstream input(path);
    if (!input) {
        add_diagnostic(inventory, "source-unavailable",
                       "unable to read dpkg status database", 0);
        return inventory;
    }

    std::string paragraph;
    std::string line;
    line.reserve(4096);
    std::size_t line_number = 0;
    std::size_t paragraph_start = 1;
    bool discard_record = false;
    auto consume = [&] {
        if (discard_record) {
            discard_record = false;
            paragraph.clear();
            return;
        }
        if (paragraph.empty()) return;
        PackageFact package{
            field(paragraph, "Package"),
            field(paragraph, "Status"),
            field(paragraph, "Version"),
            field(paragraph, "Architecture"),
            field(paragraph, "Source"),
            field(paragraph, "Maintainer")};
        if (package.name.empty()) {
            add_diagnostic(inventory, "malformed-record",
                           "package record has no Package field",
                           paragraph_start);
        } else {
            inventory.packages.push_back(std::move(package));
        }
        paragraph.clear();
    };

    auto process_line = [&](const std::string& current_line) {
        ++line_number;
        if (current_line.empty()) {
            consume();
            paragraph_start = line_number + 1;
            return;
        }
        if (discard_record) return;
        const auto separator = paragraph.empty() ? 0U : 1U;
        if (paragraph.size() > MAX_DPKG_RECORD_BYTES - separator ||
            current_line.size() > MAX_DPKG_RECORD_BYTES - separator - paragraph.size()) {
            add_diagnostic(inventory, "record-too-large",
                           "dpkg status record exceeds the 1 MiB safety bound",
                           paragraph_start);
            paragraph.clear();
            discard_record = true;
            return;
        }
        if (!paragraph.empty()) paragraph.push_back('\n');
        paragraph += current_line;
    };
    bool line_too_large = false;
    char character = '\0';
    while (input.get(character)) {
        if (character == '\n') {
            if (line_too_large) {
                ++line_number;
                if (!discard_record) {
                    add_diagnostic(inventory, "record-too-large",
                                   "dpkg status record exceeds the 1 MiB safety bound",
                                   paragraph_start);
                    paragraph.clear();
                    discard_record = true;
                }
            } else {
                process_line(line);
            }
            line.clear();
            line_too_large = false;
        } else if (!line_too_large) {
            if (line.size() == MAX_DPKG_RECORD_BYTES) {
                line.clear();
                line_too_large = true;
            } else {
                line.push_back(character);
            }
        }
    }
    if (!line.empty() || line_too_large) {
        if (line_too_large) {
            ++line_number;
            if (!discard_record) {
                add_diagnostic(inventory, "record-too-large",
                               "dpkg status record exceeds the 1 MiB safety bound",
                               paragraph_start);
                paragraph.clear();
                discard_record = true;
            }
        } else {
            process_line(line);
        }
    }
    consume();
    inventory.provider_version = "native-status-format";
    return inventory;
}

Inventory read_apt_lists(const std::string& directory) {
    Inventory inventory;
    inventory.provider = "apt-lists";
    inventory.provider_version = "native-list-format";
    inventory.source_path = directory;

    std::error_code error;
    if (!std::filesystem::is_directory(directory, error)) {
        add_diagnostic(inventory, "source-unavailable",
                       "unable to read APT list directory", 0);
        return inventory;
    }

    std::vector<std::filesystem::path> entries;
    for (const auto& entry : std::filesystem::directory_iterator(directory, error)) {
        if (error) break;
        if (entry.is_regular_file()) entries.push_back(entry.path());
    }
    std::sort(entries.begin(), entries.end());
    for (const auto& path : entries) {
        const auto filename = path.filename().string();
        if (filename.size() >= 10 &&
            filename.compare(filename.size() - 10, 10, "_InRelease") == 0) {
            const auto repository = repository_from_metadata(path, "inrelease-present");
            if (repository) inventory.repositories.push_back(*repository);
            else add_diagnostic(inventory, "metadata-too-large",
                                "APT release metadata exceeds the 4 MiB safety bound", 0);
        } else if (filename.size() >= 7 &&
                   filename.compare(filename.size() - 7, 7, "_Release") == 0) {
            const auto repository = repository_from_metadata(path, "release-present");
            if (repository) inventory.repositories.push_back(*repository);
            else add_diagnostic(inventory, "metadata-too-large",
                                "APT release metadata exceeds the 4 MiB safety bound", 0);
        }
    }
    if (error) {
        add_diagnostic(inventory, "enumeration-failed",
                       "unable to enumerate APT list directory", 0);
    }
    return inventory;
}

Inventory read_apt_sources(const std::string& directory) {
    Inventory inventory;
    inventory.provider = "apt-sources";
    inventory.provider_version = "native-source-configuration";
    inventory.source_path = directory;

    std::error_code error;
    if (!std::filesystem::is_directory(directory, error)) {
        add_diagnostic(inventory, "source-unavailable",
                       "unable to read APT source directory", 0);
        return inventory;
    }

    std::vector<std::filesystem::path> entries;
    for (const auto& entry : std::filesystem::directory_iterator(directory, error)) {
        if (error) break;
        if (entry.is_regular_file()) entries.push_back(entry.path());
    }
    std::sort(entries.begin(), entries.end());
    for (const auto& path : entries) {
        const auto extension = path.extension().string();
        std::ifstream input(path);
        if (!input) {
            add_diagnostic(inventory, "source-unavailable",
                           "unable to read APT source file", 0);
            continue;
        }
        if (extension == ".sources") {
            const auto contents = read_bounded_text(path);
            if (!contents) {
                add_diagnostic(inventory, "metadata-too-large",
                               "APT source metadata exceeds the 4 MiB safety bound", 0);
                continue;
            }
            parse_deb822_source(inventory, *contents, path.string());
        } else if (extension == ".list") {
            std::string line;
            while (std::getline(input, line)) {
                const auto first = line.find_first_not_of(" \t");
                if (first == std::string::npos || line[first] == '#') continue;
                parse_legacy_source(inventory, line.substr(first), path.string());
            }
        }
    }
    if (error) {
        add_diagnostic(inventory, "enumeration-failed",
                       "unable to enumerate APT source directory", 0);
    }
    return inventory;
}

Inventory read_apt_index_targets(const std::string& apt_get_path) {
    Inventory inventory;
    inventory.provider = "apt-indextargets";
    inventory.provider_version = "native-apt-get-indextargets";
    inventory.source_path = apt_get_path;
    const std::string command = shell_quote(apt_get_path) +
        " indextargets --no-release-info --format='$(IDENTIFIER)\\t$(METAKEY)\\t$(SHORTDESC)\\t$(FILENAME)\\t$(URI)\\n' 2>/dev/null";
    FILE* stream = popen(command.c_str(), "r");
    if (stream == nullptr) {
        add_diagnostic(inventory, "provider-unavailable",
                       "unable to invoke native apt-get indextargets", 0);
        return inventory;
    }
    char buffer[4096];
    std::size_t line_number = 0;
    while (std::fgets(buffer, sizeof(buffer), stream) != nullptr) {
        ++line_number;
        std::string line(buffer);
        if (!line.empty() && line.back() == '\n') line.pop_back();
        std::istringstream fields(line);
        std::vector<std::string> values;
        std::string value;
        while (std::getline(fields, value, '\t')) values.push_back(value);
        if (values.size() != 5) {
            add_diagnostic(inventory, "malformed-provider-output",
                           "native apt-get returned an unexpected target row",
                           line_number);
            continue;
        }
        inventory.apt_index_targets.push_back(
            AptIndexTargetFact{values[0], values[1], values[2], values[3], values[4]});
    }
    const int status = pclose(stream);
    if (status == -1 || !WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        add_diagnostic(inventory, "native-provider-failed",
                       "apt-get indextargets failed", 0);
    }
    return inventory;
}

std::string to_json(const Inventory& inventory) {
    std::ostringstream output;
    output << "{\"format\":\"" << json_escape(inventory.format)
           << "\",\"version\":" << inventory.version
           << ",\"provider\":\"" << json_escape(inventory.provider)
           << "\",\"provider_version\":\""
           << json_escape(inventory.provider_version)
           << "\",\"source_path\":\"" << json_escape(inventory.source_path)
           << "\",\"packages\":[";
    for (std::size_t index = 0; index < inventory.packages.size(); ++index) {
        if (index != 0) output << ',';
        const auto& package = inventory.packages[index];
        output << "{\"name\":\"" << json_escape(package.name)
               << "\",\"status\":\"" << json_escape(package.status)
               << "\",\"version\":\"" << json_escape(package.version)
               << "\",\"architecture\":\""
               << json_escape(package.architecture)
               << "\",\"source\":\"" << json_escape(package.source)
               << "\",\"maintainer\":\""
               << json_escape(package.maintainer) << "\"}";
    }
    output << "],\"repositories\":[";
    for (std::size_t index = 0; index < inventory.repositories.size(); ++index) {
        if (index != 0) output << ',';
        const auto& repository = inventory.repositories[index];
        output << "{\"identity\":\"" << json_escape(repository.identity)
               << "\",\"origin\":\"" << json_escape(repository.origin)
               << "\",\"suite\":\"" << json_escape(repository.suite)
               << "\",\"codename\":\""
               << json_escape(repository.codename)
               << "\",\"components\":\""
               << json_escape(repository.components)
               << "\",\"architectures\":\""
               << json_escape(repository.architectures)
               << "\",\"metadata_path\":\""
               << json_escape(repository.metadata_path)
               << "\",\"authentication\":\""
               << json_escape(repository.authentication) << "\"}";
    }
    output << "],\"apt_sources\":[";
    for (std::size_t index = 0; index < inventory.apt_sources.size(); ++index) {
        if (index != 0) output << ',';
        const auto& source = inventory.apt_sources[index];
        output << "{\"configuration_path\":\""
               << json_escape(source.configuration_path)
               << "\",\"type\":\"" << json_escape(source.type)
               << "\",\"uri\":\"" << json_escape(source.uri)
               << "\",\"suites\":\"" << json_escape(source.suites)
               << "\",\"components\":\""
               << json_escape(source.components)
               << "\",\"signed_by\":\""
               << json_escape(source.signed_by)
               << "\",\"enabled\":" << (source.enabled ? "true" : "false")
               << ",\"trusted_override\":"
               << (source.trusted_override ? "true" : "false") << "}";
    }
    output << "],\"apt_index_targets\":[";
    for (std::size_t index = 0; index < inventory.apt_index_targets.size(); ++index) {
        if (index != 0) output << ',';
        const auto& target = inventory.apt_index_targets[index];
        output << "{\"identifier\":\"" << json_escape(target.identifier)
               << "\",\"meta_key\":\"" << json_escape(target.meta_key)
               << "\",\"short_description\":\""
               << json_escape(target.short_description)
               << "\",\"filename\":\"" << json_escape(target.filename)
               << "\",\"uri\":\"" << json_escape(target.uri) << "\"}";
    }
    output << "],\"diagnostics\":[";
    for (std::size_t index = 0; index < inventory.diagnostics.size(); ++index) {
        if (index != 0) output << ',';
        const auto& diagnostic = inventory.diagnostics[index];
        output << "{\"code\":\"" << json_escape(diagnostic.code)
               << "\",\"message\":\""
               << json_escape(diagnostic.message)
               << "\",\"line\":" << diagnostic.line << "}";
    }
    output << "]}";
    return output.str();
}

JsonResult to_json_checked(const Inventory& inventory) noexcept {
    try {
        return {true, to_json(inventory), {}};
    } catch (const std::bad_alloc&) {
        return {false, {}, "package inventory serialization exhausted memory"};
    } catch (const std::exception& error) {
        return {false, {}, error.what()};
    } catch (...) {
        return {false, {}, "unknown non-standard package inventory serialization failure"};
    }
}

} // namespace frankencore::packages
