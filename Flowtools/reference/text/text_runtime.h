#ifndef FLOW_TEXT_RUNTIME_H
#define FLOW_TEXT_RUNTIME_H

#include <stddef.h>

typedef enum FlowTextOutcomeCode {
    FLOW_TEXT_SUCCESS = 0,
    FLOW_TEXT_INVALID_INPUT = 1,
    FLOW_TEXT_EXHAUSTED = 2,
    FLOW_TEXT_PROVIDER_UNAVAILABLE = 3
} FlowTextOutcomeCode;

typedef struct FlowTextOutcome {
    FlowTextOutcomeCode code;
    char *value;
} FlowTextOutcome;

/*
 * The caller must initialize an outcome as
 * {FLOW_TEXT_PROVIDER_UNAVAILABLE, NULL} and dispose any owned value before
 * reusing it. The result pointer is provider-owned until dispose.
 */
FlowTextOutcomeCode flow_text_concat_outcome(const char *left, const char *right, FlowTextOutcome *outcome);
/* Atomic value carrier used by the backend-neutral TextOutcome boundary. */
FlowTextOutcome flow_text_concat_value(const char *left, const char *right);
void flow_text_outcome_dispose(FlowTextOutcome *outcome);

/* Final-owner cleanup for a successful TextOutcome value. */
int flow_text_dispose(char *value);

/* Flow-level recovery probe: returns a stable TextFailure code without
 * transferring owned storage to the caller. */
FlowTextOutcomeCode flow_text_concat_status(const char *left, const char *right);

/* Transitional compatibility adapter used by the current lowering slice. */
char *flow_text_concat(const char *left, const char *right);

#endif
