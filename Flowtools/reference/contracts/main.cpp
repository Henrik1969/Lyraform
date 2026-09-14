#include <frankencore/contracts.hpp>

#include <cassert>
#include <string>

int main() {
    using namespace frankencore::contracts;

    VerificationEvidence evidence{
        "flowcore", "0.26", "binary", "x86_64", "sha256:example",
        "apt", "2.8.3", "native-apt", "repo", "key-id", "trusted",
        "matched", "supplier_authenticated", false, {}, {},
        PolicyOutcome::allowed};
    assert(validate(evidence).valid);
    evidence.key_state = "unknown";
    assert(!validate(evidence).valid);
    evidence.key_state = "trusted";
    evidence.operator_override = true;
    evidence.authenticity = "owner_attested";
    assert(!validate(evidence).valid);
    evidence.operator_override = false;
    evidence.authenticity = "unverified";
    evidence.policy_outcome = PolicyOutcome::allowed_with_isolation;
    assert(validate(evidence).valid);
    evidence.integrity = "mismatched";
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
    isolation.enforcement = "locally_verified";
    isolation.requested_boundary.clear();
    assert(!validate(isolation).valid);
    isolation.requested_boundary = "project execution";
    isolation.assurance_level = "isolated";
    isolation.enforcement = "locally_verified";
    assert(!validate(isolation).valid);
    isolation.assurance_level = "hardened";
    isolation.enforcement = "unknown";
    assert(!validate(isolation).valid);
    isolation.assurance_level = "none";
    isolation.enforcement = "self_report";
    assert(validate(isolation).valid);
    isolation.requested_boundary = std::string(4097, 'x');
    assert(!validate(isolation).valid);
    isolation.requested_boundary = "project execution";
    evidence.artifact_identity = std::string(4097, 'x');
    assert(!validate(evidence).valid);

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
    auto oversized_policy = policy;
    oversized_policy.targets.resize(100001);
    assert(!validate(oversized_policy).valid);

    FacadeInvocation invocation;
    invocation.facade = "ls";
    invocation.backend = "/usr/bin/ls";
    assert(validate(invocation).valid);
    auto oversized_invocation = invocation;
    oversized_invocation.arguments.resize(100001);
    assert(!validate(oversized_invocation).valid);
    return 0;
}
