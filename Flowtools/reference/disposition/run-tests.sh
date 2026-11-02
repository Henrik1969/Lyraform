#!/bin/sh
set -eu
build=$1
root=$2
runner=$3
tmpdir=$(mktemp -d /tmp/lyraform-activation-runtime.XXXXXX)
trap 'rm -rf "$tmpdir"' EXIT
for stem in text_outcome_consumer text_outcome_two_routes; do
    specimen="$root/Lyraform/compiler/examples/disposition/$stem.flow"
    policy="$root/Lyraform/compiler/examples/disposition/policy"
    "$build/flowmini/flowmini" --dump-frontend-bundle "$specimen" > "$tmpdir/frontend.json"
    set -- --lowering-plan-version 2 --graph-plan-version 2 --host-disposition-policy "$policy/$stem.host-policy.json"
    if test "$stem" = text_outcome_two_routes; then
        set -- "$@" --disposition-policy "$policy/$stem.policy.json"
    fi
    "$build/flowanalyst/flowanalyst" "$@" "$tmpdir/frontend.json" > "$tmpdir/semantic.json"
    "$runner" "$tmpdir/semantic.json"
done
