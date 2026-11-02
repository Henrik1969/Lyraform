// Unit-only access to structured callable emission. This deliberately does not
// grant whole-graph execution or bypass the public compiler's admission check.
#include <flowcontracts/validate.hpp>
#include "../src/structured_plan.hpp"
#include <fstream>
#include <iterator>
#include <iostream>

int main(int argc, char** argv) {
    try {
        const bool record_unit = argc == 3 && std::string(argv[1]) == "--record-plan";
        if (argc != 2 && !record_unit) throw std::runtime_error("expected a prepared source graph or unit record plan");
        std::ifstream input(argv[record_unit ? 2 : 1]);
        if (!input) throw std::runtime_error("cannot read prepared source graph");
        auto root = flowcontracts::json::parse(std::string(std::istreambuf_iterator<char>(input), {}));
        if (!record_unit) flowcontracts::validate_backend_lowering_artifact(root);
        using namespace flowcontracts::json;
        const auto& plan = object(required(object(root), "lowering_plan"));
        if (record_unit) {
            if (string(required(object(root), "format"), "$.format") != "flowanalyst.semantic_report" ||
                string(required(plan, "status"), "$.status") != "blocked" ||
                !optional(plan, "ordinary_record_types"))
                throw std::runtime_error("record unit requires the original blocked semantic record plan");
            flowcontracts::validate_lowering_plan(required(object(root), "lowering_plan"));
            // Unit adapter changes the outer envelope only. Preserve the source
            // lowering plan exactly, including refusal and all typed operands.
            auto wrapper = object(root);
            wrapper["format"] = std::string("flowcore.backend_lowering_artifact");
            wrapper["version"] = Integer{2};
            root = wrapper;
        } else if (string(required(plan, "status"), "$.status") != "blocked" ||
            integer(required(plan, "disposition_graph_version"), "$.version") != 3)
            throw std::runtime_error("ABI unit fixture must retain its blocked graph-v3 claim");
        const Value binding = Object{};
        flowlower::structured::Emitter emitter(root, binding);
        if (!emitter.applicable()) throw std::runtime_error("unsupported unit callable body");
        std::cout << emitter.emit();
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
