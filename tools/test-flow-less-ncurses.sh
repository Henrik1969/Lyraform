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
printf '%s\n' alpha beta gamma delta epsilon > "$tmpdir/input.txt"
printf q | FLOW_PAGER_PATH="$tmpdir/input.txt" FLOWCORE_GRAPH_TRACE=1 TERM=xterm script -qec "$tmpdir/app/flow_less" "$tmpdir/terminal.log" >/dev/null
grep -Fq -- '-- page 1/3 --' "$tmpdir/terminal.log"
grep -Fq alpha "$tmpdir/terminal.log"
grep -Fq beta "$tmpdir/terminal.log"
grep -Fq '"node_id":"navigate"' "$tmpdir/terminal.log"
grep -Fq '"node_id":"display"' "$tmpdir/terminal.log"
set +e
FLOW_PAGER_PATH=/definitely/missing/flow_less.txt "$tmpdir/app/flow_less" > "$tmpdir/out" 2> "$tmpdir/err"
status=$?
set -e
test "$status" -eq 70
test ! -s "$tmpdir/out"
grep -Fq 'unable to open text file' "$tmpdir/err"
tail -1 "$tmpdir/err" | jq -e '.format == "flowcore.graph_failure" and .activation.node_id == "navigate" and .code == 2' >/dev/null
# An injected terminal transport reports EOF; it must close its acquired window.
mkdir "$tmpdir/terminal"
cat > "$tmpdir/terminal.cpp" <<'CPP'
#include <cstdio>
#include <cstdlib>
extern "C" void* initscr() { static int window; return &window; }
extern "C" int endwin() { auto* file = std::fopen(std::getenv("CLEANUP_LOG"), "w"); if (!file) return -1; std::fputs("closed\n", file); return std::fclose(file); }
extern "C" int noecho() { return 0; }
extern "C" int cbreak() { return 0; }
extern "C" int keypad(void*, bool) { return 0; }
extern "C" int wrefresh(void*) { return 0; }
extern "C" int wgetch(void*) { return -1; }
CPP
"${FLOWGRAPH_CXX:-c++}" -shared -fPIC "$tmpdir/terminal.cpp" -o "$tmpdir/terminal/libncursesw.so.6"
set +e
CLEANUP_LOG="$tmpdir/closed" LD_LIBRARY_PATH="$tmpdir/terminal${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}" FLOW_PAGER_PATH="$tmpdir/input.txt" "$tmpdir/app/flow_less" > "$tmpdir/out" 2> "$tmpdir/err"
status=$?
set -e
test "$status" -eq 70
test ! -s "$tmpdir/out"
grep -Fxq closed "$tmpdir/closed"
grep -Fq 'terminal input failed' "$tmpdir/err"
tail -1 "$tmpdir/err" | jq -e '.format == "flowcore.graph_failure" and .activation.node_id == "navigate" and .code == 2' >/dev/null
printf '%s\n' 'Native pager terminal input selection: PASS'
