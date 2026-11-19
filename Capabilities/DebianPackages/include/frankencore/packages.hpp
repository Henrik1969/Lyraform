#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace frankencore::packages {

struct PackageFact {
    std::string name;
    std::string status;
    std::string version;
    std::string architecture;
    std::string source;
    std::string maintainer;
};

struct Diagnostic {
    std::string code;
    std::string message;
    std::size_t line = 0;
};

struct RepositoryFact {
    std::string identity;
    std::string origin;
    std::string suite;
    std::string codename;
    std::string components;
    std::string architectures;
    std::string metadata_path;
    std::string authentication;
};

struct AptSourceFact {
    std::string configuration_path;
    std::string type;
    std::string uri;
    std::string suites;
    std::string components;
    std::string signed_by;
    bool enabled = true;
    bool trusted_override = false;
};

struct AptIndexTargetFact {
    std::string identifier;
    std::string meta_key;
    std::string short_description;
    std::string filename;
    std::string uri;
};

struct Inventory {
    std::string format = "frankencore.package_inventory";
    int version = 1;
    std::string provider = "dpkg-status";
    std::string provider_version;
    std::string source_path;
    std::vector<PackageFact> packages;
    std::vector<RepositoryFact> repositories;
    std::vector<AptSourceFact> apt_sources;
    std::vector<AptIndexTargetFact> apt_index_targets;
    std::vector<Diagnostic> diagnostics;
};

struct JsonResult {
    bool valid = false;
    std::string json;
    std::string error;
};

struct InventoryResult {
    bool valid = false;
    Inventory inventory;
    std::string error;
};

// Read-only projection of the native dpkg status database. Individual
// paragraphs are bounded to 1 MiB; oversized provider records are skipped and
// reported as diagnostics rather than accumulated without limit.
Inventory read_dpkg_status(const std::string& path);

// Read-only projection of APT's locally acquired list metadata. Files are
// bounded to 4 MiB. Signature verification remains APT's responsibility; this
// function reports evidence shape and does not claim cryptographic
// verification itself.
Inventory read_apt_lists(const std::string& directory);

// Read-only projection of .sources and legacy .list configuration files.
// Deb822 source files are bounded to 4 MiB.
Inventory read_apt_sources(const std::string& directory);

// Delegate read-only target discovery to the native apt-get interface.
Inventory read_apt_index_targets(const std::string& apt_get_path = "/usr/bin/apt-get");

// Non-throwing provider boundaries. On failure, no partially collected
// inventory is admitted to the consumer.
InventoryResult read_dpkg_status_checked(const std::string& path) noexcept;
InventoryResult read_apt_lists_checked(const std::string& directory) noexcept;
InventoryResult read_apt_sources_checked(const std::string& directory) noexcept;
InventoryResult read_apt_index_targets_checked(
    const std::string& apt_get_path = "/usr/bin/apt-get") noexcept;

// JSON is an inspectable projection, not the canonical C++ representation.
std::string to_json(const Inventory& inventory);

// Checked public projection boundary. On failure, json is empty and no
// serialized artifact is admitted.
JsonResult to_json_checked(const Inventory& inventory) noexcept;

} // namespace frankencore::packages
