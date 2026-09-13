#include "flowmini_testabi.h"

#include <stdio.h>

int point_sum(FlowminiTestAbiPoint p) {
    return p.x + p.y;
}

int point_weighted_sum(FlowminiTestAbiPoint p) {
    return (p.x * 10) + p.y;
}

FlowminiTestAbiPoint point_input(void) {
    return (FlowminiTestAbiPoint){1, 2};
}

int point_observe(int value) {
    printf("%d\n", value);
    return 0;
}
