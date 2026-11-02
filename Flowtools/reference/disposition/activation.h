#ifndef FLOW_ACTIVATION_H
#define FLOW_ACTIVATION_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Private generated-code ABI. All pointed-to evidence and payload storage is
 * caller owned and must remain immutable until the host consumes the result.
 * A frame is serial, initialized once, and has no independently publishing work.
 * These functions validate execution against supplied authority; they never
 * select a policy, resolve a name, allocate, truncate evidence, or terminate. */
typedef struct FlowActivationAuthority {
    const char *module, *revision, *activation, *producer, *obligation;
    const char *success_type, *failure_type, *fault_type;
    const char *failure_wire, *fault_wire, *containment, *policy, *host_policy, *provenance;
    const char *commit_law, *response_provenance;
    int64_t operation, function, disposition, response_function, response_parameter, route;
} FlowActivationAuthority;

typedef struct FlowActivationValue {
    const char *type;
    const void *data;
    size_t size;
} FlowActivationValue;

typedef struct FlowActivationEnvelope {
    FlowActivationAuthority authority;
    const char *attempt, *correlation, *commit;
    FlowActivationValue payload;
} FlowActivationEnvelope;

typedef enum FlowActivationDisposition {
    FLOW_ACTIVATION_PUBLISHED = 1,
    FLOW_ACTIVATION_QUARANTINED = 2
} FlowActivationDisposition;

typedef struct FlowActivationResult {
    FlowActivationDisposition disposition;
    FlowActivationValue value;
    const FlowActivationEnvelope *envelope;
    int obligation_closed;
    int64_t response_function;
    const char *response_attempt;
    const char *response_provenance;
    const char *containment;
} FlowActivationResult;

/* Storage is accessible to trusted generated code only. Lyraform source gets
 * const envelope projections, never write access to this implementation state. */
typedef struct FlowActivationFrame {
    int phase;
    FlowActivationAuthority authority;
    const char *attempt, *correlation;
    FlowActivationEnvelope envelope;
    FlowActivationResult result;
} FlowActivationFrame;

/* Return 1 for a valid transition, 0 for refusal with the frame unchanged.
 * Refusal is an ABI contract error, not a language failure/fault disposition. */
int flow_activation_begin(FlowActivationFrame *frame,
    const FlowActivationAuthority *authority, const char *attempt, const char *correlation);
int flow_activation_publish(FlowActivationFrame *frame, FlowActivationValue value);
int flow_activation_fail(FlowActivationFrame *frame, FlowActivationValue value, const char *commit);
const FlowActivationEnvelope *flow_activation_failure(const FlowActivationFrame *frame);
int flow_activation_recover(FlowActivationFrame *frame, int64_t response_function,
    int64_t response_parameter, int64_t route, const char *response_attempt,
    const char *response_provenance, FlowActivationValue value);
int flow_activation_quarantine(FlowActivationFrame *frame,
    FlowActivationValue value, const char *commit);
const FlowActivationResult *flow_activation_result(const FlowActivationFrame *frame);

#ifdef __cplusplus
}
#endif
#endif
