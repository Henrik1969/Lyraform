#!/bin/sh
set -eu

root=${FLOWCORE_ROOT:?}
flowmini=${FLOWMINI_BIN:?}
analyst=${FLOWANALYST_BIN:?}
parallel=${FLOWPARALLEL_BIN:?}
optimize=${FLOWOPTIMIZE_BIN:?}
prepare=${FLOWPREPARE_BIN:?}
bind=${FLOWBIND_BIN:?}
tiny_lower=${FLOWTINYLOWER_BIN:?}
text_runtime=${FLOWTEXT_RUNTIME:?}

tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT
policy=$tmpdir/policy
printf '%s\n' \
  'allow libflowtext.so flow_text_concat c memory Text,Text Text' \
  'allow libc.so.6 puts c io Text c_int' > "$policy"
export LD_LIBRARY_PATH=$(dirname "$text_runtime")${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}

source=$root/Lyraform/compiler/examples/text/text_runtime.flow
"$flowmini" --dump-frontend-bundle "$source" > "$tmpdir/frontend.json"
"$analyst" --lowering-plan-version 2 < "$tmpdir/frontend.json" > "$tmpdir/semantic.json"
"$parallel" < "$tmpdir/semantic.json" > "$tmpdir/parallel.json"
"$optimize" < "$tmpdir/parallel.json" > "$tmpdir/optimized.json"
"$bind" --policy "$policy" < "$tmpdir/semantic.json" > "$tmpdir/binding.json"
"$prepare" --binding-report "$tmpdir/binding.json" "$tmpdir/optimized.json" > "$tmpdir/lowering.json"

set +e
"$tiny_lower" "$tmpdir/lowering.json" "$tmpdir/text.tvm" > "$tmpdir/result.json"
tiny_status=$?
set -e
test "$tiny_status" -eq 2
jq -e '.status == "unsupported" and (.reason | contains("external provider tuple"))' "$tmpdir/result.json" >/dev/null

echo 'Runtime Text TinyVM boundary: PASS (explicit unsupported result until owned VM storage exists)'
