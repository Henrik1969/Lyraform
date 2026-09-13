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

FlowminiTestAbiLongValue long_input(void) {
    return (FlowminiTestAbiLongValue){42};
}

size_t long_count(void) {
    return 2;
}

FlowminiTestAbiLongValue long_item(size_t index) {
    return (FlowminiTestAbiLongValue){42 + (long)index};
}

int long_sum(FlowminiTestAbiLongValue value) {
    return (int)value.value;
}

int point_observe(int value) {
    printf("%d\n", value);
    return 0;
}
