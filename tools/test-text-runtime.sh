#!/bin/sh
set -eu

root=${FLOWCORE_ROOT:?}
flowmini=${FLOWMINI_BIN:?}
analyst=${FLOWANALYST_BIN:?}
parallel=${FLOWPARALLEL_BIN:?}
optimize=${FLOWOPTIMIZE_BIN:?}
bind=${FLOWBIND_BIN:?}
lower=${FLOWLOWER_BIN:?}
validate=${FLOWVALIDATE_BIN:?}
text_runtime=${FLOWTEXT_RUNTIME:?}

tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT
policy=$tmpdir/policy
printf '%s\n' \
  'allow libflowtext.so flow_text_concat c memory Text,Text Text' \
  'allow libc.so.6 puts c io Text c_int' > "$policy"
export LD_LIBRARY_PATH=$(dirname "$text_runtime")${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}

source=$root/Lyraform/compiler/examples/text/text_runtime.flow
"$flowmini" --dump-frontend-bundle "$source" > "$tmpdir/frontend.json"
"$analyst" --lowering-plan-version 2 < "$tmpdir/frontend.json" > "$tmpdir/semantic.json"
jq -e '
  .status == "ok" and
  any(.lowering_plan.operations[]; .kind == "text_outcome" and .provider.symbol == "flow_text_concat" and .provider.parameter_types == "Text,Text" and .provider.return_type == "Text") and
  any(.lowering_plan.operations[]; .kind == "text_outcome" and .provider.symbol == "flow_text_concat" and .result_outcome.type == "Outcome" and .result_outcome.representation == "tagged" and .result_outcome.failure_type == "TextFailure" and (.result_outcome.failure_codes | index("exhausted"))) and
  any(.lowering_plan.operations[]; .kind == "external_call" and .provider.symbol == "puts" and .provider.parameter_types == "Text")
' "$tmpdir/semantic.json" >/dev/null
"$validate" "$tmpdir/semantic.json" | jq -e '.classification == "valid"' >/dev/null
jq '.lowering_plan.operations = [.lowering_plan.operations[] | if .kind == "text_outcome" then del(.result_outcome) else . end]' \
  "$tmpdir/semantic.json" > "$tmpdir/missing-outcome.json"
if "$validate" "$tmpdir/missing-outcome.json" >/dev/null 2>&1; then
  echo 'flowvalidate accepted a text_outcome operation without its typed outcome' >&2
  exit 1
fi
"$parallel" < "$tmpdir/semantic.json" > "$tmpdir/parallel.json"
"$optimize" < "$tmpdir/parallel.json" > "$tmpdir/optimized.json"
"$bind" --policy "$policy" < "$tmpdir/semantic.json" > "$tmpdir/binding.json"
"$lower" --emit-llvm "$tmpdir/text.ll" --binding-report "$tmpdir/binding.json" < "$tmpdir/optimized.json" > "$tmpdir/lowering.json"
clang ${FLOWTEXT_CXX_FLAGS:-} "$tmpdir/text.ll" "$text_runtime" -o "$tmpdir/text"
printf '%s\n' 'Lyraform' 'Lyraform — Igor' 'Lyraform / tester' 'Lyraform — Igor' > "$tmpdir/expected"
"$tmpdir/text" > "$tmpdir/output"
cmp -s "$tmpdir/expected" "$tmpdir/output"

printf 'import "%s/Lyraform/compiler/std/abi/libc.flow"\nimport "%s/Lyraform/compiler/std/abi/text.flow"\n\nprogram text_runtime_overflow\n\nfn join(left : Text, right : Text): Text {\n    value : Text(left + right)\n    return value\n}\n\nmain {\n    value : Text(join("' "$root" "$root" > "$tmpdir/overflow.flow"
awk 'BEGIN { for (i = 0; i < 4097; ++i) printf "A" }' >> "$tmpdir/overflow.flow"
printf '%s\n' '", "x"))' '    print value' '}' >> "$tmpdir/overflow.flow"
"$flowmini" --dump-frontend-bundle "$tmpdir/overflow.flow" > "$tmpdir/overflow.frontend.json"
"$analyst" --lowering-plan-version 2 < "$tmpdir/overflow.frontend.json" > "$tmpdir/overflow.semantic.json"
"$parallel" < "$tmpdir/overflow.semantic.json" > "$tmpdir/overflow.parallel.json"
"$optimize" < "$tmpdir/overflow.parallel.json" > "$tmpdir/overflow.optimized.json"
"$bind" --policy "$policy" < "$tmpdir/overflow.semantic.json" > "$tmpdir/overflow.binding.json"
set +e
"$lower" --emit-llvm "$tmpdir/overflow.ll" --binding-report "$tmpdir/overflow.binding.json" < "$tmpdir/overflow.optimized.json" >/dev/null
clang ${FLOWTEXT_CXX_FLAGS:-} "$tmpdir/overflow.ll" "$text_runtime" -o "$tmpdir/overflow"
"$tmpdir/overflow" >/dev/null 2>&1
overflow_status=$?
set -e
test "$overflow_status" -ne 0

echo 'Runtime Text boundary: PASS (owned bounded concat, exact authorization, and exhaustion trap)'
