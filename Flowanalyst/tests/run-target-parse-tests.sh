#!/bin/sh
set -eu
build=$1
compiler="$build/flowmini/flowmini"
direct="$build/flowmini/flowmini_scalar_path_test"
analyst="$build/flowanalyst/flowanalyst"
tmpdir=$(mktemp -d /tmp/lyraform-target-tests.XXXXXX)
trap 'rm -rf "$tmpdir"' EXIT
export_target() {
    printf 'program target_probe\nmain {\nx : int(0)\n20 -> %s\n}\n' "$1" > "$tmpdir/input.flow"
    "$compiler" --dump-frontend-bundle "$tmpdir/input.flow" > "$tmpdir/frontend.json"
    "$compiler" --dump-frontend-bundle "$tmpdir/input.flow" > "$tmpdir/repeated.json"
    cmp "$tmpdir/frontend.json" "$tmpdir/repeated.json"
}
for target in x p.x p.a.b 'items[i]' 'matrix[r,c]' 'a[i,j]' 'a[(i + 1),j * 2]' 'a[true]' ; do
    export_target "$target"
    jq -e 'all(.ast.statement_pool[]; .kind != "unknown")' "$tmpdir/frontend.json" >/dev/null
    if test "$target" = x; then
        jq -e '.parse_validity.state=="canonical_valid"' "$tmpdir/frontend.json" >/dev/null
        "$analyst" "$tmpdir/frontend.json" > "$tmpdir/scalar.json"
        "$direct" --test-forbid-legacy "$tmpdir/input.flow"
    else
        jq -e '.parse_validity.state=="outside_scope"' "$tmpdir/frontend.json" >/dev/null
    fi
done
export_target p.a.b
jq -e '[.ast.statement_pool[] | select(.kind=="placement") | .payload.target.fields[].name] == ["a","b"]' "$tmpdir/frontend.json" >/dev/null
export_target 'matrix[r,c]'
jq -e '.ast as $ast | [$ast.statement_pool[] | select(.kind=="placement") | .payload.target.indexes[] as $id | $ast.expression_pool[] | select(.id==$id) | .payload.name] == ["r","c"]' "$tmpdir/frontend.json" >/dev/null
count=0
while IFS= read -r target; do
    export_target "$target"
    jq -e '.parse_validity.state=="recovered" and .parse_validity.recovery_used and any(.ast.statement_pool[]; .kind=="unknown")' "$tmpdir/frontend.json" >/dev/null
    if "$analyst" "$tmpdir/frontend.json" > "$tmpdir/refused.json"; then exit 1; fi
    jq -e '.status=="error" and .lowering_plan.status=="blocked"' "$tmpdir/refused.json" >/dev/null
    if "$direct" --test-forbid-legacy --trace true "$tmpdir/input.flow" > /dev/null 2> "$tmpdir/direct.err"; then exit 1; fi
    grep -q FLOWMINI_PARSE_NOT_EXECUTABLE "$tmpdir/direct.err"
    if grep -qE 'source-path: (runtime-adapter|legacy-compatibility)' "$tmpdir/direct.err"; then exit 1; fi
    jq '.status="ok" | .lowering_plan.status="ready"' "$tmpdir/refused.json" > "$tmpdir/forged.json"
    if "$build/flowtools/flowparallel/flowparallel" "$tmpdir/forged.json" >/dev/null 2>&1; then exit 1; fi
    count=$((count + 1))
done <<'TARGETS'
p.x[i]
items[i].field
items[i][j]
matrix[r,c].field
a.b[c].d
a.b[i].c
a[i].b[j]
a[i,j][k]
a.
a..b
a[
a[]
a[i
a[i].
a[i]junk
x unexpected
p.x unexpected
items[i] unexpected
x : Bool
a[,i]
a[i,]
a[i,,j]
a[i j]
a[(i]
a[i)]
a[i +]
a[i] extra [j]
TARGETS
echo "Target parse: PASS (8 positive structural cases; member/index order; $count refused targets; deterministic export; no fallback; staged refusal)"
