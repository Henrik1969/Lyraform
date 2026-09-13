#!/bin/sh
set -eu

build=${FLOWCORE_BUILD:?FLOWCORE_BUILD is required}
cmake=${CMAKE_COMMAND_PATH:?CMAKE_COMMAND_PATH is required}
root=${FLOWCORE_ROOT:?FLOWCORE_ROOT is required}
prefix=$(mktemp -d)
trap 'rm -rf "$prefix"' EXIT

"$cmake" --install "$build" --prefix "$prefix" >/dev/null
for binary in flowtinylower flowtinyvalidate flowtinyrun; do
    test -x "$prefix/bin/$binary"
done
for policy in llvm-host.json tinyvm-portable.json; do
    test -f "$prefix/share/flowcore/target-policies/$policy"
done
for header in artifact.h artifact_v2.h isa_v1.h runtime_provider.h tinyvm.h; do
    test -f "$prefix/include/flowcore/tinyvm/tinyvm/$header"
done
test -f "$prefix/share/doc/lyraform/tinyvm/README.md"
test "$(head -n 1 "$prefix/share/doc/lyraform/README.md")" = '# Lyraform'

"$prefix/bin/flowtarget" --policy-root "$prefix/share/flowcore/target-policies" tinyvm-portable > "$prefix/tinyvm-policy.json"
jq -e '.name == "tinyvm-portable" and .backend.name == "tinyvm" and .fallback.mode == "none"' "$prefix/tinyvm-policy.json" >/dev/null
"$prefix/bin/flowtinylower" "$root/Flowlower/tests/captured-empty-lowering.json" "$prefix/empty.tvm" > "$prefix/lowering.json"
jq -e '.status == "emitted" and .backend == "tinyvm"' "$prefix/lowering.json" >/dev/null
"$prefix/bin/flowtinyvalidate" "$prefix/empty.tvm" | jq -e '.status == "valid"' >/dev/null
"$prefix/bin/flowtinyrun" "$prefix/empty.tvm" | jq -e '.status == "completed" and .result == 0' >/dev/null

echo 'TinyVM installed backend shape: PASS'
