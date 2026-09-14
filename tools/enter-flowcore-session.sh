#!/usr/bin/env bash
set -euo pipefail

# enter-flowcore-session.sh (compatibility name)
#
# Enter a Lyraform project session. The script name and FLOWMINI_* variables
# remain compatibility surfaces for existing developer workflows.
#
# Intended use:
#   source tools/enter-flowcore-session.sh
#   source tools/enter-flowcore-session.sh --note
#   eval "$(tools/enter-flowcore-session.sh --print-env)"
#
# This script avoids editor/window/Obsidian/plugin assumptions.
# It prepares project coordinates; cockpit layout stays local.

SCRIPT_PATH="${BASH_SOURCE[0]}"
SCRIPT_DIR="$(cd -- "$(dirname -- "$SCRIPT_PATH")" && pwd)"
DEFAULT_TOP="$(cd -- "$SCRIPT_DIR/.." && pwd)"

TOP_ARG=""
FLOWMINI_ARG=""
CREATE_NOTE=0
PRINT_ENV=0
RUN_SUITE=0
QUIET=0

usage() {
    cat <<'USAGE'
Usage:
  source tools/enter-flowcore-session.sh [options]
  tools/enter-flowcore-session.sh [options]

Options:
  --top DIR          Project root. Default: parent of tools/
  --flowmini DIR     Active compiler root. Default: Lyraform/compiler
  --note             Create a dated session note under docs/sessions/
  --suite            Run the Lyraform compatibility suite after entering.
  --print-env        Print export commands and exit. Useful with eval.
  --quiet            Suppress dashboard.
  -h, --help         Show this help.

Exports:
  TOP
  FLOWLAB_TOP
  FLOWMINI_ROOT
  FLOWMINI_STDIN
  FLOWMINI_BUILD_DIR
  FLOWMINI_EXAMPLES_DIR
  FLOWMINI_TESTS_DIR
  FLOWMINI_REPORT_DIR
  FLOWCORE_DOCS
  FLOWCORE_NOTES
  FLOWCORE_SESSIONS
  FLOWCORE_TOOLS
  FLOWCORE_SESSION_NOTE, only with --note
USAGE
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --top) TOP_ARG="$2"; shift 2 ;;
        --flowmini) FLOWMINI_ARG="$2"; shift 2 ;;
        --note) CREATE_NOTE=1; shift ;;
        --suite) RUN_SUITE=1; shift ;;
        --print-env) PRINT_ENV=1; shift ;;
        --quiet) QUIET=1; shift ;;
        -h|--help) usage; return 0 2>/dev/null || exit 0 ;;
        *) echo "unknown argument: $1" >&2; usage >&2; return 2 2>/dev/null || exit 2 ;;
    esac
done

resolve_abs() {
    local p="$1"
    cd -- "$p" && pwd
}

TOP_RESOLVED="${TOP_ARG:-${TOP:-$DEFAULT_TOP}}"
TOP_RESOLVED="$(resolve_abs "$TOP_RESOLVED")"

discover_flowmini_from_current() {
    if [[ -d "$TOP_RESOLVED/Lyraform/compiler" ]]; then
        printf "%s/Lyraform/compiler" "$TOP_RESOLVED"
        return
    fi

    if [[ -n "${FLOWMINI_ROOT:-}" && -d "${FLOWMINI_ROOT:-}" ]]; then
        resolve_abs "$FLOWMINI_ROOT"
        return
    fi

    find "$TOP_RESOLVED/Lyraform" -maxdepth 1 -type d -name 'flowmini_v*' 2>/dev/null | sort -V | tail -n 1
}

if [[ -n "$FLOWMINI_ARG" ]]; then
    FLOWMINI_RESOLVED="$(resolve_abs "$FLOWMINI_ARG")"
else
    FLOWMINI_RESOLVED="$(discover_flowmini_from_current)"
fi

if [[ ! -d "$FLOWMINI_RESOLVED" ]]; then
    echo "error: could not resolve Lyraform compiler root: $FLOWMINI_RESOLVED" >&2
    return 2 2>/dev/null || exit 2
fi

export TOP="$TOP_RESOLVED"
export FLOWLAB_TOP="$TOP_RESOLVED"
export FLOWMINI_ROOT="$FLOWMINI_RESOLVED"
export FLOWMINI_STDIN="${FLOWMINI_STDIN:-5}"
export FLOWMINI_BUILD_DIR="${FLOWMINI_BUILD_DIR:-build}"
export FLOWMINI_EXAMPLES_DIR="${FLOWMINI_EXAMPLES_DIR:-examples}"
export FLOWMINI_TESTS_DIR="${FLOWMINI_TESTS_DIR:-tests}"
export FLOWMINI_REPORT_DIR="${FLOWMINI_REPORT_DIR:-test-report}"
export FLOWCORE_DOCS="$TOP_RESOLVED/docs"
export FLOWCORE_NOTES="$TOP_RESOLVED/docs/notes"
export FLOWCORE_SESSIONS="$TOP_RESOLVED/docs/sessions"
export FLOWCORE_TOOLS="$TOP_RESOLVED/tools"

print_env() {
    cat <<EOF
export TOP="$TOP"
export FLOWLAB_TOP="$FLOWLAB_TOP"
export FLOWMINI_ROOT="$FLOWMINI_ROOT"
export FLOWMINI_STDIN="$FLOWMINI_STDIN"
export FLOWMINI_BUILD_DIR="$FLOWMINI_BUILD_DIR"
export FLOWMINI_EXAMPLES_DIR="$FLOWMINI_EXAMPLES_DIR"
export FLOWMINI_TESTS_DIR="$FLOWMINI_TESTS_DIR"
export FLOWMINI_REPORT_DIR="$FLOWMINI_REPORT_DIR"
export FLOWCORE_DOCS="$FLOWCORE_DOCS"
export FLOWCORE_NOTES="$FLOWCORE_NOTES"
export FLOWCORE_SESSIONS="$FLOWCORE_SESSIONS"
export FLOWCORE_TOOLS="$FLOWCORE_TOOLS"
EOF
}

if [[ "$PRINT_ENV" -eq 1 ]]; then
    print_env
    return 0 2>/dev/null || exit 0
fi

short_git_status() {
    if git -C "$TOP" rev-parse --is-inside-work-tree >/dev/null 2>&1; then
        local branch dirty sb
        branch="$(git -C "$TOP" branch --show-current 2>/dev/null || echo '?')"
        dirty="$(git -C "$TOP" status --porcelain | wc -l | tr -d ' ')"
        sb="$(git -C "$TOP" status -sb | head -n 1)"
        printf "%s, changed paths: %s\n    %s" "$branch" "$dirty" "$sb"
    else
        printf "not a git worktree"
    fi
}

latest_session_note_name() {
    mkdir -p "$FLOWCORE_SESSIONS"
    local date_part title file n
    date_part="$(date +%Y-%m-%d)"
    title="session"
    file="$FLOWCORE_SESSIONS/${date_part}-${title}.md"

    if [[ ! -e "$file" ]]; then
        printf "%s" "$file"
        return
    fi

    n=2
    while [[ -e "$FLOWCORE_SESSIONS/${date_part}-${title}-${n}.md" ]]; do
        n=$((n + 1))
    done
    printf "%s" "$FLOWCORE_SESSIONS/${date_part}-${title}-${n}.md"
}

create_session_note() {
    mkdir -p "$FLOWCORE_SESSIONS"
    local note git_line
    note="$(latest_session_note_name)"
    git_line="$(git -C "$TOP" status -sb 2>/dev/null | head -n 1 || echo "not available")"

    cat > "$note" <<EOF
---
title: Lyraform session $(date +%Y-%m-%d)
status: draft
kind: session-note
tags:
  - flowcore
  - session
  - flowmini
---

# Lyraform session $(date +%Y-%m-%d)

## Starting point

- TOP: \`$TOP\`
- FLOWMINI_ROOT: \`$FLOWMINI_ROOT\`
- Git: $git_line

## Goal

- 

## Decisions

- 

## Files changed

- 

## Tests run

\`\`\`bash
$TOP/tools/run-flowmini-test-suite.sh
\`\`\`

## Result

- 

## Next

- 

## Related

- [Lyraform compiler current](../../Lyraform/CURRENT.md)
- [Flowmini compatibility roadmap](../flowmini/roadmap.md)
- [Flowmini compatibility testing](../flowmini/testing.md)
EOF

    printf "%s" "$note"
}

print_dashboard() {
    echo "== Lyraform session =="
    echo "TOP:             $TOP"
    echo "FLOWMINI_ROOT:   $FLOWMINI_ROOT"
    echo "DOCS:            $FLOWCORE_DOCS"
    echo "NOTES:           $FLOWCORE_NOTES"
    echo "SESSIONS:        $FLOWCORE_SESSIONS"
    echo "TOOLS:           $FLOWCORE_TOOLS"
    echo
    echo "Git:"
    short_git_status | sed 's/^/    /'
    echo
    echo "Current Lyraform compiler:"
    if [[ -f "$TOP/Lyraform/CURRENT.md" ]]; then
        sed -n '1,34p' "$TOP/Lyraform/CURRENT.md" | sed 's/^/    /'
    else
        echo "    no Lyraform/CURRENT.md"
    fi
    echo
    echo "Useful commands:"
    echo "    \$TOP/tools/run-flowmini-test-suite.sh"
    echo "    FLOWMINI_NO_BUILD=1 \$TOP/tools/run-flowmini-test-suite.sh"
    echo "    FLOWMINI_VALGRIND=1 \$TOP/tools/run-flowmini-test-suite.sh"
    echo "    nvim \$FLOWCORE_DOCS/index.md"
    echo "    nvim \$TOP/backlog.txt"
}

if [[ "$CREATE_NOTE" -eq 1 ]]; then
    note_path="$(create_session_note)"
    export FLOWCORE_SESSION_NOTE="$note_path"
fi

if [[ "$QUIET" -eq 0 ]]; then
    print_dashboard
    if [[ "${FLOWCORE_SESSION_NOTE:-}" != "" ]]; then
        echo
        echo "Session note:"
        echo "    $FLOWCORE_SESSION_NOTE"
    fi
fi

if [[ "$RUN_SUITE" -eq 1 ]]; then
    "$TOP/tools/run-flowmini-test-suite.sh"
fi

return 0 2>/dev/null || exit 0
