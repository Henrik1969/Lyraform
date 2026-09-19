#!/bin/sh
set -eu
tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT
source_file() {
    printf 'program scalar_source_probe\nmain%s {\n%s\n}\n' "${2:-}" "$1" > "$tmpdir/program.flow"
}
valid_count=0
parity_count=0
refusal_count=0
valid() {
    body=$1
    observed=$2
    expected=$3
    oracle=$4
    source_file "$body
print $observed"
    "$FLOWMINI_TEST_BIN" --test-forbid-legacy --trace true "$tmpdir/program.flow" > "$tmpdir/result" 2> "$tmpdir/trace"
    test "$(cat "$tmpdir/result")" = "$expected"
    grep -q '^source-path: canonical-scalar$' "$tmpdir/trace"
    grep -q '^source-path: runtime-adapter$' "$tmpdir/trace"
    grep -q 'scalar-origin: .*declaration=0 .*destination=[0-9].*source=.*:3:1 ast=/statement_pool/0' "$tmpdir/trace"
    if grep -q 'legacy-compatibility' "$tmpdir/trace"; then exit 1; fi
    "$FLOWMINI_BIN" "$tmpdir/program.flow" > "$tmpdir/production" 2> "$tmpdir/production.err"
    cmp "$tmpdir/result" "$tmpdir/production"
    test ! -s "$tmpdir/production.err"
    set +e
    "$FLOWMINI_TEST_BIN" --test-legacy-oracle "$tmpdir/program.flow" > "$tmpdir/legacy" 2> "$tmpdir/legacy.err"
    status=$?
    set -e
    case "$oracle" in
        AGREES) test "$status" -eq 0; cmp "$tmpdir/result" "$tmpdir/legacy" ;;
        KNOWN_LEGACY_DEFECT) test "$status" -eq 1; grep -q 'missing record path:' "$tmpdir/legacy.err" ;;
        *) echo "unclassified oracle: $oracle" >&2; exit 1 ;;
    esac
    valid_count=$((valid_count + 1))
    echo "valid $valid_count: $oracle"
}
parity() {
    body=$1
    observed=$2
    expected=$3
    oracle=$4
    valid "$body" "$observed" "$expected" "$oracle"
    case "$expected" in
        true|false)
            if test "$expected" = true; then result=1; else result=0; fi
            observe="scalar_observation : int(0)
if $observed { 1 -> scalar_observation }
scalar_observation -> return"
            ;;
        *) result=$expected; observe="$observed -> return" ;;
    esac
    # The same scalar body uses the existing print observation in the direct
    # runtime, and an existing integer return observation in the two backends.
    source_file "$body
$observe"
    "$FLOWMINI_BIN" --dump-frontend-bundle "$tmpdir/program.flow" > "$tmpdir/frontend.json"
    for version in 1 2; do
        "$FLOWANALYST_BIN" --lowering-plan-version "$version" "$tmpdir/frontend.json" > "$tmpdir/semantic.json"
        "$FLOWBIND_BIN" "$tmpdir/semantic.json" > "$tmpdir/binding.json"
        "$FLOWPARALLEL_BIN" "$tmpdir/semantic.json" > "$tmpdir/execution.json"
        "$FLOWOPTIMIZE_BIN" "$tmpdir/execution.json" > "$tmpdir/optimization.json"
        "$FLOWPREPARE_BIN" "$tmpdir/optimization.json" > "$tmpdir/lowering.json"
        jq -S '[.lowering_plan.operations[].scalar_fact // empty]' "$tmpdir/semantic.json" > "$tmpdir/facts"
        for stage in semantic execution optimization lowering; do
            "$FLOWVALIDATE_BIN" --canonical "$tmpdir/$stage.json" > "$tmpdir/canonical.json"
            jq -S '[.lowering_plan.operations[].scalar_fact // empty]' "$tmpdir/canonical.json" > "$tmpdir/imported-facts"
            cmp "$tmpdir/facts" "$tmpdir/imported-facts"
        done
        "$FLOWLOWER_BIN" --emit-llvm "$tmpdir/program.ll" "$tmpdir/lowering.json" > "$tmpdir/llvm.json"
        clang "$tmpdir/program.ll" -o "$tmpdir/program"
        set +e
        "$tmpdir/program"
        status=$?
        set -e
        test "$status" -eq "$result"
        "$FLOWTINYLOWER_BIN" "$tmpdir/lowering.json" "$tmpdir/program.tvm" > "$tmpdir/tiny.json"
        "$FLOWTINYRUN_BIN" "$tmpdir/program.tvm" | jq -e --argjson expected "$result" '.result == $expected' >/dev/null
    done
    parity_count=$((parity_count + 1))
}
parity 'x : int(20)' x 20 AGREES
parity 'a : int(20)
b : int(a)' b 20 KNOWN_LEGACY_DEFECT
parity 'flag : Bool(false)' flag false AGREES
parity 'source_flag : Bool(true)
flag : Bool(source_flag)' flag true KNOWN_LEGACY_DEFECT
parity 'x : int(0)
20 -> x
30 -> x' x 30 AGREES
parity 'a : int(20)
b : int(0)
a -> b' b 20 AGREES
parity 'a : Bool(true)
b : Bool(false)
a -> b' b true AGREES
parity 'a : int(20)
b : int(a)
30 -> a' b 20 KNOWN_LEGACY_DEFECT
parity 'x : int(20)
y : int(3)
x + y * 2 - 5 -> x' x 21 AGREES
parity 'x : int(8)
(x + 2) * 3 -> x' x 30 AGREES
parity 'x : int(8)
flag : Bool(x < 9)
flag == false -> flag' flag false AGREES
valid 'flag : Bool(true)
not flag -> flag' flag false AGREES

refuse() {
    source_file "$1" "${2:-}"
    set +e
    "$FLOWMINI_TEST_BIN" --test-forbid-legacy --trace true --emit-flowir "$tmpdir/refused.flowir" "$tmpdir/program.flow" > "$tmpdir/refused.out" 2> "$tmpdir/refused.err"
    status=$?
    set -e
    test "$status" -eq 1
    grep -q '^source-path: canonical-scalar$' "$tmpdir/refused.err"
    grep -q 'fatal in canonical-scalar:' "$tmpdir/refused.err"
    if grep -qE 'source-path: (runtime-adapter|legacy-compatibility)|test-source-path' "$tmpdir/refused.err"; then exit 1; fi
    test ! -e "$tmpdir/refused.flowir"
    test ! -s "$tmpdir/refused.out"
    if "$FLOWMINI_TEST_BIN" --test-legacy-oracle "$tmpdir/program.flow" > "$tmpdir/negative-legacy.out" 2> "$tmpdir/negative-legacy.err"; then
        echo 'negative legacy oracle: UNDECIDED (legacy admitted a refused form)' >&2; exit 1
    fi
    refusal_count=$((refusal_count + 1))
    echo "refusal $refusal_count: AGREES (legacy also refuses)"
}
refuse 'x : int(true)'
refuse 'x : int(0)
true -> x'
refuse 'flag : Bool(false)
20 -> flag'
refuse 'flag : Bool(true)
x : int(flag)'
refuse 'x : int(20)
flag : Bool(x)'
refuse 'x : int(20)
flag : Bool(false)
flag -> x'
refuse 'x : int(x)'
refuse '20 -> later
later : int(0)'
refuse '20 -> absent'
refuse 'x : int(1)
x : int(2)'
refuse 'x : int'
refuse 'x : int(0)
20 -> x : int'
refuse 'x : int(20) stray'
refuse 'x : int(20)()'
refuse 'x : int(20'
refuse 'x : int(0)
print absent'
refuse 'x : int(20 + true)'
refuse 'x : int(20 junk)'
refuse 'x : int(20) garbage()'
refuse ''
refuse 'x : int(20)' '()()'
refuse 'x : int("text")'
refuse 'x : int(20)
flag : Bool(false)
x -> flag'

# Semantic admission is not a promise of backend support. This operator is
# proved by the shared algebra but has no existing runtime atom mapping.
source_file 'x : int(1)
flag : Bool(x <= 2)'
if "$FLOWMINI_TEST_BIN" --test-forbid-legacy --trace true "$tmpdir/program.flow" > "$tmpdir/backend.out" 2> "$tmpdir/backend.err"; then exit 1; fi
grep -q '^source-path: scalar-facts-admitted$' "$tmpdir/backend.err"
grep -q 'runtime backend does not implement admitted operator <=' "$tmpdir/backend.err"
if grep -q 'legacy-compatibility' "$tmpdir/backend.err"; then exit 1; fi
test ! -s "$tmpdir/backend.out"

# Compatibility is explicitly construct-owned, not chosen after a refusal.
source_file 'x : int(1)
if true { 2 -> x }
print x'
"$FLOWMINI_BIN" --trace true "$tmpdir/program.flow" > "$tmpdir/compatibility.out" 2> "$tmpdir/compatibility.err"
test "$(cat "$tmpdir/compatibility.out")" = 2
grep -q '^source-path: legacy-compatibility$' "$tmpdir/compatibility.err"
if "$FLOWMINI_TEST_BIN" --test-forbid-legacy "$tmpdir/program.flow" >/dev/null 2> "$tmpdir/forbidden.err"; then exit 1; fi
grep -q 'legacy parser invocation forbidden by test' "$tmpdir/forbidden.err"

# The canonical adapter still exports ordinary compatibility runtime IR.
source_file 'a : int(20)
b : int(a)
print b'
"$FLOWMINI_TEST_BIN" --test-forbid-legacy --emit-flowir "$tmpdir/export.flowir" "$tmpdir/program.flow"
"$FLOWMINI_BIN" "$tmpdir/export.flowir" > "$tmpdir/reimported"
test "$(cat "$tmpdir/reimported")" = 20
# A source file with an artifact suffix still belongs to its source construct.
cp "$tmpdir/program.flow" "$tmpdir/source.flowir"
"$FLOWMINI_TEST_BIN" --test-forbid-legacy "$tmpdir/source.flowir" > "$tmpdir/source-with-artifact-suffix"
test "$(cat "$tmpdir/source-with-artifact-suffix")" = 20
echo "Canonical scalar source: PASS ($valid_count direct/oracle cases; $parity_count LLVM/TinyVM parity bodies, plan v1/v2; $refusal_count pre-adapter refusals; 1 backend refusal; explicit compatibility boundary; runtime IR reimport)"
