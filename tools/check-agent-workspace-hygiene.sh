#!/bin/sh
set -eu

root=${FLOWCORE_ROOT:-$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)}

fail() {
    printf 'agent/workspace hygiene: %s\n' "$1" >&2
    exit 1
}

require_text() {
    grep -Fq "$2" "$1" || fail "$1 lacks: $2"
}

test ! -e "$root/.codex-run-state" || fail 'tracked/shared root run state still exists'
test -f "$root/docs/tasks/README.md" || fail 'mission lifecycle index is missing'
test -f "$root/docs/development/mission-convergence-rule.md" || fail 'mission convergence rule is missing'

require_text "$root/AGENTS.md" 'Convergence Rule]'
require_text "$root/AGENTS.md" '.git/codex/runs/<mission-id>/'
require_text "$root/docs/development/project-hygiene.md" 'output/'
require_text "$root/docs/tasks/README.md" 'New-mission convergence contract'
require_text "$root/.gitignore" '.codex-run-state'
require_text "$root/.gitignore" '.agents/'
require_text "$root/.gitignore" '.codex/'
require_text "$root/.gitignore" 'output/'

git -C "$root" ls-files --error-unmatch \
    docs/development/mission-convergence-rule.md \
    docs/tasks/agent-authority-and-workspace-hygiene.md \
    docs/tasks/README.md \
    tools/run-autonomous-codex >/dev/null 2>&1 ||
    fail 'authority or runner file is not tracked'

if git -C "$root" ls-files --error-unmatch .codex-run-state >/dev/null 2>&1; then
    fail '.codex-run-state remains tracked'
fi

printf '%s\n' 'Agent/workspace hygiene: PASS'
