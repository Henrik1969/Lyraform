cmake_minimum_required(VERSION 3.22)
project(flowoptimize LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(flowoptimize src/main.cpp)
target_compile_options(flowoptimize PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(flowoptimize PRIVATE flowcontracts)

# Test-only process boundary: prove optimization exhaustion produces no
# output artifact and a structured refusal.
add_executable(flowoptimize_allocation_fault src/main.cpp)
target_compile_definitions(flowoptimize_allocation_fault PRIVATE FLOWOPTIMIZE_TEST_ALLOCATION_FAILURE)
target_compile_options(flowoptimize_allocation_fault PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(flowoptimize_allocation_fault PRIVATE flowcontracts)

enable_testing()
add_test(NAME flowoptimize_pipeline
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWOPTIMIZE_BIN=$<TARGET_FILE:flowoptimize>
        FLOWPARALLEL_BIN=$<TARGET_FILE:flowparallel>
        FLOWANALYST_BIN=$<TARGET_FILE:flowanalyst>
        FLOWMINI_BIN=$<TARGET_FILE:flowmini>
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/run-flowoptimize-tests.sh)
add_test(NAME flowoptimize_allocation_fault
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWOPTIMIZE_ALLOCATION_FAULT_BIN=$<TARGET_FILE:flowoptimize_allocation_fault>
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/run-flowoptimize-allocation-fault.sh)
