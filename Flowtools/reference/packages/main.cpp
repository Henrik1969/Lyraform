#include <frankencore/packages.hpp>

#include <cassert>
#include <filesystem>
#include <fstream>
#include <string>

int main() {
    const auto path = std::filesystem::temp_directory_path() /
                      "frankencore-package-status-test";
    {
        std::ofstream output(path);
        output << "Package: example\n"
                  "Status: install ok installed\n"
                  "Version: 1.2.3\n"
                  "Architecture: amd64\n"
                  "Source: example (1.2.3)\n"
                  "Maintainer: Example Maintainer <example@example.invalid>\n\n"
                  "Package: second\n"
                  "Status: deinstall ok config-files\n"
                  "Version: 0.1\n";
    }

    const auto inventory = frankencore::packages::read_dpkg_status(path);
    assert(inventory.packages.size() == 2);
    assert(inventory.packages[0].name == "example");
    assert(inventory.packages[0].status == "install ok installed");
    assert(inventory.packages[1].name == "second");
    assert(inventory.diagnostics.empty());
    const auto json = frankencore::packages::to_json(inventory);
    assert(json.find("\"example\"") != std::string::npos);
    const auto checked_json = frankencore::packages::to_json_checked(inventory);
    assert(checked_json.valid);
    assert(checked_json.json == json);

    const auto missing = frankencore::packages::read_dpkg_status(
        "/path/that/does/not/exist");
    assert(missing.packages.empty());
    assert(missing.diagnostics.size() == 1);
    assert(missing.diagnostics[0].code == "source-unavailable");

    const auto apt_lists = frankencore::packages::read_apt_lists(
        "/path/that/does/not/exist");
    assert(apt_lists.repositories.empty());
    assert(apt_lists.diagnostics.size() == 1);

    const auto apt_fixture = std::filesystem::temp_directory_path() /
                             "frankencore-apt-lists-test";
    std::filesystem::create_directories(apt_fixture);
    {
        std::ofstream output(apt_fixture / "example_dists_noble_InRelease");
        output << "Origin: Example\nSuite: noble\nCodename: noble\n"
                  "Components: main\nArchitectures: amd64\n"
                  "SHA256:\n";
    }
    const auto repositories = frankencore::packages::read_apt_lists(
        apt_fixture.string());
    assert(repositories.repositories.size() == 1);
    assert(repositories.repositories[0].origin == "Example");
    assert(repositories.repositories[0].authentication == "inrelease-present");
    std::filesystem::remove_all(apt_fixture);

    const auto source_fixture = std::filesystem::temp_directory_path() /
                                "frankencore-apt-sources-test";
    std::filesystem::create_directories(source_fixture);
    {
        std::ofstream output(source_fixture / "system.sources");
        output << "Types: deb deb-src\n"
                  "URIs: https://example.invalid/repo\n"
                  "Suites: stable\nComponents: main\n"
                  "Signed-By: /etc/apt/keyrings/example.gpg\n\n";
    }
    {
        std::ofstream output(source_fixture / "local.list");
        output << "deb [trusted=yes] file:/srv/local stable main\n";
    }
    const auto sources = frankencore::packages::read_apt_sources(
        source_fixture.string());
    assert(sources.apt_sources.size() == 3);
    bool saw_signed_by = false;
    bool saw_trusted_override = false;
    for (const auto& source : sources.apt_sources) {
        saw_signed_by = saw_signed_by ||
                        source.signed_by == "/etc/apt/keyrings/example.gpg";
        saw_trusted_override = saw_trusted_override || source.trusted_override;
    }
    assert(saw_signed_by);
    assert(saw_trusted_override);
    std::filesystem::remove_all(source_fixture);

    const auto apt_targets = frankencore::packages::read_apt_index_targets();
    assert(!apt_targets.apt_index_targets.empty() ||
           !apt_targets.diagnostics.empty());
    const auto missing_apt = frankencore::packages::read_apt_index_targets(
        "/path/that/does/not/exist");
    assert(missing_apt.apt_index_targets.empty());
    assert(!missing_apt.diagnostics.empty());

    const auto oversized = std::filesystem::temp_directory_path() /
                           "frankencore-package-oversized-record-test";
    {
        std::ofstream output(oversized);
        output << "Package: oversized\nMaintainer: "
               << std::string(1024U * 1024U, 'x') << "\n\n";
    }
    const auto oversized_inventory =
        frankencore::packages::read_dpkg_status(oversized);
    assert(oversized_inventory.packages.empty());
    assert(oversized_inventory.diagnostics.size() == 1);
    assert(oversized_inventory.diagnostics[0].code == "record-too-large");
    std::filesystem::remove(oversized);

    std::filesystem::remove(path);
    return 0;
}
