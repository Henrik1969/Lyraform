#include <flowcontracts/failure_response_transition.hpp>

#include <iostream>
#include <stdexcept>

namespace {
void require(bool condition) {
    if (!condition) throw std::runtime_error("failure-response transition contract regression");
}

template <typename Mutation>
void require_refusal(const std::vector<flowcontracts::FailureResponseTransition>& source,
                     const flowcontracts::FailureConsumer& consumer,
                     const std::vector<flowcontracts::FailureConsumerFunction>& functions,
                     Mutation mutation) {
    auto hostile = source;
    mutation(hostile);
    require(!flowcontracts::failure_response_transitions_refusal(hostile, consumer, functions).empty());
}
}

int main() {
    using namespace flowcontracts;
    try {
        const std::vector<FailureConsumerFunction> functions{
            {10, "failure_envelope", "GuardViolation", "SafeReading", "definition"},
            {11, "failure_envelope", "ResourceUnavailable", "NetworkUnavailable", "definition"}};
        const FailureConsumer consumer{3, 7,
            {"GuardViolation", "ResourceUnavailable"},
            {{20, "GuardViolation", 10}, {21, "ResourceUnavailable", 11}}};
        const std::vector<FailureResponseTransition> transitions{
            {30, 3, 20, 10, "GuardViolation", "failure_envelope", "recover",
             "success", "SafeReading", "close_original", "preserve_origin_commit",
             "link_response_to_origin", "separate_obligations"},
            {31, 3, 21, 11, "ResourceUnavailable", "failure_envelope", "transform",
             "failure", "NetworkUnavailable", "linked_successor", "preserve_origin_commit",
             "link_response_to_origin", "separate_obligations"}};

        require(failure_response_transitions_refusal(transitions, consumer, functions).empty());
        std::vector<FailureResponseTransition> roundtrip;
        for (std::size_t index = 0; index < transitions.size(); ++index) {
            const auto fact = failure_response_transition_fact(transitions[index]);
            const auto decoded = read_failure_response_transition(
                fact, "$.transitions[" + std::to_string(index) + "]");
            require(json::serialize(failure_response_transition_fact(decoded)) ==
                    json::serialize(fact));
            roundtrip.push_back(decoded);
        }
        require(failure_response_transitions_refusal(roundtrip, consumer, functions).empty());

        require_refusal(transitions, consumer, functions, [](auto& value) { value.clear(); });
        require_refusal(transitions, consumer, functions, [](auto& value) { value[0].id = -1; });
        require_refusal(transitions, consumer, functions, [](auto& value) { value[1].id = value[0].id; });
        require_refusal(transitions, consumer, functions, [](auto& value) { value[0].consumer = 4; });
        require_refusal(transitions, consumer, functions, [](auto& value) { value[0].route = 99; });
        require_refusal(transitions, consumer, functions, [](auto& value) { value[1].route = value[0].route; });
        require_refusal(transitions, consumer, functions, [](auto& value) { value.pop_back(); });
        require_refusal(transitions, consumer, functions, [](auto& value) { value[0].function = 11; });
        require_refusal(transitions, consumer, functions, [](auto& value) { value[0].incoming_failure_type = "OtherFailure"; });
        require_refusal(transitions, consumer, functions, [](auto& value) { value[0].input_projection = "payload"; });
        require_refusal(transitions, consumer, functions, [](auto& value) { value[0].outgoing_payload_type = "OtherType"; });
        require_refusal(transitions, consumer, functions, [](auto& value) { value[0].response_class = "propagate"; });
        require_refusal(transitions, consumer, functions, [](auto& value) { value[0].outgoing_disposition = "failure"; });
        require_refusal(transitions, consumer, functions, [](auto& value) { value[0].obligation_transition = "linked_successor"; });
        require_refusal(transitions, consumer, functions, [](auto& value) { value[1].outgoing_disposition = "success"; });
        require_refusal(transitions, consumer, functions, [](auto& value) { value[1].obligation_transition = "close_original"; });
        require_refusal(transitions, consumer, functions, [](auto& value) { value[0].origin_commit_law = "replace"; });
        require_refusal(transitions, consumer, functions, [](auto& value) { value[0].provenance_law = "drop"; });
        require_refusal(transitions, consumer, functions, [](auto& value) { value[0].response_attempt_failure_law = "erase"; });

        auto incompatible_functions = functions;
        incompatible_functions[0].result_type = "void";
        require(!failure_response_transitions_refusal(transitions, consumer, incompatible_functions).empty());
        incompatible_functions = functions;
        incompatible_functions[0].availability = "declaration";
        require(!failure_response_transitions_refusal(transitions, consumer, incompatible_functions).empty());

        auto executable = json::object(failure_response_transition_fact(transitions.front()));
        executable["status"] = "ready";
        try {
            (void)read_failure_response_transition(executable, "$.transition");
            require(false);
        } catch (const json::Error&) {}

        auto future = json::object(failure_response_transition_fact(transitions.front()));
        future["version"] = 2;
        try {
            (void)read_failure_response_transition(future, "$.transition");
            require(false);
        } catch (const json::Error&) {}

        auto missing = json::object(failure_response_transition_fact(transitions.front()));
        missing.erase("provenance_law");
        try {
            (void)read_failure_response_transition(missing, "$.transition");
            require(false);
        } catch (const json::Error&) {}

        std::cout << "Declarative failure response transitions: recover, transform, complete route coverage and hostile refusals PASS\n";
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
