#!/bin/sh
set -eu

root=${FLOWCORE_ROOT:?FLOWCORE_ROOT is required}
flowmini=${FLOWMINI_BIN:?FLOWMINI_BIN is required}
analyst=${FLOWANALYST_BIN:?FLOWANALYST_BIN is required}
parallel=${FLOWPARALLEL_BIN:?FLOWPARALLEL_BIN is required}
optimizer=${FLOWOPTIMIZE_BIN:?FLOWOPTIMIZE_BIN is required}
bind=${FLOWBIND_BIN:?FLOWBIND_BIN is required}
lower=${FLOWLOWER_BIN:?FLOWLOWER_BIN is required}
source=$root/Lyraform/compiler/examples/pass/text_value.flow
tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT

"$flowmini" --dump-frontend-bundle "$source" > "$tmpdir/frontend.json"
"$analyst" --lowering-plan-version 2 < "$tmpdir/frontend.json" > "$tmpdir/semantic.json"
jq -e '
  .status == "ok" and
  ([.lowering_plan.operations[] | select(.kind == "value_definition" and .operands[0].type == "Text")] | length) == 6 and
  ([.lowering_plan.operations[] | select(.kind == "external_call" and .callee == "print" and .provider.parameter_types == "Text")] | length) == 3 and
  ([.lowering_plan.operations[] | select(.kind == "value_definition" and .operands[0].kind == "call_result" and .operands[0].type == "Text")] | length) == 1 and
  ([.lowering_plan.operations[].operands[] | select(.kind == "string_literal" and .type == "Text" and .value == "Lyraform — Igor")] | length) == 1
' "$tmpdir/semantic.json" >/dev/null
"$parallel" < "$tmpdir/semantic.json" > "$tmpdir/parallel.json"
"$optimizer" < "$tmpdir/parallel.json" > "$tmpdir/optimized.json"
jq -e '
  .lowering_plan.operations | all(.[]; .id >= 0) and
  any(.[]; .kind == "external_call" and .provider.parameter_types == "Text")
' "$tmpdir/optimized.json" >/dev/null
printf '%s\n' 'allow libc.so.6 puts c io Text c_int' > "$tmpdir/policy"
"$bind" --policy "$tmpdir/policy" < "$tmpdir/semantic.json" > "$tmpdir/binding.json"
jq -e '.status == "ready" and .capabilities[0].parameter_types == "Text" and .capabilities[0].status == "authorized"' "$tmpdir/binding.json" >/dev/null
"$lower" --emit-llvm "$tmpdir/text.ll" --binding-report "$tmpdir/binding.json" < "$tmpdir/optimized.json" > "$tmpdir/lowering.json"
grep -Fq 'declare i32 @puts(ptr)' "$tmpdir/text.ll"
grep -Fq 'call i32 @puts(ptr' "$tmpdir/text.ll"
clang "$tmpdir/text.ll" -o "$tmpdir/text"
printf '\nLyraform — Igor\nreturned — Igor\n' > "$tmpdir/expected"
"$tmpdir/text" > "$tmpdir/output"
cmp -s "$tmpdir/expected" "$tmpdir/output"

bad=$root/Lyraform/compiler/examples/fail/bad_text_cstring.flow
set +e
"$flowmini" --dump-frontend-bundle "$bad" | "$analyst" > "$tmpdir/bad-cstring.json"
bad_status=$?
set -e
test "$bad_status" -eq 2
jq -e '.status == "error" and any(.diagnostics[]; .code == "FLOWANALYST_TEXT_CSTRING_CONFUSION")' "$tmpdir/bad-cstring.json" >/dev/null

dynamic=$root/Lyraform/compiler/examples/fail/bad_text_dynamic_concat.flow
set +e
"$flowmini" --dump-frontend-bundle "$dynamic" | "$analyst" > "$tmpdir/bad-dynamic.json"
dynamic_status=$?
set -e
test "$dynamic_status" -eq 2
jq -e '.status == "error" and any(.diagnostics[]; .code == "FLOWANALYST_TEXT_DYNAMIC_CONCAT")' "$tmpdir/bad-dynamic.json" >/dev/null

printf 'program bad_text_encoding\n\nmain {\n    value : Text("bad-' > "$tmpdir/bad-encoding.flow"
printf '\377' >> "$tmpdir/bad-encoding.flow"
printf '%s\n' '")' '}' >> "$tmpdir/bad-encoding.flow"
set +e
"$flowmini" --diagnostics json --dump-frontend-bundle "$tmpdir/bad-encoding.flow" > "$tmpdir/bad-encoding.stdout.json" 2> "$tmpdir/bad-encoding.json"
encoding_status=$?
set -e
test "$encoding_status" -eq 1
test ! -s "$tmpdir/bad-encoding.stdout.json"
jq -e '.status == "failed" and .code == "FLOW_DIAGNOSTIC_ERROR" and .stage == "source" and (.message | contains("invalid UTF-8 at byte 57")) and .disposition == "no_artifact"' "$tmpdir/bad-encoding.json" >/dev/null

printf '%s\n' 'Text value boundary: PASS (semantic, authorized native lowering, output, and refusals)'
