#include <frankencore/language.hpp>

#include <cassert>
#include <string>

int main() {
    frankencore::contracts::LanguageMap map;
    map.id = "Danish";
    map.revision = "Danish.v1";
    map.parser = "surface.v1";
    map.parent = "canonical";
    const auto result = frankencore::language::resolve_moniker(map, "ask");
    assert(!result.resolved);
    assert(result.diagnostic.find("unknown") == std::string::npos);
    assert(result.diagnostic.find("resolution failed") != std::string::npos);
    return 0;
}
