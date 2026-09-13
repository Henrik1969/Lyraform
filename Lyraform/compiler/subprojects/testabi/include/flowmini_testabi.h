#ifndef FLOWMINI_TESTABI_H
#define FLOWMINI_TESTABI_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Shared compatibility contract for the current struct-by-value ABI probe.
 * This is test-provider infrastructure, not a canonical Flowmini ABI type.
 */
typedef struct FlowminiTestAbiPoint {
    int x;
    int y;
} FlowminiTestAbiPoint;

typedef struct FlowminiTestAbiLongValue {
    long value;
} FlowminiTestAbiLongValue;

int point_sum(FlowminiTestAbiPoint point);
int point_weighted_sum(FlowminiTestAbiPoint point);
FlowminiTestAbiPoint point_input(void);
int point_observe(int value);
FlowminiTestAbiLongValue long_input(void);
size_t long_count(void);
FlowminiTestAbiLongValue long_item(size_t index);
int long_sum(FlowminiTestAbiLongValue value);

#ifdef __cplusplus
}
#endif

#endif
