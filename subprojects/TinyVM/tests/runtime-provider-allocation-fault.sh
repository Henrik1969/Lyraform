#!/usr/bin/env bash
set -eu

fault=$1
tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT
policy="$tmpdir/policy.txt"
printf '%s\n' \
    'allow libc.so.6 strlen c pure c_string c_size_t' \
    'allow libc.so.6 flow_text_concat_value c memory Text,Text TextOutcome' > "$policy"
"$fault" "$policy" | grep -q 'allocation faults refused with no outcome'
