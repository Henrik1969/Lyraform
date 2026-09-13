#!/bin/sh
set -eu

build=${FLOWCORE_BUILD:?FLOWCORE_BUILD is required}
cmake=${CMAKE_COMMAND_PATH:?CMAKE_COMMAND_PATH is required}
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

echo 'TinyVM installed backend shape: PASS'
