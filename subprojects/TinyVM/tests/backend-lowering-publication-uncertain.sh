#!/bin/sh
set -eu

lower=$1
validate=$2
fixture=$3
build=$4
artifact="$build/backend-publication-uncertain.tvm"
stdout="$build/backend-publication-uncertain.stdout"
stderr="$build/backend-publication-uncertain.stderr"

for mode in directory-sync directory-close
do
    printf 'previous\n' >"$artifact"
    set +e
    TINYVM_TEST_PUBLICATION_FAULT=$mode "$lower" --diagnostics json "$fixture" "$artifact" >"$stdout" 2>"$stderr"
    status=$?
    set -e

    test "$status" -eq 1
    test ! -s "$stdout"
    jq -e '
      .status == "failed" and
      .code == "FLOWTINYLOWER_OUTPUT_DURABILITY_UNCERTAIN" and
      .stage == "output" and
      .disposition == "artifact_published_durability_uncertain" and
      (.message | contains("parent directory durability is uncertain"))
    ' "$stderr" >/dev/null
    "$validate" "$artifact" | grep -q '"status":"valid"'
    test -z "$(find "$build" -maxdepth 1 -name 'backend-publication-uncertain.tvm.tmp.*' -print -quit)"
done

rm -f "$artifact" "$stdout" "$stderr"
echo 'TinyVM backend publication uncertainty: PASS'
