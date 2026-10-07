#include <flowcontracts/failure_response_attempt.hpp>

#include <iostream>
#include <stdexcept>

namespace {
void require(bool condition) {
    if (!condition) throw std::runtime_error("response-attempt failure regression");
}

flowcontracts::FailureFlowClosedSetPlan stage(
    long long plan_id, long long disposition, long long operation,
    long long producer_function, long long consumer, long long route,
    long long function, long long transition, std::string failure_type,
    std::string output_type) {
    using namespace flowcontracts;
    FailureFlowClosedSetPlan result;
    result.id = plan_id;
    result.schedule = "serial_closed_failure_set_v1";
    result.producer = {disposition, operation, producer_function, output_type,
                       {failure_type}, "failure", "exactly_one",
                       "failure_publishes_no_normal_state", "sensor.flow:4:5"};
    result.wires = {{disposition + 100, disposition, operation, "failure",
                     consumer, route, "failure_envelope", failure_type}};
    result.consumer = {consumer, consumer + 1, {failure_type},
                       {{route, failure_type, function}}};
    result.functions = {{function, "failure_envelope", failure_type,
                         output_type, "definition"}};
    result.selections = {{consumer, failure_type, route,
                          "response_failure_policy", "v1"}};
    result.transitions = {{transition, consumer, route, function, failure_type,
                           "failure_envelope", "recover", "success",
                           output_type, "close_original",
                           "preserve_origin_commit", "link_response_to_origin",
                           "separate_obligations"}};
    return result;
}

flowcontracts::FailureResponseAttemptPlan plan() {
    using namespace flowcontracts;
    FailureResponseAttemptPlan result;
    result.id = 700;
    result.schedule = "serial_response_failure_recovery_v1";
    result.original = stage(100, 10, 20, 21, 30, 40, 50, 60,
                            "ReadFailure", "Reading");
    result.response_failure = stage(101, 11, 22, 50, 31, 41, 51, 61,
                                    "CacheUnavailable", "Reading");
    result.link = {70, 100, "ReadFailure", 40, 50, 60, 101, 11, 22,
                   "CacheUnavailable", "remain_open_until_rejoin",
                   "distinct_linked_obligation", "preserve_correlation",
                   "response_failure_publishes_no_transition",
                   "response_failure_provenance_to_successor_origin",
                   "recovered_payload_resumes_original_transition"};
    return result;
}

flowcontracts::FailureEnvelope origin() {
    using namespace flowcontracts;
    return {10, 200, 20, "read-attempt", "correlation-1", "ReadFailure",
            json::Object{{"code", "sensor_unavailable"}}, "no_commit",
            "sensor.flow:4:5/provider:thermometer"};
}

template <typename Mutation>
void require_plan_refusal(Mutation mutation) {
    auto hostile = plan();
    mutation(hostile);
    require(!flowcontracts::failure_response_attempt_plan_refusal(hostile).empty());
}
}

int main() {
    using namespace flowcontracts;
    try {
        const auto authority = plan();
        require(failure_response_attempt_plan_refusal(authority).empty());
        const auto fact = failure_response_attempt_plan_fact(authority);
        const auto decoded = read_failure_response_attempt_plan(fact, "$.plan");
        require(failure_response_attempt_plan_refusal(decoded).empty());
        require(json::serialize(failure_response_attempt_plan_fact(decoded)) ==
                json::serialize(fact));

        const std::map<long long, FailureResponseCallable> response{{50,
            [](const FailureEnvelope& input) {
                require(input.obligation == 200);
                return FailureResponseResult{
                    "failure", "CacheUnavailable",
                    json::Object{{"cache", "offline"}},
                    "use_cache:attempt:failure"};
            }}};
        const std::map<long long, FailureResponseCallable> recovery{{51,
            [](const FailureEnvelope& input) {
                require(input.disposition == 11);
                require(input.obligation == 201);
                require(input.producer_operation == 22);
                require(input.attempt == "cache-attempt");
                require(input.correlation == "correlation-1");
                require(input.failure_type == "CacheUnavailable");
                require(input.origin_commit == "no_commit");
                require(input.provenance == "use_cache:attempt:failure");
                return FailureResponseResult{"success", "Reading", 21,
                                             "fallback:recover"};
            }}};
        const auto source = origin();
        const auto receipt = execute_failure_response_attempt_recovery(
            authority, source, "cache-attempt", 201, "fallback-attempt",
            response, recovery);
        require(receipt.original_obligation == 200);
        require(receipt.response_failure_obligation == 201);
        require(receipt.response_failure_recovery.original_obligation == 201);
        require(receipt.response_failure_recovery.successor_obligation == -1);
        require(receipt.final_disposition == "success");
        require(receipt.final_payload_type == "Reading");
        require(failure_response_attempt_receipt_refusal(
                    receipt, authority, source).empty());

        require_plan_refusal([](auto& value) { value.id = -1; });
        require_plan_refusal([](auto& value) { value.schedule = "recursive"; });
        require_plan_refusal([](auto& value) {
            value.response_failure.id = value.original.id;
        });
        require_plan_refusal([](auto& value) { value.link.original_plan = 999; });
        require_plan_refusal([](auto& value) { value.link.original_route = 999; });
        require_plan_refusal([](auto& value) { value.link.original_function = 999; });
        require_plan_refusal([](auto& value) { value.link.original_transition = 999; });
        require_plan_refusal([](auto& value) {
            value.link.response_failure_plan = 999;
        });
        require_plan_refusal([](auto& value) {
            value.link.response_failure_disposition = 999;
        });
        require_plan_refusal([](auto& value) {
            value.link.response_failure_operation = 999;
        });
        require_plan_refusal([](auto& value) {
            value.link.response_failure_type = "Other";
        });
        require_plan_refusal([](auto& value) {
            value.response_failure.producer.function = 999;
        });
        require_plan_refusal([](auto& value) {
            value.response_failure.transitions.front().outgoing_payload_type = "Text";
            value.response_failure.functions.front().result_type = "Text";
        });
        require_plan_refusal([](auto& value) {
            value.response_failure.consumer.routes.front().function = 50;
            value.response_failure.functions.front().id = 50;
            value.response_failure.transitions.front().function = 50;
        });
        require_plan_refusal([](auto& value) {
            value.link.original_obligation_law = "close_on_failure";
        });
        require_plan_refusal([](auto& value) {
            value.link.response_failure_obligation_law = "reuse_original";
        });
        require_plan_refusal([](auto& value) { value.link.correlation_law = "replace"; });
        require_plan_refusal([](auto& value) { value.link.commit_law = "commit"; });
        require_plan_refusal([](auto& value) { value.link.provenance_law = "erase"; });
        require_plan_refusal([](auto& value) { value.link.rejoin_law = "infer"; });

        try {
            (void)execute_failure_response_attempt_recovery(
                authority, source, "cache-attempt", 200, "fallback-attempt",
                response, recovery);
            require(false);
        } catch (const json::Error&) {}
        const std::map<long long, FailureResponseCallable> unexpected_success{{50,
            [](const FailureEnvelope&) {
                return FailureResponseResult{"success", "Reading", 21,
                                             "use_cache:success"};
            }}};
        try {
            (void)execute_failure_response_attempt_recovery(
                authority, source, "cache-attempt", 201, "fallback-attempt",
                unexpected_success, recovery);
            require(false);
        } catch (const json::Error&) {}
        const std::map<long long, FailureResponseCallable> wrong_failure{{50,
            [](const FailureEnvelope&) {
                return FailureResponseResult{"failure", "NetworkUnavailable",
                                             "offline", "use_cache:failure"};
            }}};
        try {
            (void)execute_failure_response_attempt_recovery(
                authority, source, "cache-attempt", 201, "fallback-attempt",
                wrong_failure, recovery);
            require(false);
        } catch (const json::Error&) {}
        const std::map<long long, FailureResponseCallable> fault{{50,
            [](const FailureEnvelope&) {
                return FailureResponseResult{"fault", "ProviderIntegrityFault",
                                             "broken", "provider:fault"};
            }}};
        try {
            (void)execute_failure_response_attempt_recovery(
                authority, source, "cache-attempt", 201, "fallback-attempt",
                fault, recovery);
            require(false);
        } catch (const json::Error&) {}
        const std::map<long long, FailureResponseCallable> throwing{{50,
            [](const FailureEnvelope&) -> FailureResponseResult {
                throw std::runtime_error("host exception");
            }}};
        try {
            (void)execute_failure_response_attempt_recovery(
                authority, source, "cache-attempt", 201, "fallback-attempt",
                throwing, recovery);
            require(false);
        } catch (const json::Error&) {}

        auto hostile_receipt = receipt;
        hostile_receipt.original_obligation = 201;
        require(!failure_response_attempt_receipt_refusal(
                    hostile_receipt, authority, source).empty());
        hostile_receipt = receipt;
        hostile_receipt.response_failure_envelope.provenance = "erased";
        require(!failure_response_attempt_receipt_refusal(
                    hostile_receipt, authority, source).empty());
        hostile_receipt = receipt;
        hostile_receipt.final_payload = 22;
        require(!failure_response_attempt_receipt_refusal(
                    hostile_receipt, authority, source).empty());

        auto future = json::object(fact);
        future["version"] = 2;
        try {
            (void)read_failure_response_attempt_plan(future, "$.plan");
            require(false);
        } catch (const json::Error&) {}

        std::cout << "Canonical response-attempt failure: separate obligation, exact recovery route, typed rejoin and hostile refusal PASS\n";
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
