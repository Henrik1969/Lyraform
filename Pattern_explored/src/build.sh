#!/usr/bin/env bash
set -Eeuo pipefail

usage() {
    cat <<'EOF'
Usage: Pattern_explored/src/build.sh [--build-dir DIR]

Build the original pattern experiments into Pattern_explored/build by default.
Sources stay in src and generated binaries/logs never dirty that directory.
EOF
}

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
build_dir=$script_dir/../build
while (($#)); do
    case "$1" in
        --help|-h) usage; exit 0 ;;
        --build-dir)
            (($# >= 2)) || { echo 'error: --build-dir requires a directory' >&2; exit 2; }
            build_dir=$2
            shift 2
            ;;
        *) echo "error: unknown argument '$1'" >&2; exit 2 ;;
    esac
done
if [[ $build_dir != /* ]]; then build_dir=$PWD/$build_dir; fi
mkdir -p "$build_dir"

failed=0
for source in "$script_dir"/*.cpp; do
    [[ -f $source ]] || continue
    name=${source##*/}
    target=${name%.cpp}
    logtarget=$build_dir/$target.log
    echo "Building $name -> $build_dir/$target"
    if g++ -std=c++17 -Wall -Wextra -pedantic "$source" -o "$build_dir/$target" >"$logtarget" 2>&1; then
        echo "OK: $target"
    else
        echo "FAILED: $name (see $logtarget)" >&2
        failed=1
    fi
done
exit "$failed"
