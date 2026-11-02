add_library(flowactivation_runtime SHARED activation.c activation.h)
target_include_directories(flowactivation_runtime PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(flowactivation_runtime PROPERTIES OUTPUT_NAME flowactivation)
target_compile_options(flowactivation_runtime PRIVATE -Wall -Wextra -Wpedantic)
add_executable(flowactivation_runtime_tests activation_tests.cpp)
target_link_libraries(flowactivation_runtime_tests PRIVATE flowactivation_runtime flowcontracts)
target_compile_options(flowactivation_runtime_tests PRIVATE -Wall -Wextra -Wpedantic)
add_test(NAME flowactivation_source_runtime
    COMMAND /bin/sh ${CMAKE_CURRENT_SOURCE_DIR}/run-tests.sh
        ${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR} $<TARGET_FILE:flowactivation_runtime_tests>)
