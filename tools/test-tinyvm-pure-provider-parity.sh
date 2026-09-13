#!/bin/sh
set -eu

root=${FLOWCORE_ROOT:?}
tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT
policy="$tmpdir/policy"
printf '%s\n' \
  'allow libc.so.6 abs c pure c_int c_int' \
  'allow libc.so.6 labs c pure c_long c_long' \
  'allow libc.so.6 strlen c pure c_string c_size_t' \
  'allow libc.so.6 puts c io c_string c_int' \
  'allow libc.so.6 getpid c readonly' \
  'allow libc.so.6 getuid c readonly' \
  'allow libc.so.6 getgid c readonly' \
  'allow libc.so.6 geteuid c readonly' \
  'allow libc.so.6 getegid c readonly' \
  'allow libc.so.6 getppid c readonly' \
  'allow libc.so.6 getpgrp c readonly' > "$policy"
printf '%s\n' \
  'allow libc.so.6 getpgid c readonly c_int c_int' \
  'allow libc.so.6 getsid c readonly c_int c_int' \
  'allow libc.so.6 getpriority c readonly c_int,c_int c_int' >> "$policy"
printf '%s\n' 'allow libc.so.6 abs c readonly c_int c_int' > "$tmpdir/wrong-policy"

# Parse integers exactly: unsigned TinyVM results can exceed shell arithmetic
# and jq's floating-point precision. Native exit status exposes eight bits.
native_status() {
  tail -n 1 "$1" | python3 -c 'import json, sys; print(json.load(sys.stdin)["result"] & 255)'
}

# Keep a negative result independent of the invoking process's nice value.
cat > "$tmpdir/negative-provider-result.flow" <<EOF
import "$root/Lyraform/compiler/std/abi/libc.flow"
program negative_provider_result
main {
    input : c_long(-42)
    output : c_long(0)
    labs(input) -> output
    return -output
}
EOF

for name in \
  negative-provider-result abi_abs_main abi_strlen_main tinyvm_provider_labs test_licbinds profile_free_args_index abi_kernel_getpid_main abi_kernel_getuid_main \
  abi_kernel_getgid_main abi_kernel_geteuid_main abi_kernel_getegid_main \
  abi_kernel_getppid_main abi_kernel_getpgrp_main abi_kernel_getpgid_main \
  abi_kernel_getsid_main abi_kernel_getpriority_main
do
  echo "governed-provider parity fixture: $name"
  source="$root/Lyraform/compiler/examples/pass/$name.flow"
  if test "$name" = negative-provider-result; then
    source="$tmpdir/negative-provider-result.flow"
  fi
  "$FLOWMINI_BIN" --dump-frontend-bundle "$source" > "$tmpdir/$name.frontend.json"
  "$FLOWANALYST_BIN" < "$tmpdir/$name.frontend.json" > "$tmpdir/$name.semantic.json"
  "$FLOWBIND_BIN" --policy "$policy" < "$tmpdir/$name.semantic.json" > "$tmpdir/$name.binding.json"
  "$FLOWPARALLEL_BIN" < "$tmpdir/$name.semantic.json" > "$tmpdir/$name.execution.json"
  "$FLOWOPTIMIZE_BIN" < "$tmpdir/$name.execution.json" > "$tmpdir/$name.optimization.json"
  "$FLOWPREPARE_BIN" --binding-report "$tmpdir/$name.binding.json" "$tmpdir/$name.optimization.json" > "$tmpdir/$name.lowering.json"
  "$FLOWLOWER_BIN" --emit-llvm "$tmpdir/$name.ll" "$tmpdir/$name.lowering.json" >/dev/null
  clang "$tmpdir/$name.ll" -o "$tmpdir/$name.llvm"
  "$FLOWTINYLOWER_BIN" "$tmpdir/$name.lowering.json" "$tmpdir/$name.tvm" >/dev/null
  "$FLOWTINYVALIDATE_BIN" "$tmpdir/$name.tvm" | grep -q '"status":"valid"'

  set +e
  "$tmpdir/$name.llvm" > "$tmpdir/$name.llvm.stdout"
  llvm_status=$?
  set -e
  "$FLOWTINYRUN_BIN" --policy "$policy" "$tmpdir/$name.tvm" > "$tmpdir/$name.execution.json"
  tiny_status=$(native_status "$tmpdir/$name.execution.json")
  test "$tiny_status" -eq "$llvm_status"
  if test "$name" = negative-provider-result; then
    test "$llvm_status" -eq 214
  fi
  sed '$d' "$tmpdir/$name.execution.json" > "$tmpdir/$name.tiny.stdout"
  cmp -s "$tmpdir/$name.llvm.stdout" "$tmpdir/$name.tiny.stdout"
  "$FLOWTINYRUN_BIN" --engine computed --policy "$policy" "$tmpdir/$name.tvm" > "$tmpdir/$name.computed.execution.json"
  tiny_computed_status=$(native_status "$tmpdir/$name.computed.execution.json")
  test "$tiny_computed_status" -eq "$llvm_status"
  sed '$d' "$tmpdir/$name.computed.execution.json" > "$tmpdir/$name.computed.tiny.stdout"
  cmp -s "$tmpdir/$name.llvm.stdout" "$tmpdir/$name.computed.tiny.stdout"
  if test "$name" = profile_free_args_index; then
    set +e
    "$tmpdir/$name.llvm" source-selected > "$tmpdir/$name.selected.llvm.stdout"
    llvm_selected_status=$?
    set -e
    "$FLOWTINYRUN_BIN" --policy "$policy" "$tmpdir/$name.tvm" source-selected > "$tmpdir/$name.selected.execution"
    tiny_selected_status=$(native_status "$tmpdir/$name.selected.execution")
    test "$tiny_selected_status" -eq "$llvm_selected_status"
    sed '$d' "$tmpdir/$name.selected.execution" > "$tmpdir/$name.selected.tiny.stdout"
    cmp -s "$tmpdir/$name.selected.llvm.stdout" "$tmpdir/$name.selected.tiny.stdout"
    "$FLOWTINYRUN_BIN" --engine computed --policy "$policy" "$tmpdir/$name.tvm" source-selected > "$tmpdir/$name.selected.computed.execution"
    tiny_selected_computed_status=$(native_status "$tmpdir/$name.selected.computed.execution")
    test "$tiny_selected_computed_status" -eq "$llvm_selected_status"
    sed '$d' "$tmpdir/$name.selected.computed.execution" > "$tmpdir/$name.selected.computed.tiny.stdout"
    cmp -s "$tmpdir/$name.selected.llvm.stdout" "$tmpdir/$name.selected.computed.tiny.stdout"
  fi

  if "$FLOWTINYRUN_BIN" "$tmpdir/$name.tvm" >/dev/null 2>&1; then
    echo 'TinyVM ran an import artifact without active policy' >&2
    exit 1
  fi
  if "$FLOWTINYRUN_BIN" --policy "$tmpdir/wrong-policy" "$tmpdir/$name.tvm" >/dev/null 2>&1; then
    echo 'TinyVM ran an import under a non-matching policy' >&2
    exit 1
  fi
done

echo 'governed LLVM/TinyVM provider parity: PASS'
