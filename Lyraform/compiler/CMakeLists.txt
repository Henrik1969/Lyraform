# Lyraform compiler project build and control file
# ------------------------------------------------

cmake_minimum_required(VERSION 3.22)

project(lyraform_compiler LANGUAGES C CXX)

# We use CTest-compatible project structure.
# Note: include(CTest) creates extra dashboard targets in some IDEs.
include(CTest)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ------------------------------------------------
# Subprojects / internal prerequisite providers
# ------------------------------------------------
#
# These subprojects provide libraries/tools used by Flowmini itself or by the
# test/example environment.
#
# testabi is an internal prerequisite provider for ABI examples/tests. It builds
# libflowmini_testabi.so.

add_subdirectory(subprojects/testabi)
add_subdirectory(subprojects/TokenTree/tokentree_v1)
add_subdirectory(subprojects/SymbolTable/SymbolTable_v1_1)

# ------------------------------------------------
# Main executable
# ------------------------------------------------

add_executable(flowmini
        src/flow_common.cpp
        src/flowmini_payload.cpp
        src/flowmini_lexer.cpp
        src/flowmini_parser.cpp
        src/flowmini_runtime.cpp
        src/flowmini_structural.cpp
        src/flowmini_token_tree_bridge.cpp
        src/flowmini_ast.cpp
        src/flowmini_ast_builder.cpp
        src/flowmini_symbol_projection.cpp
        src/flowmini_frontend_bundle.cpp
        src/main.cpp

        include/flowmini_ast.h
        include/flowmini_ast_builder.h
        include/flowmini_symbol_projection.h
        include/flowmini_frontend_bundle.h
)

target_include_directories(flowmini PRIVATE
        include
        subprojects/testabi/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../../Flowcontracts/include
)

target_compile_options(flowmini PRIVATE
        -Wall
        -Wextra
        -pedantic
)

# We link static because we do not want these internal libraries as dynamic
# runtime dependencies for Flowmini itself right now.
target_link_libraries(flowmini PRIVATE
        TokenTree_static
        SymbolTable::static
)

find_library(DL_LIBRARY dl)
if(DL_LIBRARY)
    target_link_libraries(flowmini PRIVATE ${DL_LIBRARY})
endif()

# Ensure the ABI test provider is built when Flowmini is built.
add_dependencies(flowmini flowmini_testabi)

# ------------------------------------------------
# Internal test prerequisite preparation
# ------------------------------------------------
#
# Current ABI examples expect:
#
#     ./build/libflowmini_testabi.so
#
# That is a legacy/historical path assumption. The real provider is the CMake
# target:
#
#     flowmini_testabi
#
# This target bridges the current source expectation to the actual build output.
# Future Flowcore/Flowmini prerequisite resolution should replace this with a
# declared `prerequisite shared_library ...` mechanism.

add_custom_target(flowmini_prepare_test_dependencies
        COMMAND ${CMAKE_COMMAND} -E make_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/build
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:flowmini_testabi>
        ${CMAKE_CURRENT_SOURCE_DIR}/build/libflowmini_testabi.so
        DEPENDS flowmini_testabi
        COMMENT "Preparing Flowmini ABI test dependency"
        VERBATIM
)

# ------------------------------------------------
# Project-visible workflow targets
# ------------------------------------------------
#
# These targets make the normal Flowmini workflows visible to CMake-aware tools
# such as CLion. Shell scripts may still exist as helpers, but the important
# project actions should be reachable as CMake targets.

add_custom_target(flowmini_suite
        COMMAND
        ${CMAKE_COMMAND} -E env
        FLOWMINI_ROOT=${CMAKE_CURRENT_SOURCE_DIR}
        FLOWMINI_BUILD_DIR=${CMAKE_CURRENT_BINARY_DIR}
        FLOWMINI_NO_BUILD=1
        FLOWMINI_JOBS=20
        ${CMAKE_CURRENT_SOURCE_DIR}/../../tools/run-flowmini-test-suite.sh
        DEPENDS
        flowmini
        flowmini_prepare_test_dependencies
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Running Flowmini categorized test suite"
        VERBATIM
)

add_custom_target(flowmini_dump_token_tree_simple
        COMMAND
        $<TARGET_FILE:flowmini>
        --dump-token-tree-bridge
        simple
        examples/pass/fn_demo.flow
        DEPENDS flowmini
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Dumping Flowmini TokenTree bridge simple view"
        VERBATIM
)

add_custom_target(flowmini_dump_token_tree_json
        COMMAND
        $<TARGET_FILE:flowmini>
        --dump-token-tree-bridge
        json
        examples/pass/fn_demo.flow
        DEPENDS flowmini
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Dumping Flowmini TokenTree bridge JSON view"
        VERBATIM
)

add_custom_target(flowmini_dump_ast
        COMMAND
        $<TARGET_FILE:flowmini>
        --dump-ast
        examples/pass/fn_demo.flow
        DEPENDS flowmini
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Dumping Flowmini AST JSON view"
        VERBATIM
)

add_custom_target(flowmini_dump_ast_symbols
    COMMAND
        $<TARGET_FILE:flowmini>
        --dump-ast-symbols
        -
        examples/ast/type_reference_probe.flow
    DEPENDS flowmini
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Dumping Flowmini AST SymbolTable projection"
    VERBATIM
)

add_custom_target(flowmini_dump_frontend_bundle
    COMMAND
        $<TARGET_FILE:flowmini>
        --dump-frontend-bundle
        examples/ast/type_reference_probe.flow
    DEPENDS flowmini
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Dumping Flowmini frontend bundle JSON"
    VERBATIM
)

add_custom_target(flowmini_symbol_projection_tests
    COMMAND
        ${CMAKE_COMMAND} -E env
        FLOWMINI_ROOT=${CMAKE_CURRENT_SOURCE_DIR}
        FLOWMINI_BIN=$<TARGET_FILE:flowmini>
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/run-flowmini-symbol-projection-tests.sh
    DEPENDS flowmini
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Running Flowmini AST SymbolTable projection golden tests"
    VERBATIM
)

add_custom_target(flowmini_frontend_bundle_tests
    COMMAND
        ${CMAKE_COMMAND} -E env
        FLOWMINI_ROOT=${CMAKE_CURRENT_SOURCE_DIR}
        FLOWMINI_BIN=$<TARGET_FILE:flowmini>
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/run-flowmini-frontend-bundle-tests.sh
    DEPENDS flowmini
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Running independent Flowmini frontend bundle consumer tests"
    VERBATIM
)

add_custom_target(flowmini_ast_golden_tests
    COMMAND
        ${CMAKE_COMMAND} -E env
        FLOWMINI_BIN=$<TARGET_FILE:flowmini>
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/run-flowmini-ast-golden-tests.sh
    DEPENDS flowmini
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Running Flowmini AST golden regression tests"
    VERBATIM
)

add_test(NAME flowmini_utf8_source_boundary
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWMINI_BIN=$<TARGET_FILE:flowmini>
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/run-flowmini-utf8-tests.sh)
