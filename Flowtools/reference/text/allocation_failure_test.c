#include "text_runtime.h"

#include <assert.h>
#include <stddef.h>

void *__real_malloc(size_t size);

static int fail_small_allocation = 0;

void *__wrap_malloc(size_t size) {
    if (fail_small_allocation && size == 3) return NULL;
    return __real_malloc(size);
}

int main(void) {
    FlowTextOutcome outcome = {FLOW_TEXT_PROVIDER_UNAVAILABLE, NULL};

    fail_small_allocation = 1;
    assert(flow_text_concat_outcome("a", "b", &outcome) == FLOW_TEXT_EXHAUSTED);
    assert(outcome.code == FLOW_TEXT_EXHAUSTED);
    assert(outcome.value == NULL);

    fail_small_allocation = 0;
    assert(flow_text_concat_outcome("a", "b", &outcome) == FLOW_TEXT_SUCCESS);
    assert(outcome.code == FLOW_TEXT_SUCCESS);
    assert(outcome.value != NULL);
    assert(outcome.value[0] == 'a' && outcome.value[1] == 'b' && outcome.value[2] == '\0');
    flow_text_outcome_dispose(&outcome);
    assert(outcome.code == FLOW_TEXT_PROVIDER_UNAVAILABLE);
    assert(outcome.value == NULL);
    return 0;
}
