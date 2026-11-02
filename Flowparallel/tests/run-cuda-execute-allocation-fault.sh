#!/bin/sh
set -eu

bin=${FLOWPARALLEL_CUDA_EXECUTE_ALLOCATION_FAULT_BIN:?FLOWPARALLEL_CUDA_EXECUTE_ALLOCATION_FAULT_BIN is required}
test -x "$bin"
tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT

# Exercise library cleanup after the injected allocation failure using isolated
# libraries, without depending on vendor-driver constructors or GPU hardware.
cat > "$tmpdir/library.c" <<'C'
#include <stdio.h>
#include <stdlib.h>
__attribute__((destructor)) static void unloaded(void) {
    const char *path = getenv("FLOW_TEST_LIBRARY_UNLOAD_LOG");
    if (!path) return;
    FILE *out = fopen(path, "a");
    if (out) { fputs("closed\n", out); fclose(out); }
}
C
"${CC:-cc}" -shared -fPIC "$tmpdir/library.c" -o "$tmpdir/libcudart.so.12"
"${CC:-cc}" -shared -fPIC "$tmpdir/library.c" -o "$tmpdir/libcublas.so.12"
export LD_LIBRARY_PATH="$tmpdir${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
export FLOW_TEST_LIBRARY_UNLOAD_LOG="$tmpdir/unloaded"

set +e
"$bin" --size 2 --diagnostics json >"$tmpdir/stdout" 2>"$tmpdir/stderr"
status=$?
set -e

test "$status" -eq 1
test ! -s "$tmpdir/stdout"
test "$(wc -l < "$tmpdir/unloaded")" -eq 2
jq -e '.status == "failed" and .code == "FLOWPARALLEL_CUDA_EXECUTE_RESOURCE_EXHAUSTED" and .stage == "runtime" and .disposition == "no_artifact"' \
  "$tmpdir/stderr" >/dev/null
echo 'Flowparallel CUDA execution allocation-fault containment: PASS'
