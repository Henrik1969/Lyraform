#!/bin/sh
set -eu

root=${FLOWCORE_ROOT:?}
flowmini=${FLOWMINI_BIN:?}
analyst=${FLOWANALYST_BIN:?}
parallel=${FLOWPARALLEL_BIN:?}
optimize=${FLOWOPTIMIZE_BIN:?}
prepare=${FLOWPREPARE_BIN:?}
llvm_lower=${FLOWLOWER_BIN:?}
tiny_lower=${FLOWTINYLOWER_BIN:?}
tiny_validate=${FLOWTINYVALIDATE_BIN:?}
tiny_run=${FLOWTINYRUN_BIN:?}

tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT

for name in \
    profile_free_return profile_free_expression profile_free_local_value \
    profile_free_branch_compare profile_free_integer_loop profile_free_args_length
do
    echo "parity fixture: $name"
    source="$root/Lyraform/compiler/examples/pass/$name.flow"
    "$flowmini" --dump-frontend-bundle "$source" > "$tmpdir/$name.frontend.json"
    "$analyst" < "$tmpdir/$name.frontend.json" > "$tmpdir/$name.semantic.json"
    "$parallel" < "$tmpdir/$name.semantic.json" > "$tmpdir/$name.execution.json"
    "$optimize" < "$tmpdir/$name.execution.json" > "$tmpdir/$name.optimization.json"
    "$prepare" "$tmpdir/$name.optimization.json" > "$tmpdir/$name.lowering.json"

    "$llvm_lower" --emit-llvm "$tmpdir/$name.ll" "$tmpdir/$name.lowering.json" >/dev/null
    clang "$tmpdir/$name.ll" -o "$tmpdir/$name.llvm"
    "$tiny_lower" "$tmpdir/$name.lowering.json" "$tmpdir/$name.tvm" >/dev/null
    "$tiny_validate" "$tmpdir/$name.tvm" | grep -q '"status":"valid"'

    set +e
    "$tmpdir/$name.llvm"
    llvm_status=$?
    set -e
    tiny_result=$("$tiny_run" "$tmpdir/$name.tvm" | jq -r '.result')
    test "$tiny_result" -eq "$llvm_status"
    tiny_computed_result=$("$tiny_run" --engine computed "$tmpdir/$name.tvm" | jq -r '.result')
    test "$tiny_computed_result" -eq "$llvm_status"
    if test "$name" = profile_free_args_length; then
        set +e
        "$tmpdir/$name.llvm" selected
        llvm_selected_status=$?
        set -e
        tiny_selected_result=$("$tiny_run" "$tmpdir/$name.tvm" selected | jq -r '.result')
        test "$tiny_selected_result" -eq "$llvm_selected_status"
        tiny_selected_computed_result=$("$tiny_run" --engine computed "$tmpdir/$name.tvm" selected | jq -r '.result')
        test "$tiny_selected_computed_result" -eq "$llvm_selected_status"
    fi
done

echo 'provider-free scalar LLVM/TinyVM parity: PASS'
