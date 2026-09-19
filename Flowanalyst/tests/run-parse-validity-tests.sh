#!/bin/sh
set -eu
tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT
source_file() { printf 'program validity_probe\nmain {\n%s\n}\n' "$1" > "$tmpdir/program.flow"; }
export_source() {
    "$FLOWMINI_BIN" --dump-frontend-bundle "$tmpdir/program.flow" > "$tmpdir/frontend.json"
    "$FLOWMINI_BIN" --dump-frontend-bundle "$tmpdir/program.flow" > "$tmpdir/repeated.json"
    cmp "$tmpdir/frontend.json" "$tmpdir/repeated.json"
}
refuse() {
    source_file "$1"
    export_source
    jq -e '.parse_validity.state | IN("invalid","incomplete","recovered")' "$tmpdir/frontend.json" >/dev/null
    set +e
    "$FLOWANALYST_BIN" "$tmpdir/frontend.json" > "$tmpdir/refused.json"
    status=$?
    set -e
    test "$status" -eq 2
    jq -e '.status=="error" and .lowering_plan.status!="ready" and any(.diagnostics[]; .code=="FLOWMINI_PARSE_NOT_EXECUTABLE")' "$tmpdir/refused.json" >/dev/null
    if "$FLOWMINI_TEST_BIN" --test-forbid-legacy --trace true "$tmpdir/program.flow" > "$tmpdir/direct.out" 2> "$tmpdir/direct.err"; then exit 1; fi
    grep -q 'FLOWMINI_PARSE_NOT_EXECUTABLE' "$tmpdir/direct.err"
    if grep -qE 'source-path: (runtime-adapter|legacy-compatibility)' "$tmpdir/direct.err"; then exit 1; fi
    for consumer in "$FLOWBIND_BIN" "$FLOWPARALLEL_BIN" "$FLOWOPTIMIZE_BIN"; do
        if "$consumer" "$tmpdir/refused.json" > /dev/null 2>&1; then exit 1; fi
    done
    jq '.status="ok" | .lowering_plan.status="ready"' "$tmpdir/refused.json" > "$tmpdir/forged.json"
    if "$FLOWPARALLEL_BIN" "$tmpdir/forged.json" >/dev/null 2>&1; then exit 1; fi
}
refuse 'x : int(0)
20 -> x : Bool'
refuse 'x : int(0)
20 -> x : Bool
x -> return'
refuse 'x : int(20) junk 99
print x'
refuse 'x : int(20 99)
print x'
refuse 'x : int(0)
20 -> x junk'
refuse 'x : int(20)
unknown 99
print x'
refuse 'x : int(20)
{ true -> x }
print x'
refuse 'x : int((20)'
refuse 'x : int(20))'
refuse 'x : int(20)
print x junk'
refuse 'x : int(20 +)'
refuse 'if true'

for body in 'x : int(20)' 'flag : Bool(false)' 'a : int(20)
b : int(a)' 'x : int(0)
20 -> x
30 -> x' 'x : int((2 + 3) * 4)'; do
    source_file "$body"
    export_source
    jq -e '.parse_validity.state=="canonical_valid" and .parse_validity.coverage=="complete"' "$tmpdir/frontend.json" >/dev/null
    "$FLOWANALYST_BIN" "$tmpdir/frontend.json" > "$tmpdir/valid.json"
    "$FLOWANALYST_BIN" "$tmpdir/frontend.json" > "$tmpdir/again.json"
    cmp "$tmpdir/valid.json" "$tmpdir/again.json"
    "$FLOWMINI_TEST_BIN" --test-forbid-legacy "$tmpdir/program.flow"
done
# Empty main retains staged compatibility, not canonical completeness proof.
source_file ''
export_source
jq -e '.parse_validity.state=="outside_scope" and .parse_validity.scope=="compatibility" and .parse_validity.coverage=="unassessed"' "$tmpdir/frontend.json" >/dev/null
"$FLOWANALYST_BIN" "$tmpdir/frontend.json" > "$tmpdir/empty-report.json"
jq -e '.status=="ok"' "$tmpdir/empty-report.json" >/dev/null
if "$FLOWMINI_TEST_BIN" --test-forbid-legacy "$tmpdir/program.flow" > /dev/null 2> "$tmpdir/empty.err"; then exit 1; fi
grep -q 'lacks parser validity' "$tmpdir/empty.err"
# Make a valid prepared artifact for downstream hostile-evidence tests.
source_file 'x : int(20)
x -> return'
export_source
"$FLOWANALYST_BIN" "$tmpdir/frontend.json" > "$tmpdir/valid.json"
"$FLOWPARALLEL_BIN" "$tmpdir/valid.json" > "$tmpdir/execution.json"
"$FLOWOPTIMIZE_BIN" "$tmpdir/execution.json" > "$tmpdir/optimization.json"
"$FLOWPREPARE_BIN" "$tmpdir/optimization.json" > "$tmpdir/lowering.json"
for stage in valid execution optimization lowering; do
    jq -S '.lowering_plan.parse_validity' "$tmpdir/$stage.json" > "$tmpdir/evidence-$stage"
    cmp "$tmpdir/evidence-valid" "$tmpdir/evidence-$stage"
done
# Historical imports remain admissible, without asserting completeness.
jq 'del(.parse_validity)' "$tmpdir/frontend.json" > "$tmpdir/historical.json"
"$FLOWANALYST_BIN" "$tmpdir/historical.json" > "$tmpdir/historical-report.json"
jq -e '.status=="ok" and (.lowering_plan | has("parse_validity") | not)' "$tmpdir/historical-report.json" >/dev/null
for mutation in \
 '.parse_validity.version=99' \
 '.parse_validity.state="invented"' \
 '.parse_validity.state="canonical_valid" | .parse_validity.scope="canonical_scalar" | .parse_validity.coverage="incomplete"' \
 '.parse_validity.recovery_used=true' \
 '.parse_validity.message="unconsumed suffix"' \
 '.parse_validity.coverage="unassessed"' \
 '.parse_validity.extra="conflict"' \
 '.ast.statement_pool[0].kind="unknown"'; do
    jq "$mutation" "$tmpdir/frontend.json" > "$tmpdir/hostile.json"
    if "$FLOWANALYST_BIN" "$tmpdir/hostile.json" > "$tmpdir/hostile-report.json" 2>/dev/null; then exit 1; fi
done
# Duplicate JSON fields must not select a winning interpretation.
sed 's/"parse_validity": {/"parse_validity": {}, "parse_validity": {/' "$tmpdir/frontend.json" > "$tmpdir/duplicate.json"
if "$FLOWANALYST_BIN" "$tmpdir/duplicate.json" >/dev/null 2>&1; then exit 1; fi
for mutation in \
 '.lowering_plan.parse_validity.version=99' \
 '.lowering_plan.parse_validity.state="recovered" | .lowering_plan.parse_validity.coverage="unassessed" | .lowering_plan.parse_validity.recovery_used=true | .lowering_plan.parse_validity.message="recovery"' \
 '.lowering_plan.parse_validity.state="invalid" | .lowering_plan.parse_validity.coverage="incomplete" | .lowering_plan.parse_validity.message="suffix"'; do
    jq "$mutation" "$tmpdir/lowering.json" > "$tmpdir/hostile-plan.json"
    if "$FLOWVALIDATE_BIN" "$tmpdir/hostile-plan.json" >/dev/null 2>&1; then exit 1; fi
    if "$FLOWLOWER_BIN" --emit-llvm "$tmpdir/forbidden.ll" "$tmpdir/hostile-plan.json" >/dev/null 2>&1; then exit 1; fi
    if "$FLOWTINYLOWER_BIN" "$tmpdir/hostile-plan.json" "$tmpdir/forbidden.tvm" >/dev/null 2>&1; then exit 1; fi
    test ! -e "$tmpdir/forbidden.ll"
    test ! -e "$tmpdir/forbidden.tvm"
done
echo 'Parse validity: PASS (12 refusals; 5 positives; deterministic exports/analysis; 9 hostile frontend inputs; 3 hostile backend artifacts; unproven historical import)'
