#include "text_runtime.h"

#include <stdlib.h>
#include <string.h>

/* The first runtime Text provider is intentionally bounded and owns results. */
enum { FLOW_TEXT_MAX_BYTES = 4096 };

FlowTextOutcomeCode flow_text_concat_outcome(const char *left, const char *right, FlowTextOutcome *outcome) {
    if (!outcome) return FLOW_TEXT_PROVIDER_UNAVAILABLE;
    outcome->code = FLOW_TEXT_PROVIDER_UNAVAILABLE;
    outcome->value = NULL;
    if (!left || !right) {
        outcome->code = FLOW_TEXT_INVALID_INPUT;
        return outcome->code;
    }
    const size_t left_bytes = strnlen(left, FLOW_TEXT_MAX_BYTES + 1);
    const size_t right_bytes = strnlen(right, FLOW_TEXT_MAX_BYTES + 1);
    if (left_bytes > FLOW_TEXT_MAX_BYTES || right_bytes > FLOW_TEXT_MAX_BYTES ||
        left_bytes > FLOW_TEXT_MAX_BYTES - right_bytes) {
        outcome->code = FLOW_TEXT_EXHAUSTED;
        return outcome->code;
    }
    char *result = malloc(left_bytes + right_bytes + 1);
    if (!result) return outcome->code;
    memcpy(result, left, left_bytes);
    memcpy(result + left_bytes, right, right_bytes);
    result[left_bytes + right_bytes] = '\0';
    outcome->code = FLOW_TEXT_SUCCESS;
    outcome->value = result;
    return outcome->code;
}

FlowTextOutcome flow_text_concat_value(const char *left, const char *right) {
    FlowTextOutcome outcome = {FLOW_TEXT_PROVIDER_UNAVAILABLE, NULL};
    flow_text_concat_outcome(left, right, &outcome);
    return outcome;
}

void flow_text_outcome_dispose(FlowTextOutcome *outcome) {
    if (!outcome) return;
    free(outcome->value);
    outcome->value = NULL;
    outcome->code = FLOW_TEXT_PROVIDER_UNAVAILABLE;
}

int flow_text_dispose(char *value) {
    free(value);
    return 0;
}

FlowTextOutcomeCode flow_text_concat_status(const char *left, const char *right) {
    FlowTextOutcome outcome;
    const FlowTextOutcomeCode code = flow_text_concat_outcome(left, right, &outcome);
    flow_text_outcome_dispose(&outcome);
    return code;
}

char *flow_text_concat(const char *left, const char *right) {
    FlowTextOutcome outcome;
    if (flow_text_concat_outcome(left, right, &outcome) != FLOW_TEXT_SUCCESS) return NULL;
    return outcome.value;
}
