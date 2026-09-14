cmake_minimum_required(VERSION 3.22)
project(flowlower LANGUAGES CXX)

find_package(Threads REQUIRED)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(flowlower src/main.cpp)
target_compile_options(flowlower PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(flowlower PRIVATE flowcontracts)
add_executable(flowlower_allocation_fault src/main.cpp)
target_compile_definitions(flowlower_allocation_fault PRIVATE FLOWLOWER_TEST_ALLOCATION_FAILURE)
target_compile_options(flowlower_allocation_fault PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(flowlower_allocation_fault PRIVATE flowcontracts)
add_executable(flowprepare src/prepare.cpp)
target_compile_options(flowprepare PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(flowprepare PRIVATE flowcontracts)
add_executable(flowtarget src/target.cpp)
target_compile_options(flowtarget PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(flowtarget PRIVATE flowcontracts)

enable_testing()
add_test(NAME flowlower_backend_artifact
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWPREPARE_BIN=$<TARGET_FILE:flowprepare>
        FLOWVALIDATE_BIN=$<TARGET_FILE:flowvalidate>
        FLOWLOWER_BIN=$<TARGET_FILE:flowlower>
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/run-backend-artifact-tests.sh)
add_test(NAME flowtarget_policy_boundary
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWTARGET_BIN=$<TARGET_FILE:flowtarget>
        FLOWVALIDATE_BIN=$<TARGET_FILE:flowvalidate>
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/run-target-policy-tests.sh)
add_test(NAME flowtarget_cross_compile
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWTARGET_BIN=$<TARGET_FILE:flowtarget>
        FLOWPREPARE_BIN=$<TARGET_FILE:flowprepare>
        FLOWLOWER_BIN=$<TARGET_FILE:flowlower>
        FLOWTINYLOWER_BIN=$<TARGET_FILE:flowtinylower>
        FLOWTINYRUN_BIN=$<TARGET_FILE:flowtinyrun>
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/run-target-cross-compile-tests.sh)
add_test(NAME flowlower_pipeline
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWLOWER_BIN=$<TARGET_FILE:flowlower>
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/run-flowlower-tests.sh)
add_test(NAME flowlower_allocation_fault
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWLOWER_ALLOCATION_FAULT_BIN=$<TARGET_FILE:flowlower_allocation_fault>
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/run-flowlower-allocation-fault.sh)
if(TARGET flowmini AND TARGET flowanalyst AND TARGET flowparallel AND TARGET flowoptimize AND TARGET flowbind)
    set_tests_properties(flowlower_pipeline PROPERTIES
        ENVIRONMENT "FLOWMINI_BIN=$<TARGET_FILE:flowmini>;FLOWANALYST_BIN=$<TARGET_FILE:flowanalyst>;FLOWPARALLEL_BIN=$<TARGET_FILE:flowparallel>;FLOWOPTIMIZE_BIN=$<TARGET_FILE:flowoptimize>;FLOWBIND_BIN=$<TARGET_FILE:flowbind>")
endif()
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set_tests_properties(flowlower_pipeline PROPERTIES
        ENVIRONMENT "FLOWMINI_BIN=$<TARGET_FILE:flowmini>;FLOWANALYST_BIN=$<TARGET_FILE:flowanalyst>;FLOWPARALLEL_BIN=$<TARGET_FILE:flowparallel>;FLOWOPTIMIZE_BIN=$<TARGET_FILE:flowoptimize>;FLOWBIND_BIN=$<TARGET_FILE:flowbind>")
elseif(EXISTS "${CMAKE_BINARY_DIR}/flowmini/flowmini")
    set_tests_properties(flowlower_pipeline PROPERTIES
        ENVIRONMENT "FLOWMINI_BIN=${CMAKE_BINARY_DIR}/flowmini/flowmini;FLOWANALYST_BIN=${CMAKE_BINARY_DIR}/flowanalyst/flowanalyst;FLOWPARALLEL_BIN=${CMAKE_BINARY_DIR}/flowtools/flowparallel/flowparallel;FLOWOPTIMIZE_BIN=${CMAKE_BINARY_DIR}/flowoptimize/flowoptimize;FLOWBIND_BIN=${CMAKE_BINARY_DIR}/flowbind/flowbind")
endif()

add_test(NAME flowcore_pipeline_matrix
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWCORE_ROOT=${CMAKE_CURRENT_SOURCE_DIR}/..
        FLOWLOWER_BIN=$<TARGET_FILE:flowlower>
        ${CMAKE_CURRENT_SOURCE_DIR}/../tools/run-flowcore-pipeline-matrix.sh)

add_test(NAME flowcore_pass_corpus
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWCORE_ROOT=${CMAKE_CURRENT_SOURCE_DIR}/..
        FLOWLOWER_BIN=$<TARGET_FILE:flowlower>
        ${CMAKE_CURRENT_SOURCE_DIR}/../tools/run-flowcore-pass-corpus.sh)
if(TARGET flowmini AND TARGET flowanalyst AND TARGET flowparallel AND TARGET flowoptimize AND TARGET flowbind)
    set_tests_properties(flowcore_pipeline_matrix flowcore_pass_corpus PROPERTIES
        ENVIRONMENT "FLOWMINI_BIN=$<TARGET_FILE:flowmini>;FLOWANALYST_BIN=$<TARGET_FILE:flowanalyst>;FLOWPARALLEL_BIN=$<TARGET_FILE:flowparallel>;FLOWOPTIMIZE_BIN=$<TARGET_FILE:flowoptimize>;FLOWBIND_BIN=$<TARGET_FILE:flowbind>")
endif()

add_library(flowgraph_runtime SHARED src/graph_runtime.cpp)
target_compile_options(flowgraph_runtime PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(flowgraph_runtime PRIVATE Threads::Threads)
add_executable(flowgraph_runtime_bounds_test tests/graph_runtime_bounds.cpp src/graph_runtime.cpp)
target_compile_options(flowgraph_runtime_bounds_test PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(flowgraph_runtime_bounds_test PRIVATE Threads::Threads)
add_test(NAME flowgraph_runtime_bounds COMMAND flowgraph_runtime_bounds_test)
add_executable(flowgraph_runtime_launch_fault_test tests/graph_runtime_launch_fault.cpp src/graph_runtime.cpp)
target_compile_definitions(flowgraph_runtime_launch_fault_test PRIVATE FLOWCORE_GRAPH_RUNTIME_TEST_FAULT)
target_compile_options(flowgraph_runtime_launch_fault_test PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(flowgraph_runtime_launch_fault_test PRIVATE Threads::Threads)
add_test(NAME flowgraph_runtime_launch_fault COMMAND flowgraph_runtime_launch_fault_test)
