#!/bin/sh
set -eu
tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT
cat > "$tmpdir/graph.flow" <<'FLOW'
program graph_capture
producer source : stdin.text
node receiver : fn transform
wire source.out => receiver.in
policy source.label = "wire => stays text"
policy source.limit = -7
policy source.enabled = true
fn transform(value : c_string): int {
    return 42
}
main { return 0 }
FLOW
"$FLOWMINI_BIN" --dump-frontend-bundle "$tmpdir/graph.flow" > "$tmpdir/frontend.json"
if "$FLOWANALYST_BIN" --lowering-plan-version 2 < "$tmpdir/frontend.json" > "$tmpdir/semantic.json"; then
    echo 'native graph unexpectedly admitted' >&2; exit 1
fi
jq '.lowering_plan.source_graph' "$tmpdir/semantic.json" > "$tmpdir/graph.json"
jq -e --slurpfile front "$tmpdir/frontend.json" '.syntax == $front[0].graph_syntax and
    .syntax.policies[0].value_kind == "string" and .syntax.policies[0].value_text == "wire => stays text" and
    .syntax.policies[1].value_kind == "integer" and .syntax.policies[1].value_text == "-7" and
    .syntax.policies[2].value_kind == "boolean" and .syntax.policies[2].value_text == "true" and
    .syntax.policies[0].provenance.line == 5 and .receivers[0].node_id == "receiver"' "$tmpdir/graph.json" >/dev/null
# Each invocation reads a captured artifact after the producer has exited.
"$FLOWVALIDATE_BIN" "$tmpdir/graph.json" | jq -e '.classification == "valid"' >/dev/null
"$FLOWVALIDATE_BIN" --canonical "$tmpdir/graph.json" > "$tmpdir/canonical.json"
"$FLOWVALIDATE_BIN" --canonical "$tmpdir/canonical.json" | cmp -s - "$tmpdir/canonical.json"
for mutation in \
    '.version = 99' \
    '.status = "ready"' \
    '.syntax.version = 99' \
    '.syntax.nodes += [.syntax.nodes[0]]' \
    '.syntax.wires += [.syntax.wires[0]]' \
    '.syntax.wires[0].to.port_id = "wrong"' \
    '.syntax.wires[0].from.node_id = "absent"' \
    '.syntax.wires[0].provenance.line = 0' \
    '.syntax.policies += [.syntax.policies[0]]' \
    '.syntax.policies[0].node_id = "absent"' \
    '.syntax.policies[1].value_text = "9223372036854775808"' \
    '.syntax.policies[2].value_text = "1"' \
    'del(.syntax.policies)' \
    '.receivers[0].function_symbol_id = -1' \
    '.receivers[0].activation_contract = "persistent"' \
    '.receivers = []'
do
    jq "$mutation" "$tmpdir/graph.json" > "$tmpdir/mutated.json"
    if "$FLOWVALIDATE_BIN" "$tmpdir/mutated.json" > "$tmpdir/validation.json"; then
        echo "hostile source graph accepted: $mutation" >&2; exit 1
    fi
    jq -e '.classification == "invalid"' "$tmpdir/validation.json" >/dev/null
done
# Generate otherwise valid artifacts and attach the retained non-executable graph.
# Outer status laundering cannot erase the graph at any independent consumer.
printf 'program scalar\nmain { return 0 }\n' > "$tmpdir/scalar.flow"
"$FLOWMINI_BIN" --dump-frontend-bundle "$tmpdir/scalar.flow" > "$tmpdir/scalar.frontend.json"
"$FLOWANALYST_BIN" --lowering-plan-version 2 < "$tmpdir/scalar.frontend.json" > "$tmpdir/scalar.semantic.json"
"$FLOWPARALLEL_BIN" < "$tmpdir/scalar.semantic.json" > "$tmpdir/scalar.execution.json"
"$FLOWOPTIMIZE_BIN" < "$tmpdir/scalar.execution.json" > "$tmpdir/scalar.optimization.json"
for artifact in semantic execution optimization; do
    jq --slurpfile graph "$tmpdir/graph.json" '.lowering_plan.source_graph = $graph[0]' "$tmpdir/scalar.$artifact.json" > "$tmpdir/forged.$artifact.json"
done
refuse() {
    input=$1
    shift
    if "$@" < "$input" > "$tmpdir/result" 2> "$tmpdir/error"; then
        echo "source graph was silently dropped by $*" >&2; exit 1
    fi
    cat "$tmpdir/result" "$tmpdir/error" | grep -Fq 'source graph execution is not admitted'
}
refuse "$tmpdir/forged.semantic.json" "$FLOWPARALLEL_BIN"
refuse "$tmpdir/forged.semantic.json" "$FLOWBIND_BIN"
refuse "$tmpdir/forged.semantic.json" "$FLOWOPTIMIZE_BIN"
refuse "$tmpdir/forged.execution.json" "$FLOWOPTIMIZE_BIN"
refuse "$tmpdir/forged.optimization.json" "$FLOWLOWER_BIN"
refuse "$tmpdir/forged.optimization.json" "$FLOWPREPARE_BIN"
refuse "$tmpdir/forged.optimization.json" "$FLOWLOWER_BIN" --emit-llvm "$tmpdir/forged.ll"
test ! -e "$tmpdir/forged.ll"
"$FLOWPREPARE_BIN" < "$tmpdir/scalar.optimization.json" > "$tmpdir/backend.json"
jq --slurpfile graph "$tmpdir/graph.json" '.lowering_plan.source_graph = $graph[0]' "$tmpdir/backend.json" > "$tmpdir/forged.backend.json"
refuse "$tmpdir/forged.backend.json" "$FLOWLOWER_BIN"
refuse "$tmpdir/forged.backend.json" "$FLOWVALIDATE_BIN"
echo 'Source graph artifact: PASS'
# An explicit map resolves arbitrary provider names to external source identity.
cat > "$tmpdir/providers.json" <<'JSON'
{"format":"flowcore.graph_provider_map","version":1,"providers":[{"implementation":"injected.renamed","source_callable":"host.selected","activation":"startup_once","output_port":"out"}]}
JSON
cat > "$tmpdir/producer.flow" <<'FLOW'
program arbitrarily_selected_producer
abi host {
    library "libc.so.6"
    convention c
    extern fn selected(): c_int {
        symbol "getpid"
        effect readonly
    }
}
producer source : injected.renamed
node receiver : fn identity
wire source.out => receiver.in
fn identity(value : c_int): c_int { return value }
main { return 0 }
FLOW
"$FLOWVALIDATE_BIN" "$tmpdir/providers.json" | jq -e '.classification == "valid"' >/dev/null
"$FLOWMINI_BIN" --dump-frontend-bundle "$tmpdir/producer.flow" > "$tmpdir/producer.frontend.json"
if "$FLOWANALYST_BIN" --lowering-plan-version 2 --graph-providers "$tmpdir/providers.json" < "$tmpdir/producer.frontend.json" > "$tmpdir/producer.semantic.json"; then
    echo 'producer selection became execution authority' >&2; exit 1
fi
jq '.lowering_plan.source_graph' "$tmpdir/producer.semantic.json" > "$tmpdir/producer.graph.json"
jq -e '.providers[0] | .node_id == "source" and .source_callable == "host.selected" and .output_type == "c_int" and .provider.symbol == "getpid" and .function_symbol_id >= 0' "$tmpdir/producer.graph.json" >/dev/null
"$FLOWVALIDATE_BIN" "$tmpdir/producer.graph.json" | jq -e '.classification == "valid"' >/dev/null
for mutation in '.version = 3' '.providers += [.providers[0]]' '.providers[0].activation = "stream"' '.providers[0].source_callable = ""'; do
    jq "$mutation" "$tmpdir/providers.json" > "$tmpdir/bad-providers.json"
    if "$FLOWVALIDATE_BIN" "$tmpdir/bad-providers.json" >/dev/null; then
        echo "invalid provider map accepted: $mutation" >&2; exit 1
    fi
done
cat > "$tmpdir/stream-providers.json" <<'JSON'
{"format":"flowcore.graph_provider_map","version":2,"providers":[{"implementation":"injected.stream","count_callable":"host.count","item_callable":"host.item","activation":"finite_stream_once","max_items":4096,"output_port":"out"}]}
JSON
"$FLOWVALIDATE_BIN" "$tmpdir/stream-providers.json" | jq -e '.classification == "valid"' >/dev/null
for mutation in \
    '.providers[0].count_callable = ""' \
    '.providers[0].item_callable = ""' \
    '.providers[0].max_items = 0' \
    '.providers[0].max_items = 4097' \
    '.providers[0].activation = "startup_once"'; do
    jq "$mutation" "$tmpdir/stream-providers.json" > "$tmpdir/bad-stream-providers.json"
    if "$FLOWVALIDATE_BIN" "$tmpdir/bad-stream-providers.json" >/dev/null; then
        echo "invalid stream provider map accepted: $mutation" >&2; exit 1
    fi
done
cat > "$tmpdir/stream-selection.json" <<'JSON'
{"format":"flowcore.graph_provider_map","version":2,"providers":[{"implementation":"injected.renamed","count_callable":"host.count","item_callable":"host.item","activation":"finite_stream_once","max_items":4096,"output_port":"out"}]}
JSON
jq --slurpfile selection "$tmpdir/stream-selection.json" '
    .provider_selection = $selection[0]
    | .providers[0] |= (
        del(.source_callable)
        | .activation = "finite_stream_once"
        | .count_callable = "host.count"
        | .item_callable = "host.item"
        | .max_items = 4096
        | .count_function_symbol_id = (.function_symbol_id + 1)
        | .provider.parameter_types = "c_size_t"
        | .count_provider = (.provider | .parameter_types = "" | .return_type = "c_size_t")
    )
' "$tmpdir/producer.graph.json" > "$tmpdir/stream.graph.json"
"$FLOWVALIDATE_BIN" "$tmpdir/stream.graph.json" | jq -e '.classification == "valid"' >/dev/null
for mutation in \
    '.providers[0].count_callable = "host.other"' \
    '.providers[0].max_items = 0' \
    '.providers[0].provider.parameter_types = ""' \
    '.providers[0].count_provider.return_type = "c_int"' \
    '.providers[0].count_function_symbol_id = -1'; do
    jq "$mutation" "$tmpdir/stream.graph.json" > "$tmpdir/bad-stream.graph.json"
    if "$FLOWVALIDATE_BIN" "$tmpdir/bad-stream.graph.json" >/dev/null; then
        echo "invalid stream graph accepted: $mutation" >&2; exit 1
    fi
done
for mutation in '.providers[0].node_id = "receiver"' '.providers[0].implementation = "other"' '.providers[0].provider.parameter_types = "c_int"' '.providers[0].output_type = "Bool"' '.providers[0].provider.evidence = "invented"' '.syntax.wires[0].from.port_id = "wrong"'; do
    jq "$mutation" "$tmpdir/producer.graph.json" > "$tmpdir/bad-producer.graph.json"
    if "$FLOWVALIDATE_BIN" "$tmpdir/bad-producer.graph.json" >/dev/null; then
        echo "invalid producer resolution accepted: $mutation" >&2; exit 1
    fi
done

echo 'source graph artifact and provider selection: PASS'
