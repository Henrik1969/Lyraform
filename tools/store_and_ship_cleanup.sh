#!/usr/bin/env bash
set -euo pipefail

# store-and-ship-clean.sh
#
# Clean local build/output artifacts from the Lyraform/Flowmini lab tree so the
# workspace is in a clean "store and ship" state.
#
# Default mode is DRY RUN. Use --apply to actually delete.

usage() {
    cat <<'USAGE'
Usage:
  tools/store-and-ship-clean.sh [--apply] [--root PATH]
  tools/store-and-ship-clean.sh --dry-run [--root PATH]

Options:
  --dry-run      Show what would be removed. This is the default.
  --apply        Actually remove matched artifacts.
  --root PATH    Project root to clean. Defaults to parent of this script's dir.
  -h, --help     Show this help.

Purpose:
  Remove local CMake/IDE build state and compiled artifacts while keeping source,
  examples, docs, std libraries, tests, and subproject code intact.
USAGE
}

apply=0
script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
root="$(cd -- "${script_dir}/.." && pwd)"

while [[ $# -gt 0 ]]; do
    case "$1" in
        --dry-run)
            apply=0
            shift
            ;;
        --apply)
            apply=1
            shift
            ;;
        --root)
            if [[ $# -lt 2 ]]; then
                echo "error: --root requires a path" >&2
                exit 2
            fi
            root="$(cd -- "$2" && pwd)"
            shift 2
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            echo "error: unknown argument: $1" >&2
            usage >&2
            exit 2
            ;;
    esac
done

if [[ ! -d "$root" ]]; then
    echo "error: root is not a directory: $root" >&2
    exit 2
fi

if [[ "$root" == "/" ]]; then
    echo "error: refusing to clean filesystem root" >&2
    exit 2
fi

run_rm() {
    local path="$1"

    # Safety rail: only remove paths inside the selected root.
    case "$path" in
        "$root"/*) ;;
        *)
            echo "skip outside root: $path" >&2
            return 0
            ;;
    esac

    if [[ $apply -eq 1 ]]; then
        rm -rf -- "$path"
        printf 'removed: %s\n' "$path"
    else
        printf 'would remove: %s\n' "$path"
    fi
}

echo "store-and-ship clean"
echo "root: $root"
if [[ $apply -eq 1 ]]; then
    echo "mode: APPLY"
else
    echo "mode: DRY RUN"
fi
echo

# Whole generated build directories. These are local working state.
while IFS= read -r -d '' dir; do
    run_rm "$dir"
done < <(
    find "$root" \
        -path "$root/.git" -prune -o \
        -type d \( \
            -name build -o \
            -name 'cmake-build-*' -o \
            -name CMakeFiles -o \
            -name CMakeTmp -o \
            -name Testing \
        \) -print0
)

# Stray generated CMake files and compiled outputs outside removed build dirs.
while IFS= read -r -d '' file; do
    run_rm "$file"
done < <(
    find "$root" \
        -path "$root/.git" -prune -o \
        -type f \( \
            -name CMakeCache.txt -o \
            -name cmake_install.cmake -o \
            -name CTestTestfile.cmake -o \
            -name DartConfiguration.tcl -o \
            -name compile_commands.json -o \
            -name '*.o' -o \
            -name '*.o.d' -o \
            -name '*.a' -o \
            -name '*.so' -o \
            -name '*.dylib' -o \
            -name '*.dll' -o \
            -name '*.exe' \
        \) -print0
)

echo
if [[ $apply -eq 1 ]]; then
    echo "store-and-ship clean complete."
else
    echo "dry run complete. Re-run with --apply to remove these paths."
fi
