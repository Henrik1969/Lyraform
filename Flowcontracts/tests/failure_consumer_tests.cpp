#include <flowcontracts/failure_consumer.hpp>

#include <iostream>
#include <stdexcept>

namespace {
void require(bool condition) {
    if (!condition) throw std::runtime_error("failure-consumer contract regression");
}
}

int main() {
    using namespace flowcontracts;
    try {
        const std::vector<FailureConsumerFunction> functions{
            {10, "failure_envelope", "GuardViolation", "RecoveryDecision", "definition"},
            {11, "failure_envelope", "ResourceUnavailable", "RetryDecision", "definition"},
            {12, "failure_envelope", "ResourceUnavailable", "FailureDisposition", "definition"}};
        FailureConsumer consumer{3, 7,
            {"GuardViolation", "ResourceUnavailable"},
            {{20, "GuardViolation", 10},
             {21, "ResourceUnavailable", 11},
             {22, "ResourceUnavailable", 12}}};

        require(failure_consumer_refusal(consumer, functions).empty());
        const auto roundtrip = read_failure_consumer(failure_consumer_fact(consumer), "$.consumer");
        require(failure_consumer_refusal(roundtrip, functions).empty());
        require(flowcontracts::json::serialize(failure_consumer_fact(roundtrip)) ==
                flowcontracts::json::serialize(failure_consumer_fact(consumer)));

        FailurePolicySelection selection{3, "ResourceUnavailable", 21,
                                         "network.profile", "revision-1"};
        require(failure_policy_selection_refusal(selection, consumer, functions).empty());
        const auto selected = read_failure_policy_selection(
            failure_policy_selection_fact(selection), "$.selection");
        require(failure_policy_selection_refusal(selected, consumer, functions).empty());

        auto hostile = consumer;
        hostile.accepted_failure_types.clear();
        require(!failure_consumer_refusal(hostile, functions).empty());
        hostile = consumer;
        hostile.accepted_failure_types.push_back("GuardViolation");
        require(!failure_consumer_refusal(hostile, functions).empty());
        hostile = consumer;
        hostile.routes.erase(hostile.routes.begin());
        require(!failure_consumer_refusal(hostile, functions).empty());
        hostile = consumer;
        hostile.routes[0].failure_type = "PermissionDenied";
        require(!failure_consumer_refusal(hostile, functions).empty());
        hostile = consumer;
        hostile.routes[1].id = hostile.routes[0].id;
        require(!failure_consumer_refusal(hostile, functions).empty());
        hostile = consumer;
        hostile.routes[1].function = 99;
        require(!failure_consumer_refusal(hostile, functions).empty());
        hostile = consumer;
        hostile.routes.push_back(hostile.routes.back());
        hostile.routes.back().id = 23;
        require(!failure_consumer_refusal(hostile, functions).empty());

        auto incompatible = functions;
        incompatible[0].failure_payload_type = "OtherFailure";
        require(!failure_consumer_refusal(consumer, incompatible).empty());
        incompatible = functions;
        incompatible[0].parameter_projection = "payload";
        require(!failure_consumer_refusal(consumer, incompatible).empty());
        incompatible = functions;
        incompatible[0].parameter_projection.clear();
        require(!failure_consumer_refusal(consumer, incompatible).empty());
        incompatible = functions;
        incompatible[0].result_type = "void";
        require(!failure_consumer_refusal(consumer, incompatible).empty());
        incompatible = functions;
        incompatible[0].availability = "declaration";
        require(!failure_consumer_refusal(consumer, incompatible).empty());
        incompatible = functions;
        incompatible.push_back(incompatible.front());
        require(!failure_consumer_refusal(consumer, incompatible).empty());

        auto wrong_selection = selection;
        wrong_selection.consumer = 4;
        require(!failure_policy_selection_refusal(wrong_selection, consumer, functions).empty());
        wrong_selection = selection;
        wrong_selection.failure_type = "GuardViolation";
        require(!failure_policy_selection_refusal(wrong_selection, consumer, functions).empty());
        wrong_selection = selection;
        wrong_selection.route = 99;
        require(!failure_policy_selection_refusal(wrong_selection, consumer, functions).empty());
        wrong_selection = selection;
        wrong_selection.policy.clear();
        require(!failure_policy_selection_refusal(wrong_selection, consumer, functions).empty());
        wrong_selection = selection;
        wrong_selection.policy_revision.clear();
        require(!failure_policy_selection_refusal(wrong_selection, consumer, functions).empty());

        hostile = consumer;
        hostile.routes.clear();
        require(!failure_policy_selection_refusal(selection, hostile, functions).empty());

        auto executable = flowcontracts::json::object(failure_consumer_fact(consumer));
        executable["status"] = "ready";
        try {
            (void)read_failure_consumer(executable, "$.consumer");
            require(false);
        } catch (const flowcontracts::json::Error&) {}

        auto open = flowcontracts::json::object(failure_consumer_fact(consumer));
        open["closed_set"] = false;
        try {
            (void)read_failure_consumer(open, "$.consumer");
            require(false);
        } catch (const flowcontracts::json::Error&) {}

        auto future = flowcontracts::json::object(failure_consumer_fact(consumer));
        future["version"] = 2;
        try {
            (void)read_failure_consumer(future, "$.consumer");
            require(false);
        } catch (const flowcontracts::json::Error&) {}

        auto executable_selection = flowcontracts::json::object(
            failure_policy_selection_fact(selection));
        executable_selection["status"] = "ready";
        try {
            (void)read_failure_policy_selection(executable_selection, "$.selection");
            require(false);
        } catch (const flowcontracts::json::Error&) {}

        auto future_selection = flowcontracts::json::object(
            failure_policy_selection_fact(selection));
        future_selection["version"] = 2;
        try {
            (void)read_failure_policy_selection(future_selection, "$.selection");
            require(false);
        } catch (const flowcontracts::json::Error&) {}

        std::cout << "Declarative failure consumer: closed types, authorized functions, policy selection and hostile refusals PASS\n";
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
