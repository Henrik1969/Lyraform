add_library(flowtext_runtime SHARED text_runtime.c text_runtime.h)
target_include_directories(flowtext_runtime PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(flowtext_runtime PROPERTIES OUTPUT_NAME flowtext)
target_compile_options(flowtext_runtime PRIVATE -Wall -Wextra -Wpedantic)

add_executable(flowtext_runtime_allocation_failure_test allocation_failure_test.c text_runtime.c)
target_include_directories(flowtext_runtime_allocation_failure_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(flowtext_runtime_allocation_failure_test PRIVATE -Wall -Wextra -Wpedantic)
target_link_options(flowtext_runtime_allocation_failure_test PRIVATE "-Wl,--wrap=malloc")
add_test(NAME flowtext_runtime_allocation_failure
    COMMAND flowtext_runtime_allocation_failure_test)
