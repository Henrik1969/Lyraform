#!/bin/sh
set -eu
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT HUP INT TERM
"$PROJECTION_TEST_BIN" "$work/valid.json"
"$FLOWVALIDATE_BIN" --canonical "$work/valid.json" > "$work/roundtrip.json"
cmp "$work/valid.json" "$work/roundtrip.json"
# These cases reach the actual CLI; no runtime attempt is launched.
count=0
for variant in missing type ambiguous fault forged; do
    case "$variant" in
        missing) filter='.graph_side.lanes = []'; code=MISSING_ROUTE ;;
        type) filter='.graph_side.lanes[0].semantics.payload_type = "Wrong"'; code=WRONG_DESTINATION_TYPE ;;
        ambiguous) filter='.graph_side.lanes += [.graph_side.lanes[0]]'; code=AMBIGUOUS_ROUTE ;;
        fault) filter='.graph_side.lanes[1].semantics.destination_kind = "response_function"'; code=FAULT_TO_RECOVERY ;;
        forged) filter='.projection.graph_id = "foreign"'; code=FORGED_ARTIFACT ;;
    esac
    jq "$filter" "$work/valid.json" > "$work/invalid.json"
    if "$FLOWVALIDATE_BIN" "$work/invalid.json" > "$work/machine.json"; then exit 1; fi
    jq -e --arg code "LYRAFORM_DISPOSITION_$code" '
        .code == $code and .classification == "refusal" and
        .runtime_attempt_created == false and .primary.source == "imported.flow" and
        .primary.line == 4 and .primary.column == 5 and .primary.verified == false
    ' "$work/machine.json" >/dev/null
    if "$FLOWVALIDATE_BIN" --human "$work/invalid.json" > "$work/human.txt"; then exit 1; fi
    sed -n 's/^  evidence: //p' "$work/human.txt" > "$work/human.json"
    cmp "$work/machine.json" "$work/human.json"
    count=$((count + 1))
done
printf '%s CLI diagnostic refusals agree in human and machine views\n' "$count"
