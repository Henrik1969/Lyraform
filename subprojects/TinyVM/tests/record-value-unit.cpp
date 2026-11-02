// Uninstalled unit adapter. The real blocked source plan is not rewritten.
#define main flowtiny_public_main_for_record_unit
#include "../tools/lower.cpp"
#undef main

int main(int argc, char** argv) {
    try {
        if (argc != 2) throw std::runtime_error("expected blocked source semantic report");
        std::ifstream input(argv[1]);
        const auto value = parse(std::string(std::istreambuf_iterator<char>(input), {}));
        const auto& root = object(value);
        if (string(required(root, "format"), "$.format") != "flowanalyst.semantic_report")
            throw std::runtime_error("unit requires a semantic report");
        Compiler compiler(root, "record-unit-source", "record-unit-lowering");
        compiler.compile_record_unit();
        if (!compiler.imports.empty() || !compiler.strings.empty() || !compiler.storage.empty())
            throw std::runtime_error("record unit admits only scalar/nested values");
        TinyvmArtifactV2 artifact;
        tinyvm_artifact_v2_init(&artifact);
        artifact.isa_version = compiler.isa_version();
        artifact.data_words = compiler.slot_count(); artifact.stack_words = 16;
        copy(artifact.artifact_id, "record-unit-artifact"); copy(artifact.source_id, "record-unit-source");
        copy(artifact.target_policy_id, "record-unit-target"); copy(artifact.lowering_plan_id, "record-unit-lowering");
        copy(artifact.optimization_id, "record-unit-optimization");
        artifact.code = compiler.code.data(); artifact.code_count = compiler.code.size();
        artifact.constants = compiler.constants.data(); artifact.constant_count = compiler.constants.size();
        artifact.provenance = compiler.provenance.data(); artifact.provenance_count = compiler.provenance.size();
        char diagnostic[256];
        if (!tinyvm_artifact_v2_validate(&artifact, diagnostic, sizeof diagnostic)) throw std::runtime_error(diagnostic);
        std::vector<Integer> records;
        for (const auto& item : required_array(required_object(root, "lowering_plan"), "operations")) {
            const auto& operation = object(item);
            if (string(required(operation, "kind"), "$.kind") != "value_definition") continue;
            const auto& operands = required_array(operation, "operands");
            if (!operands.empty() && string(required(object(operands.front()), "type"), "$.type") == "ordinary_record")
                records.push_back(integer(required(operation, "result_symbol_id"), "$.symbol"));
        }
        if (records.size() < 2) throw std::runtime_error("unit requires original and copied record");
        const auto original = compiler.record_unit_leaves(records[0]);
        const auto copied = compiler.record_unit_leaves(records[1]);
        if (original.size() != copied.size() || original.empty()) throw std::runtime_error("copy shape mismatch");
        std::uint64_t result = 0;
        for (int engine = 0; engine < 2; ++engine) {
            struct Context {
                TinyvmIsaV1Context value{};
                ~Context() { tinyvm_isa_v1_context_destroy(&value); }
            } context;
            if (!tinyvm_isa_v1_context_init(&context.value, compiler.slot_count(), 100000))
                throw std::runtime_error("cannot allocate unit VM context");
            const bool okay = engine ? tinyvm_isa_v1_run_computed(&artifact, &context.value)
                                     : tinyvm_isa_v1_run_switch(&artifact, &context.value);
            if (!okay || context.value.trap || !context.value.returned) throw std::runtime_error("record unit did not return normally");
            if (engine && result != context.value.result.bits) throw std::runtime_error("VM engine result mismatch");
            result = context.value.result.bits;
            for (std::size_t index = 0; index < original.size(); ++index) {
                if (original[index] == copied[index]) throw std::runtime_error("record copy aliases original field slot");
                const auto prior = context.value.slots[copied[index]].bits;
                if (context.value.slots[original[index]].bits != prior) throw std::runtime_error("record copy changed field value");
                // Unit observation after source execution: this is not source
                // member-write authority. Prove distinct runtime storage.
                context.value.slots[original[index]].bits ^= 1;
                if (context.value.slots[copied[index]].bits != prior) throw std::runtime_error("record copy storage is not independent");
            }
        }
        std::cout << "{\"result\":" << result << ",\"trap\":0,\"engines\":2,\"independent_fields\":" << copied.size() << "}\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n'; return 1;
    }
}
