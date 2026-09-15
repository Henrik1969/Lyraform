#!/bin/sh
set -eu

root=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
lowerer=${FLOWLOWER_BIN:?FLOWLOWER_BIN is required}
optimizer=${FLOWOPTIMIZE_BIN:-$root/Flowoptimize/build/flowoptimize}
analyst=${FLOWANALYST_BIN:-$root/Flowanalyst/build/flowanalyst}
parallel=${FLOWPARALLEL_BIN:-$root/Flowparallel/build/flowparallel}
bind=${FLOWBIND_BIN:-$root/Flowbind/build/flowbind}
flowmini=${FLOWMINI_BIN:-$root/Lyraform/compiler/cmake-build-debug/flowmini}
fixture="$root/Lyraform/compiler/examples/ast/call_expression_probe.flow"
test -x "$flowmini"
test -x "$parallel"
test -x "$lowerer"

report=$("$flowmini" --dump-frontend-bundle "$fixture" | "$analyst" | "$parallel" | "$optimizer" | "$lowerer")
printf '%s\n' "$report" | grep -q '"format": "flowlower.lowering_report"'
printf '%s\n' "$report" | grep -q '"name": "llvm"'
printf '%s\n' "$report" | grep -q '"format": "llvm-ir"'
printf '%s\n' "$report" | jq -e --arg source "$fixture" '.source.path == $source' >/dev/null

if printf '%s' '{"format":"flowoptimize.optimization_report","version":1,"status":"blocked"}' | "$lowerer" >/dev/null 2>&1; then
    echo 'blocked optimization report unexpectedly lowered' >&2
    exit 1
fi

for hostile in \
  '{"format":"flowoptimize.optimization_report","format":"flowoptimize.optimization_report","version":1,"status":"ready"}' \
  '{"decoy":{"format":"flowoptimize.optimization_report","version":1,"status":"ready"}}' \
  '{"format":"flowoptimize.optimization_report","version":9223372036854775808,"status":"ready"}'
do
  if printf '%s' "$hostile" | "$lowerer" >/dev/null 2>&1; then
    echo 'Flowlower accepted malformed or non-authoritative envelope' >&2
    exit 1
  fi
done

tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT
# Report-only mode validates the same artifact authority as LLVM emission.
"$flowmini" --dump-frontend-bundle "$fixture" | "$analyst" | "$parallel" | "$optimizer" > "$tmpdir/valid.optimization.json"
for mutation in \
    'del(.lowering_plan)' \
    '.lowering_plan.status = "blocked"' \
    '.lowering_plan.version = 99' \
    '.lowering_plan.operations[0].operands = {}' \
    '.lowering_plan.operations[1].id = .lowering_plan.operations[0].id' \
    '.transforms[0].semantics_preserved = "yes"'
do
    jq "$mutation" "$tmpdir/valid.optimization.json" > "$tmpdir/malformed.optimization.json"
    if "$lowerer" < "$tmpdir/malformed.optimization.json" > "$tmpdir/refused.json" 2> "$tmpdir/refused.log"; then
        echo "report-only mode accepted malformed authority: $mutation" >&2; exit 1
    fi
    jq -e '.status == "blocked" and .diagnostic.code == "FLOWLOWER_CONTRACT" and (.diagnostic.path | startswith("$."))' "$tmpdir/refused.json" >/dev/null
done
trial="$root/Flowlower/tests/empty_program_main.flow"
"$flowmini" --dump-frontend-bundle "$trial" | "$analyst" > "$tmpdir/trial.semantic.json"
jq -e '.lowering_plan.format == "flowcore.lowering_plan" and .lowering_plan.version == 1 and (.lowering_plan.operations | length) == 0' "$tmpdir/trial.semantic.json" >/dev/null
"$parallel" < "$tmpdir/trial.semantic.json" | "$optimizer" | "$lowerer" --emit-llvm "$tmpdir/trial.ll" > "$tmpdir/lowering-report.json"
grep -q '"status": "emitted"' "$tmpdir/lowering-report.json"
test -s "$tmpdir/trial.ll"
clang "$tmpdir/trial.ll" -o "$tmpdir/trial"
"$tmpdir/trial"

policy="$tmpdir/abi.policy"
printf '%s\n' \
    'allow libc.so.6 strlen c pure' \
    'allow libc.so.6 abs c pure' \
    'allow libc.so.6 labs c pure' \
    'allow libc.so.6 puts c io' \
    'allow libc.so.6 getpid c readonly' \
    'allow libc.so.6 getuid c readonly' \
    'allow libc.so.6 getgid c readonly' \
    'allow libc.so.6 geteuid c readonly' \
    'allow libc.so.6 getegid c readonly' \
    'allow libc.so.6 getppid c readonly' \
    'allow libc.so.6 getpgrp c readonly' \
    'allow libc.so.6 getpgid c readonly' \
    'allow libc.so.6 getsid c readonly' \
    'allow libc.so.6 getpriority c readonly' \
    'allow libc.so.6 clock_gettime c readonly' \
    'allow libc.so.6 getrandom c readonly' \
    'allow libc.so.6 uname c readonly' \
    'allow libc.so.6 openat c filesystem' \
    'allow libc.so.6 read c filesystem' \
    'allow libc.so.6 write c filesystem' \
    'allow libc.so.6 lseek c filesystem' \
    'allow libc.so.6 unlinkat c filesystem' \
    'allow libc.so.6 rmdir c filesystem' \
    'allow libc.so.6 pipe2 c process_ipc' \
    'allow libc.so.6 fork c process_ipc' \
    'allow libc.so.6 waitpid c process_ipc' \
    'allow libc.so.6 socketpair c socket_ipc' \
    'allow libc.so.6 socket c loopback' \
    'allow libc.so.6 bind c loopback' \
    'allow libc.so.6 listen c loopback' \
    'allow libc.so.6 poll c loopback' \
    'allow libc.so.6 accept4 c loopback' \
    'allow libc.so.6 connect c loopback' \
    'allow libc.so.6 unshare c namespace' \
    'allow libc.so.6 sethostname c namespace' \
    'allow libc.so.6 gethostname c namespace' \
    'allow libc.so.6 open c io' \
    'allow libc.so.6 read c io' \
    'allow libc.so.6 write c io' \
    'allow libc.so.6 sendfile c io' \
    'allow libc.so.6 close c io' > "$policy"
abs_source="$root/Lyraform/compiler/examples/pass/abi_abs_main.flow"
"$flowmini" --dump-frontend-bundle "$abs_source" > "$tmpdir/abs.bundle.json"
"$analyst" < "$tmpdir/abs.bundle.json" > "$tmpdir/abs.semantic.json"
jq -e '.lowering_plan.format == "flowcore.lowering_plan"' "$tmpdir/abs.semantic.json" >/dev/null
"$bind" --policy "$policy" < "$tmpdir/abs.semantic.json" > "$tmpdir/abs.binding.json"
"$parallel" < "$tmpdir/abs.semantic.json" > "$tmpdir/abs.parallel.json"
"$optimizer" < "$tmpdir/abs.parallel.json" > "$tmpdir/abs.optimized.json"
"$lowerer" --emit-llvm "$tmpdir/abs.ll" --binding-report "$tmpdir/abs.binding.json" < "$tmpdir/abs.optimized.json" > "$tmpdir/abs.lowering.json"
grep -q '"status": "emitted"' "$tmpdir/abs.lowering.json"
clang "$tmpdir/abs.ll" -o "$tmpdir/abs"
set +e
"$tmpdir/abs"
abs_rc=$?
set -e
test "$abs_rc" -eq 42

strlen_source="$root/Lyraform/compiler/examples/pass/abi_strlen_main.flow"
"$flowmini" --dump-frontend-bundle "$strlen_source" > "$tmpdir/strlen.bundle.json"
"$analyst" < "$tmpdir/strlen.bundle.json" > "$tmpdir/strlen.semantic.json"
jq -e '.lowering_plan.format == "flowcore.lowering_plan"' "$tmpdir/strlen.semantic.json" >/dev/null
"$bind" --policy "$policy" < "$tmpdir/strlen.semantic.json" > "$tmpdir/strlen.binding.json"
"$parallel" < "$tmpdir/strlen.semantic.json" > "$tmpdir/strlen.parallel.json"
"$optimizer" < "$tmpdir/strlen.parallel.json" > "$tmpdir/strlen.optimized.json"
"$lowerer" --emit-llvm "$tmpdir/strlen.ll" --binding-report "$tmpdir/strlen.binding.json" < "$tmpdir/strlen.optimized.json" > "$tmpdir/strlen.lowering.json"
grep -q '"status": "emitted"' "$tmpdir/strlen.lowering.json"
clang "$tmpdir/strlen.ll" -o "$tmpdir/strlen"
set +e
"$tmpdir/strlen"
strlen_rc=$?
set -e
test "$strlen_rc" -eq 8

licbinds_source="$root/Lyraform/compiler/examples/pass/test_licbinds.flow"
"$flowmini" --dump-frontend-bundle "$licbinds_source" > "$tmpdir/licbinds.bundle.json"
"$analyst" < "$tmpdir/licbinds.bundle.json" > "$tmpdir/licbinds.semantic.json"
"$bind" --policy "$policy" < "$tmpdir/licbinds.semantic.json" > "$tmpdir/licbinds.binding.json"
"$parallel" < "$tmpdir/licbinds.semantic.json" > "$tmpdir/licbinds.parallel.json"
"$optimizer" < "$tmpdir/licbinds.parallel.json" > "$tmpdir/licbinds.optimized.json"
"$lowerer" --emit-llvm "$tmpdir/licbinds.ll" --binding-report "$tmpdir/licbinds.binding.json" < "$tmpdir/licbinds.optimized.json" > "$tmpdir/licbinds.lowering.json"
grep -q '"status": "emitted"' "$tmpdir/licbinds.lowering.json"
grep -q 'generic structured lowering plan' "$tmpdir/licbinds.ll"
clang -g "$tmpdir/licbinds.ll" -o "$tmpdir/licbinds"
"$tmpdir/licbinds" > "$tmpdir/licbinds.output"
printf '%s\n' 'Flowcore libc bindings' | cmp -s - "$tmpdir/licbinds.output"
gdb -q -batch -ex 'set pagination off' -ex 'break main' -ex run -ex bt -ex quit "$tmpdir/licbinds" > "$tmpdir/licbinds.gdb.txt" 2>&1
grep -q 'Breakpoint 1' "$tmpdir/licbinds.gdb.txt"
if grep -q 'ptrace: Operation not permitted' "$tmpdir/licbinds.gdb.txt"; then
    grep -q 'During startup program exited' "$tmpdir/licbinds.gdb.txt"
else
    grep -q 'main' "$tmpdir/licbinds.gdb.txt"
fi

kernel_getpid_source="$root/Lyraform/compiler/examples/pass/abi_kernel_getpid_main.flow"
"$flowmini" --dump-frontend-bundle "$kernel_getpid_source" > "$tmpdir/kernel-getpid.bundle.json"
"$analyst" < "$tmpdir/kernel-getpid.bundle.json" > "$tmpdir/kernel-getpid.semantic.json"
grep -q '"symbol":"getpid"' "$tmpdir/kernel-getpid.semantic.json"
"$bind" --policy "$policy" < "$tmpdir/kernel-getpid.semantic.json" > "$tmpdir/kernel-getpid.binding.json"
"$parallel" < "$tmpdir/kernel-getpid.semantic.json" > "$tmpdir/kernel-getpid.parallel.json"
"$optimizer" < "$tmpdir/kernel-getpid.parallel.json" > "$tmpdir/kernel-getpid.optimized.json"
"$lowerer" --emit-llvm "$tmpdir/kernel-getpid.ll" --binding-report "$tmpdir/kernel-getpid.binding.json" < "$tmpdir/kernel-getpid.optimized.json" > "$tmpdir/kernel-getpid.lowering.json"
grep -q '"status": "emitted"' "$tmpdir/kernel-getpid.lowering.json"
grep -q 'call i32 @getpid' "$tmpdir/kernel-getpid.ll"
clang "$tmpdir/kernel-getpid.ll" -o "$tmpdir/kernel-getpid"
"$tmpdir/kernel-getpid"

kernel_getuid_source="$root/Lyraform/compiler/examples/pass/abi_kernel_getuid_main.flow"
"$flowmini" --dump-frontend-bundle "$kernel_getuid_source" > "$tmpdir/kernel-getuid.bundle.json"
"$analyst" < "$tmpdir/kernel-getuid.bundle.json" > "$tmpdir/kernel-getuid.semantic.json"
"$bind" --policy "$policy" < "$tmpdir/kernel-getuid.semantic.json" > "$tmpdir/kernel-getuid.binding.json"
"$parallel" < "$tmpdir/kernel-getuid.semantic.json" > "$tmpdir/kernel-getuid.parallel.json"
"$optimizer" < "$tmpdir/kernel-getuid.parallel.json" > "$tmpdir/kernel-getuid.optimized.json"
"$lowerer" --emit-llvm "$tmpdir/kernel-getuid.ll" --binding-report "$tmpdir/kernel-getuid.binding.json" < "$tmpdir/kernel-getuid.optimized.json" > "$tmpdir/kernel-getuid.lowering.json"
grep -q '"status": "emitted"' "$tmpdir/kernel-getuid.lowering.json"
grep -q 'call i32 @getuid' "$tmpdir/kernel-getuid.ll"
clang "$tmpdir/kernel-getuid.ll" -o "$tmpdir/kernel-getuid"
expected_uid=$(id -u)
set +e
"$tmpdir/kernel-getuid"
actual_uid=$?
set -e
test "$actual_uid" -eq $((expected_uid % 256))

kernel_getgid_source="$root/Lyraform/compiler/examples/pass/abi_kernel_getgid_main.flow"
"$flowmini" --dump-frontend-bundle "$kernel_getgid_source" > "$tmpdir/kernel-getgid.bundle.json"
"$analyst" < "$tmpdir/kernel-getgid.bundle.json" > "$tmpdir/kernel-getgid.semantic.json"
"$bind" --policy "$policy" < "$tmpdir/kernel-getgid.semantic.json" > "$tmpdir/kernel-getgid.binding.json"
"$parallel" < "$tmpdir/kernel-getgid.semantic.json" > "$tmpdir/kernel-getgid.parallel.json"
"$optimizer" < "$tmpdir/kernel-getgid.parallel.json" > "$tmpdir/kernel-getgid.optimized.json"
"$lowerer" --emit-llvm "$tmpdir/kernel-getgid.ll" --binding-report "$tmpdir/kernel-getgid.binding.json" < "$tmpdir/kernel-getgid.optimized.json" > "$tmpdir/kernel-getgid.lowering.json"
grep -q '"status": "emitted"' "$tmpdir/kernel-getgid.lowering.json"
grep -q 'call i32 @getgid' "$tmpdir/kernel-getgid.ll"
clang "$tmpdir/kernel-getgid.ll" -o "$tmpdir/kernel-getgid"
expected_gid=$(id -g)
set +e
"$tmpdir/kernel-getgid"
actual_gid=$?
set -e
test "$actual_gid" -eq $((expected_gid % 256))

kernel_geteuid_source="$root/Lyraform/compiler/examples/pass/abi_kernel_geteuid_main.flow"
"$flowmini" --dump-frontend-bundle "$kernel_geteuid_source" > "$tmpdir/kernel-geteuid.bundle.json"
"$analyst" < "$tmpdir/kernel-geteuid.bundle.json" > "$tmpdir/kernel-geteuid.semantic.json"
"$bind" --policy "$policy" < "$tmpdir/kernel-geteuid.semantic.json" > "$tmpdir/kernel-geteuid.binding.json"
"$parallel" < "$tmpdir/kernel-geteuid.semantic.json" > "$tmpdir/kernel-geteuid.parallel.json"
"$optimizer" < "$tmpdir/kernel-geteuid.parallel.json" > "$tmpdir/kernel-geteuid.optimized.json"
"$lowerer" --emit-llvm "$tmpdir/kernel-geteuid.ll" --binding-report "$tmpdir/kernel-geteuid.binding.json" < "$tmpdir/kernel-geteuid.optimized.json" > "$tmpdir/kernel-geteuid.lowering.json"
grep -q '"status": "emitted"' "$tmpdir/kernel-geteuid.lowering.json"
grep -q 'call i32 @geteuid' "$tmpdir/kernel-geteuid.ll"
clang "$tmpdir/kernel-geteuid.ll" -o "$tmpdir/kernel-geteuid"
expected_euid=$(id -u)
set +e
"$tmpdir/kernel-geteuid"
actual_euid=$?
set -e
test "$actual_euid" -eq $((expected_euid % 256))

kernel_getegid_source="$root/Lyraform/compiler/examples/pass/abi_kernel_getegid_main.flow"
"$flowmini" --dump-frontend-bundle "$kernel_getegid_source" > "$tmpdir/kernel-getegid.bundle.json"
"$analyst" < "$tmpdir/kernel-getegid.bundle.json" > "$tmpdir/kernel-getegid.semantic.json"
"$bind" --policy "$policy" < "$tmpdir/kernel-getegid.semantic.json" > "$tmpdir/kernel-getegid.binding.json"
"$parallel" < "$tmpdir/kernel-getegid.semantic.json" > "$tmpdir/kernel-getegid.parallel.json"
"$optimizer" < "$tmpdir/kernel-getegid.parallel.json" > "$tmpdir/kernel-getegid.optimized.json"
"$lowerer" --emit-llvm "$tmpdir/kernel-getegid.ll" --binding-report "$tmpdir/kernel-getegid.binding.json" < "$tmpdir/kernel-getegid.optimized.json" > "$tmpdir/kernel-getegid.lowering.json"
grep -q '"status": "emitted"' "$tmpdir/kernel-getegid.lowering.json"
grep -q 'call i32 @getegid' "$tmpdir/kernel-getegid.ll"
clang "$tmpdir/kernel-getegid.ll" -o "$tmpdir/kernel-getegid"
expected_egid=$(id -g)
set +e
"$tmpdir/kernel-getegid"
actual_egid=$?
set -e
test "$actual_egid" -eq $((expected_egid % 256))

kernel_getppid_source="$root/Lyraform/compiler/examples/pass/abi_kernel_getppid_main.flow"
"$flowmini" --dump-frontend-bundle "$kernel_getppid_source" > "$tmpdir/kernel-getppid.bundle.json"
"$analyst" < "$tmpdir/kernel-getppid.bundle.json" > "$tmpdir/kernel-getppid.semantic.json"
"$bind" --policy "$policy" < "$tmpdir/kernel-getppid.semantic.json" > "$tmpdir/kernel-getppid.binding.json"
"$parallel" < "$tmpdir/kernel-getppid.semantic.json" > "$tmpdir/kernel-getppid.parallel.json"
"$optimizer" < "$tmpdir/kernel-getppid.parallel.json" > "$tmpdir/kernel-getppid.optimized.json"
"$lowerer" --emit-llvm "$tmpdir/kernel-getppid.ll" --binding-report "$tmpdir/kernel-getppid.binding.json" < "$tmpdir/kernel-getppid.optimized.json" > "$tmpdir/kernel-getppid.lowering.json"
grep -q '"status": "emitted"' "$tmpdir/kernel-getppid.lowering.json"
grep -q 'call i32 @getppid' "$tmpdir/kernel-getppid.ll"
clang "$tmpdir/kernel-getppid.ll" -o "$tmpdir/kernel-getppid"
expected_ppid=$$
set +e
"$tmpdir/kernel-getppid"
actual_ppid=$?
set -e
test "$actual_ppid" -eq $((expected_ppid % 256))

kernel_getpgrp_source="$root/Lyraform/compiler/examples/pass/abi_kernel_getpgrp_main.flow"
"$flowmini" --dump-frontend-bundle "$kernel_getpgrp_source" > "$tmpdir/kernel-getpgrp.bundle.json"
"$analyst" < "$tmpdir/kernel-getpgrp.bundle.json" > "$tmpdir/kernel-getpgrp.semantic.json"
"$bind" --policy "$policy" < "$tmpdir/kernel-getpgrp.semantic.json" > "$tmpdir/kernel-getpgrp.binding.json"
"$parallel" < "$tmpdir/kernel-getpgrp.semantic.json" > "$tmpdir/kernel-getpgrp.parallel.json"
"$optimizer" < "$tmpdir/kernel-getpgrp.parallel.json" > "$tmpdir/kernel-getpgrp.optimized.json"
"$lowerer" --emit-llvm "$tmpdir/kernel-getpgrp.ll" --binding-report "$tmpdir/kernel-getpgrp.binding.json" < "$tmpdir/kernel-getpgrp.optimized.json" > "$tmpdir/kernel-getpgrp.lowering.json"
grep -q '"status": "emitted"' "$tmpdir/kernel-getpgrp.lowering.json"
grep -q 'call i32 @getpgrp' "$tmpdir/kernel-getpgrp.ll"
clang "$tmpdir/kernel-getpgrp.ll" -o "$tmpdir/kernel-getpgrp"
expected_pgrp=$(ps -o pgid= -p $$ | tr -d ' ')
set +e
"$tmpdir/kernel-getpgrp"
actual_pgrp=$?
set -e
test "$actual_pgrp" -eq $((expected_pgrp % 256))

kernel_getpgid_source="$root/Lyraform/compiler/examples/pass/abi_kernel_getpgid_main.flow"
"$flowmini" --dump-frontend-bundle "$kernel_getpgid_source" > "$tmpdir/kernel-getpgid.bundle.json"
"$analyst" < "$tmpdir/kernel-getpgid.bundle.json" > "$tmpdir/kernel-getpgid.semantic.json"
"$bind" --policy "$policy" < "$tmpdir/kernel-getpgid.semantic.json" > "$tmpdir/kernel-getpgid.binding.json"
"$parallel" < "$tmpdir/kernel-getpgid.semantic.json" > "$tmpdir/kernel-getpgid.parallel.json"
"$optimizer" < "$tmpdir/kernel-getpgid.parallel.json" > "$tmpdir/kernel-getpgid.optimized.json"
"$lowerer" --emit-llvm "$tmpdir/kernel-getpgid.ll" --binding-report "$tmpdir/kernel-getpgid.binding.json" < "$tmpdir/kernel-getpgid.optimized.json" > "$tmpdir/kernel-getpgid.lowering.json"
grep -q '"status": "emitted"' "$tmpdir/kernel-getpgid.lowering.json"
grep -q 'call i32 @getpgid' "$tmpdir/kernel-getpgid.ll"
clang "$tmpdir/kernel-getpgid.ll" -o "$tmpdir/kernel-getpgid"
expected_pgpid=$(ps -o pgid= -p $$ | tr -d ' ')
set +e
"$tmpdir/kernel-getpgid"
actual_pgpid=$?
set -e
test "$actual_pgpid" -eq $((expected_pgpid % 256))

kernel_getsid_source="$root/Lyraform/compiler/examples/pass/abi_kernel_getsid_main.flow"
"$flowmini" --dump-frontend-bundle "$kernel_getsid_source" > "$tmpdir/kernel-getsid.bundle.json"
"$analyst" < "$tmpdir/kernel-getsid.bundle.json" > "$tmpdir/kernel-getsid.semantic.json"
"$bind" --policy "$policy" < "$tmpdir/kernel-getsid.semantic.json" > "$tmpdir/kernel-getsid.binding.json"
"$parallel" < "$tmpdir/kernel-getsid.semantic.json" > "$tmpdir/kernel-getsid.parallel.json"
"$optimizer" < "$tmpdir/kernel-getsid.parallel.json" > "$tmpdir/kernel-getsid.optimized.json"
"$lowerer" --emit-llvm "$tmpdir/kernel-getsid.ll" --binding-report "$tmpdir/kernel-getsid.binding.json" < "$tmpdir/kernel-getsid.optimized.json" > "$tmpdir/kernel-getsid.lowering.json"
grep -q '"status": "emitted"' "$tmpdir/kernel-getsid.lowering.json"
grep -q 'call i32 @getsid' "$tmpdir/kernel-getsid.ll"
clang "$tmpdir/kernel-getsid.ll" -o "$tmpdir/kernel-getsid"
expected_sid=$(ps -o sid= -p $$ | tr -d ' ')
set +e
"$tmpdir/kernel-getsid"
actual_sid=$?
set -e
test "$actual_sid" -eq $((expected_sid % 256))

kernel_getpriority_source="$root/Lyraform/compiler/examples/pass/abi_kernel_getpriority_main.flow"
"$flowmini" --dump-frontend-bundle "$kernel_getpriority_source" > "$tmpdir/kernel-getpriority.bundle.json"
"$analyst" < "$tmpdir/kernel-getpriority.bundle.json" > "$tmpdir/kernel-getpriority.semantic.json"
"$bind" --policy "$policy" < "$tmpdir/kernel-getpriority.semantic.json" > "$tmpdir/kernel-getpriority.binding.json"
"$parallel" < "$tmpdir/kernel-getpriority.semantic.json" > "$tmpdir/kernel-getpriority.parallel.json"
"$optimizer" < "$tmpdir/kernel-getpriority.parallel.json" > "$tmpdir/kernel-getpriority.optimized.json"
"$lowerer" --emit-llvm "$tmpdir/kernel-getpriority.ll" --binding-report "$tmpdir/kernel-getpriority.binding.json" < "$tmpdir/kernel-getpriority.optimized.json" > "$tmpdir/kernel-getpriority.lowering.json"
grep -q '"status": "emitted"' "$tmpdir/kernel-getpriority.lowering.json"
grep -q 'call i32 @getpriority' "$tmpdir/kernel-getpriority.ll"
clang "$tmpdir/kernel-getpriority.ll" -o "$tmpdir/kernel-getpriority"
expected_priority=$(ps -o ni= -p $$ | tr -d ' ')
set +e
"$tmpdir/kernel-getpriority"
actual_priority=$?
set -e
# Process exit status retains only the low eight bits, including negative nice values.
test "$actual_priority" -eq "$((expected_priority & 255))"

kernel_clock_source="$root/Lyraform/compiler/examples/pass/abi_kernel_clock_main.flow"
"$flowmini" --dump-frontend-bundle "$kernel_clock_source" > "$tmpdir/kernel-clock.bundle.json"
"$analyst" < "$tmpdir/kernel-clock.bundle.json" > "$tmpdir/kernel-clock.semantic.json"
jq -e 'any(.lowering_plan.operations[]; .operands[0].kind == "writable_storage" and .operands[0].storage.bytes == 16)' "$tmpdir/kernel-clock.semantic.json" >/dev/null
"$bind" --policy "$policy" < "$tmpdir/kernel-clock.semantic.json" > "$tmpdir/kernel-clock.binding.json"
"$parallel" < "$tmpdir/kernel-clock.semantic.json" > "$tmpdir/kernel-clock.parallel.json"
"$optimizer" < "$tmpdir/kernel-clock.parallel.json" > "$tmpdir/kernel-clock.optimized.json"
"$lowerer" --emit-llvm "$tmpdir/kernel-clock.ll" --binding-report "$tmpdir/kernel-clock.binding.json" < "$tmpdir/kernel-clock.optimized.json" > "$tmpdir/kernel-clock.lowering.json"
grep -q '"status": "emitted"' "$tmpdir/kernel-clock.lowering.json"
grep -q 'call i32 @clock_gettime' "$tmpdir/kernel-clock.ll"
grep -q 'alloca \[16 x i8\]' "$tmpdir/kernel-clock.ll"
clang "$tmpdir/kernel-clock.ll" -o "$tmpdir/kernel-clock"
"$tmpdir/kernel-clock"

jq '(.lowering_plan.operations[] | select(.operands[0].kind == "writable_storage") | .operands[0].storage.bytes) = 0' \
    "$tmpdir/kernel-clock.semantic.json" > "$tmpdir/kernel-clock-invalid-storage.json"
if "$bind" --policy "$policy" < "$tmpdir/kernel-clock-invalid-storage.json" >/dev/null 2>&1; then
    echo 'zero-sized writable storage unexpectedly authorized' >&2
    exit 1
fi

kernel_random_source="$root/Lyraform/compiler/examples/pass/abi_kernel_random_main.flow"
"$flowmini" --dump-frontend-bundle "$kernel_random_source" > "$tmpdir/kernel-random.bundle.json"
"$analyst" < "$tmpdir/kernel-random.bundle.json" > "$tmpdir/kernel-random.semantic.json"
"$bind" --policy "$policy" < "$tmpdir/kernel-random.semantic.json" > "$tmpdir/kernel-random.binding.json"
"$parallel" < "$tmpdir/kernel-random.semantic.json" > "$tmpdir/kernel-random.parallel.json"
"$optimizer" < "$tmpdir/kernel-random.parallel.json" > "$tmpdir/kernel-random.optimized.json"
"$lowerer" --emit-llvm "$tmpdir/kernel-random.ll" --binding-report "$tmpdir/kernel-random.binding.json" < "$tmpdir/kernel-random.optimized.json" > "$tmpdir/kernel-random.lowering.json"
grep -q '"status": "emitted"' "$tmpdir/kernel-random.lowering.json"
grep -q 'call i64 @getrandom' "$tmpdir/kernel-random.ll"
clang "$tmpdir/kernel-random.ll" -o "$tmpdir/kernel-random"
"$tmpdir/kernel-random"

kernel_uname_source="$root/Lyraform/compiler/examples/pass/abi_kernel_uname_main.flow"
"$flowmini" --dump-frontend-bundle "$kernel_uname_source" > "$tmpdir/kernel-uname.bundle.json"
"$analyst" < "$tmpdir/kernel-uname.bundle.json" > "$tmpdir/kernel-uname.semantic.json"
jq -e 'any(.lowering_plan.operations[]; .operands[0].kind == "writable_storage" and .operands[0].storage.bytes == 390)' "$tmpdir/kernel-uname.semantic.json" >/dev/null
"$bind" --policy "$policy" < "$tmpdir/kernel-uname.semantic.json" > "$tmpdir/kernel-uname.binding.json"
"$parallel" < "$tmpdir/kernel-uname.semantic.json" > "$tmpdir/kernel-uname.parallel.json"
"$optimizer" < "$tmpdir/kernel-uname.parallel.json" > "$tmpdir/kernel-uname.optimized.json"
"$lowerer" --emit-llvm "$tmpdir/kernel-uname.ll" --binding-report "$tmpdir/kernel-uname.binding.json" < "$tmpdir/kernel-uname.optimized.json" > "$tmpdir/kernel-uname.lowering.json"
grep -q '"status": "emitted"' "$tmpdir/kernel-uname.lowering.json"
grep -q 'call i32 @uname' "$tmpdir/kernel-uname.ll"
grep -q 'alloca \[390 x i8\]' "$tmpdir/kernel-uname.ll"
clang "$tmpdir/kernel-uname.ll" -o "$tmpdir/kernel-uname"
"$tmpdir/kernel-uname"

kernel_openat_source="$root/Lyraform/compiler/examples/pass/abi_kernel_openat_main.flow"
"$flowmini" --dump-frontend-bundle "$kernel_openat_source" > "$tmpdir/kernel-openat.bundle.json"
"$analyst" < "$tmpdir/kernel-openat.bundle.json" > "$tmpdir/kernel-openat.semantic.json"
"$bind" --policy "$policy" < "$tmpdir/kernel-openat.semantic.json" > "$tmpdir/kernel-openat.binding.json"
"$parallel" < "$tmpdir/kernel-openat.semantic.json" > "$tmpdir/kernel-openat.parallel.json"
"$optimizer" < "$tmpdir/kernel-openat.parallel.json" > "$tmpdir/kernel-openat.optimized.json"
"$lowerer" --emit-llvm "$tmpdir/kernel-openat.ll" --binding-report "$tmpdir/kernel-openat.binding.json" < "$tmpdir/kernel-openat.optimized.json" > "$tmpdir/kernel-openat.lowering.json"
grep -q '"status": "emitted"' "$tmpdir/kernel-openat.lowering.json"
grep -q 'call i32 @openat' "$tmpdir/kernel-openat.ll"
clang "$tmpdir/kernel-openat.ll" -o "$tmpdir/kernel-openat"
"$tmpdir/kernel-openat"

kernel_read_source="$root/Lyraform/compiler/examples/pass/abi_kernel_read_main.flow"
"$flowmini" --dump-frontend-bundle "$kernel_read_source" > "$tmpdir/kernel-read.bundle.json"
"$analyst" < "$tmpdir/kernel-read.bundle.json" > "$tmpdir/kernel-read.semantic.json"
"$bind" --policy "$policy" < "$tmpdir/kernel-read.semantic.json" > "$tmpdir/kernel-read.binding.json"
"$parallel" < "$tmpdir/kernel-read.semantic.json" > "$tmpdir/kernel-read.parallel.json"
"$optimizer" < "$tmpdir/kernel-read.parallel.json" > "$tmpdir/kernel-read.optimized.json"
"$lowerer" --emit-llvm "$tmpdir/kernel-read.ll" --binding-report "$tmpdir/kernel-read.binding.json" < "$tmpdir/kernel-read.optimized.json" > "$tmpdir/kernel-read.lowering.json"
grep -q '"status": "emitted"' "$tmpdir/kernel-read.lowering.json"
grep -q 'call i64 @read' "$tmpdir/kernel-read.ll"
clang "$tmpdir/kernel-read.ll" -o "$tmpdir/kernel-read"
"$tmpdir/kernel-read"

kernel_write_source="$root/Lyraform/compiler/examples/pass/abi_kernel_write_main.flow"
"$flowmini" --dump-frontend-bundle "$kernel_write_source" > "$tmpdir/kernel-write.bundle.json"
"$analyst" < "$tmpdir/kernel-write.bundle.json" > "$tmpdir/kernel-write.semantic.json"
"$bind" --policy "$policy" < "$tmpdir/kernel-write.semantic.json" > "$tmpdir/kernel-write.binding.json"
"$parallel" < "$tmpdir/kernel-write.semantic.json" > "$tmpdir/kernel-write.parallel.json"
"$optimizer" < "$tmpdir/kernel-write.parallel.json" > "$tmpdir/kernel-write.optimized.json"
"$lowerer" --emit-llvm "$tmpdir/kernel-write.ll" --binding-report "$tmpdir/kernel-write.binding.json" < "$tmpdir/kernel-write.optimized.json" > "$tmpdir/kernel-write.lowering.json"
grep -q '"status": "emitted"' "$tmpdir/kernel-write.lowering.json"
grep -q 'call i64 @write' "$tmpdir/kernel-write.ll"
clang "$tmpdir/kernel-write.ll" -o "$tmpdir/kernel-write"
"$tmpdir/kernel-write"

kernel_lseek_source="$root/Lyraform/compiler/examples/pass/abi_kernel_lseek_main.flow"
"$flowmini" --dump-frontend-bundle "$kernel_lseek_source" > "$tmpdir/kernel-lseek.bundle.json"
"$analyst" < "$tmpdir/kernel-lseek.bundle.json" > "$tmpdir/kernel-lseek.semantic.json"
"$bind" --policy "$policy" < "$tmpdir/kernel-lseek.semantic.json" > "$tmpdir/kernel-lseek.binding.json"
"$parallel" < "$tmpdir/kernel-lseek.semantic.json" > "$tmpdir/kernel-lseek.parallel.json"
"$optimizer" < "$tmpdir/kernel-lseek.parallel.json" > "$tmpdir/kernel-lseek.optimized.json"
"$lowerer" --emit-llvm "$tmpdir/kernel-lseek.ll" --binding-report "$tmpdir/kernel-lseek.binding.json" < "$tmpdir/kernel-lseek.optimized.json" > "$tmpdir/kernel-lseek.lowering.json"
grep -q '"status": "emitted"' "$tmpdir/kernel-lseek.lowering.json"
grep -q 'call i64 @lseek' "$tmpdir/kernel-lseek.ll"
clang "$tmpdir/kernel-lseek.ll" -o "$tmpdir/kernel-lseek"
"$tmpdir/kernel-lseek"

kernel_unlinkat_source="$root/Lyraform/compiler/examples/pass/abi_kernel_unlinkat_main.flow"
"$flowmini" --dump-frontend-bundle "$kernel_unlinkat_source" > "$tmpdir/kernel-unlinkat.bundle.json"
"$analyst" < "$tmpdir/kernel-unlinkat.bundle.json" > "$tmpdir/kernel-unlinkat.semantic.json"
"$bind" --policy "$policy" < "$tmpdir/kernel-unlinkat.semantic.json" > "$tmpdir/kernel-unlinkat.binding.json"
"$parallel" < "$tmpdir/kernel-unlinkat.semantic.json" > "$tmpdir/kernel-unlinkat.parallel.json"
"$optimizer" < "$tmpdir/kernel-unlinkat.parallel.json" > "$tmpdir/kernel-unlinkat.optimized.json"
"$lowerer" --emit-llvm "$tmpdir/kernel-unlinkat.ll" --binding-report "$tmpdir/kernel-unlinkat.binding.json" < "$tmpdir/kernel-unlinkat.optimized.json" > "$tmpdir/kernel-unlinkat.lowering.json"
grep -q '"status": "emitted"' "$tmpdir/kernel-unlinkat.lowering.json"
grep -q 'call i32 @unlinkat' "$tmpdir/kernel-unlinkat.ll"
clang "$tmpdir/kernel-unlinkat.ll" -o "$tmpdir/kernel-unlinkat"
"$tmpdir/kernel-unlinkat"

rmdir_source="$root/Lyraform/compiler/examples/pass/abi_kernel_rmdir_main.flow"
"$flowmini" --dump-frontend-bundle "$rmdir_source" > "$tmpdir/kernel-rmdir.bundle.json"
"$analyst" < "$tmpdir/kernel-rmdir.bundle.json" > "$tmpdir/kernel-rmdir.semantic.json"
"$bind" --policy "$policy" < "$tmpdir/kernel-rmdir.semantic.json" > "$tmpdir/kernel-rmdir.binding.json"
"$parallel" < "$tmpdir/kernel-rmdir.semantic.json" | "$optimizer" > "$tmpdir/kernel-rmdir.optimized.json"
"$lowerer" --emit-llvm "$tmpdir/kernel-rmdir.ll" --binding-report "$tmpdir/kernel-rmdir.binding.json" < "$tmpdir/kernel-rmdir.optimized.json" > "$tmpdir/kernel-rmdir.lowering.json"
grep -q 'call i32 @rmdir(ptr %flow_load_' "$tmpdir/kernel-rmdir.ll"
if grep -q 'call i32 @rmdir(ptr null)' "$tmpdir/kernel-rmdir.ll"; then
    echo 'generic rmdir lowering discarded the source path' >&2
    exit 1
fi
clang "$tmpdir/kernel-rmdir.ll" -o "$tmpdir/kernel-rmdir"
"$tmpdir/kernel-rmdir"

for kernel_name in fork socket listen unshare sethostname pipe2 waitpid socketpair bind poll accept4 connect gethostname; do
    kernel_source="$root/Lyraform/compiler/examples/pass/abi_kernel_${kernel_name}_main.flow"
    "$flowmini" --dump-frontend-bundle "$kernel_source" | "$analyst" > "$tmpdir/kernel-${kernel_name}.semantic.json"
    "$bind" --policy "$policy" < "$tmpdir/kernel-${kernel_name}.semantic.json" > "$tmpdir/kernel-${kernel_name}.binding.json"
    "$parallel" < "$tmpdir/kernel-${kernel_name}.semantic.json" | "$optimizer" > "$tmpdir/kernel-${kernel_name}.optimized.json"
    "$lowerer" --emit-llvm "$tmpdir/kernel-${kernel_name}.ll" --binding-report "$tmpdir/kernel-${kernel_name}.binding.json" < "$tmpdir/kernel-${kernel_name}.optimized.json" > "$tmpdir/kernel-${kernel_name}.lowering.json"
    grep -q "call i32 @${kernel_name}" "$tmpdir/kernel-${kernel_name}.ll"
    clang "$tmpdir/kernel-${kernel_name}.ll" -o "$tmpdir/kernel-${kernel_name}"
    "$tmpdir/kernel-${kernel_name}"
done

flowcat_source="$root/Lyraform/compiler/examples/apps/flowcat/flowcat.flow"
"$flowmini" --dump-frontend-bundle "$flowcat_source" > "$tmpdir/flowcat.bundle.json"
"$analyst" < "$tmpdir/flowcat.bundle.json" > "$tmpdir/flowcat.semantic.json"
jq -e '([.lowering_plan.operations[] | select(.kind == "loop")] | length) == 2 and any(.lowering_plan.operations[]; .kind == "assignment") and any(.lowering_plan.operations[]; .kind == "external_call" and .provider.symbol == "sendfile")' "$tmpdir/flowcat.semantic.json" >/dev/null
"$bind" --policy "$policy" < "$tmpdir/flowcat.semantic.json" > "$tmpdir/flowcat.binding.json"
"$parallel" < "$tmpdir/flowcat.semantic.json" > "$tmpdir/flowcat.parallel.json"
"$optimizer" < "$tmpdir/flowcat.parallel.json" > "$tmpdir/flowcat.optimized.json"

# Removing source loops leaves child blocks unreachable and must be rejected.
jq '.lowering_plan.operations |= map(select(.kind != "loop"))' "$tmpdir/flowcat.optimized.json" > "$tmpdir/flowcat-loopless.optimized.json"
if "$lowerer" --emit-llvm "$tmpdir/flowcat-loopless.ll" --binding-report "$tmpdir/flowcat.binding.json" < "$tmpdir/flowcat-loopless.optimized.json" >/dev/null 2>&1; then
    echo 'file-copy lowering accepted a capability set without source loops' >&2
    exit 1
fi

"$lowerer" --emit-llvm "$tmpdir/flowcat.ll" --binding-report "$tmpdir/flowcat.binding.json" < "$tmpdir/flowcat.optimized.json" > "$tmpdir/flowcat.lowering.json"
grep -q '"status": "emitted"' "$tmpdir/flowcat.lowering.json"
grep -q 'generic structured lowering plan' "$tmpdir/flowcat.ll"
clang "$tmpdir/flowcat.ll" -o "$tmpdir/flowcat"
printf '%s\n' alpha > "$tmpdir/alpha.txt"
printf '%s\n' beta > "$tmpdir/beta.txt"
"$tmpdir/flowcat" "$tmpdir/alpha.txt" "$tmpdir/beta.txt" > "$tmpdir/flowcat.output"
printf '%s\n' alpha beta | cmp -s - "$tmpdir/flowcat.output"
dd if=/dev/zero of="$tmpdir/large.bin" bs=1048576 count=2 status=none
"$tmpdir/flowcat" "$tmpdir/large.bin" > "$tmpdir/large.output"
cmp -s "$tmpdir/large.bin" "$tmpdir/large.output"

sed \
    -e "s|\"../../../std/|\"$root/Lyraform/compiler/std/|" \
    -e 's/^program flowcat$/program arbitrary_stream_copy/' \
    "$flowcat_source" > "$tmpdir/arbitrary-stream-copy.flow"
"$flowmini" --dump-frontend-bundle "$tmpdir/arbitrary-stream-copy.flow" | "$analyst" > "$tmpdir/arbitrary-stream-copy.semantic.json"
jq -e '([.lowering_plan.operations[] | select(.kind == "loop")] | length) == 2' "$tmpdir/arbitrary-stream-copy.semantic.json" >/dev/null
"$bind" --policy "$policy" < "$tmpdir/arbitrary-stream-copy.semantic.json" > "$tmpdir/arbitrary-stream-copy.binding.json"
"$parallel" < "$tmpdir/arbitrary-stream-copy.semantic.json" | "$optimizer" > "$tmpdir/arbitrary-stream-copy.optimized.json"
"$lowerer" --emit-llvm "$tmpdir/arbitrary-stream-copy.ll" --binding-report "$tmpdir/arbitrary-stream-copy.binding.json" < "$tmpdir/arbitrary-stream-copy.optimized.json" >/dev/null
grep -q 'generic structured lowering plan' "$tmpdir/arbitrary-stream-copy.ll"
"$parallel" < "$tmpdir/trial.semantic.json" | "$optimizer" > "$tmpdir/trial.optimization.json"
target_report=$(jq '.targets = [{"symbol_id":0,"name":"cli","main_count":1},{"symbol_id":1,"name":"daemon","main_count":1}]' "$tmpdir/trial.optimization.json")
printf '%s\n' "$target_report" | "$lowerer" --target cli > "$tmpdir/target-cli.json"
jq -e '.status == "ready" and .target.name == "cli"' "$tmpdir/target-cli.json" >/dev/null
set +e
printf '%s\n' "$target_report" | "$lowerer" > "$tmpdir/target-missing.json"
target_rc=$?
set -e
test "$target_rc" -eq 2
jq -e '.status == "blocked" and (.reason | contains("explicit --target"))' "$tmpdir/target-missing.json" >/dev/null
target_artifact_report=$target_report
printf '%s\n' "$target_artifact_report" | "$lowerer" --target cli --emit-llvm "$tmpdir/cli.ll" > "$tmpdir/cli-lowering.json"
printf '%s\n' "$target_artifact_report" | "$lowerer" --target daemon --emit-llvm "$tmpdir/daemon.ll" > "$tmpdir/daemon-lowering.json"
jq -e '.status == "ready" and .target.name == "cli" and .artifact.target_specific == true and .artifact.status == "emitted"' "$tmpdir/cli-lowering.json" >/dev/null
jq -e '.status == "ready" and .target.name == "daemon" and .artifact.target_specific == true and .artifact.status == "emitted"' "$tmpdir/daemon-lowering.json" >/dev/null
grep -q '; Flowcore target artifact: cli' "$tmpdir/cli.ll"
grep -q '; Flowcore target artifact: daemon' "$tmpdir/daemon.ll"
if cmp -s "$tmpdir/cli.ll" "$tmpdir/daemon.ll"; then
    echo 'target artifacts unexpectedly identical' >&2
    exit 1
fi
clang "$tmpdir/cli.ll" -o "$tmpdir/cli"
clang "$tmpdir/daemon.ll" -o "$tmpdir/daemon"
"$tmpdir/cli"
"$tmpdir/daemon"

set +e
printf '%s' '{"format":"wrong","version":1}' | "$lowerer" --diagnostics json >"$tmpdir/hostile-stdout" 2>"$tmpdir/hostile-stderr"
hostile_rc=$?
set -e
test "$hostile_rc" -eq 1
test ! -s "$tmpdir/hostile-stdout"
jq -e '.status == "failed" and .code == "FLOWLOWER_INPUT_INVALID" and .stage == "input" and (.message | length > 0) and .disposition == "no_artifact"' "$tmpdir/hostile-stderr" >/dev/null

set +e
"$lowerer" --diagnostics json "$tmpdir/missing-report.json" >"$tmpdir/missing-stdout" 2>"$tmpdir/missing-stderr"
missing_rc=$?
set -e
test "$missing_rc" -eq 1
test ! -s "$tmpdir/missing-stdout"
jq -e '.status == "failed" and .code == "FLOWLOWER_INPUT_INVALID" and .stage == "input" and .disposition == "no_artifact"' "$tmpdir/missing-stderr" >/dev/null
echo 'Flowlower tests: PASS'
