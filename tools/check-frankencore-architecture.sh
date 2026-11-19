#!/usr/bin/env bash
set -euo pipefail

root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
strict=0
if [[ "${1:-}" == "--strict" ]]; then strict=1; elif [[ "$#" -ne 0 ]]; then
    echo "usage: $0 [--strict]" >&2
    exit 2
fi

inventory="$root/docs/architecture/frankencore-contract-inventory.json"
if command -v jq >/dev/null 2>&1; then
    jq empty "$inventory"
else
    echo "architecture check: jq unavailable; inventory syntax cannot be validated" >&2
    exit 2
fi

debt=0
active_cmake=(
    "$root/Flowanalyst/CMakeLists.txt"
    "$root/Flowbind/CMakeLists.txt"
    "$root/Flowoptimize/CMakeLists.txt"
    "$root/Flowlower/CMakeLists.txt"
    "$root/Flowkernel/CMakeLists.txt"
    "$root/Lyraform/compiler/CMakeLists.txt"
)

echo "Lyraform capability architecture checks"
echo "  scope: build paths and presentation boundaries; not constitutional certification"
echo "  contract inventory JSON syntax: PASS"

if rg -n '\.\./(Flowmini|Flowanalyst|Flowbind|Flowoptimize|Flowlower)/((cmake-)?build[^/]*)' "${active_cmake[@]}"; then
    debt=1
    echo "  build-path debt: PRESENT"
else
    echo "  build-path debt: none"
fi

semantic_dirs=(
    "$root/Flowanalyst/src"
    "$root/Flowbind/src"
    "$root/Flowoptimize/src"
    "$root/Flowlower/src"
    "$root/Lyraform/compiler/src"
)
if rg -n '#include [<"](gtk|Qt|ncurses|wayland|X11|xcb)' "${semantic_dirs[@]}"; then
    debt=1
    echo "  presentation leakage: PRESENT"
else
    echo "  presentation leakage: none"
fi

if (( strict && debt )); then
    echo "architecture check: FAIL (known debt requires remediation)" >&2
    exit 1
fi
if (( debt )); then
    echo "architecture check: PASS WITH RECORDED DEBT"
else
    echo "architecture check: PASS"
fi
