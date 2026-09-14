#!/bin/sh
set -eu

root=${FLOWCORE_ROOT:?}
runtime=${FLOWTEXT_RUNTIME:?}
cc=${CC:-cc}
cc=${FLOWTEXT_CC:-$cc}
tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT

cat > "$tmpdir/probe.c" <<'EOF'
#include "text_runtime.h"
#include <string.h>

int main(void) {
    FlowTextOutcome outcome = {FLOW_TEXT_PROVIDER_UNAVAILABLE, NULL};
    if (flow_text_concat_outcome("Lyra", "form", &outcome) != FLOW_TEXT_SUCCESS) return 1;
    if (outcome.code != FLOW_TEXT_SUCCESS || strcmp(outcome.value, "Lyraform") != 0) return 2;
    flow_text_outcome_dispose(&outcome);
    if (outcome.value != NULL) return 3;

    outcome = flow_text_concat_value("Lyra", "form");
    if (outcome.code != FLOW_TEXT_SUCCESS || strcmp(outcome.value, "Lyraform") != 0) return 4;
    if (flow_text_dispose(outcome.value) != 0) return 5;
    outcome.value = NULL;

    if (flow_text_concat_outcome(NULL, "x", &outcome) != FLOW_TEXT_INVALID_INPUT) return 6;
    if (outcome.code != FLOW_TEXT_INVALID_INPUT || outcome.value != NULL) return 6;

    char input[4098];
    memset(input, 'A', sizeof(input) - 1);
    input[sizeof(input) - 1] = '\0';
    if (flow_text_concat_outcome(input, "x", &outcome) != FLOW_TEXT_EXHAUSTED) return 7;
    if (outcome.code != FLOW_TEXT_EXHAUSTED || outcome.value != NULL) return 8;
    if (flow_text_concat_status("Lyra", "form") != FLOW_TEXT_SUCCESS) return 9;
    if (flow_text_concat_status(input, "x") != FLOW_TEXT_EXHAUSTED) return 10;
    return 0;
}
EOF
"$cc" ${FLOWTEXT_C_FLAGS:-} -std=c11 -I"$root/Flowtools/reference/text" "$tmpdir/probe.c" "$runtime" -o "$tmpdir/probe"
LD_LIBRARY_PATH=$(dirname "$runtime")${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH} "$tmpdir/probe"
echo 'Runtime Text tagged API: PASS (codes, owned success, and cleanup)'
