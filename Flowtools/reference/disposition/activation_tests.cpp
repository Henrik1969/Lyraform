#include "activation.h"
#include <flowcontracts/disposition_facts.hpp>
#include <fstream>
#include <iterator>
#include <iostream>
#include <map>
#include <cstring>
#include <cstdlib>
#include <array>
#include <type_traits>

namespace {
void require(bool value) {
    if (!value) throw std::runtime_error("source activation frame regression");
}
template<class Action>
void refuse_unchanged(FlowActivationFrame& frame, Action action) {
    std::array<unsigned char, sizeof(frame)> before;
    std::memcpy(before.data(), &frame, sizeof(frame));
    require(action() == 0);
    require(std::memcmp(before.data(), &frame, sizeof(frame)) == 0);
}

int route_dispose_calls = 0;
FlowActivationTextOutcome route_failure() { return {2, nullptr}; }
const char* route_borrowed(const FlowActivationEnvelope*) { return "borrowed"; }
const char* route_owned(const FlowActivationEnvelope*) {
    char* value = static_cast<char*>(std::malloc(6));
    if (value) std::memcpy(value, "owned", 6);
    return value;
}
int route_rejoin(const char* value) {
    return value && (std::strcmp(value, "borrowed") == 0 || std::strcmp(value, "owned") == 0) ? 9 : 70;
}
int route_dispose(char* value) {
    ++route_dispose_calls;
    std::free(value);
    return 0;
}
}

int main(int argc, char** argv) {
    using namespace flowcontracts::json;
    try {
        require(argc == 2);
        std::ifstream input(argv[1]);
        require(input.good());
        const auto document = parse(std::string(std::istreambuf_iterator<char>(input), {}));
        const auto& plan_value = required(object(document), "lowering_plan");
        flowcontracts::validate_disposition_facts(plan_value, "$.lowering_plan");
        const auto& plan = object(plan_value);
        const auto version = integer(required(plan, "disposition_graph_version"), "$.version");
        require(version == 2 || version == 3);
        const auto& topology = object(required(plan, "source_disposition"));
        const auto& producer = object(required(topology, "producer"));
        const auto& module = object(required(topology, "module"));
        const auto& junction = object(required(topology, "junction"));
        const auto& containment = object(required(topology, "containment"));
        const auto& selection = object(array(required(object(required(plan, "source_disposition_policy")), "selections"), "$.selections").front());
        auto number = [](const Object& value, const char* key) { return integer(required(value, key), key); };
        const auto function = number(selection, "response_function_symbol_id");
        const Object* response = nullptr;
        for (const auto& raw : array(required(topology, "response_functions"), "$.responses"))
            if (number(object(raw), "function_symbol_id") == function) response = &object(raw);
        require(response != nullptr);
        // Storage remains alive for all result consumers; source authority is
        // projected by identity, with no function-name or fixture-name lookup.
        std::map<std::string, std::string> storage;
        auto text = [&](const char* key, const Value& value) {
            return storage.emplace(key, string(value, key)).first->second.c_str();
        };
        auto serialized = [&](const char* key, const Value& value) {
            return storage.emplace(key, serialize(value)).first->second.c_str();
        };
        FlowActivationAuthority authority{};
        authority.module = text("module", required(module, "identity"));
        authority.revision = text("revision", required(module, "revision"));
        authority.activation = "activation:runtime-test:1";
        authority.producer = text("producer", required(producer, "graph_node_id"));
        authority.obligation = text("obligation", required(producer, "obligation_id"));
        authority.success_type = text("success", required(object(required(producer, "success_type")), "identity"));
        authority.failure_type = text("failure", required(object(required(*response, "incoming_failure_type")), "identity"));
        authority.fault_type = text("fault", required(object(array(required(producer, "fault_types"), "$.faults").front()), "identity"));
        authority.failure_wire = text("failure_wire", required(junction, "failure_wire_id"));
        authority.fault_wire = text("fault_wire", required(containment, "wire_id"));
        authority.containment = text("containment", required(containment, "node_id"));
        authority.policy = serialized("policy", required(plan, "source_disposition_policy"));
        authority.host_policy = serialized("host_policy", required(plan, "host_disposition_policy"));
        authority.commit_law = text("commit_law", required(producer, "commit_law"));
        authority.provenance = serialized("provenance", required(producer, "provenance"));
        authority.response_provenance = serialized("response_provenance", required(*response, "provenance"));
        authority.operation = number(producer, "operation_id");
        authority.function = number(producer, "function_symbol_id");
        authority.disposition = number(producer, "disposition_id");
        authority.response_function = function;
        authority.response_parameter = number(*response, "parameter_symbol_id");
        authority.route = number(selection, "route_id");
        const char success[] = "ordinary response result";
        const int failure = 2, fault = 17;
        FlowActivationValue normal{authority.success_type, success, sizeof(success)};
        FlowActivationValue failed{authority.failure_type, &failure, sizeof(failure)};
        FlowActivationValue faulted{authority.fault_type, &fault, sizeof(fault)};
        FlowActivationFrame frame{};
        auto malformed = authority; malformed.policy = "";
        refuse_unchanged(frame, [&] { return flow_activation_begin(&frame, &malformed, "attempt:1", "correlation:1"); });
        require(flow_activation_begin(&frame, &authority, "attempt:1", "correlation:1"));
        refuse_unchanged(frame, [&] { return flow_activation_begin(&frame, &authority, "attempt:2", "correlation:2"); });
        require(flow_activation_result(&frame) == nullptr);
        refuse_unchanged(frame, [&] { return flow_activation_fail(&frame, faulted, "no_commit"); });
        refuse_unchanged(frame, [&] { return flow_activation_fail(&frame, failed, "committed"); });
        require(flow_activation_fail(&frame, failed, "no_commit"));
        static_assert(std::is_same_v<decltype(flow_activation_failure(&frame)), const FlowActivationEnvelope*>);
        const auto* envelope = flow_activation_failure(&frame);
        require(envelope && envelope->payload.data == &failure && envelope->payload.size == sizeof(failure));
        require(envelope->authority.operation == authority.operation && envelope->authority.disposition == authority.disposition);
        require(envelope->authority.function == authority.function && envelope->authority.obligation == authority.obligation);
        require(envelope->authority.provenance == authority.provenance && envelope->authority.policy == authority.policy);
        require(envelope->authority.host_policy == authority.host_policy && envelope->authority.commit_law == authority.commit_law);
        require(std::strcmp(envelope->attempt, "attempt:1") == 0 && std::strcmp(envelope->correlation, "correlation:1") == 0);
        refuse_unchanged(frame, [&] { return flow_activation_publish(&frame, normal); });
        refuse_unchanged(frame, [&] { return flow_activation_fail(&frame, failed, "no_commit"); });
        auto recover = [&](int64_t fn, int64_t parameter, int64_t route, const char* attempt, FlowActivationValue value) {
            return flow_activation_recover(&frame, fn, parameter, route, attempt, authority.response_provenance, value);
        };
        refuse_unchanged(frame, [&] { return recover(function + 1000, authority.response_parameter, authority.route, "response:1", normal); });
        refuse_unchanged(frame, [&] { return recover(function, authority.response_parameter + 1000, authority.route, "response:1", normal); });
        refuse_unchanged(frame, [&] { return recover(function, authority.response_parameter, authority.route + 1000, "response:1", normal); });
        refuse_unchanged(frame, [&] { return recover(function, authority.response_parameter, authority.route, "attempt:1", normal); });
        refuse_unchanged(frame, [&] { return recover(function, authority.response_parameter, authority.route, "response:1", failed); });
        refuse_unchanged(frame, [&] { return flow_activation_recover(&frame, function, authority.response_parameter, authority.route, "response:1", "foreign source span", normal); });
        require(recover(function, authority.response_parameter, authority.route, "response:1", normal));
        const auto* result = flow_activation_result(&frame);
        require(result && result->disposition == FLOW_ACTIVATION_PUBLISHED && result->obligation_closed);
        require(result->envelope == envelope && result->value.data == success && result->response_function == function);
        require(envelope->authority.provenance == authority.provenance && envelope->payload.data == &failure);
        refuse_unchanged(frame, [&] { return recover(function, authority.response_parameter, authority.route, "response:2", normal); });
        refuse_unchanged(frame, [&] { return flow_activation_quarantine(&frame, faulted, "no_commit"); });
        FlowActivationFrame direct{};
        require(flow_activation_begin(&direct, &authority, "attempt:2", "correlation:2"));
        require(flow_activation_publish(&direct, normal));
        require(flow_activation_result(&direct)->envelope == nullptr);
        refuse_unchanged(direct, [&] { return flow_activation_publish(&direct, normal); });
        FlowActivationFrame quarantined{};
        // Long evidence must remain whole; no fixed-size evidence buffers.
        std::string long_evidence(65536, 'p');
        auto long_authority = authority; long_authority.provenance = long_evidence.c_str();
        require(flow_activation_begin(&quarantined, &long_authority, "attempt:3", "correlation:3"));
        refuse_unchanged(quarantined, [&] { return flow_activation_quarantine(&quarantined, failed, "no_commit"); });
        require(flow_activation_quarantine(&quarantined, faulted, "no_commit"));
        const auto* contained = flow_activation_result(&quarantined);
        require(contained && contained->disposition == FLOW_ACTIVATION_QUARANTINED);
        require(contained->value.data == nullptr && contained->containment == authority.containment);
        require(contained->envelope->payload.data == &fault && !contained->obligation_closed);
        require(std::strlen(contained->envelope->authority.provenance) == long_evidence.size());
        refuse_unchanged(quarantined, [&] { return flow_activation_publish(&quarantined, normal); });
        refuse_unchanged(quarantined, [&] { return flow_activation_fail(&quarantined, failed, "no_commit"); });
        refuse_unchanged(quarantined, [&] { return flow_activation_recover(&quarantined, function, authority.response_parameter, authority.route, "response:3", "source", normal); });
        refuse_unchanged(quarantined, [&] { return flow_activation_quarantine(&quarantined, faulted, "no_commit"); });
        require(flow_activation_result(&frame) == result && result->value.data == success);
        FlowActivationFrame routed{};
        route_dispose_calls = 0;
        require(flow_activation_route_text(&routed, &authority, route_failure, route_owned,
                                           route_rejoin, route_dispose, route_dispose,
                                           authority.response_function, authority.response_parameter,
                                           authority.route, "attempt:owned", "correlation:owned",
                                           "response:owned", authority.response_provenance) == 9);
        require(route_dispose_calls == 1);
        require(flow_activation_result(&routed)->value.data != nullptr);
        routed = FlowActivationFrame{};
        route_dispose_calls = 0;
        require(flow_activation_route_text(&routed, &authority, route_failure, route_borrowed,
                                           route_rejoin, route_dispose, nullptr,
                                           authority.response_function, authority.response_parameter,
                                           authority.route, "attempt:borrowed", "correlation:borrowed",
                                           "response:borrowed", authority.response_provenance) == 9);
        require(route_dispose_calls == 0);
        std::cout << "PASS source-derived activation frame: exclusive publication, recovery evidence, quarantine, unchanged refusals; source execution unsupported\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n'; return 1;
    }
}
