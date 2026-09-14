cmake_minimum_required(VERSION 3.22)
project(flowcontracts LANGUAGES CXX)

add_library(flowcontracts INTERFACE)
target_include_directories(flowcontracts INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(flowcontracts INTERFACE cxx_std_20)

add_executable(flowcontracts_tests tests/json_tests.cpp)
target_link_libraries(flowcontracts_tests PRIVATE flowcontracts)
target_compile_options(flowcontracts_tests PRIVATE -Wall -Wextra -Wpedantic)

add_executable(flowvalidate src/flowvalidate.cpp)
target_link_libraries(flowvalidate PRIVATE flowcontracts)
target_compile_options(flowvalidate PRIVATE -Wall -Wextra -Wpedantic)

# Test-only process boundary: verify canonical artifact validation converts
# allocation exhaustion into a structured no-artifact result.
add_executable(flowvalidate_allocation_fault src/flowvalidate.cpp)
target_link_libraries(flowvalidate_allocation_fault PRIVATE flowcontracts)
target_compile_options(flowvalidate_allocation_fault PRIVATE -Wall -Wextra -Wpedantic)
target_compile_definitions(flowvalidate_allocation_fault PRIVATE FLOWVALIDATE_TEST_ALLOCATION_FAILURE)

enable_testing()
add_test(NAME flowcontracts_json COMMAND flowcontracts_tests)
add_test(NAME flowvalidate_artifacts
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWVALIDATE_BIN=$<TARGET_FILE:flowvalidate>
        FLOWMINI_BIN=$<TARGET_FILE:flowmini>
        FLOWANALYST_BIN=$<TARGET_FILE:flowanalyst>
        FLOWBIND_BIN=$<TARGET_FILE:flowbind>
        FLOWPARALLEL_BIN=$<TARGET_FILE:flowparallel>
        FLOWOPTIMIZE_BIN=$<TARGET_FILE:flowoptimize>
        FLOWLOWER_BIN=$<TARGET_FILE:flowlower>
        FLOWVALIDATE_FIXTURE=${CMAKE_SOURCE_DIR}/Lyraform/compiler/examples/ast/call_expression_probe.flow
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/run-flowvalidate-tests.sh)
add_test(NAME flowvalidate_allocation_fault
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWVALIDATE_ALLOCATION_FAULT_BIN=$<TARGET_FILE:flowvalidate_allocation_fault>
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/run-flowvalidate-allocation-fault.sh)
add_test(NAME flowcontracts_identity_preservation
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWMINI_BIN=$<TARGET_FILE:flowmini>
        FLOWANALYST_BIN=$<TARGET_FILE:flowanalyst>
        FLOWBIND_BIN=$<TARGET_FILE:flowbind>
        FLOWPARALLEL_BIN=$<TARGET_FILE:flowparallel>
        FLOWOPTIMIZE_BIN=$<TARGET_FILE:flowoptimize>
        FLOWLOWER_BIN=$<TARGET_FILE:flowlower>
        FLOW_IDENTITY_FIXTURE=${CMAKE_SOURCE_DIR}/Lyraform/compiler/examples/apps/flowcat/flowcat.flow
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/run-identity-preservation-tests.sh)
