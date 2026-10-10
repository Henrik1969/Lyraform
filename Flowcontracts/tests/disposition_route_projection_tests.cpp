#include <flowcontracts/validate.hpp>
#include <flowcontracts/disposition_evidence_epoch.hpp>
#include <fstream>
#include <iostream>
#include <stdexcept>

using namespace flowcontracts;
using namespace flowcontracts::json;
namespace {
void require(bool value) { if (!value) throw std::runtime_error("disposition projection regression"); }
Object& obj(Value& v) { return std::get<Object>(v); }
Array& arr(Value& v) { return std::get<Array>(v); }
Value origin() { return Object{{"source", "imported.flow"}, {"line", 4}, {"column", 5}}; }
Value fixture() {
    FailureFlowPlan failure;
    failure.id = 100;
    failure.schedule = "serial_explicit_failure_route_v1";
    failure.producer = {10, 20, 30, "Reading", "ReadFailure", "failure", "exactly_one",
        "failure_publishes_no_normal_state", "imported.flow:4:5"};
    failure.wire = {40, 10, 20, "failure", 50, 60, "failure_envelope", "ReadFailure"};
    failure.consumer = {50, 70, {"ReadFailure"}, {{60, "ReadFailure", 80}}};
    failure.functions = {{80, "failure_envelope", "ReadFailure", "Reading", "definition"}};
    failure.selection = {50, "ReadFailure", 60, "laboratory", "v1"};
    failure.transitions = {{90, 50, 60, 80, "ReadFailure", "failure_envelope", "recover",
        "success", "Reading", "close_original", "preserve_origin_commit",
        "link_response_to_origin", "separate_obligations"}};
    FaultContainmentPlan fault{101, "serial_halt_quarantine_v1",
        {11, 21, 80, "IntegrityFault", "fault", "exactly_one", "fault_publishes_no_normal_state", "imported.flow:4:5"},
        {41, 11, 21, "fault", 51, "fault_envelope", "IntegrityFault"},
        {51, 71, "activation", "IntegrityFault", "halt_and_quarantine", "halted_quarantined",
         "suppressed", "none", "preserve_fault_envelope"},
        {51, "IntegrityFault", "halt_and_quarantine", "safety", "v1"}};
    Array plans{failure_flow_plan_fact(failure), fault_containment_plan_fact(fault)};
    Array nodes, wires, lanes, bindings;
    int index = 0;
    for (const auto& plan : plans) {
        const auto semantics = disposition_route_semantics(plan);
        const auto& facts = object(semantics, "$");
        const auto from = "producer-" + std::to_string(index);
        const auto to = "destination-" + std::to_string(index);
        const auto wire = "wire-" + std::to_string(index++);
        // v1 unresolved provider nodes are declaration placeholders, not source
        // functions or provider selections. The test makes no execution claim.
        for (const auto& node : {from, to})
            nodes.emplace_back(Object{{"node_id", node}, {"role", "node"},
                {"implementation_kind", "provider_atom"}, {"implementation_name", "unresolved"},
                {"provenance", origin()}});
        wires.emplace_back(Object{{"wire_id", wire}, {"provenance", origin()},
            {"from", Object{{"node_id", from}, {"port_id", facts.at("from_port")}, {"provenance", origin()}}},
            {"to", Object{{"node_id", to}, {"port_id", facts.at("to_port")}, {"provenance", origin()}}}});
        lanes.emplace_back(Object{{"graph_wire_id", wire}, {"from_node", from}, {"to_node", to}, {"semantics", semantics}});
        bindings.emplace_back(Object{{"node_id", from}, {"kind", "producer"},
            {"operation_id", facts.at("producer_operation_id")}, {"disposition_id", facts.at("producer_disposition_id")},
            {"function_id", facts.at("producer_function_id")}, {"payload_type", facts.at("payload_type")},
            {"provenance", facts.at("provenance")}});
        bindings.emplace_back(Object{{"node_id", to}, {"kind", facts.at("destination_kind")},
            {"identity", facts.at("destination_id")}, {"scope_id", facts.at("scope_id")},
            {"scope_kind", facts.at("scope_kind")}, {"payload_type", facts.at("payload_type")}});
    }
    Value graph = Object{{"format", "lyraform.disposition_graph_projection"}, {"version", 1},
        {"status", "declarative"}, {"graph_id", "graph-A"}, {"provenance", origin()},
        {"node_bindings", bindings}, {"lanes", lanes},
        {"source_graph", Object{{"format", "flowcore.source_graph"}, {"version", 1},
            {"status", "non_executable"}, {"receivers", Array{}},
            {"syntax", Object{{"format", "flowmini.graph_syntax"}, {"version", 1},
                {"nodes", nodes}, {"wires", wires}, {"policies", Array{}}}}}}};
    return Object{{"format", "lyraform.disposition_route_bundle"}, {"version", 1},
        {"status", "declarative"}, {"graph_side", graph}, {"plans", plans},
        {"projection", make_disposition_route_projection(graph, plans)}};
}
int mutations = 0;
template<class F> void refuse(F mutate) {
    auto raw = fixture(); mutate(raw);
    // Reparse the actual serialization consumed by both entry points.
    const auto parsed = parse(serialize(raw));
    bool rejected = false;
    try { validate_disposition_route_bundle(parsed); } catch (const Error&) { rejected = true; }
    if (!rejected) throw std::runtime_error("accepted mutation " + std::to_string(mutations));
    require(validate(parsed).classification == ValidationClass::invalid);
    ++mutations;
}
Object& graph(Value& v) { return obj(obj(v).at("graph_side")); }
Array& lanes(Value& v) { return arr(graph(v).at("lanes")); }
Object& syntax(Value& v) { return obj(obj(graph(v).at("source_graph")).at("syntax")); }
}
int main(int argc, char** argv) {
    try {
        auto raw = fixture();
        const auto encoded = serialize(raw);
        require(serialize(parse(encoded)) == encoded);
        validate_disposition_route_bundle(parse(encoded));
        require(validate(raw).classification == ValidationClass::valid);
        if (argc == 2) { std::ofstream(argv[1]) << encoded << '\n'; }
        for (const auto* field : {"lane", "payload_type", "from_port", "to_port", "scope_kind",
                                 "commit_law", "obligation_law", "provenance", "correlation_law", "route_id"}) {
            refuse([&](Value& v) { obj(obj(lanes(v)[0]).at("semantics"))[field] = "forged"; });
        }
        for (const auto* field : {"plan_id", "plan_wire_id", "producer_operation_id", "producer_disposition_id",
                                 "producer_function_id", "destination_id", "scope_id"}) {
            refuse([&](Value& v) { obj(obj(lanes(v)[0]).at("semantics"))[field] = 999; });
        }
        refuse([](Value& v) { lanes(v).clear(); });
        refuse([](Value& v) { lanes(v).push_back(lanes(v)[0]); });
        refuse([](Value& v) { std::swap(lanes(v)[0], lanes(v)[1]); });
        refuse([](Value& v) { obj(lanes(v)[0])["graph_wire_id"] = "missing"; });
        refuse([](Value& v) { obj(lanes(v)[0])["from_node"] = "renamed"; });
        refuse([](Value& v) { obj(obj(obj(arr(syntax(v).at("wires"))[0]).at("from")).at("provenance"))["line"] = 99; });
        refuse([](Value& v) { arr(syntax(v).at("wires")).clear(); });
        refuse([](Value& v) { auto& w = arr(syntax(v).at("wires")); w.push_back(w[0]); });
        refuse([](Value& v) { obj(obj(arr(syntax(v).at("wires"))[0]).at("from"))["port_id"] = "fault"; });
        refuse([](Value& v) { obj(obj(arr(syntax(v).at("wires"))[0]).at("to"))["node_id"] = "foreign"; });
        refuse([](Value& v) { auto& w = arr(syntax(v).at("wires")); auto extra = w[0]; obj(extra)["wire_id"] = "extra"; w.push_back(extra); });
        refuse([](Value& v) { arr(graph(v).at("node_bindings")).clear(); });
        refuse([](Value& v) { auto& b = arr(graph(v).at("node_bindings")); b.push_back(b[0]); });
        refuse([](Value& v) { obj(arr(graph(v).at("node_bindings"))[1])["scope_id"] = 999; });
        refuse([](Value& v) { obj(arr(graph(v).at("node_bindings"))[1])["payload_type"] = "Wrong"; });
        refuse([](Value& v) { obj(arr(graph(v).at("node_bindings"))[0])["operation_id"] = 999; });
        refuse([](Value& v) { obj(obj(v).at("projection"))["graph_id"] = "foreign"; });
        refuse([](Value& v) { obj(obj(v).at("projection"))["status"] = "ready"; });
        refuse([](Value& v) { obj(obj(v).at("projection"))["version"] = 2; });
        refuse([](Value& v) { graph(v)["status"] = "ready"; });
        refuse([](Value& v) { obj(v)["status"] = "ready"; });
        refuse([](Value& v) { obj(v)["version"] = 2; });
        refuse([](Value& v) { arr(obj(v).at("plans")).clear(); });
        refuse([](Value& v) { auto& p = arr(obj(v).at("plans")); std::swap(p[0], p[1]); });
        refuse([](Value& v) { obj(obj(arr(obj(v).at("plans"))[0]).at("wire"))["route_id"] = 999; });
        refuse([](Value& v) { obj(obj(arr(obj(v).at("plans"))[0]).at("policy_selection"))["profile"] = "forged"; });
        refuse([](Value& v) { obj(obj(arr(obj(v).at("plans"))[1]).at("authority"))["action"] = "recover"; });
        refuse([](Value& v) { obj(obj(arr(obj(v).at("plans"))[1]).at("authority"))["scope_kind"] = "process"; });
        refuse([](Value& v) { obj(obj(arr(obj(v).at("plans"))[0]).at("producer"))["failure_type"] = "Forged"; });
        refuse([](Value& v) { obj(arr(obj(v).at("plans"))[0]).erase("producer"); });
        int diagnostics = 0;
        for (auto kind : {DispositionDiagnosticKind::missing_route, DispositionDiagnosticKind::wrong_type,
                          DispositionDiagnosticKind::ambiguous_route, DispositionDiagnosticKind::fault_to_recovery,
                          DispositionDiagnosticKind::unsupported_source, DispositionDiagnosticKind::forged_artifact}) {
            DispositionDiagnosticContext context{"imported.flow", 4, 5, 20, 10, "failure", "ReadFailure",
                "wire-0", "graph-A", "canonical cause", false, true};
            const auto diagnostic = disposition_diagnostic(kind, context);
            const auto machine = serialize(diagnostic);
            const auto human = render_disposition_diagnostic(diagnostic);
            require(human.find("imported.flow:4:5") != std::string::npos);
            require(human.find(machine) != std::string::npos);
            require(machine.find("declared_propagation") == std::string::npos);
            require(machine.find("explicit_failure_policy_sink") == std::string::npos);
            require(machine.find("supply_reviewed_producer_and_association_contract") == std::string::npos);
            require(!boolean(object(diagnostic, "$").at("runtime_attempt_created"), "$"));
            context.redacted = true;
            const auto redacted = serialize(disposition_diagnostic(kind, context));
            require(redacted.find("canonical cause") == std::string::npos);
            require(redacted.find("imported.flow") == std::string::npos);
            ++diagnostics;
        }
        const auto plan = read_failure_flow_plan(arr(obj(raw).at("plans"))[0], "$.plan");
        Value correlation = Object{{"graph_id", "graph-A"}, {"signal_id", "signal-7"},
            {"delivery_id", "delivery-9"}, {"activation_id", "activation-11"}, {"attempt_id", "attempt-13"}};
        FailureEnvelope envelope{10, 200, 20, "attempt-13", serialize(correlation), "ReadFailure",
            Object{{"reason", "unavailable"}}, "no_commit", "imported.flow:4:5"};
        const std::map<long long, FailureResponseCallable> functions{{80, [](const FailureEnvelope&) {
            return FailureResponseResult{"success", "Reading", 21, "response-origin"};
        }}};
        const auto receipt = execute_failure_flow(plan, envelope, "attempt-14", -1, functions);
        const auto closure = disposition_epoch_closure(1, "graph-A", plan, envelope, correlation, receipt);
        validate_disposition_epoch_closure(parse(serialize(closure)), 1, "graph-A", plan, envelope, correlation);
        DispositionEvidenceEpoch epoch(1, "graph-A", plan, envelope, correlation, true);
        int epoch_refusals = 0;
        auto rejects = [&](auto action) {
            bool rejected = false;
            try { action(); } catch (const Error&) { rejected = true; }
            require(rejected); ++epoch_refusals;
        };
        rejects([&] { epoch.release(); });
        require(epoch.live() && !epoch.closed());
        for (const auto* field : {"origin_provenance", "origin_commit", "boundary", "obligation_state", "historical_disposition"}) {
            auto hostile = closure; obj(hostile)[field] = "forged";
            rejects([&] { epoch.accept(hostile); });
            require(epoch.live() && !epoch.closed());
        }
        auto hostile = closure; obj(hostile).erase("policy_selection");
        rejects([&] { epoch.accept(hostile); });
        hostile = closure; obj(obj(hostile).at("correlation"))["delivery_id"] = "wrong-delivery";
        rejects([&] { epoch.accept(hostile); });
        auto over_budget = envelope; over_budget.payload = std::string(disposition_epoch_live_bytes, 'x');
        rejects([&] { DispositionEvidenceEpoch excessive(2, "graph-A", plan, over_budget, correlation, false); });
        auto large_receipt = receipt; large_receipt.outgoing_payload = std::string(disposition_epoch_receipt_bytes, 'x');
        rejects([&] { (void)disposition_epoch_closure(1, "graph-A", plan, envelope, correlation, large_receipt); });
        require(epoch.live() && !epoch.closed());
        epoch.accept(closure);
        rejects([&] { epoch.release(); });
        rejects([&] { epoch.accept(closure); });
        epoch.observer_published(); epoch.release();
        require(!epoch.live() && epoch.closed() && serialize(epoch.closure()) == serialize(closure));
        std::cout << diagnostics << " diagnostic classes; " << epoch_refusals << " evidence refusals; ";
        std::cout << "2 declarative lanes round-trip; " << mutations << " hostile mutations rejected by 2 consumers\n";
    } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
