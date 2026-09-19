#!/bin/sh
# Preserve the Phase 4 witnesses without executing the historical unsafe plan.
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
log=$(mktemp /tmp/lyraform-phase5-collisions.XXXXXX)
trap 'rm -f "$log"' EXIT HUP INT TERM
LYRAFORM_RECON_FIXTURES_ONLY=1 sh "$root/docs/recon/v1-source-completeness-probes-2026-09-18.sh" "$1" > "$log"
out=$(sed -n 's/^evidence_directory=//p' "$log")
test -d "$out"
test "$(awk 'NR>1 {n++} END {print n}' "$out/pairs.tsv")" -eq 23
# Phase 6 intentionally replaces incomplete target projections with recovery
# statements. Preserve every source pair, not the old lossy AST equality.
# Every original pair has an explicit disposition. Deferred classifications do
# not assert a new language policy or authorize execution as canonical scalar.
while read -r base variant category; do
    case "$category" in
        CANONICAL_INVALID)
            jq -e '.parse_validity.state == "canonical_valid" or
                (.parse_validity.state == "outside_scope" and .parse_validity.coverage == "complete")' "$out/$base.frontend.json" > /dev/null
            jq -e '.parse_validity.state == "invalid" or .parse_validity.state == "incomplete" or .parse_validity.state == "recovered"' "$out/$variant.frontend.json" > /dev/null
            jq -e '.status != "ok" and .lowering_plan.status != "ready"' "$out/$variant.analysis.json" > /dev/null
            ;;
        EXPECTED_NORMALIZATION|REQUIRES_DISTINCT_PROJECTION|UNDECIDED)
            jq -e '.parse_validity.state != "canonical_valid"' "$out/$variant.frontend.json" > /dev/null
            case "$variant" in member_dot|member_index|index_missing|index_field|index_again)
                jq -e '.parse_validity.state == "recovered"' "$out/$variant.frontend.json" > /dev/null
                jq -e '.status != "ok" and .lowering_plan.status != "ready"' "$out/$variant.analysis.json" > /dev/null
                ;;
            esac
            ;;
        *) exit 1 ;;
    esac
    printf '%s / %s: %s\n' "$base" "$variant" "$category"
done <<'PAIRS'
scalar decl_tail CANONICAL_INVALID
scalar literal_tail CANONICAL_INVALID
scalar unknown CANONICAL_INVALID
scalar nested_block CANONICAL_INVALID
place place_type CANONICAL_INVALID
place place_junk CANONICAL_INVALID
return_keyword return_tail CANONICAL_INVALID
member member_dot UNDECIDED
member member_index REQUIRES_DISTINCT_PROJECTION
index index_missing UNDECIDED
index index_field REQUIRES_DISTINCT_PROJECTION
index index_again REQUIRES_DISTINCT_PROJECTION
type_bare type_field EXPECTED_NORMALIZATION
type_bare type_tail UNDECIDED
if_ok if_tail UNDECIDED
while_ok while_tail UNDECIDED
function parameter_gap UNDECIDED
function call_gap UNDECIDED
list list_gap UNDECIDED
record_literal record_literal_bad UNDECIDED
import_ok import_tail UNDECIDED
target target_extra REQUIRES_DISTINCT_PROJECTION
abi abi_unknown UNDECIDED
PAIRS
printf 'PASS: 23 preserved collisions; 7 rejected suffix/loss witnesses. Evidence: %s\n' "$out"
