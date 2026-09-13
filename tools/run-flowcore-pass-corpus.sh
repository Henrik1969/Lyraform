#!/bin/sh
set -eu

root=${FLOWCORE_ROOT:-$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)}
flowmini=${FLOWMINI_BIN:-$root/Lyraform/compiler/cmake-build-debug/flowmini}
analyst=${FLOWANALYST_BIN:-$root/Flowanalyst/build/flowanalyst}
parallel=${FLOWPARALLEL_BIN:-$root/Flowparallel/build/flowparallel}
optimizer=${FLOWOPTIMIZE_BIN:-$root/Flowoptimize/build/flowoptimize}
lowerer=${FLOWLOWER_BIN:-$root/Flowlower/build/flowlower}
bind=${FLOWBIND_BIN:-$root/Flowbind/build/flowbind}
pass_root=$root/Lyraform/compiler/examples/pass
tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT
policy=$tmpdir/abi.policy
printf '%s\n' \
    'allow libc.so.6 strlen c pure' \
    'allow libc.so.6 strnlen c pure' \
    'allow libc.so.6 abs c pure' \
    'allow libc.so.6 labs c pure' \
    'allow libc.so.6 puts c io' \
    'allow libc.so.6 puts c io Text c_int' \
    'allow libc.so.6 strlen c pure' \
    'allow libc.so.6 abs c pure' \
    'allow libncursesw.so.6 initscr c terminal' \
    'allow libncursesw.so.6 endwin c terminal' \
    'allow libncursesw.so.6 noecho c terminal' \
    'allow libncursesw.so.6 cbreak c terminal' \
    'allow libncursesw.so.6 waddnstr c terminal' \
    'allow libncursesw.so.6 wrefresh c terminal' \
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
    'allow libc.so.6 tolower c pure' \
    'allow libc.so.6 toupper c pure' \
    'allow libm.so.6 floor c pure' \
    'allow libm.so.6 sqrt c pure' \
    'allow libc.so.6 memset c io' \
    'allow libc.so.6 memcpy c io' \
    'allow libc.so.6 memmove c io' \
    'allow libc.so.6 memcmp c pure' \
    'allow libc.so.6 open c io' \
    'allow libc.so.6 read c io' \
    'allow libc.so.6 write c io' \
    'allow libc.so.6 close c io' > "$policy"
for grant in \
    'rmdir filesystem' 'pipe2 process_ipc' 'fork process_ipc' 'waitpid process_ipc' \
    'socketpair socket_ipc' 'socket loopback' 'bind loopback' 'listen loopback' \
    'poll loopback' 'accept4 loopback' 'connect loopback' 'unshare namespace' \
    'sethostname namespace' 'gethostname namespace'; do
    set -- $grant
    printf 'allow libc.so.6 %s c %s\n' "$1" "$2" >> "$policy"
done

count=0
for source in "$pass_root"/*.flow; do
    name=${source##*/}
    name=${name%.flow}
    bundle=$tmpdir/$name.bundle.json
    semantic=$tmpdir/$name.semantic.json
    optimized=$tmpdir/$name.optimized.json
    lowered=$tmpdir/$name.lowered.json

    "$flowmini" --dump-frontend-bundle "$source" > "$bundle"
    "$analyst" --lowering-plan-version 2 < "$bundle" > "$semantic"
    grep -q '"status": "ok"' "$semantic"
    "$parallel" < "$semantic" > "$tmpdir/$name.parallel.json"
    "$optimizer" < "$tmpdir/$name.parallel.json" > "$optimized"
    jq -e '.status == "ready"' "$optimized" >/dev/null
    binding=$tmpdir/$name.binding.json
    if jq -e '((.aggregate_abi_layouts // []) | length) == 0' "$semantic" >/dev/null; then
        "$bind" --policy "$policy" < "$semantic" > "$binding"
        "$lowerer" --binding-report "$binding" < "$optimized" > "$lowered"
    else
        "$lowerer" < "$optimized" > "$lowered"
    fi
    grep -q '"status": "ready"' "$lowered"
    count=$((count + 1))
done

echo "Flowcore pass corpus: $count programs passed semantic and lowering boundaries"
