cmake_minimum_required(VERSION 3.22)
project(flowanalyst LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(flowanalyst src/main.cpp)
target_link_libraries(flowanalyst PRIVATE flowcontracts)
target_compile_options(flowanalyst PRIVATE -Wall -Wextra -Wpedantic)

# Test-only process boundary: prove semantic-artifact exhaustion produces no
# output artifact and a structured refusal.
add_executable(flowanalyst_allocation_fault src/main.cpp)
target_compile_definitions(flowanalyst_allocation_fault PRIVATE FLOWANALYST_TEST_ALLOCATION_FAILURE)
target_link_libraries(flowanalyst_allocation_fault PRIVATE flowcontracts)
target_compile_options(flowanalyst_allocation_fault PRIVATE -Wall -Wextra -Wpedantic)

enable_testing()
add_test(NAME flowanalyst_pipeline
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWANALYST_BIN=$<TARGET_FILE:flowanalyst>
        FLOWMINI_BIN=$<TARGET_FILE:flowmini>
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/run-flowanalyst-tests.sh)
add_test(NAME flowanalyst_allocation_fault
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWANALYST_ALLOCATION_FAULT_BIN=$<TARGET_FILE:flowanalyst_allocation_fault>
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/run-flowanalyst-allocation-fault.sh)
