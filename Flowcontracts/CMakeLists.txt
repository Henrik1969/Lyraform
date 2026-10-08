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
add_executable(flowcontracts_ownership_transfer_tests tests/ownership_transfer_tests.cpp)
target_link_libraries(flowcontracts_ownership_transfer_tests PRIVATE flowcontracts)
target_compile_options(flowcontracts_ownership_transfer_tests PRIVATE -Wall -Wextra -Wpedantic)
add_test(NAME flowcontracts_ownership_transfer COMMAND flowcontracts_ownership_transfer_tests)
add_executable(flowcontracts_failure_consumer_tests tests/failure_consumer_tests.cpp)
target_link_libraries(flowcontracts_failure_consumer_tests PRIVATE flowcontracts)
target_compile_options(flowcontracts_failure_consumer_tests PRIVATE -Wall -Wextra -Wpedantic)
add_test(NAME flowcontracts_failure_consumer COMMAND flowcontracts_failure_consumer_tests)
add_executable(flowcontracts_failure_response_transition_tests tests/failure_response_transition_tests.cpp)
target_link_libraries(flowcontracts_failure_response_transition_tests PRIVATE flowcontracts)
target_compile_options(flowcontracts_failure_response_transition_tests PRIVATE -Wall -Wextra -Wpedantic)
add_test(NAME flowcontracts_failure_response_transition COMMAND flowcontracts_failure_response_transition_tests)
add_executable(flowcontracts_failure_flow_execution_tests tests/failure_flow_execution_tests.cpp)
target_link_libraries(flowcontracts_failure_flow_execution_tests PRIVATE flowcontracts)
target_compile_options(flowcontracts_failure_flow_execution_tests PRIVATE -Wall -Wextra -Wpedantic)
add_test(NAME flowcontracts_failure_flow_execution COMMAND flowcontracts_failure_flow_execution_tests)
add_executable(flowcontracts_failure_flow_closed_set_tests tests/failure_flow_closed_set_tests.cpp)
target_link_libraries(flowcontracts_failure_flow_closed_set_tests PRIVATE flowcontracts)
target_compile_options(flowcontracts_failure_flow_closed_set_tests PRIVATE -Wall -Wextra -Wpedantic)
add_test(NAME flowcontracts_failure_flow_closed_set COMMAND flowcontracts_failure_flow_closed_set_tests)
add_executable(flowcontracts_failure_flow_chain_tests tests/failure_flow_chain_tests.cpp)
target_link_libraries(flowcontracts_failure_flow_chain_tests PRIVATE flowcontracts)
target_compile_options(flowcontracts_failure_flow_chain_tests PRIVATE -Wall -Wextra -Wpedantic)
add_test(NAME flowcontracts_failure_flow_chain COMMAND flowcontracts_failure_flow_chain_tests)

add_executable(flowcontracts_failure_response_attempt_tests tests/failure_response_attempt_tests.cpp)
target_link_libraries(flowcontracts_failure_response_attempt_tests PRIVATE flowcontracts)
target_compile_options(flowcontracts_failure_response_attempt_tests PRIVATE -Wall -Wextra -Wpedantic)
add_test(NAME flowcontracts_failure_response_attempt COMMAND flowcontracts_failure_response_attempt_tests)
add_executable(flowcontracts_fault_containment_tests tests/fault_containment_tests.cpp)
target_link_libraries(flowcontracts_fault_containment_tests PRIVATE flowcontracts)
target_compile_options(flowcontracts_fault_containment_tests PRIVATE -Wall -Wextra -Wpedantic)
add_test(NAME flowcontracts_fault_containment COMMAND flowcontracts_fault_containment_tests)
add_executable(flowcontracts_failure_response_fault_tests tests/failure_response_fault_tests.cpp)
target_link_libraries(flowcontracts_failure_response_fault_tests PRIVATE flowcontracts)
target_compile_options(flowcontracts_failure_response_fault_tests PRIVATE -Wall -Wextra -Wpedantic)
add_test(NAME flowcontracts_failure_response_fault COMMAND flowcontracts_failure_response_fault_tests)
add_executable(flowcontracts_effect_scheduling_tests tests/effect_scheduling_tests.cpp)
target_link_libraries(flowcontracts_effect_scheduling_tests PRIVATE flowcontracts)
target_compile_options(flowcontracts_effect_scheduling_tests PRIVATE -Wall -Wextra -Wpedantic)
add_test(NAME flowcontracts_effect_scheduling COMMAND flowcontracts_effect_scheduling_tests)
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
