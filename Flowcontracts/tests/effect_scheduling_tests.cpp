#include <flowcontracts/effect_scheduling.hpp>

#include <iostream>
#include <stdexcept>

namespace {
void require(bool condition) {
    if (!condition) throw std::runtime_error("effect-scheduling contract regression");
}

flowcontracts::json::Object capability(const std::string& symbol) {
    static const std::string digest(64, 'a');
    static const std::string provider_digest(64, 'b');
    return {{"contract", "observation"}, {"library", "libobservation.so"},
            {"symbol", symbol}, {"convention", "c"}, {"effect", "readonly"},
            {"parameter_types", "c_int"}, {"return_type", "c_int"},
            {"evidence", "flowcore.generated_binding.v1:" + digest + ":" + provider_digest}};
}
}

int main() {
    using namespace flowcontracts;
    try {
        ProviderEffectProfile left_profile{1, capability("left_observe"), "fixture:left", "observe",
                                           "concurrent_observation_v1", "infallible_scalar_v1"};
        ProviderEffectProfile right_profile{2, capability("right_observe"), "fixture:right", "observe",
                                            "concurrent_observation_v1", "infallible_scalar_v1"};
        require(provider_effect_profile_refusal(left_profile).empty());
        const auto left_roundtrip = read_provider_effect_profile(provider_effect_profile_fact(left_profile), "$.profile");
        require(provider_effect_profile_refusal(left_roundtrip).empty());
        require(json::serialize(provider_effect_profile_fact(left_roundtrip)) ==
                json::serialize(provider_effect_profile_fact(left_profile)));
        const auto profile_set = read_provider_effect_profiles(
            provider_effect_profile_set_fact({left_profile, right_profile}));
        require(profile_set.size() == 2);

        EffectAccessFact left{10, 100, "left", 1, capability_identity(left_profile.capability, "$.capability"),
                              "readonly", "fixture:left", "observe", "concurrent_observation_v1", "operation:10"};
        EffectAccessFact right{11, 101, "right", 2, capability_identity(right_profile.capability, "$.capability"),
                               "readonly", "fixture:right", "observe", "concurrent_observation_v1", "operation:11"};
        require(effect_access_refusal(left, left_profile).empty());
        require(effect_access_refusal(right, right_profile).empty());
        const auto access_roundtrip = read_effect_access(effect_access_fact(left), "$.access");
        require(effect_access_refusal(access_roundtrip, left_profile).empty());

        EffectConflictFact pair{10, 11, "independent", "concurrent_observation_v1", "none"};
        require(effect_conflict_refusal(pair, left, right).empty());
        EffectScheduleFact schedule{"fixture-graph", "parallel_independent_v1",
                                    "deterministic_activation_order_v1", "publish_after_wave_join_v1",
                                    "no_partial_wave_publication_v1", {10, 11}};
        require(effect_schedule_refusal(schedule, {left, right}, {pair}).empty());

        auto hostile_profile = left_profile;
        hostile_profile.id = -1;
        require(!provider_effect_profile_refusal(hostile_profile).empty());
        hostile_profile = left_profile;
        hostile_profile.resource_domain.clear();
        require(!provider_effect_profile_refusal(hostile_profile).empty());
        hostile_profile = left_profile;
        hostile_profile.access = "write";
        require(!provider_effect_profile_refusal(hostile_profile).empty());
        hostile_profile = left_profile;
        hostile_profile.concurrency = "unknown";
        require(!provider_effect_profile_refusal(hostile_profile).empty());
        hostile_profile = left_profile;
        hostile_profile.failure = "fallible";
        require(!provider_effect_profile_refusal(hostile_profile).empty());
        hostile_profile = left_profile;
        hostile_profile.capability["effect"] = "io";
        require(!provider_effect_profile_refusal(hostile_profile).empty());
        hostile_profile = left_profile;
        hostile_profile.capability["parameter_types"] = "c_pointer";
        require(!provider_effect_profile_refusal(hostile_profile).empty());
        hostile_profile = left_profile;
        hostile_profile.capability["evidence"] = "";
        require(!provider_effect_profile_refusal(hostile_profile).empty());
        hostile_profile = left_profile;
        hostile_profile.capability.erase("parameter_types");
        require(!provider_effect_profile_refusal(hostile_profile).empty());

        auto duplicate_profiles = json::object(provider_effect_profile_set_fact({left_profile, right_profile}));
        auto& duplicate_values = std::get<json::Array>(duplicate_profiles.at("profiles"));
        std::get<json::Object>(duplicate_values[1])["profile_id"] = 1;
        try {
            (void)read_provider_effect_profiles(duplicate_profiles);
            require(false);
        } catch (const json::Error&) {}

        auto hostile_access = left;
        hostile_access.profile = 2;
        require(!effect_access_refusal(hostile_access, left_profile).empty());
        hostile_access = left;
        hostile_access.capability = right.capability;
        require(!effect_access_refusal(hostile_access, left_profile).empty());
        hostile_access = left;
        hostile_access.resource_domain = "fixture:other";
        require(!effect_access_refusal(hostile_access, left_profile).empty());

        auto hostile_pair = pair;
        hostile_pair.relation = "conflict";
        require(!effect_conflict_refusal(hostile_pair, left, right).empty());
        hostile_pair = pair;
        hostile_pair.right_operation = 12;
        require(!effect_conflict_refusal(hostile_pair, left, right).empty());

        auto hostile_schedule = schedule;
        hostile_schedule.operations.pop_back();
        require(!effect_schedule_refusal(hostile_schedule, {left, right}, {pair}).empty());
        hostile_schedule = schedule;
        hostile_schedule.operations.push_back(11);
        require(!effect_schedule_refusal(hostile_schedule, {left, right}, {pair}).empty());
        hostile_schedule = schedule;
        hostile_schedule.publication = "publish_as_completed";
        require(!effect_schedule_refusal(hostile_schedule, {left, right}, {pair}).empty());
        require(!effect_schedule_refusal(schedule, {left, right}, {}).empty());

        auto executable = json::object(provider_effect_profile_fact(left_profile));
        executable["status"] = "ready";
        try {
            (void)read_provider_effect_profile(executable, "$.profile");
            require(false);
        } catch (const json::Error&) {}

        std::cout << "Effect scheduling contracts: profiles, access, conflicts, schedules and hostile refusals PASS\n";
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
