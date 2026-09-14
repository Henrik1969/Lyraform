#!/usr/bin/env bash
set -eu

roundtrip=$1
fault=$2
build_dir=$3
tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT
artifact="$tmpdir/input.tvm"
"$roundtrip" "$artifact" --keep --no-import >/dev/null
"$fault" "$artifact" >"$tmpdir/stdout" 2>"$tmpdir/stderr"
grep -q 'TinyVM artifact v2 allocation fault refused with no artifact' "$tmpdir/stdout"
