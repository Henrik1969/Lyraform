#include <stddef.h>
#include <stdlib.h>
#include <string.h>

/* The first runtime Text provider is intentionally bounded and owns results. */
enum { FLOW_TEXT_MAX_BYTES = 4096 };

char *flow_text_concat(const char *left, const char *right) {
    if (!left || !right) return NULL;
    const size_t left_bytes = strnlen(left, FLOW_TEXT_MAX_BYTES + 1);
    const size_t right_bytes = strnlen(right, FLOW_TEXT_MAX_BYTES + 1);
    if (left_bytes > FLOW_TEXT_MAX_BYTES || right_bytes > FLOW_TEXT_MAX_BYTES ||
        left_bytes > FLOW_TEXT_MAX_BYTES - right_bytes) return NULL;
    char *result = malloc(left_bytes + right_bytes + 1);
    if (!result) return NULL;
    memcpy(result, left, left_bytes);
    memcpy(result + left_bytes, right, right_bytes);
    result[left_bytes + right_bytes] = '\0';
    return result;
}
