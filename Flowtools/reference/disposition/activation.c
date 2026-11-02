#include "activation.h"
#include <string.h>

enum { EMPTY, ACTIVE, FAILURE, PUBLISHED, QUARANTINED };

static int present(const char *text) { return text && *text; }
static int same(const char *left, const char *right) {
    return left && right && strcmp(left, right) == 0;
}
static int valid_value(FlowActivationValue value, const char *type) {
    return same(value.type, type) && value.data && value.size > 0;
}
static int valid_authority(const FlowActivationAuthority *a) {
    return a && present(a->module) && present(a->revision) && present(a->activation) &&
        present(a->producer) && present(a->obligation) && present(a->success_type) &&
        present(a->failure_type) && present(a->fault_type) && present(a->failure_wire) &&
        present(a->fault_wire) && present(a->containment) && present(a->policy) &&
        present(a->host_policy) && present(a->provenance) &&
        same(a->commit_law, "atomic_tagged_result") && present(a->response_provenance) && a->operation >= 0 && a->function >= 0 &&
        a->disposition >= 0 && a->response_function >= 0 && a->response_parameter >= 0 &&
        a->route >= 0 && !same(a->failure_wire, a->fault_wire);
}

int flow_activation_begin(FlowActivationFrame *f, const FlowActivationAuthority *a,
                          const char *attempt, const char *correlation) {
    if (!f || f->phase != EMPTY || !valid_authority(a) ||
        !present(attempt) || !present(correlation)) return 0;
    f->authority = *a;
    f->attempt = attempt;
    f->correlation = correlation;
    f->phase = ACTIVE;
    return 1;
}

int flow_activation_publish(FlowActivationFrame *f, FlowActivationValue value) {
    if (!f || f->phase != ACTIVE || !valid_value(value, f->authority.success_type)) return 0;
    f->result = (FlowActivationResult){FLOW_ACTIVATION_PUBLISHED, value, NULL, 0, -1, NULL, NULL, NULL};
    f->phase = PUBLISHED;
    return 1;
}

int flow_activation_fail(FlowActivationFrame *f, FlowActivationValue value, const char *commit) {
    if (!f || f->phase != ACTIVE || !valid_value(value, f->authority.failure_type) ||
        !same(commit, "no_commit")) return 0;
    f->envelope = (FlowActivationEnvelope){f->authority, f->attempt, f->correlation, commit, value};
    f->phase = FAILURE;
    return 1;
}

const FlowActivationEnvelope *flow_activation_failure(const FlowActivationFrame *f) {
    return f && f->phase == FAILURE ? &f->envelope : NULL;
}

int flow_activation_recover(FlowActivationFrame *f, int64_t function, int64_t parameter,
                            int64_t route, const char *attempt, const char *provenance,
                            FlowActivationValue value) {
    if (!f || f->phase != FAILURE || function != f->authority.response_function ||
        parameter != f->authority.response_parameter || route != f->authority.route ||
        !present(attempt) || same(attempt, f->attempt) || !same(provenance, f->authority.response_provenance) ||
        !valid_value(value, f->authority.success_type)) return 0;
    f->result = (FlowActivationResult){FLOW_ACTIVATION_PUBLISHED, value, &f->envelope,
        1, function, attempt, provenance, NULL};
    f->phase = PUBLISHED;
    return 1;
}

int flow_activation_quarantine(FlowActivationFrame *f, FlowActivationValue value, const char *commit) {
    if (!f || f->phase != ACTIVE || !valid_value(value, f->authority.fault_type) ||
        !same(commit, "no_commit")) return 0;
    f->envelope = (FlowActivationEnvelope){f->authority, f->attempt, f->correlation, commit, value};
    f->result = (FlowActivationResult){FLOW_ACTIVATION_QUARANTINED,
        {NULL, NULL, 0}, &f->envelope, 0, -1, NULL, NULL, f->authority.containment};
    f->phase = QUARANTINED;
    return 1;
}

const FlowActivationResult *flow_activation_result(const FlowActivationFrame *f) {
    return f && (f->phase == PUBLISHED || f->phase == QUARANTINED) ? &f->result : NULL;
}
