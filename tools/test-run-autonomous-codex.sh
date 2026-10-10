#!/bin/sh
set -eu

root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT HUP INT TERM
repo=$tmpdir/repo
fakebin=$tmpdir/fake-bin
mkdir -p "$repo/docs/tasks" "$repo/docs/checkpoints" "$repo/docs/context" "$fakebin"
git -C "$repo" init -q
git -C "$repo" config user.email test@example.invalid
git -C "$repo" config user.name runner-test

printf '%s\n' '# Test mission' '## Convergence contract' >"$repo/docs/tasks/task.md"
printf '%s\n' '# Second mission' '## Convergence contract' >"$repo/docs/tasks/second.md"
printf '%s\n' '# Missing contract' >"$repo/docs/tasks/no-contract.md"
printf '%s\n' '# checkpoint' >"$repo/docs/checkpoints/checkpoint.md"
printf '%s\n' '# architecture' >"$repo/docs/context/architecture.md"
printf '%s\n' '# ledger' >"$repo/docs/context/ledger.md"
printf '%s\n' '# arbitrary file' >"$repo/arbitrary.md"
printf '%s\n' '# external mission' '## Convergence contract' >"$tmpdir/external.md"
ln -s "$tmpdir/external.md" "$repo/docs/tasks/symlink.md"
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
export LYRAFORM_CODEX_LOG_DIR=$tmpdir/logs

expect_rc() {
    expected=$1
    shift
    set +e
    "$@" >"$tmpdir/stdout" 2>"$tmpdir/stderr"
    actual=$?
    set -e
    test "$actual" -eq "$expected" || {
        echo "expected exit $expected, got $actual: $*" >&2
        cat "$tmpdir/stdout" >&2
        cat "$tmpdir/stderr" >&2
        exit 1
    }
}

expect_rc 64 sh -c "cd '$repo' && ./runner"
expect_rc 66 sh -c "cd '$repo' && ./runner --task '$tmpdir/external.md'"
expect_rc 66 sh -c "cd '$repo' && ./runner --task arbitrary.md"
expect_rc 66 sh -c "cd '$repo' && ./runner --task docs/checkpoints/checkpoint.md"
expect_rc 66 sh -c "cd '$repo' && ./runner --task docs/tasks/symlink.md"
expect_rc 66 sh -c "cd '$repo' && ./runner --task docs/tasks/no-contract.md"

printf '%s\n' '# untracked mission' '## Convergence contract' >"$repo/docs/tasks/untracked.md"
expect_rc 66 sh -c "cd '$repo' && ./runner --task docs/tasks/untracked.md"
rm "$repo/docs/tasks/untracked.md"

# A launch creates mission-specific Git-local state/manifest and no worktree dirt.
expect_rc 4 sh -c "cd '$repo' && LYRAFORM_CODEX_MAX_ITERATIONS=1 LYRAFORM_CODEX_STALL_LIMIT=1 ./runner --task docs/tasks/task.md"
grep -Fq 'Task: docs/tasks/task.md' "$tmpdir/stdout"
grep -Fq 'Mission ID: task-' "$tmpdir/stdout"
state=$(find "$repo/.git/codex/runs" -type f -name state -print)
manifest=$(dirname "$state")/manifest
test -f "$manifest"
test "$(tr -d '[:space:]' <"$state")" = CONTINUE
grep -Fq 'task_path=docs/tasks/task.md' "$manifest"
grep -Fq 'starting_branch=master' "$manifest"
test -z "$(git -C "$repo" status --porcelain)"
test "$(find "$tmpdir/logs" -mindepth 1 -maxdepth 1 -type d | wc -l)" -eq 1

# External state and cross-mission state reuse are refused.
expect_rc 66 sh -c "cd '$repo' && ./runner --task docs/tasks/task.md --state-file '$tmpdir/state'"
expect_rc 65 sh -c "cd '$repo' && ./runner --task docs/tasks/second.md --state-file '$state'"

# A stale or forged manifest is refused deterministically.
cp "$manifest" "$tmpdir/manifest.ok"
printf '%s\n' 'task_path=docs/tasks/task.md' 'task_digest=forged' 'mission_id=forged' 'starting_branch=master' 'starting_head=0000000000000000000000000000000000000000' >"$manifest"
expect_rc 65 sh -c "cd '$repo' && ./runner --task docs/tasks/task.md"
cp "$tmpdir/manifest.ok" "$manifest"

# Valid terminal state never invokes Codex; invalid state is rejected.
printf '%s\n' BROKEN >"$state"
expect_rc 65 sh -c "cd '$repo' && ./runner --task docs/tasks/task.md"
printf '%s\n' DONE >"$state"
before_log=$(wc -l <"$FAKE_CODEX_LOG" 2>/dev/null || printf 0)
expect_rc 0 sh -c "cd '$repo' && ./runner --task docs/tasks/task.md --architecture docs/context/architecture.md --ledger docs/context/ledger.md --context docs/context/ledger.md"
test "$(wc -l <"$FAKE_CODEX_LOG")" -eq "$before_log"
printf '%s\n' BLOCKED >"$state"
expect_rc 2 sh -c "cd '$repo' && ./runner --task docs/tasks/task.md"

echo 'run-autonomous-codex focused tests: PASS'
