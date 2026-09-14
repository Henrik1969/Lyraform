#include <frankencore/language.hpp>

#include <cassert>
#include <string>

int main() {
    frankencore::contracts::LanguageMap map;
    map.id = "Danish";
    map.revision = "Danish.v1";
    map.parser = "surface.v1";
    map.parent = "canonical";
    map.monikers["ask"] = {"Spørg"};
    map.monikers["echo"] = {"skriv"};

    const auto canonical = frankencore::language::resolve_moniker(map, "Spørg");
    assert(canonical.resolved && canonical.canonical == "ask");
    const auto identity = frankencore::language::resolve_moniker(map, "ask");
    assert(identity.resolved && identity.canonical == "ask");
    const auto missing = frankencore::language::resolve_moniker(map, "Frage");
    assert(!missing.resolved);
    const auto oversized = frankencore::language::resolve_moniker(map, std::string(4097, 'x'));
    assert(!oversized.resolved && oversized.diagnostic.find("4096") != std::string::npos);

    map.monikers["question"] = {"Spørg"};
    const auto collision = frankencore::language::resolve_moniker(map, "Spørg");
    assert(!collision.resolved);
    return 0;
}
