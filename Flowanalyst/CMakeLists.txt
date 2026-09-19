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
add_executable(lyraform_scalar_semantics_test tests/scalar_semantics.cpp)
target_link_libraries(lyraform_scalar_semantics_test PRIVATE flowcontracts)
add_test(NAME lyraform_scalar_semantics COMMAND lyraform_scalar_semantics_test)
add_test(NAME lyraform_scalar_authority
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWMINI_BIN=$<TARGET_FILE:flowmini>
        FLOWMINI_ORACLE_BIN=$<TARGET_FILE:flowmini_scalar_path_test>
        FLOWANALYST_BIN=$<TARGET_FILE:flowanalyst>
        FLOWBIND_BIN=$<TARGET_FILE:flowbind>
        FLOWPARALLEL_BIN=$<TARGET_FILE:flowparallel>
        FLOWOPTIMIZE_BIN=$<TARGET_FILE:flowoptimize>
        FLOWPREPARE_BIN=$<TARGET_FILE:flowprepare>
        FLOWLOWER_BIN=$<TARGET_FILE:flowlower>
        FLOWTINYLOWER_BIN=$<TARGET_FILE:flowtinylower>
        FLOWTINYRUN_BIN=$<TARGET_FILE:flowtinyrun>
        FLOWVALIDATE_BIN=$<TARGET_FILE:flowvalidate>
        sh ${CMAKE_CURRENT_SOURCE_DIR}/tests/run-scalar-authority-tests.sh)
add_test(NAME flowanalyst_pipeline
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWANALYST_BIN=$<TARGET_FILE:flowanalyst>
        FLOWMINI_BIN=$<TARGET_FILE:flowmini>
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/run-flowanalyst-tests.sh)
add_test(NAME lyraform_scalar_source_path
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWMINI_BIN=$<TARGET_FILE:flowmini>
        FLOWMINI_TEST_BIN=$<TARGET_FILE:flowmini_scalar_path_test>
        FLOWANALYST_BIN=$<TARGET_FILE:flowanalyst>
        FLOWBIND_BIN=$<TARGET_FILE:flowbind>
        FLOWPARALLEL_BIN=$<TARGET_FILE:flowparallel>
        FLOWOPTIMIZE_BIN=$<TARGET_FILE:flowoptimize>
        FLOWPREPARE_BIN=$<TARGET_FILE:flowprepare>
        FLOWLOWER_BIN=$<TARGET_FILE:flowlower>
        FLOWTINYLOWER_BIN=$<TARGET_FILE:flowtinylower>
        FLOWTINYRUN_BIN=$<TARGET_FILE:flowtinyrun>
        FLOWVALIDATE_BIN=$<TARGET_FILE:flowvalidate>
        sh ${CMAKE_CURRENT_SOURCE_DIR}/../Lyraform/compiler/tools/run-scalar-source-path-tests.sh)
add_test(NAME flowanalyst_allocation_fault
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWANALYST_ALLOCATION_FAULT_BIN=$<TARGET_FILE:flowanalyst_allocation_fault>
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/run-flowanalyst-allocation-fault.sh)
add_test(NAME lyraform_scalar_parse_validity
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWMINI_BIN=$<TARGET_FILE:flowmini>
        FLOWMINI_TEST_BIN=$<TARGET_FILE:flowmini_scalar_path_test>
        FLOWANALYST_BIN=$<TARGET_FILE:flowanalyst>
        FLOWBIND_BIN=$<TARGET_FILE:flowbind>
        FLOWPARALLEL_BIN=$<TARGET_FILE:flowparallel>
        FLOWOPTIMIZE_BIN=$<TARGET_FILE:flowoptimize>
        FLOWPREPARE_BIN=$<TARGET_FILE:flowprepare>
        FLOWLOWER_BIN=$<TARGET_FILE:flowlower>
        FLOWTINYLOWER_BIN=$<TARGET_FILE:flowtinylower>
        FLOWVALIDATE_BIN=$<TARGET_FILE:flowvalidate>
        sh ${CMAKE_CURRENT_SOURCE_DIR}/tests/run-parse-validity-tests.sh)
add_test(NAME lyraform_scalar_parse_collisions
    COMMAND sh ${CMAKE_CURRENT_SOURCE_DIR}/tests/run-parse-collision-tests.sh ${CMAKE_BINARY_DIR})
add_test(NAME lyraform_target_parse_completeness
    COMMAND sh ${CMAKE_CURRENT_SOURCE_DIR}/tests/run-target-parse-tests.sh ${CMAKE_BINARY_DIR})
add_test(NAME lyraform_target_preservation
    COMMAND sh ${CMAKE_CURRENT_SOURCE_DIR}/tests/run-target-preservation-tests.sh ${CMAKE_BINARY_DIR})
add_test(NAME lyraform_field_path_resolution
    COMMAND sh ${CMAKE_CURRENT_SOURCE_DIR}/tests/run-field-resolution-tests.sh ${CMAKE_BINARY_DIR})
