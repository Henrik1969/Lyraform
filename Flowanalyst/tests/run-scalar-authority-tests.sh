#!/bin/sh
set -eu
tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT

source_file() {
    printf 'program scalar_authority_probe\nmain {\n%s\n}\n' "$1" > "$tmpdir/program.flow"
}

refuse() {
    source_file "$1"
    "$FLOWMINI_BIN" --dump-frontend-bundle "$tmpdir/program.flow" > "$tmpdir/frontend.json"
    for version in 1 2; do
    set +e
    "$FLOWANALYST_BIN" --lowering-plan-version "$version" "$tmpdir/frontend.json" > "$tmpdir/refused.json"
    rc=$?
    set -e
    test "$rc" -eq 2
    jq -e '.status == "error" and .lowering_plan.status != "ready" and
        any(.diagnostics[]; .code == "FLOWANALYST_SCALAR_FLOW_REFUSED" and
            .provenance.line > 0 and .provenance.column > 0 and
            (.provenance.ast_path | startswith("/statement_pool/")))' "$tmpdir/refused.json" >/dev/null
    if "$FLOWMINI_ORACLE_BIN" --test-legacy-oracle "$tmpdir/program.flow" > "$tmpdir/legacy.out" 2>/dev/null; then
        echo 'legacy oracle unexpectedly admitted invalid scalar flow' >&2; exit 1
    fi
    for consumer in "$FLOWBIND_BIN" "$FLOWPARALLEL_BIN" "$FLOWOPTIMIZE_BIN"; do
        if "$consumer" "$tmpdir/refused.json" > "$tmpdir/blocked.json" 2>/dev/null; then
            echo 'stage admitted a refused semantic report' >&2; exit 1
        fi
    done
    # A forged outer status must not turn retained refused facts into proof.
    jq '.status="ok" | .lowering_plan.status="ready"' "$tmpdir/refused.json" > "$tmpdir/forged.json"
    if "$FLOWPARALLEL_BIN" "$tmpdir/forged.json" >/dev/null 2>&1; then
        echo 'planner admitted a refused scalar fact' >&2; exit 1
    fi
    done
}

refuse 'x : int(0)
true -> x'
refuse 'x : int(true)'
refuse 'flag : Bool(true)
x : int(flag)'
refuse 'flag : Bool(20)'
refuse 'flag : Bool(false)
20 -> flag'
refuse 'x : int(0)
flag : Bool(true)
flag -> x'
refuse 'x : int(0)
x < 2 -> x'
refuse 'x : int(0)
x + true -> x'
refuse 'x : int("text")'
refuse '20 -> absent'
refuse 'x : int(x)'
refuse '20 -> later
later : int(0)'

facts() {
    jq -S '[.lowering_plan.operations[] | .scalar_fact // empty]' "$1"
}

admit() {
    body=$1
    expected=$2
    expected_legacy_status=${3:-0}
    # The same body is observed through the retained direct runtime oracle.
    source_file "$body
print x"
    set +e
    "$FLOWMINI_ORACLE_BIN" --test-legacy-oracle "$tmpdir/program.flow" > "$tmpdir/legacy.out" 2> "$tmpdir/legacy.err"
    legacy_status=$?
    set -e
    test "$legacy_status" -eq "$expected_legacy_status"
    if test "$legacy_status" -eq 0; then
        test "$(cat "$tmpdir/legacy.out")" = "$expected"
    else
        # Existing compatibility lowerExprToPath(identifier) returns the source
        # path; declaration callers ignore that return instead of copying.
        grep -q 'missing record path:' "$tmpdir/legacy.err"
    fi
    source_file "$body
x -> return"
    "$FLOWMINI_BIN" --dump-frontend-bundle "$tmpdir/program.flow" > "$tmpdir/frontend.json"
    for version in 1 2; do
        "$FLOWANALYST_BIN" --lowering-plan-version "$version" "$tmpdir/frontend.json" > "$tmpdir/semantic.json"
        "$FLOWANALYST_BIN" --lowering-plan-version "$version" "$tmpdir/frontend.json" > "$tmpdir/repeat.json"
        cmp "$tmpdir/semantic.json" "$tmpdir/repeat.json"
        jq -e '.status == "ok" and any(.lowering_plan.operations[]; .scalar_fact.compatibility == "admitted")' "$tmpdir/semantic.json" >/dev/null
        facts "$tmpdir/semantic.json" > "$tmpdir/expected-facts.json"
        "$FLOWBIND_BIN" "$tmpdir/semantic.json" > "$tmpdir/binding.json"
        "$FLOWPARALLEL_BIN" "$tmpdir/semantic.json" > "$tmpdir/execution.json"
        "$FLOWOPTIMIZE_BIN" "$tmpdir/execution.json" > "$tmpdir/optimization.json"
        "$FLOWPREPARE_BIN" "$tmpdir/optimization.json" > "$tmpdir/lowering.json"
        for stage in semantic execution optimization lowering; do
            "$FLOWVALIDATE_BIN" --canonical "$tmpdir/$stage.json" > "$tmpdir/canonical.json"
            facts "$tmpdir/canonical.json" > "$tmpdir/actual-facts.json"
            cmp "$tmpdir/expected-facts.json" "$tmpdir/actual-facts.json"
        done
        "$FLOWLOWER_BIN" --emit-llvm "$tmpdir/program.ll" "$tmpdir/lowering.json" > "$tmpdir/llvm.json"
        clang "$tmpdir/program.ll" -o "$tmpdir/program"
        set +e
        "$tmpdir/program"
        rc=$?
        set -e
        test "$rc" -eq "$expected"
        "$FLOWTINYLOWER_BIN" "$tmpdir/lowering.json" "$tmpdir/program.tvm" > "$tmpdir/tiny.json"
        "$FLOWTINYRUN_BIN" "$tmpdir/program.tvm" | jq -e --argjson expected "$expected" '.result == $expected' >/dev/null
    done
}

admit 'x : int(20)
30 -> x' 30
admit 'x : int(0)
flag : Bool(false)
true -> flag
if flag { 31 -> x }' 31
admit 'x : int(1)
y : int(2)
flag : Bool(x < y)
flag == false -> flag
if flag { 10 -> x }
x + y -> x' 3
admit 'seed : int(24)
x : int(seed)
source_flag : Bool(true)
flag : Bool(source_flag)
if flag { 25 -> x }' 25 1

# Coordinates belong to the originating file, not just the expanded program.
jq '.source_map.files[0].path="imported-scalar.flow" | .source_map.lines[].source_line += 100' "$tmpdir/frontend.json" > "$tmpdir/mapped.json"
"$FLOWANALYST_BIN" "$tmpdir/mapped.json" > "$tmpdir/mapped-report.json"
jq -e '[.lowering_plan.operations[].scalar_fact // empty] |
    length > 0 and all(.[]; .provenance.source == "imported-scalar.flow" and .provenance.line > 100)' "$tmpdir/mapped-report.json" >/dev/null

# A projected symbol type cannot override the named type in the source AST.
jq '(.symbol_table.symbols[] | select(.name == "x") | .facts[] |
    select(.key == "declared_type_spelling") | .value.value) = "Bool"' "$tmpdir/frontend.json" > "$tmpdir/conflicting.json"
set +e
"$FLOWANALYST_BIN" "$tmpdir/conflicting.json" > "$tmpdir/conflicting-report.json"
conflicting_status=$?
set -e
test "$conflicting_status" -eq 2
jq -e '.status == "error" and any(.diagnostics[]; .code == "FLOWANALYST_SCALAR_FLOW_REFUSED")' "$tmpdir/conflicting-report.json" >/dev/null

# Mutate captured input without reopening source. All three consumers must
# reject contradictory or malformed proof before an executable is published.
for mutation in \
  '.lowering_plan.operations[0].scalar_fact.source_type="Bool"' \
  '.lowering_plan.operations[0].scalar_fact.version=99' \
  '.lowering_plan.operations[0].scalar_fact.destination_symbol_id=99999' \
  '.lowering_plan.operations[0].scalar_fact.declaration_statement_id=99999' \
  '.lowering_plan.operations[0].scalar_fact.provenance.line=0' \
  '.lowering_plan.operations[0].operands[0]={expression_id:.lowering_plan.operations[0].expression_id,kind:"bool_literal",type:"bool",value:"true"}'
do
    jq "$mutation" "$tmpdir/lowering.json" > "$tmpdir/hostile.json"
    if "$FLOWVALIDATE_BIN" "$tmpdir/hostile.json" > /dev/null 2>&1; then exit 1; fi
    if "$FLOWLOWER_BIN" --emit-llvm "$tmpdir/forbidden.ll" "$tmpdir/hostile.json" >/dev/null 2>&1; then exit 1; fi
    if "$FLOWTINYLOWER_BIN" "$tmpdir/hostile.json" "$tmpdir/forbidden.tvm" >/dev/null 2>&1; then exit 1; fi
    test ! -e "$tmpdir/forbidden.ll"
    test ! -e "$tmpdir/forbidden.tvm"
done
echo 'Canonical scalar authority: PASS (12 refusals; 4 LLVM/TinyVM parity cases, 3 legacy matches and 1 known legacy initializer failure, plan v1/v2; 6 hostile artifacts; mapped origins; conflicting frontend types)'
