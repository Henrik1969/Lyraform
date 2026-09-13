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
    'allow libc.so.6 unlinkat c filesystem' > "$policy"
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
    "$analyst" < "$bundle" > "$semantic"
    grep -q '"status": "ok"' "$semantic"
    "$parallel" < "$semantic" > "$tmpdir/$name.parallel.json"
    "$optimizer" < "$tmpdir/$name.parallel.json" > "$optimized"
    jq -e '.status == "ready"' "$optimized" >/dev/null
    case "$name" in
        abi_abs_main|abi_strlen_main|test_licbinds|text_value|abi_ncurses_main|abi_kernel_getpid_main|abi_kernel_getuid_main|abi_kernel_getgid_main|abi_kernel_geteuid_main|abi_kernel_getegid_main|abi_kernel_getppid_main|abi_kernel_getpgrp_main|abi_kernel_getpgid_main|abi_kernel_getsid_main|abi_kernel_getpriority_main|abi_kernel_clock_main|abi_kernel_random_main|abi_kernel_uname_main|abi_kernel_openat_main|abi_kernel_read_main|abi_kernel_write_main|abi_kernel_lseek_main|abi_kernel_unlinkat_main|abi_kernel_rmdir_main|abi_kernel_pipe2_main|abi_kernel_fork_main|abi_kernel_waitpid_main|abi_kernel_socketpair_main|abi_kernel_socket_main|abi_kernel_bind_main|abi_kernel_listen_main|abi_kernel_poll_main|abi_kernel_accept4_main|abi_kernel_connect_main|abi_kernel_unshare_main|abi_kernel_sethostname_main|abi_kernel_gethostname_main)
        binding=$tmpdir/$name.binding.json
        "$bind" --policy "$policy" < "$semantic" > "$binding"
        "$lowerer" --binding-report "$binding" < "$optimized" > "$lowered"
        ;;
    *)
        "$lowerer" < "$optimized" > "$lowered"
        ;;
    esac
    grep -q '"status": "ready"' "$lowered"
    count=$((count + 1))
done

echo "Flowcore pass corpus: $count programs passed semantic and lowering boundaries"
