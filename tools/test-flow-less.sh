#!/bin/sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
example="$root/Lyraform/compiler/examples/apps/flow_less"
tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT
# The current pager has provider-dependent integer division without an admitted
# numeric failure route. Refusal is the current support boundary, not evidence
# that paging or terminal interaction executed successfully.
set +e
"$example/build-flow-less.sh" "$tmpdir/app" > "$tmpdir/build.log" 2>&1
build_status=$?
set -e
if test "$build_status" -ne 0; then
    test "$build_status" -eq 2
    jq -e '.status == "error" and .lowering_plan.status == "blocked" and
        (.diagnostics | length) > 0 and all(.diagnostics[];
            .code == "FLOWANALYST_INTEGER_DIVISION_UNPROVEN" and
            (.message | contains("statically established integer operands")))' \
        "$tmpdir/app/semantic.json" >/dev/null
    test ! -e "$tmpdir/app/flow_less"
    echo 'pager admission: refused unproved division; runtime paging NOT exercised'
    exit 0
fi
pager="$tmpdir/app/flow_less"
check() {
    commands=$1 expected=$2
    FLOW_PAGER_COMMANDS="$commands" FLOWCORE_GRAPH_TRACE=1 "$pager" > "$tmpdir/out" 2> "$tmpdir/trace"
    printf '%s\n' "$expected" > "$tmpdir/expected"
    cmp "$tmpdir/expected" "$tmpdir/out"
    jq -se 'any(.[]; .event == "enter" and .node_id == "navigate" and .wire_id == "wire:0") and any(.[]; .event == "drop" and .node_id == "display" and .wire_id == "wire:1")' "$tmpdir/trace" >/dev/null
}
check 'pgdown,end' '-- page 3/3 --
epsilon'
check 'end,home' '-- page 1/3 --
alpha
beta'
check 'up,up,down,q,end' '-- page 2/3 --
gamma
delta'
check 'end,down,down' '-- page 3/3 --
epsilon'
FLOW_PAGER_LINES='' FLOW_PAGER_COMMANDS=end "$pager" > "$tmpdir/empty" 2>/dev/null
printf '%s\n\n' '-- page 1/1 --' > "$tmpdir/expected"
cmp "$tmpdir/expected" "$tmpdir/empty"
for failure in size command; do
    set +e
    if [ "$failure" = size ]; then
        FLOW_PAGER_PAGE_SIZE=0 "$pager" > "$tmpdir/out" 2> "$tmpdir/err"
    else
        FLOW_PAGER_COMMANDS=invented "$pager" > "$tmpdir/out" 2> "$tmpdir/err"
    fi
    status=$?
    set -e
    test "$status" -eq 70
    test ! -s "$tmpdir/out"
    if [ "$failure" = size ]; then grep -Fq 'page_size must be positive' "$tmpdir/err"; else grep -Fq 'unknown command: invented' "$tmpdir/err"; fi
    tail -1 "$tmpdir/err" | jq -e '.format == "flowcore.graph_failure" and .activation.node_id == "navigate" and .activation.wire_id == "wire:0"' >/dev/null
done
# Identical capabilities with changed source command meaning must change behavior.
# Inline placement also proves a closing brace cannot swallow subsequent functions.
sed -e 's/program flow_less/program another_pager/' -e 's/if comparison == 0 { return 4 }/if comparison == 0 { return 3 }/' -e 's/if status < 0 {/if status < 0 {/' "$example/flow_less.flow" > "$tmpdir/changed.flow"
python3 - "$tmpdir/changed.flow" <<'PY'
from pathlib import Path
import sys
p=Path(sys.argv[1]); s=p.read_text().replace('if status < 0 {\n        runtime.raise(4) -> failure\n    }','if status < 0 { runtime.raise(4) -> failure }'); p.write_text(s)
PY
FLOWPAGER_SOURCE="$tmpdir/changed.flow" "$example/build-flow-less.sh" "$tmpdir/changed" >/dev/null
FLOW_PAGER_COMMANDS=end "$tmpdir/changed/flow_less" > "$tmpdir/out" 2>/dev/null
printf '%s\n' '-- page 1/3 --' alpha beta > "$tmpdir/expected"
cmp "$tmpdir/expected" "$tmpdir/out"
# Output errors are transport facts; source selects failure code and suppresses output activation.
cat > "$tmpdir/output.cpp" <<'CPP'
extern "C" int pager_write_text(const char*) { return -1; }
extern "C" int pager_write_line(const char*) { return -1; }
extern "C" int pager_write_integer(int) { return -1; }
extern "C" int pager_diagnostic(const char*) { return 0; }
extern "C" int pager_error_text(const char*) { return 0; }
CPP
"${FLOWGRAPH_CXX:-c++}" -shared -fPIC "$tmpdir/output.cpp" -o "$tmpdir/output.so"
FLOWPAGER_OUTPUT="$tmpdir/output.so" "$example/build-flow-less.sh" "$tmpdir/failed-output" >/dev/null
set +e
"$tmpdir/failed-output/flow_less" > "$tmpdir/out" 2> "$tmpdir/err"
status=$?
set -e
test "$status" -eq 70
test ! -s "$tmpdir/out"
jq -e '.format == "flowcore.graph_failure" and .activation.node_id == "display" and .code == 4' "$tmpdir/err" >/dev/null
printf '%s\n' 'Flow-owned native pager: PASS'
