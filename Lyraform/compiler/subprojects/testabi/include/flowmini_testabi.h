#ifndef FLOWMINI_TESTABI_H
#define FLOWMINI_TESTABI_H

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

int point_sum(FlowminiTestAbiPoint point);
int point_weighted_sum(FlowminiTestAbiPoint point);
FlowminiTestAbiPoint point_input(void);
int point_observe(int value);

#ifdef __cplusplus
}
#endif

#endif
