#!/bin/sh
set -eu
build=$1
scratch=$(mktemp -d /tmp/lyraform-record-admission.XXXXXX)
trap 'rm -rf "$scratch"' EXIT
for literal in '{left: left_value, right: right_value}' '{right: right_value, left: left_value}' '{left: left_value}' '{left: left_value, right: right_value, spare: 9}' '{left: "bad", right: right_value}'; do
    cat > "$scratch/input.flow" <<SOURCE
program ordinary_value_admission
record Sample { left : int32
right : int32
}
main {
    left_value : int32(7)
    right_value : int32(30)
    item : Sample($literal)
    0 -> return
}
SOURCE
    "$build/flowmini/flowmini" --dump-frontend-bundle "$scratch/input.flow" > "$scratch/frontend.json"
    set +e
    "$build/flowanalyst/flowanalyst" --lowering-plan-version 2 < "$scratch/frontend.json" > "$scratch/result.json"
    result=$?
    set -e
    test "$result" -eq 2
    set +e
    "$build/flowanalyst/flowanalyst" --lowering-plan-version 2 < "$scratch/frontend.json" > "$scratch/repeated.json"
    repeated=$?
    set -e
    test "$repeated" -eq 2
    cmp "$scratch/result.json" "$scratch/repeated.json"
    jq -e '.status == "error" and .lowering_plan.status == "blocked" and
        .lowering_plan.source_operation_coverage.status == "refused" and
        .lowering_plan.source_operation_coverage.refused_count > 0 and
        any(.diagnostics[]; .code == "FLOWANALYST_SOURCE_OPERATION_GAP" and .provenance.line > 0)' "$scratch/result.json" > /dev/null
    # Declarative target inspection retains its narrower non-execution role.
    set +e
    "$build/flowanalyst/flowanalyst" < "$scratch/frontend.json" > "$scratch/declarative.json"
    declarative=$?
    set -e
    case "$literal" in
        '{left: left_value}'|'{left: left_value, right: right_value, spare: 9}'|'{left: "bad", right: right_value}') test "$declarative" -eq 2 ;;
        *) test "$declarative" -eq 0 ;;
    esac
done
cat > "$scratch/scalar.flow" <<'SOURCE'
program unrelated_scalar
main {
    value : int32(7)
    result : c_int(value)
    result -> return
}
SOURCE
"$build/flowmini/flowmini" --dump-frontend-bundle "$scratch/scalar.flow" > "$scratch/scalar.json"
"$build/flowanalyst/flowanalyst" --lowering-plan-version 2 < "$scratch/scalar.json" > "$scratch/scalar-result.json"
jq -e '.status == "ok" and .lowering_plan.status == "ready" and .lowering_plan.source_operation_coverage.status == "complete"' "$scratch/scalar-result.json" > /dev/null

# The admitted finite scalar/nested bound must cross the public pipeline and
# produce a real binary. This target wrapper is ordinary source syntax; no
# record-specific compiler or runtime entry point is used.
cat > "$scratch/public.flow" <<'SOURCE'
program ordinary_record_public
record Leaf { count : int32
enabled : Bool
}
record Pair { left : Leaf
right : Leaf
}
target Demo {
main {
 left_value : int32(2 + 3)
 right_value : int32(7)
 nested_enabled : Bool(true)
 item : Pair({right: {enabled: true, count: right_value}, left: {count: left_value, enabled: nested_enabled}})
 copied : Pair(item)
 selected : Leaf(copied.left)
 number : int32(selected.count)
 other : int32(copied.right.count)
 enabled : Bool(item.right.enabled)
 total : int32(12)
 if enabled {
  total -> return
 } else {
  73 -> return
 }
}
}
SOURCE
"$build/flowmini/flowmini" --dump-frontend-bundle "$scratch/public.flow" > "$scratch/public-frontend.json"
"$build/flowanalyst/flowanalyst" --lowering-plan-version 2 < "$scratch/public-frontend.json" > "$scratch/public-semantic.json"
jq -e '.status == "ok" and .lowering_plan.status == "ready" and
    .lowering_plan.ordinary_record_execution.status == "ready" and
    .lowering_plan.source_operation_coverage.status == "complete"' \
    "$scratch/public-semantic.json" > /dev/null
"$build/flowtools/flowparallel/flowparallel" < "$scratch/public-semantic.json" > "$scratch/public-parallel.json"
"$build/flowoptimize/flowoptimize" < "$scratch/public-parallel.json" > "$scratch/public-optimized.json"
"$build/flowcontracts/flowvalidate" < "$scratch/public-optimized.json" > "$scratch/public-valid.json"
jq -e '.classification == "valid"' "$scratch/public-valid.json" > /dev/null
for mutation in missing-contract wrong-bound blocked-plan; do
    case "$mutation" in
        missing-contract) jq 'del(.lowering_plan.ordinary_record_execution)' "$scratch/public-optimized.json" > "$scratch/hostile.json" ;;
        wrong-bound) jq '.lowering_plan.ordinary_record_execution.bound = "recursive"' "$scratch/public-optimized.json" > "$scratch/hostile.json" ;;
        blocked-plan) jq '.lowering_plan.status = "blocked"' "$scratch/public-optimized.json" > "$scratch/hostile.json" ;;
    esac
    set +e
    "$build/flowcontracts/flowvalidate" < "$scratch/hostile.json" > "$scratch/hostile-result.json"
    hostile_result=$?
    set -e
    test "$hostile_result" -ne 0
    jq -e '.classification == "invalid"' "$scratch/hostile-result.json" > /dev/null
done
"$build/flowlower/flowlower" --target Demo --emit-llvm "$scratch/public.ll" \
    < "$scratch/public-optimized.json" > "$scratch/public-lowered.json"
jq -e '.status == "ready" and .artifact.status == "emitted"' "$scratch/public-lowered.json" > /dev/null
clang-18 "$scratch/public.ll" -o "$scratch/public-bin"
set +e
"$scratch/public-bin"
result=$?
set -e
test "$result" -eq 12
"$build/flowlower/flowprepare" --target Demo "$scratch/public-optimized.json" > "$scratch/public-backend.json"
"$build/tinyvm/flowtinylower" "$scratch/public-backend.json" "$scratch/public.tvm"
"$build/tinyvm/flowtinyrun" "$scratch/public.tvm" > "$scratch/public-vm-result.json"
jq -e '.status == "completed" and .result == 12 and .trap == 0' \
    "$scratch/public-vm-result.json" > /dev/null

cat > "$scratch/function-boundary.flow" <<'SOURCE'
program ordinary_record_function_boundary
record Leaf { count : int32
enabled : Bool
}
record Pair { left : Leaf
right : Leaf
}
target Demo {
fn make(): Pair {
 left_value : int32(5)
 right_value : int32(7)
 item : Pair({left: {count: left_value, enabled: true}, right: {count: right_value, enabled: false}})
 item -> return
}
fn read(item : Pair): int32 {
 value : int32(item.left.count)
 value -> return
}
main {
 result : int32(read(make()))
 result -> return
}
}
SOURCE
"$build/flowmini/flowmini" --dump-frontend-bundle "$scratch/function-boundary.flow" > "$scratch/function-frontend.json"
"$build/flowanalyst/flowanalyst" --lowering-plan-version 2 < "$scratch/function-frontend.json" > "$scratch/function-semantic.json"
jq -e '.status == "ok" and .lowering_plan.status == "ready" and
    any(.lowering_plan.functions[]; .record_symbol_id == 5) and
    any(.lowering_plan.functions[].parameters[]?; .record_symbol_id == 5)' \
    "$scratch/function-semantic.json" > /dev/null
"$build/flowtools/flowparallel/flowparallel" < "$scratch/function-semantic.json" > "$scratch/function-parallel.json"
"$build/flowoptimize/flowoptimize" < "$scratch/function-parallel.json" > "$scratch/function-optimized.json"
"$build/flowlower/flowlower" --target Demo --emit-llvm "$scratch/function.ll" \
    < "$scratch/function-optimized.json" > "$scratch/function-lowered.json"
clang-18 "$scratch/function.ll" -o "$scratch/function-bin"
set +e
"$scratch/function-bin"
result=$?
set -e
test "$result" -eq 5
"$build/flowlower/flowprepare" --target Demo "$scratch/function-optimized.json" > "$scratch/function-backend.json"
"$build/tinyvm/flowtinylower" "$scratch/function-backend.json" "$scratch/function.tvm"
"$build/tinyvm/flowtinyrun" "$scratch/function.tvm" > "$scratch/function-vm-result.json"
jq -e '.status == "completed" and .result == 5 and .trap == 0' \
    "$scratch/function-vm-result.json" > /dev/null

python3 "$(dirname "$0")/run-record-shape-tests.py" "$build"

python3 "$(dirname "$0")/run-record-catalog-tests.py" "$build"

python3 "$(dirname "$0")/run-record-operands-tests.py" "$build"

python3 "$(dirname "$0")/run-record-projection-tests.py" "$build"

python3 "$(dirname "$0")/run-record-flow-tests.py" "$build"
