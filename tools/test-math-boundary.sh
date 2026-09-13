#!/bin/sh
set -eu

root=${FLOWCORE_ROOT:?}
flowmini=${FLOWMINI_BIN:?}
analyst=${FLOWANALYST_BIN:?}
parallel=${FLOWPARALLEL_BIN:?}
optimize=${FLOWOPTIMIZE_BIN:?}
bind=${FLOWBIND_BIN:?}
lower=${FLOWLOWER_BIN:?}

tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT
policy=$tmpdir/policy
printf '%s\n' \
  'allow libm.so.6 sqrt c pure c_double c_double' \
  'allow libm.so.6 floor c pure c_double c_double' \
  'allow libc.so.6 puts c io c_string c_int' > "$policy"

fixture=$root/Lyraform/compiler/examples/pass/abi_math_main.flow
"$flowmini" --dump-frontend-bundle "$fixture" > "$tmpdir/frontend.json"
"$analyst" --lowering-plan-version 2 < "$tmpdir/frontend.json" > "$tmpdir/semantic.json"
jq -e '
  .status == "ok" and
  any(.binding_requirements[]; .library == "libm.so.6" and .symbol == "sqrt" and .parameter_types == "c_double" and .return_type == "c_double") and
  any(.binding_requirements[]; .library == "libm.so.6" and .symbol == "floor" and .parameter_types == "c_double" and .return_type == "c_double") and
  any(.lowering_plan.operations[]; .kind == "external_call" and .provider.symbol == "sqrt" and .provider.return_type == "c_double") and
  any(.lowering_plan.operations[]; .kind == "external_call" and .provider.symbol == "floor" and .provider.return_type == "c_double") and
  any(.lowering_plan.operations[].operands[]?; .kind == "float_literal" and .type == "c_double" and .value == "9.0")
' "$tmpdir/semantic.json" >/dev/null
"$parallel" < "$tmpdir/semantic.json" > "$tmpdir/parallel.json"
"$optimize" < "$tmpdir/parallel.json" > "$tmpdir/optimized.json"
LD_LIBRARY_PATH= "$bind" --policy "$policy" < "$tmpdir/semantic.json" > "$tmpdir/binding.json"
"$lower" --emit-llvm "$tmpdir/math.ll" --binding-report "$tmpdir/binding.json" < "$tmpdir/optimized.json" >/dev/null
grep -q 'call double @sqrt' "$tmpdir/math.ll"
grep -q 'call double @floor' "$tmpdir/math.ll"
grep -q 'fcmp oeq double' "$tmpdir/math.ll"
clang "$tmpdir/math.ll" -lm -o "$tmpdir/math"
"$tmpdir/math" > "$tmpdir/output"
printf '%s\n' 'libm ok' 'libm floor ok' > "$tmpdir/expected"
cmp -s "$tmpdir/expected" "$tmpdir/output"

echo 'libm Flow boundary: PASS (exact c_double authorization, typed LLVM call, and source-derived comparison)'
