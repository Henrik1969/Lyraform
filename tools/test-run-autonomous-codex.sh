#!/bin/sh
set -eu

root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT HUP INT TERM
repo=$tmpdir/repo
fakebin=$tmpdir/fake-bin
mkdir -p "$repo/tasks" "$repo/docs" "$fakebin"
git -C "$repo" init -q
git -C "$repo" config user.email test@example.invalid
git -C "$repo" config user.name runner-test
printf '%s\n' '# test mission' >"$repo/tasks/task.md"
printf '%s\n' '# architecture' >"$repo/docs/architecture.md"
printf '%s\n' '# ledger' >"$repo/docs/ledger.md"
cp "$root/tools/run-autonomous-codex" "$repo/runner"
chmod +x "$repo/runner"
git -C "$repo" add .
git -C "$repo" commit -qm baseline

# This stub is the only codex command visible to the runner tests.
cat >"$fakebin/codex" <<'EOF'
#!/bin/sh
printf '%s\n' "$*" >>"$FAKE_CODEX_LOG"
exit 0
EOF
chmod +x "$fakebin/codex"
export FAKE_CODEX_LOG=$tmpdir/codex.log
export PATH=$fakebin:$PATH

expect_rc() {
    expected=$1
    shift
    set +e
    "$@" >"$tmpdir/stdout" 2>"$tmpdir/stderr"
    actual=$?
    set -e
    test "$actual" -eq "$expected" || {
        echo "expected exit $expected, got $actual: $*" >&2
        cat "$tmpdir/stderr" >&2
        exit 1
    }
}

expect_rc 64 sh -c "cd '$repo' && ./runner"
outside=$tmpdir/outside.md
printf '%s\n' outside >"$outside"
expect_rc 66 sh -c "cd '$repo' && ./runner --task '$outside'"

# A missing default state is initialized below .git and never appears in status.
expect_rc 4 sh -c "cd '$repo' && LYRAFORM_CODEX_MAX_ITERATIONS=1 LYRAFORM_CODEX_STALL_LIMIT=1 ./runner --task tasks/task.md"
default_state=$(cd "$repo" && git rev-parse --git-path codex/run-state)
case "$default_state" in /*) ;; *) default_state=$repo/$default_state ;; esac
test "$(tr -d '[:space:]' <"$default_state")" = CONTINUE
test -z "$(git -C "$repo" status --porcelain)"

state=$tmpdir/state
printf '%s\n' BROKEN >"$state"
expect_rc 65 sh -c "cd '$repo' && ./runner --task tasks/task.md --state-file '$state'"
printf '%s\n' DONE >"$state"
before_log=$(wc -l <"$FAKE_CODEX_LOG" 2>/dev/null || printf 0)
expect_rc 0 sh -c "cd '$repo' && ./runner --task tasks/task.md --state-file '$state' --architecture docs/architecture.md --ledger docs/ledger.md --context docs/ledger.md"
test "$(wc -l <"$FAKE_CODEX_LOG")" -eq "$before_log"
printf '%s\n' BLOCKED >"$state"
expect_rc 2 sh -c "cd '$repo' && ./runner --task tasks/task.md --state-file '$state'"

echo 'run-autonomous-codex focused tests: PASS'
