// Test oracle only. This executable is never linked into the Lyraform port.
#include <configresolve/configresolve.hpp>
#include <iostream>
#include <cstdint>
int main() {
    std::int64_t a, b;
    int ap, bp, mode;
    while (std::cin >> a >> ap >> b >> bp >> mode) {
        using namespace configresolve;
        if (ap == 0 || bp == 0 || mode < 0 || mode > 3) return 2;
        FactSet facts;
        facts.add(KeyPath("probe.value"), Value(a), Source::file("candidate-a"), ap);
        facts.add(KeyPath("probe.value"), Value(b), Source::file("candidate-b"), bp);
        PolicySet policy;
        const ConflictPolicy modes[] = {ConflictPolicy::HighestPrecedenceWins,
            ConflictPolicy::RejectConflict, ConflictPolicy::FirstWins, ConflictPolicy::LastWins};
        policy.conflict(KeyPath("probe.value"), modes[mode]);
        auto result = resolve(facts, policy);
        if (!result.ok()) {
            if (result.diagnostics().items().size() != 1) return 3;
            const auto& code = result.diagnostics().items()[0].code;
            if (code == "CONFIG_AMBIGUOUS_PRECEDENCE") std::cout << 11 << '\n';
            else if (code == "CONFIG_CONFLICT") std::cout << 12 << '\n';
            else return 4;
        } else {
            const auto* entry = result.config().explain(KeyPath("probe.value"));
            if (!entry || entry->candidates.size() != 2) return 5;
            if (entry->chosen_source.name() == "candidate-a") std::cout << 1 << '\n';
            else if (entry->chosen_source.name() == "candidate-b") std::cout << 2 << '\n';
            else return 6;
        }
    }
}
