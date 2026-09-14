#!/bin/sh
set -eu

output=""
while [ "$#" -gt 0 ]; do
    argument=$1
    case "$argument" in
        --output)
            shift
            output=${1:?--output requires a path}
            ;;
        -h|--help)
            cat <<'EOF'
scan-native-bindings - inventory host shared libraries and C/C++ headers

Usage: scan-native-bindings [--output inventory.json]

The result is discovery evidence only. It never authorizes, loads, calls, or
generates bindings for a provider.
EOF
            exit 0
            ;;
        -a|--about)
            printf '%s\n' 'Inventory shared-library and C/C++ header candidates for later Lyraform binding review.'
            exit 0
            ;;
        -v|--version)
            printf '%s\n' '0.1.0'
            exit 0
            ;;
        *)
            printf 'scan-native-bindings: unknown option: %s\n' "$argument" >&2
            exit 2
            ;;
    esac
    shift
done

tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT

ldconfig -p 2>/dev/null | awk '
    NR > 1 && /=>/ {
        soname=$1; arch=$2; path=$NF;
        if (soname ~ /^lib.*\.so(\.|$)/ && path ~ /^\//) print soname "\t" path "\t" arch;
    }
' | sort -u > "$tmpdir/libraries.tsv"

find /usr/include /usr/local/include -type f \( \
    -name '*.h' -o -name '*.hh' -o -name '*.hpp' -o -name '*.hxx' \
\) -readable -print 2>/dev/null | sort -u > "$tmpdir/headers.txt"

jq -Rn '[inputs | split("\t") | {soname: .[0], path: .[1], architecture: .[2]}]' < "$tmpdir/libraries.tsv" > "$tmpdir/libraries.json"
jq -Rn '[inputs | {path: ., language: (if test("\\.(hh|hpp|hxx)$") then "cpp" else "c" end)}]' < "$tmpdir/headers.txt" > "$tmpdir/headers.json"
if command -v pkg-config >/dev/null 2>&1; then
    pkg-config --list-all 2>/dev/null | awk '{name=$1; $1=""; sub(/^ /,""); print name "\t" $0}' | jq -Rn '[inputs | split("\t") | {name: .[0], description: (.[1:] | join(" "))}]' > "$tmpdir/packages.json"
else
    printf '[]\n' > "$tmpdir/packages.json"
fi

artifact=$(jq -n \
    --arg generated_at "$(date -u +%Y-%m-%dT%H:%M:%SZ)" \
    --slurpfile libraries "$tmpdir/libraries.json" \
    --slurpfile headers "$tmpdir/headers.json" \
    --slurpfile packages "$tmpdir/packages.json" \
    '{format:"flowcore.native_binding_inventory",version:1,
      status:"inventory-only",
      generated_at:$generated_at,
      discovery:{library_provider:"ldconfig",header_roots:["/usr/include","/usr/local/include"],package_provider:"pkg-config"},
      libraries:$libraries[0],headers:$headers[0],development_packages:$packages[0],
      policy:{authorization:"not-performed",execution:"not-performed",binding_generation:"not-performed"}}')

if [ -n "$output" ]; then
    printf '%s\n' "$artifact" > "$output"
else
    printf '%s\n' "$artifact"
fi
