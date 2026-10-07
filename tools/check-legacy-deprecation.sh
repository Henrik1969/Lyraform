#!/bin/sh
set -eu

root=${FLOWCORE_ROOT:?}
map=$root/docs/architecture/canonical-language-authority-map-v1.json

fail() {
    printf 'legacy-deprecation drift: %s\n' "$1" >&2
    exit 1
}

jq -e '
  any(.surfaces[];
    .id == "deprecated_legacy_source" and
    .recognition == "deprecated_legacy" and
    .semantic == "oracle_only_non_authoritative" and
    .execution == "excluded_from_canonical") and
  any(.known_bridges[];
    .id == "legacy_source_fallback" and
    .state == "deprecated_oracle_only")
' "$map" >/dev/null || fail 'authority map does not exclude legacy behavior'

help=$("$root/igor" --help)
printf '%s\n' "$help" | grep -Fq 'run                compile and run through the bounded canonical stage chain' ||
    fail 'Igor run is not documented as canonical'
printf '%s\n' "$help" | grep -Fq 'run-legacy         DEPRECATED: invoke the legacy behavior oracle' ||
    fail 'Igor legacy entry point is not explicitly deprecated'

canonical_case=$(sed -n '/^    run|run-canonical)/,/^        ;;/p' "$root/igor")
legacy_case=$(sed -n '/^    run-legacy)/,/^        ;;/p' "$root/igor")
printf '%s\n' "$canonical_case" | grep -Fq 'igor-run-canonical.sh' ||
    fail 'default run does not use the canonical runner'
if printf '%s\n' "$canonical_case" | grep -Fq 'exec "$executable"'; then
    fail 'default run can invoke the legacy executable'
fi
printf '%s\n' "$legacy_case" | grep -Fq 'legacy runtime is deprecated, non-canonical' ||
    fail 'legacy execution lacks a deprecation warning'

grep -Fq 'Status: accepted.' "$root/docs/architecture/decisions/0061-legacy-is-deprecated-oracle-only.md" ||
    fail 'ADR-0061 is missing or not accepted'
grep -Fq '`igor run` is the fail-closed canonical path' "$root/docs/current-status.md" ||
    fail 'current status does not state the default authority'

printf '%s\n' 'Legacy deprecation boundary: PASS'
