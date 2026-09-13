#include "flowmini_testabi.h"

#include <stddef.h>
#include <stdio.h>

int main(void) {
    printf("{\"format\":\"flowcore.abi_manifest\",\"version\":1,"
           "\"provider\":\"flowmini_testabi\",\"types\":["
           "{\"name\":\"LongValue\",\"size\":%zu,\"alignment\":%zu,"
           "\"fields\":[{\"name\":\"value\",\"type\":\"c_long\",\"offset\":%zu}]}]}\n",
           sizeof(FlowminiTestAbiLongValue), _Alignof(FlowminiTestAbiLongValue),
           offsetof(FlowminiTestAbiLongValue, value));
    return 0;
}
