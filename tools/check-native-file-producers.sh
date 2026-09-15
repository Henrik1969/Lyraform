#!/usr/bin/env bash
set -euo pipefail

root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$root"

source_roots=(
    Flowanalyst/src
    Flowbind/src
    Flowcontracts/src
    Flowkernel/src
    Flowlower/src
    Flowoptimize/src
    Flowparallel/src
    Frankencore/Core
    Lyraform/compiler/src
    subprojects/TinyVM/src
    subprojects/TinyVM/tools
)

mapfile -t observed < <(
    rg -l 'std::ofstream|fopen\s*\([^,]+,\s*"(w|a|wb|ab)|O_CREAT|O_WRONLY|O_RDWR' \
        "${source_roots[@]}" \
        --glob '*.{c,cc,cpp,cxx,h,hpp}' \
        --glob '!**/tests/**' |
        sort
)

expected=(
    Flowkernel/src/main.cpp
    Flowlower/src/main.cpp
    Frankencore/Core/Provenance/provenance.cpp
    Lyraform/compiler/src/main.cpp
    subprojects/TinyVM/src/artifact.c
    subprojects/TinyVM/src/artifact_v2.c
)

if [[ "${observed[*]}" != "${expected[*]}" ]]; then
    echo "native file-producer inventory changed" >&2
    echo "expected:" >&2
    printf '  %s\n' "${expected[@]}" >&2
    echo "observed:" >&2
    printf '  %s\n' "${observed[@]}" >&2
    exit 1
fi

compiler_publishers=(
    Flowlower/src/main.cpp
    Lyraform/compiler/src/main.cpp
    subprojects/TinyVM/src/artifact.c
    subprojects/TinyVM/src/artifact_v2.c
)
for source in "${compiler_publishers[@]}"; do
    rg -q 'tmp\.lyraform-v1' "$source"
    rg -q 'flock\(directory,\s*LOCK_EX\)' "$source"
    rg -q 'openat\(directory' "$source"
    rg -q 'renameat\(directory' "$source"
    rg -q 'fsync\(directory\)' "$source"
done

rg -q 'append_serialized' Frankencore/Core/Provenance/provenance.cpp
rg -q 'sync_parent_directory' Frankencore/Core/Provenance/provenance.cpp
rg -q 'mkdtemp' Flowkernel/src/main.cpp
rg -q 'unlinkat\(directory, "probe\.bin"' Flowkernel/src/main.cpp
rg -q 'rmdir\(directory_template\)' Flowkernel/src/main.cpp

echo "Native file-producer inventory: PASS (6 admitted production sources)"
