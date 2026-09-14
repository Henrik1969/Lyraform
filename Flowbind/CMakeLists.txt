cmake_minimum_required(VERSION 3.22)
project(flowbind LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenSSL REQUIRED COMPONENTS Crypto)

add_executable(flowbind src/main.cpp)
target_compile_options(flowbind PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(flowbind PRIVATE flowcontracts OpenSSL::Crypto ${CMAKE_DL_LIBS})

# Test-only process boundary: prove allocation exhaustion becomes a structured
# no-artifact result instead of escaping the public binding tool.
add_executable(flowbind_allocation_fault src/main.cpp)
target_compile_definitions(flowbind_allocation_fault PRIVATE FLOWBIND_TEST_ALLOCATION_FAILURE)
target_compile_options(flowbind_allocation_fault PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(flowbind_allocation_fault PRIVATE flowcontracts OpenSSL::Crypto ${CMAKE_DL_LIBS})

enable_testing()
add_test(NAME flowbind_provider
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWBIND_BIN=$<TARGET_FILE:flowbind>
        FLOWANALYST_BIN=$<TARGET_FILE:flowanalyst>
        FLOWMINI_BIN=$<TARGET_FILE:flowmini>
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/run-flowbind-tests.sh)
add_test(NAME flowbind_fuzz
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWBIND_BIN=$<TARGET_FILE:flowbind>
        ${CMAKE_CURRENT_SOURCE_DIR}/../tools/run-flowbind-fuzz.sh)
add_test(NAME flowbind_allocation_fault
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWBIND_ALLOCATION_FAULT_BIN=$<TARGET_FILE:flowbind_allocation_fault>
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/run-flowbind-allocation-fault.sh)
