#!/bin/sh
set -eu

root=${FLOWCORE_ROOT:?}
audit=$root/docs/architecture/canonical-source-form-audit-v1.json
ast=$root/Lyraform/compiler/src/flowmini_ast.cpp

fail() {
    printf 'canonical source-form audit drift: %s\n' "$1" >&2
    exit 1
}

jq -e '
  . as $root |
  .schema == "lyraform.canonical-source-form-audit/v1" and
  .status == "complete_under_language_freeze" and
  .baseline_commit == "ecdac7184236dab24423ca7495e882f771cdf287" and
  (.categories == [
    "canonical_and_connected_end_to_end",
    "deprecated_legacy_oracle_only_and_isolated",
    "recognized_but_explicitly_unsupported",
    "refused_before_semantic_admission",
    "historical_and_non_active"
  ]) and
  (.forms | length) > 0 and
  ([.forms[].id] | length) == ([.forms[].id] | unique | length) and
  all(.forms[];
    (.id | type == "string" and length > 0) and
    (.family | type == "string" and length > 0) and
    (.ast_kind | type == "string" and length > 0) and
    (.scope | type == "string" and length > 0) and
    (.authority | type == "string" and length > 0) and
    (.classification as $class | $root.categories | index($class) != null)) and
  all(.categories[]; . as $class | any($root.forms[]; .classification == $class))
' "$audit" >/dev/null || fail 'manifest is incomplete, duplicated, or uses an unknown classification'

check_enum_family() {
    type=$1
    family=$2
    kinds=$(sed -n "/const char\* to_string($type /,/^    }/p" "$ast" |
        sed -n 's/.*return "\([^"]*\)".*/\1/p' | sort -u)
    test -n "$kinds" || fail "could not extract $type kinds"
    printf '%s\n' "$kinds" | while IFS= read -r kind; do
        jq -e --arg family "$family" --arg kind "$kind" \
          'any(.forms[]; .family == $family and .ast_kind == $kind)' "$audit" >/dev/null ||
            fail "$family kind is unclassified: $kind"
    done
}

check_enum_family SourceUnitKind source_unit
check_enum_family TopLevelKind top_level
check_enum_family StatementKind statement
check_enum_family StatementSourceForm statement_source_form
check_enum_family ExpressionKind expression
check_enum_family TypeRefKind type_reference

for keyword in Producer Node Sink Wire Policy State; do
    grep -Fq "Keyword$keyword" "$root/Lyraform/compiler/src/flowmini_ast_builder.cpp" ||
        fail "graph syntax capture no longer exposes $keyword"
done
for kind in producer node sink wire policy state; do
    jq -e --arg kind "$kind" 'any(.forms[]; .family == "graph" and .ast_kind == $kind)' "$audit" >/dev/null ||
        fail "graph form is unclassified: $kind"
done

grep -Fq 'return flowmini::parseModule(tokens);' "$root/Lyraform/compiler/src/main.cpp" ||
    fail 'deprecated parseModule oracle route is no longer explicit'
grep -Fq 'run-legacy)' "$root/igor" || fail 'deprecated legacy execution is no longer isolated'
jq -e 'any(.forms[]; .id == "legacy.module_header" and
  .classification == "deprecated_legacy_oracle_only_and_isolated")' "$audit" >/dev/null ||
    fail 'legacy module header lacks an oracle-only classification'

printf '%s\n' 'Canonical source-form authority audit: PASS'
