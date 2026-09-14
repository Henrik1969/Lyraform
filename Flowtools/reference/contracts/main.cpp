#include <frankencore/contracts.hpp>

#include <cassert>

int main() {
    using namespace frankencore::contracts;

    VerificationEvidence evidence{
        "flowcore", "0.26", "binary", "x86_64", "sha256:example",
        "apt", "2.8.3", "native-apt", "repo", "key-id", "trusted",
        "matched", "supplier_authenticated", false, {}, {},
        PolicyOutcome::allowed};
    assert(validate(evidence).valid);
    evidence.operator_override = true;
    evidence.authenticity = "owner_attested";
    assert(!validate(evidence).valid);

    IsolationClaim isolation{
        "project execution", "constrained", "locally_verified", "namespace-provider", "1",
        "cpu=2,memory=256MiB", "uid=unprivileged", "project-read-only", "denied",
        "no-new-privileges", "joined-and-closed", "namespace-sanity-v1"};
    assert(validate(isolation).valid);
    isolation.assurance_level = "isolated";
    assert(!validate(isolation).valid);
    isolation.assurance_level = "constrained";
    isolation.enforcement = "unknown";
    assert(!validate(isolation).valid);

    LanguageMap language;
    language.id = "Danish";
    language.revision = "Danish.v1";
    language.parser = "frankencore.shell.surface.v1";
    language.parent = "canonical";
    language.monikers["ask"] = {"Spørg"};
    assert(validate(language).valid);

    ChainPolicy policy;
    policy.name = "test";
    policy.language_map = "Flowmini";
    policy.failure_policy = "diagnose_and_stop";
    policy.targets.push_back({"native", "linux", ">=1", "safe-default", "native"});
    assert(validate(policy).valid);

    FacadeInvocation invocation;
    invocation.facade = "ls";
    invocation.backend = "/usr/bin/ls";
    assert(validate(invocation).valid);
    return 0;
}
