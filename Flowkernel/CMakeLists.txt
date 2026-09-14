cmake_minimum_required(VERSION 3.22)
project(flowkernel LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(flowkernel src/main.cpp)
target_compile_options(flowkernel PRIVATE -Wall -Wextra -Wpedantic)

enable_testing()
add_test(NAME flowkernel_readonly
    COMMAND $<TARGET_FILE:flowkernel> --probe readonly)
add_test(NAME flowkernel_tempfs
    COMMAND $<TARGET_FILE:flowkernel> --probe tempfs)
add_test(NAME flowkernel_ipc
    COMMAND $<TARGET_FILE:flowkernel> --probe ipc)
add_test(NAME flowkernel_socket_ipc
    COMMAND $<TARGET_FILE:flowkernel> --probe socket_ipc)
add_test(NAME flowkernel_loopback
    COMMAND $<TARGET_FILE:flowkernel> --probe loopback)
add_test(NAME flowkernel_namespaces
    COMMAND $<TARGET_FILE:flowkernel> --probe namespaces)
add_test(NAME flowkernel_all
    COMMAND $<TARGET_FILE:flowkernel> --probe all)
add_test(NAME flowkernel_require_complete
    COMMAND /bin/sh ${CMAKE_CURRENT_SOURCE_DIR}/tests/run-require-complete.sh
        $<TARGET_FILE:flowkernel>)
