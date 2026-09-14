#!/usr/bin/env bash
set -eu

fault=$1
tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT
policy="$tmpdir/policy.txt"
printf '%s\n' 'allow libc.so.6 flow_text_concat_value c memory Text,Text TextOutcome' > "$policy"
"$fault" "$policy" | grep -q 'allocation fault refused with no outcome'
