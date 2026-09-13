#!/usr/bin/env bash
set -u -o pipefail

root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
probe_dir=$(mktemp -d "${TMPDIR:-/tmp}/lyraform-prerequisites.XXXXXX")
trap 'rm -rf "$probe_dir"' EXIT

failures=0

pass() {
    printf 'PASS  %s\n' "$*"
}

fail() {
    printf 'FAIL  %s\n' "$*" >&2
    failures=$((failures + 1))
}

warn() {
    printf 'WARN  %s\n' "$*"
}

check_command() {
    local command=$1
    if command -v "$command" >/dev/null 2>&1; then
        pass "$command: $(command -v "$command")"
    else
        fail "$command is not installed"
    fi
}

printf '%s\n' "Lyraform/Igor tester prerequisites"
printf '%s\n' "repository: $root"
printf '%s\n' ""

for command in \
    bash cmake ctest ninja git cc c++ clang jq python3 file script timeout sha256sum; do
    check_command "$command"
done

if command -v cmake >/dev/null 2>&1; then
    version=$(cmake --version | sed -n '1s/[^0-9]*//p')
    major=${version%%.*}
    remainder=${version#*.}
    minor=${remainder%%.*}
    if [ "${major:-0}" -gt 3 ] || \
        { [ "${major:-0}" -eq 3 ] && [ "${minor:-0}" -ge 22 ]; }; then
        pass "CMake version $version (>= 3.22)"
    else
        fail "CMake version $version is older than the required 3.22"
    fi
fi

if command -v cmake >/dev/null 2>&1 && command -v ninja >/dev/null 2>&1; then
    if cmake -S "$root" -B "$probe_dir/build" -G Ninja \
        -DCMAKE_BUILD_TYPE=RelWithDebInfo >/dev/null 2>&1; then
        pass "CMake resolves OpenSSL, Threads, and the Lyraform build graph"
    else
        fail "CMake could not configure the graph; inspect the configure output"
    fi
fi

if command -v c++ >/dev/null 2>&1; then
    if printf '%s\n' 'int main(void) { return 0; }' |
        c++ -x c++ - -o "$probe_dir/cxx-probe" >/dev/null 2>&1; then
        pass "C++ compiler can link a native executable"
    else
        fail "C++ compiler cannot link a native executable"
    fi
fi

if command -v cc >/dev/null 2>&1; then
    if printf '%s\n' 'int main(void) { return 0; }' |
        cc -x c - -lncursesw -o "$probe_dir/ncurses-probe" >/dev/null 2>&1; then
        pass "libncursesw is linkable for the terminal acceptance tests"
    else
        fail "libncursesw is not linkable; install the ncurses wide-character development package"
    fi
fi

if command -v valgrind >/dev/null 2>&1; then
    pass "valgrind: $(command -v valgrind) (optional review gate available)"
else
    warn "valgrind is not installed (optional review gate unavailable)"
fi

if [ "$failures" -eq 0 ]; then
    printf '%s\n' "PASS  all required onboarding prerequisites are available"
else
    printf '%s\n' "FAIL  $failures required prerequisite check(s) failed" >&2
    exit 1
fi
