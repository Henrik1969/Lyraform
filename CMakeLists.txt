cmake_minimum_required(VERSION 3.22)
project(lyraform LANGUAGES C CXX)

include(CTest)
include(GNUInstallDirs)

# This is an additive superbuild. Each sibling keeps its own CMake project,
# tests, and artifact contract; the root only supplies one clean build graph.
add_subdirectory(Flowcontracts ${CMAKE_CURRENT_BINARY_DIR}/flowcontracts)
add_subdirectory(subprojects/TinyVM ${CMAKE_CURRENT_BINARY_DIR}/tinyvm)
add_subdirectory(Lyraform/compiler
    ${CMAKE_CURRENT_BINARY_DIR}/flowmini)
add_subdirectory(Flowanalyst ${CMAKE_CURRENT_BINARY_DIR}/flowanalyst)
add_subdirectory(Flowbind ${CMAKE_CURRENT_BINARY_DIR}/flowbind)
add_subdirectory(Flowoptimize ${CMAKE_CURRENT_BINARY_DIR}/flowoptimize)

# The example pipeline must use this build's tools, including forward target
# references, rather than executables left in sibling source directories.
set(FLOWCORE_FLOWMINI "$<TARGET_FILE:flowmini>")
set(FLOWCORE_FLOWANALYST "$<TARGET_FILE:flowanalyst>")
set(FLOWCORE_FLOWBIND "$<TARGET_FILE:flowbind>")
set(FLOWCORE_FLOWOPTIMIZE "$<TARGET_FILE:flowoptimize>")
set(FLOWCORE_FLOWLOWER "$<TARGET_FILE:flowlower>")

# Flowtools owns the Frankencore reference providers and the Flowparallel
# subproject. Including it here avoids duplicating that target graph.
add_subdirectory(Flowtools ${CMAKE_CURRENT_BINARY_DIR}/flowtools)
add_subdirectory(Flowlower ${CMAKE_CURRENT_BINARY_DIR}/flowlower)
add_subdirectory(Flowkernel ${CMAKE_CURRENT_BINARY_DIR}/flowkernel)
add_subdirectory(Lyraform/compiler/examples/apps/flow_less
    ${CMAKE_CURRENT_BINARY_DIR}/flow_less_providers)


add_test(NAME flowcore_integration_corpus
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWMINI_BIN=$<TARGET_FILE:flowmini>
        FLOWANALYST_BIN=$<TARGET_FILE:flowanalyst>
        FLOWPARALLEL_BIN=$<TARGET_FILE:flowparallel>
        FLOWOPTIMIZE_BIN=$<TARGET_FILE:flowoptimize>
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/run-flowcore-integration-corpus.sh)
set_tests_properties(flowcore_integration_corpus PROPERTIES
    DEPENDS "flowanalyst_pipeline")

add_test(NAME flowcore_pipeline_matrix
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWCORE_ROOT=${CMAKE_CURRENT_SOURCE_DIR}
        FLOWMINI_BIN=$<TARGET_FILE:flowmini>
        FLOWANALYST_BIN=$<TARGET_FILE:flowanalyst>
        FLOWPARALLEL_BIN=$<TARGET_FILE:flowparallel>
        FLOWOPTIMIZE_BIN=$<TARGET_FILE:flowoptimize>
        FLOWPREPARE_BIN=$<TARGET_FILE:flowprepare>
        FLOWBIND_BIN=$<TARGET_FILE:flowbind>
        FLOWLOWER_BIN=$<TARGET_FILE:flowlower>
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/run-flowcore-pipeline-matrix.sh)

add_test(NAME flowcore_pass_corpus
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWCORE_ROOT=${CMAKE_CURRENT_SOURCE_DIR}
        FLOWMINI_BIN=$<TARGET_FILE:flowmini>
        FLOWANALYST_BIN=$<TARGET_FILE:flowanalyst>
        FLOWPARALLEL_BIN=$<TARGET_FILE:flowparallel>
        FLOWOPTIMIZE_BIN=$<TARGET_FILE:flowoptimize>
        FLOWBIND_BIN=$<TARGET_FILE:flowbind>
        FLOWLOWER_BIN=$<TARGET_FILE:flowlower>
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/run-flowcore-pass-corpus.sh)

add_test(NAME flowcore_stdlib_boundary
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWCORE_ROOT=${CMAKE_CURRENT_SOURCE_DIR}
        FLOWMINI_BIN=$<TARGET_FILE:flowmini>
        FLOWANALYST_BIN=$<TARGET_FILE:flowanalyst>
        FLOWBIND_BIN=$<TARGET_FILE:flowbind>
        FLOWPARALLEL_BIN=$<TARGET_FILE:flowparallel>
        FLOWOPTIMIZE_BIN=$<TARGET_FILE:flowoptimize>
        FLOWLOWER_BIN=$<TARGET_FILE:flowlower>
        FLOWTESTABI_LAYOUT_BIN=$<TARGET_FILE:flowmini_testabi_layout>
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/run-flowcore-stdlib-boundary.sh)
set_tests_properties(flowcore_stdlib_boundary PROPERTIES
    DEPENDS "flowbind_provider")

add_test(NAME native_binding_inventory
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tools/test-native-binding-inventory.sh)

add_test(NAME stage0_seed_capture
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWCORE_ROOT=${CMAKE_CURRENT_SOURCE_DIR}
        FLOWVALIDATE_BIN=$<TARGET_FILE:flowvalidate>
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/test-stage0-seed-capture.sh)

add_test(NAME callable_lowering_boundary
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWCORE_ROOT=${CMAKE_CURRENT_SOURCE_DIR}
        FLOWMINI_BIN=$<TARGET_FILE:flowmini>
        FLOWANALYST_BIN=$<TARGET_FILE:flowanalyst>
        FLOWPARALLEL_BIN=$<TARGET_FILE:flowparallel>
        FLOWOPTIMIZE_BIN=$<TARGET_FILE:flowoptimize>
        FLOWPREPARE_BIN=$<TARGET_FILE:flowprepare>
        FLOWVALIDATE_BIN=$<TARGET_FILE:flowvalidate>
        FLOWLOWER_BIN=$<TARGET_FILE:flowlower>
        FLOWTINYLOWER_BIN=$<TARGET_FILE:flowtinylower>
        FLOWTINYVALIDATE_BIN=$<TARGET_FILE:flowtinyvalidate>
        FLOWTINYRUN_BIN=$<TARGET_FILE:flowtinyrun>
        FLOWBIND_BIN=$<TARGET_FILE:flowbind>
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/test-callable-lowering-boundary.sh)

add_test(NAME bootstrap_gap_inventory
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWCORE_ROOT=${CMAKE_CURRENT_SOURCE_DIR}
        FLOWVALIDATE_BIN=$<TARGET_FILE:flowvalidate>
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/test-bootstrap-gap-inventory.sh)

add_test(NAME native_binding_selection
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tools/test-native-binding-selection.sh)

add_test(NAME native_binding_generation
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWCORE_ROOT=${CMAKE_CURRENT_SOURCE_DIR}
        FLOWMINI_BIN=$<TARGET_FILE:flowmini>
        FLOWANALYST_BIN=$<TARGET_FILE:flowanalyst>
        FLOWBIND_BIN=$<TARGET_FILE:flowbind>
        FLOWPARALLEL_BIN=$<TARGET_FILE:flowparallel>
        FLOWOPTIMIZE_BIN=$<TARGET_FILE:flowoptimize>
        FLOWLOWER_BIN=$<TARGET_FILE:flowlower>
        FLOWPREPARE_BIN=$<TARGET_FILE:flowprepare>
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/test-generate-flow-bindings.sh)

add_test(NAME native_ncurses_binding
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWBIND_BIN=$<TARGET_FILE:flowbind>
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/test-ncurses-binding.sh)
set_tests_properties(native_ncurses_binding PROPERTIES
    DEPENDS "flowbind_provider")

add_test(NAME ncurses_flow_pipeline
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWCORE_ROOT=${CMAKE_CURRENT_SOURCE_DIR}
        FLOWMINI_BIN=$<TARGET_FILE:flowmini>
        FLOWANALYST_BIN=$<TARGET_FILE:flowanalyst>
        FLOWPARALLEL_BIN=$<TARGET_FILE:flowparallel>
        FLOWOPTIMIZE_BIN=$<TARGET_FILE:flowoptimize>
        FLOWBIND_BIN=$<TARGET_FILE:flowbind>
        FLOWLOWER_BIN=$<TARGET_FILE:flowlower>
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/test-ncurses-pipeline.sh)
set_tests_properties(ncurses_flow_pipeline PROPERTIES
    DEPENDS "flowcore_pass_corpus;flowlower_pipeline")

add_test(NAME sel_tui_pipeline
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWCORE_ROOT=${CMAKE_CURRENT_SOURCE_DIR}
        FLOWMINI_BIN=$<TARGET_FILE:flowmini>
        FLOWANALYST_BIN=$<TARGET_FILE:flowanalyst>
        FLOWPARALLEL_BIN=$<TARGET_FILE:flowparallel>
        FLOWOPTIMIZE_BIN=$<TARGET_FILE:flowoptimize>
        FLOWBIND_BIN=$<TARGET_FILE:flowbind>
        FLOWLOWER_BIN=$<TARGET_FILE:flowlower>
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/test-sel-pipeline.sh)
set_tests_properties(sel_tui_pipeline PROPERTIES
    DEPENDS "ncurses_flow_pipeline")

add_test(NAME flowcore_graph_routing
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWMINI_BIN=$<TARGET_FILE:flowmini>
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/test-flowcore-graph-routing.sh)
set_tests_properties(flowcore_graph_routing PROPERTIES
    DEPENDS "flowcore_pass_corpus")

add_test(NAME flow_less_pager
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWMINI_BIN=$<TARGET_FILE:flowmini>
        FLOWANALYST_BIN=$<TARGET_FILE:flowanalyst>
        FLOWBIND_BIN=$<TARGET_FILE:flowbind>
        FLOWPARALLEL_BIN=$<TARGET_FILE:flowparallel>
        FLOWOPTIMIZE_BIN=$<TARGET_FILE:flowoptimize>
        FLOWPREPARE_BIN=$<TARGET_FILE:flowprepare>
        FLOWLOWER_BIN=$<TARGET_FILE:flowlower>
        FLOWGRAPH_RUNTIME=$<TARGET_FILE:flowgraph_runtime>
        FLOWPAGER_INPUT=$<TARGET_FILE:flowpager_input>
        FLOWPAGER_OUTPUT=$<TARGET_FILE:flowpager_output>
        FLOWGRAPH_CXX=${CMAKE_CXX_COMPILER}
        "FLOWGRAPH_LINK_FLAGS=${CMAKE_CXX_FLAGS}"
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/test-flow-less.sh)
set_tests_properties(flow_less_pager PROPERTIES
    DEPENDS "flowcore_graph_routing")

add_test(NAME graph_lowering_refusal
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWMINI_BIN=$<TARGET_FILE:flowmini>
        FLOWANALYST_BIN=$<TARGET_FILE:flowanalyst>
        FLOWPARALLEL_BIN=$<TARGET_FILE:flowparallel>
        sh ${CMAKE_CURRENT_SOURCE_DIR}/tools/test-graph-lowering-refusal.sh)

add_test(NAME source_receiver_frames
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWMINI_BIN=$<TARGET_FILE:flowmini>
        sh ${CMAKE_CURRENT_SOURCE_DIR}/tools/test-source-receiver-frames.sh)

add_test(NAME source_graph_artifact
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWMINI_BIN=$<TARGET_FILE:flowmini>
        FLOWANALYST_BIN=$<TARGET_FILE:flowanalyst>
        FLOWBIND_BIN=$<TARGET_FILE:flowbind>
        FLOWPARALLEL_BIN=$<TARGET_FILE:flowparallel>
        FLOWOPTIMIZE_BIN=$<TARGET_FILE:flowoptimize>
        FLOWLOWER_BIN=$<TARGET_FILE:flowlower>
        FLOWPREPARE_BIN=$<TARGET_FILE:flowprepare>
        FLOWVALIDATE_BIN=$<TARGET_FILE:flowvalidate>
        sh ${CMAKE_CURRENT_SOURCE_DIR}/tools/test-source-graph-artifact.sh)

add_test(NAME provider_call_identity
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWMINI_BIN=$<TARGET_FILE:flowmini>
        FLOWANALYST_BIN=$<TARGET_FILE:flowanalyst>
        FLOWBIND_BIN=$<TARGET_FILE:flowbind>
        FLOWPARALLEL_BIN=$<TARGET_FILE:flowparallel>
        FLOWOPTIMIZE_BIN=$<TARGET_FILE:flowoptimize>
        FLOWLOWER_BIN=$<TARGET_FILE:flowlower>
        sh ${CMAKE_CURRENT_SOURCE_DIR}/tools/test-provider-call-identity.sh)

add_test(NAME flow_less_ncurses_pager
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWMINI_BIN=$<TARGET_FILE:flowmini>
        FLOWANALYST_BIN=$<TARGET_FILE:flowanalyst>
        FLOWBIND_BIN=$<TARGET_FILE:flowbind>
        FLOWPARALLEL_BIN=$<TARGET_FILE:flowparallel>
        FLOWOPTIMIZE_BIN=$<TARGET_FILE:flowoptimize>
        FLOWPREPARE_BIN=$<TARGET_FILE:flowprepare>
        FLOWLOWER_BIN=$<TARGET_FILE:flowlower>
        FLOWGRAPH_RUNTIME=$<TARGET_FILE:flowgraph_runtime>
        FLOWPAGER_INPUT=$<TARGET_FILE:flowpager_ncurses_input>
        FLOWPAGER_OUTPUT=$<TARGET_FILE:flowpager_output>
        FLOWGRAPH_CXX=${CMAKE_CXX_COMPILER}
        "FLOWGRAPH_LINK_FLAGS=${CMAKE_CXX_FLAGS}"
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/test-flow-less-ncurses.sh)
set_tests_properties(flow_less_ncurses_pager PROPERTIES
    DEPENDS "flow_less_pager")

add_test(NAME profile_free_generic_lowering
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWCORE_ROOT=${CMAKE_CURRENT_SOURCE_DIR}
        FLOWMINI_BIN=$<TARGET_FILE:flowmini>
        FLOWANALYST_BIN=$<TARGET_FILE:flowanalyst>
        FLOWPARALLEL_BIN=$<TARGET_FILE:flowparallel>
        FLOWOPTIMIZE_BIN=$<TARGET_FILE:flowoptimize>
        FLOWBIND_BIN=$<TARGET_FILE:flowbind>
        FLOWLOWER_BIN=$<TARGET_FILE:flowlower>
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/test-profile-free-lowering.sh)
set_tests_properties(profile_free_generic_lowering PROPERTIES
    DEPENDS "flow_less_ncurses_pager")

add_test(NAME text_value_boundary
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWCORE_ROOT=${CMAKE_CURRENT_SOURCE_DIR}
        FLOWMINI_BIN=$<TARGET_FILE:flowmini>
        FLOWANALYST_BIN=$<TARGET_FILE:flowanalyst>
        FLOWPARALLEL_BIN=$<TARGET_FILE:flowparallel>
        FLOWOPTIMIZE_BIN=$<TARGET_FILE:flowoptimize>
        FLOWBIND_BIN=$<TARGET_FILE:flowbind>
        FLOWLOWER_BIN=$<TARGET_FILE:flowlower>
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/test-text-value.sh)
set_tests_properties(text_value_boundary PROPERTIES
    DEPENDS "profile_free_generic_lowering")

add_test(NAME text_runtime_boundary
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWCORE_ROOT=${CMAKE_CURRENT_SOURCE_DIR}
        FLOWMINI_BIN=$<TARGET_FILE:flowmini>
        FLOWANALYST_BIN=$<TARGET_FILE:flowanalyst>
        FLOWPARALLEL_BIN=$<TARGET_FILE:flowparallel>
        FLOWOPTIMIZE_BIN=$<TARGET_FILE:flowoptimize>
        FLOWBIND_BIN=$<TARGET_FILE:flowbind>
        FLOWLOWER_BIN=$<TARGET_FILE:flowlower>
        FLOWVALIDATE_BIN=$<TARGET_FILE:flowvalidate>
        FLOWTEXT_RUNTIME=$<TARGET_FILE:flowtext_runtime>
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/test-text-runtime.sh)
set_tests_properties(text_runtime_boundary PROPERTIES
    DEPENDS "text_value_boundary")

add_test(NAME text_runtime_tagged_api
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWCORE_ROOT=${CMAKE_CURRENT_SOURCE_DIR}
        FLOWTEXT_RUNTIME=$<TARGET_FILE:flowtext_runtime>
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/test-text-runtime-api.sh)
set_tests_properties(text_runtime_tagged_api PROPERTIES
    DEPENDS "text_runtime_boundary")

add_test(NAME text_recovery_boundary
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWCORE_ROOT=${CMAKE_CURRENT_SOURCE_DIR}
        FLOWMINI_BIN=$<TARGET_FILE:flowmini>
        FLOWANALYST_BIN=$<TARGET_FILE:flowanalyst>
        FLOWPARALLEL_BIN=$<TARGET_FILE:flowparallel>
        FLOWOPTIMIZE_BIN=$<TARGET_FILE:flowoptimize>
        FLOWPREPARE_BIN=$<TARGET_FILE:flowprepare>
        FLOWBIND_BIN=$<TARGET_FILE:flowbind>
        FLOWLOWER_BIN=$<TARGET_FILE:flowlower>
        FLOWTINYLOWER_BIN=$<TARGET_FILE:flowtinylower>
        FLOWTINYVALIDATE_BIN=$<TARGET_FILE:flowtinyvalidate>
        FLOWTINYRUN_BIN=$<TARGET_FILE:flowtinyrun>
        FLOWTEXT_RUNTIME=$<TARGET_FILE:flowtext_runtime>
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/test-text-recovery.sh)
set_tests_properties(text_recovery_boundary PROPERTIES
    DEPENDS "text_runtime_tagged_api;tinyvm_memory_parity")

add_test(NAME text_outcome_boundary
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWCORE_ROOT=${CMAKE_CURRENT_SOURCE_DIR}
        FLOWMINI_BIN=$<TARGET_FILE:flowmini>
        FLOWANALYST_BIN=$<TARGET_FILE:flowanalyst>
        FLOWPARALLEL_BIN=$<TARGET_FILE:flowparallel>
        FLOWOPTIMIZE_BIN=$<TARGET_FILE:flowoptimize>
        FLOWPREPARE_BIN=$<TARGET_FILE:flowprepare>
        FLOWBIND_BIN=$<TARGET_FILE:flowbind>
        FLOWLOWER_BIN=$<TARGET_FILE:flowlower>
        FLOWTINYLOWER_BIN=$<TARGET_FILE:flowtinylower>
        FLOWTINYVALIDATE_BIN=$<TARGET_FILE:flowtinyvalidate>
        FLOWTINYRUN_BIN=$<TARGET_FILE:flowtinyrun>
        FLOWTEXT_RUNTIME=$<TARGET_FILE:flowtext_runtime>
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/test-text-outcome.sh)
set_tests_properties(text_outcome_boundary PROPERTIES
    DEPENDS "text_recovery_boundary")

add_test(NAME math_library_boundary
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWCORE_ROOT=${CMAKE_CURRENT_SOURCE_DIR}
        FLOWMINI_BIN=$<TARGET_FILE:flowmini>
        FLOWANALYST_BIN=$<TARGET_FILE:flowanalyst>
        FLOWPARALLEL_BIN=$<TARGET_FILE:flowparallel>
        FLOWOPTIMIZE_BIN=$<TARGET_FILE:flowoptimize>
        FLOWBIND_BIN=$<TARGET_FILE:flowbind>
        FLOWLOWER_BIN=$<TARGET_FILE:flowlower>
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/test-math-boundary.sh)
set_tests_properties(math_library_boundary PROPERTIES
    DEPENDS "text_outcome_boundary")

add_test(NAME strnlen_library_boundary
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWCORE_ROOT=${CMAKE_CURRENT_SOURCE_DIR}
        FLOWMINI_BIN=$<TARGET_FILE:flowmini>
        FLOWANALYST_BIN=$<TARGET_FILE:flowanalyst>
        FLOWPARALLEL_BIN=$<TARGET_FILE:flowparallel>
        FLOWOPTIMIZE_BIN=$<TARGET_FILE:flowoptimize>
        FLOWPREPARE_BIN=$<TARGET_FILE:flowprepare>
        FLOWBIND_BIN=$<TARGET_FILE:flowbind>
        FLOWLOWER_BIN=$<TARGET_FILE:flowlower>
        FLOWTINYLOWER_BIN=$<TARGET_FILE:flowtinylower>
        FLOWTINYVALIDATE_BIN=$<TARGET_FILE:flowtinyvalidate>
        FLOWTINYRUN_BIN=$<TARGET_FILE:flowtinyrun>
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/test-strnlen-boundary.sh)
set_tests_properties(strnlen_library_boundary PROPERTIES
    DEPENDS "math_library_boundary")

add_test(NAME ctype_library_boundary
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWCORE_ROOT=${CMAKE_CURRENT_SOURCE_DIR}
        FLOWMINI_BIN=$<TARGET_FILE:flowmini>
        FLOWANALYST_BIN=$<TARGET_FILE:flowanalyst>
        FLOWPARALLEL_BIN=$<TARGET_FILE:flowparallel>
        FLOWOPTIMIZE_BIN=$<TARGET_FILE:flowoptimize>
        FLOWPREPARE_BIN=$<TARGET_FILE:flowprepare>
        FLOWBIND_BIN=$<TARGET_FILE:flowbind>
        FLOWLOWER_BIN=$<TARGET_FILE:flowlower>
        FLOWTINYLOWER_BIN=$<TARGET_FILE:flowtinylower>
        FLOWTINYVALIDATE_BIN=$<TARGET_FILE:flowtinyvalidate>
        FLOWTINYRUN_BIN=$<TARGET_FILE:flowtinyrun>
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/test-ctype-boundary.sh)
set_tests_properties(ctype_library_boundary PROPERTIES
    DEPENDS "strnlen_library_boundary")

add_test(NAME file_resource_boundary
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWCORE_ROOT=${CMAKE_CURRENT_SOURCE_DIR}
        FLOWMINI_BIN=$<TARGET_FILE:flowmini>
        FLOWANALYST_BIN=$<TARGET_FILE:flowanalyst>
        FLOWPARALLEL_BIN=$<TARGET_FILE:flowparallel>
        FLOWOPTIMIZE_BIN=$<TARGET_FILE:flowoptimize>
        FLOWPREPARE_BIN=$<TARGET_FILE:flowprepare>
        FLOWBIND_BIN=$<TARGET_FILE:flowbind>
        FLOWLOWER_BIN=$<TARGET_FILE:flowlower>
        FLOWTINYLOWER_BIN=$<TARGET_FILE:flowtinylower>
        FLOWTINYVALIDATE_BIN=$<TARGET_FILE:flowtinyvalidate>
        FLOWTINYRUN_BIN=$<TARGET_FILE:flowtinyrun>
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/test-file-resource-boundary.sh)
set_tests_properties(file_resource_boundary PROPERTIES
    DEPENDS "ctype_library_boundary")

add_test(NAME file_io_boundary
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWCORE_ROOT=${CMAKE_CURRENT_SOURCE_DIR}
        FLOWMINI_BIN=$<TARGET_FILE:flowmini>
        FLOWANALYST_BIN=$<TARGET_FILE:flowanalyst>
        FLOWPARALLEL_BIN=$<TARGET_FILE:flowparallel>
        FLOWOPTIMIZE_BIN=$<TARGET_FILE:flowoptimize>
        FLOWPREPARE_BIN=$<TARGET_FILE:flowprepare>
        FLOWBIND_BIN=$<TARGET_FILE:flowbind>
        FLOWLOWER_BIN=$<TARGET_FILE:flowlower>
        FLOWTINYLOWER_BIN=$<TARGET_FILE:flowtinylower>
        FLOWTINYVALIDATE_BIN=$<TARGET_FILE:flowtinyvalidate>
        FLOWTINYRUN_BIN=$<TARGET_FILE:flowtinyrun>
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/test-file-io-boundary.sh)
set_tests_properties(file_io_boundary PROPERTIES
    DEPENDS "file_resource_boundary")

add_test(NAME namespace_ambiguity
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWMINI_BIN=$<TARGET_FILE:flowmini>
        FLOWANALYST_BIN=$<TARGET_FILE:flowanalyst>
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/test-namespace-ambiguity.sh)
set_tests_properties(namespace_ambiguity PROPERTIES
    DEPENDS "flowanalyst_pipeline")

add_test(NAME tinyvm_scalar_backend_parity
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWCORE_ROOT=${CMAKE_CURRENT_SOURCE_DIR}
        FLOWMINI_BIN=$<TARGET_FILE:flowmini>
        FLOWANALYST_BIN=$<TARGET_FILE:flowanalyst>
        FLOWPARALLEL_BIN=$<TARGET_FILE:flowparallel>
        FLOWOPTIMIZE_BIN=$<TARGET_FILE:flowoptimize>
        FLOWPREPARE_BIN=$<TARGET_FILE:flowprepare>
        FLOWLOWER_BIN=$<TARGET_FILE:flowlower>
        FLOWTINYLOWER_BIN=$<TARGET_FILE:flowtinylower>
        FLOWTINYVALIDATE_BIN=$<TARGET_FILE:flowtinyvalidate>
        FLOWTINYRUN_BIN=$<TARGET_FILE:flowtinyrun>
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/test-tinyvm-scalar-parity.sh)
set_tests_properties(tinyvm_scalar_backend_parity PROPERTIES
    DEPENDS "flowlower_backend_artifact;tinyvm_backend_lowering_boundary")

add_test(NAME tinyvm_graph_parity
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWCORE_ROOT=${CMAKE_CURRENT_SOURCE_DIR}
        FLOWMINI_BIN=$<TARGET_FILE:flowmini>
        FLOWANALYST_BIN=$<TARGET_FILE:flowanalyst>
        FLOWBIND_BIN=$<TARGET_FILE:flowbind>
        FLOWPARALLEL_BIN=$<TARGET_FILE:flowparallel>
        FLOWOPTIMIZE_BIN=$<TARGET_FILE:flowoptimize>
        FLOWPREPARE_BIN=$<TARGET_FILE:flowprepare>
        FLOWLOWER_BIN=$<TARGET_FILE:flowlower>
        FLOWTINYLOWER_BIN=$<TARGET_FILE:flowtinylower>
        FLOWTINYVALIDATE_BIN=$<TARGET_FILE:flowtinyvalidate>
        FLOWTINYRUN_BIN=$<TARGET_FILE:flowtinyrun>
        FLOWGRAPH_RUNTIME=$<TARGET_FILE:flowgraph_runtime>
        FLOWGRAPH_CXX=${CMAKE_CXX_COMPILER}
        "FLOWGRAPH_LINK_FLAGS=${CMAKE_CXX_FLAGS}"
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/test-tinyvm-graph-parity.sh)
set_tests_properties(tinyvm_graph_parity PROPERTIES
    DEPENDS "tinyvm_scalar_backend_parity")

add_test(NAME tinyvm_stream_parity
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWCORE_ROOT=${CMAKE_CURRENT_SOURCE_DIR}
        FLOWMINI_BIN=$<TARGET_FILE:flowmini>
        FLOWANALYST_BIN=$<TARGET_FILE:flowanalyst>
        FLOWBIND_BIN=$<TARGET_FILE:flowbind>
        FLOWPARALLEL_BIN=$<TARGET_FILE:flowparallel>
        FLOWOPTIMIZE_BIN=$<TARGET_FILE:flowoptimize>
        FLOWPREPARE_BIN=$<TARGET_FILE:flowprepare>
        FLOWLOWER_BIN=$<TARGET_FILE:flowlower>
        FLOWTINYLOWER_BIN=$<TARGET_FILE:flowtinylower>
        FLOWTINYVALIDATE_BIN=$<TARGET_FILE:flowtinyvalidate>
        FLOWTINYRUN_BIN=$<TARGET_FILE:flowtinyrun>
        FLOWGRAPH_RUNTIME=$<TARGET_FILE:flowgraph_runtime>
        FLOWGRAPH_CXX=${CMAKE_CXX_COMPILER}
        "FLOWGRAPH_LINK_FLAGS=${CMAKE_CXX_FLAGS}"
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/test-tinyvm-stream-parity.sh)
set_tests_properties(tinyvm_stream_parity PROPERTIES
    DEPENDS "tinyvm_graph_parity")

add_test(NAME tinyvm_governed_provider_parity
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWCORE_ROOT=${CMAKE_CURRENT_SOURCE_DIR}
        FLOWMINI_BIN=$<TARGET_FILE:flowmini>
        FLOWANALYST_BIN=$<TARGET_FILE:flowanalyst>
        FLOWBIND_BIN=$<TARGET_FILE:flowbind>
        FLOWPARALLEL_BIN=$<TARGET_FILE:flowparallel>
        FLOWOPTIMIZE_BIN=$<TARGET_FILE:flowoptimize>
        FLOWPREPARE_BIN=$<TARGET_FILE:flowprepare>
        FLOWLOWER_BIN=$<TARGET_FILE:flowlower>
        FLOWTINYLOWER_BIN=$<TARGET_FILE:flowtinylower>
        FLOWTINYVALIDATE_BIN=$<TARGET_FILE:flowtinyvalidate>
        FLOWTINYRUN_BIN=$<TARGET_FILE:flowtinyrun>
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/test-tinyvm-pure-provider-parity.sh)
set_tests_properties(tinyvm_governed_provider_parity PROPERTIES
    DEPENDS "tinyvm_scalar_backend_parity")

add_test(NAME tinyvm_text_boundary_parity
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWCORE_ROOT=${CMAKE_CURRENT_SOURCE_DIR}
        FLOWMINI_BIN=$<TARGET_FILE:flowmini>
        FLOWANALYST_BIN=$<TARGET_FILE:flowanalyst>
        FLOWPARALLEL_BIN=$<TARGET_FILE:flowparallel>
        FLOWOPTIMIZE_BIN=$<TARGET_FILE:flowoptimize>
        FLOWPREPARE_BIN=$<TARGET_FILE:flowprepare>
        FLOWBIND_BIN=$<TARGET_FILE:flowbind>
        FLOWLOWER_BIN=$<TARGET_FILE:flowlower>
        FLOWTINYLOWER_BIN=$<TARGET_FILE:flowtinylower>
        FLOWTINYVALIDATE_BIN=$<TARGET_FILE:flowtinyvalidate>
        FLOWTINYRUN_BIN=$<TARGET_FILE:flowtinyrun>
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/test-tinyvm-text-parity.sh)
set_tests_properties(tinyvm_text_boundary_parity PROPERTIES
    DEPENDS "text_value_boundary;tinyvm_governed_provider_parity")

add_test(NAME tinyvm_runtime_text_parity
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWCORE_ROOT=${CMAKE_CURRENT_SOURCE_DIR}
        FLOWMINI_BIN=$<TARGET_FILE:flowmini>
        FLOWANALYST_BIN=$<TARGET_FILE:flowanalyst>
        FLOWPARALLEL_BIN=$<TARGET_FILE:flowparallel>
        FLOWOPTIMIZE_BIN=$<TARGET_FILE:flowoptimize>
        FLOWPREPARE_BIN=$<TARGET_FILE:flowprepare>
        FLOWBIND_BIN=$<TARGET_FILE:flowbind>
        FLOWLOWER_BIN=$<TARGET_FILE:flowlower>
        FLOWTINYLOWER_BIN=$<TARGET_FILE:flowtinylower>
        FLOWTINYVALIDATE_BIN=$<TARGET_FILE:flowtinyvalidate>
        FLOWTINYRUN_BIN=$<TARGET_FILE:flowtinyrun>
        FLOWTEXT_RUNTIME=$<TARGET_FILE:flowtext_runtime>
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/test-tinyvm-runtime-text-parity.sh)
set_tests_properties(tinyvm_runtime_text_parity PROPERTIES
    DEPENDS "text_runtime_boundary")

add_test(NAME tinyvm_memory_parity
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWCORE_ROOT=${CMAKE_CURRENT_SOURCE_DIR}
        FLOWMINI_BIN=$<TARGET_FILE:flowmini>
        FLOWANALYST_BIN=$<TARGET_FILE:flowanalyst>
        FLOWPARALLEL_BIN=$<TARGET_FILE:flowparallel>
        FLOWOPTIMIZE_BIN=$<TARGET_FILE:flowoptimize>
        FLOWPREPARE_BIN=$<TARGET_FILE:flowprepare>
        FLOWBIND_BIN=$<TARGET_FILE:flowbind>
        FLOWLOWER_BIN=$<TARGET_FILE:flowlower>
        FLOWTINYLOWER_BIN=$<TARGET_FILE:flowtinylower>
        FLOWTINYVALIDATE_BIN=$<TARGET_FILE:flowtinyvalidate>
        FLOWTINYRUN_BIN=$<TARGET_FILE:flowtinyrun>
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/test-tinyvm-memory-parity.sh)
set_tests_properties(tinyvm_memory_parity PROPERTIES
    DEPENDS "flowcore_stdlib_boundary;tinyvm_runtime_text_parity")

# Canonical installed shape. Debian packaging consumes this staged install and
# splits runtime, development, and documentation payloads without depending on
# the source-tree layout.
install(TARGETS
    flowmini
    flowanalyst
    flowbind
    flowoptimize
    flowprepare
    flowtarget
    flowlower
    flowkernel
    flowparallel
    franken_ls
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(TARGETS
    flowgraph_runtime
    flowtext_runtime
    flowparallel_cpu_runtime
    TokenTree_static
    symboltable_static
    symboltable_shared
    frankencore_core_contracts
    frankencore_core_language
    frankencore_core_packages
    frankencore_core_provenance
    frankencore_core_requirements
    frankencore_core_runtime
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/flowcore
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/flowcore
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}/Lyraform/compiler/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/flowcore/flowmini
    FILES_MATCHING PATTERN "*.h"
)

install(DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}/Flowparallel/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/flowcore
    FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h"
)

install(DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}/Lyraform/compiler/subprojects/SymbolTable/SymbolTable_v1_1/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/flowcore
    FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h"
)

install(DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}/Frankencore/Core/Contracts/include/
    ${CMAKE_CURRENT_SOURCE_DIR}/Frankencore/Core/Language/include/
    ${CMAKE_CURRENT_SOURCE_DIR}/Frankencore/Core/Packages/include/
    ${CMAKE_CURRENT_SOURCE_DIR}/Frankencore/Core/Policy/include/
    ${CMAKE_CURRENT_SOURCE_DIR}/Frankencore/Core/Provenance/include/
    ${CMAKE_CURRENT_SOURCE_DIR}/Frankencore/Core/Requirements/include/
    ${CMAKE_CURRENT_SOURCE_DIR}/Frankencore/Core/Runtime/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/flowcore
    FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h"
)

install(DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}/Lyraform/compiler/std/
    DESTINATION ${CMAKE_INSTALL_DATADIR}/flowcore/std
    FILES_MATCHING PATTERN "*.flow"
)

install(FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Lyraform/compiler/examples/apps/flowcat/policy.conf
    DESTINATION ${CMAKE_INSTALL_DATADIR}/flowcore/examples/flowcat
)

install(PROGRAMS tools/generate-flow-bindings.sh
    DESTINATION ${CMAKE_INSTALL_BINDIR} RENAME generate-flow-bindings)

install(PROGRAMS igor
    DESTINATION ${CMAKE_INSTALL_BINDIR})

install(FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/README.md
    ${CMAKE_CURRENT_SOURCE_DIR}/REFERENCES.md
    ${CMAKE_CURRENT_SOURCE_DIR}/ACKNOWLEDGEMENTS.md
    ${CMAKE_CURRENT_SOURCE_DIR}/docs/history/FLOWCORE_TO_LYRAFORM.md
    ${CMAKE_CURRENT_SOURCE_DIR}/CHANGELOG.md
    ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.md
    ${CMAKE_CURRENT_SOURCE_DIR}/docs/flowmini/stdlib-capability-matrix.md
    DESTINATION ${CMAKE_INSTALL_DOCDIR}
)

add_test(NAME native_source_graph
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWCORE_ROOT=${CMAKE_CURRENT_SOURCE_DIR}
        FLOWMINI_BIN=$<TARGET_FILE:flowmini>
        FLOWANALYST_BIN=$<TARGET_FILE:flowanalyst>
        FLOWBIND_BIN=$<TARGET_FILE:flowbind>
        FLOWPARALLEL_BIN=$<TARGET_FILE:flowparallel>
        FLOWOPTIMIZE_BIN=$<TARGET_FILE:flowoptimize>
        FLOWPREPARE_BIN=$<TARGET_FILE:flowprepare>
        FLOWLOWER_BIN=$<TARGET_FILE:flowlower>
        FLOWGRAPH_RUNTIME=$<TARGET_FILE:flowgraph_runtime>
        FLOWGRAPH_CXX=${CMAKE_CXX_COMPILER}
        "FLOWGRAPH_LINK_FLAGS=${CMAKE_CXX_FLAGS}"
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/test-native-source-graph.sh)

add_test(NAME native_aggregate_graph
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWCORE_ROOT=${CMAKE_CURRENT_SOURCE_DIR}
        FLOWMINI_BIN=$<TARGET_FILE:flowmini>
        FLOWANALYST_BIN=$<TARGET_FILE:flowanalyst>
        FLOWBIND_BIN=$<TARGET_FILE:flowbind>
        FLOWPARALLEL_BIN=$<TARGET_FILE:flowparallel>
        FLOWOPTIMIZE_BIN=$<TARGET_FILE:flowoptimize>
        FLOWPREPARE_BIN=$<TARGET_FILE:flowprepare>
        FLOWLOWER_BIN=$<TARGET_FILE:flowlower>
        FLOWTESTABI_LAYOUT_BIN=$<TARGET_FILE:flowmini_testabi_layout>
        FLOWTESTABI_LIBRARY=$<TARGET_FILE:flowmini_testabi>
        FLOWGRAPH_RUNTIME=$<TARGET_FILE:flowgraph_runtime>
        FLOWGRAPH_CXX=${CMAKE_CXX_COMPILER}
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/test-native-aggregate-graph.sh)

add_test(NAME native_parallel_graph
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWCORE_ROOT=${CMAKE_CURRENT_SOURCE_DIR}
        FLOWMINI_BIN=$<TARGET_FILE:flowmini>
        FLOWANALYST_BIN=$<TARGET_FILE:flowanalyst>
        FLOWBIND_BIN=$<TARGET_FILE:flowbind>
        FLOWPARALLEL_BIN=$<TARGET_FILE:flowparallel>
        FLOWOPTIMIZE_BIN=$<TARGET_FILE:flowoptimize>
        FLOWPREPARE_BIN=$<TARGET_FILE:flowprepare>
        FLOWLOWER_BIN=$<TARGET_FILE:flowlower>
        FLOWGRAPH_RUNTIME=$<TARGET_FILE:flowgraph_runtime>
        FLOWGRAPH_CXX=${CMAKE_CXX_COMPILER}
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/test-native-parallel-graph.sh)

add_test(NAME native_aggregate_parallel_graph
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWCORE_ROOT=${CMAKE_CURRENT_SOURCE_DIR}
        FLOWMINI_BIN=$<TARGET_FILE:flowmini>
        FLOWANALYST_BIN=$<TARGET_FILE:flowanalyst>
        FLOWBIND_BIN=$<TARGET_FILE:flowbind>
        FLOWPARALLEL_BIN=$<TARGET_FILE:flowparallel>
        FLOWOPTIMIZE_BIN=$<TARGET_FILE:flowoptimize>
        FLOWPREPARE_BIN=$<TARGET_FILE:flowprepare>
        FLOWLOWER_BIN=$<TARGET_FILE:flowlower>
        FLOWTESTABI_LAYOUT_BIN=$<TARGET_FILE:flowmini_testabi_layout>
        FLOWTESTABI_LIBRARY=$<TARGET_FILE:flowmini_testabi>
        FLOWGRAPH_RUNTIME=$<TARGET_FILE:flowgraph_runtime>
        FLOWGRAPH_CXX=${CMAKE_CXX_COMPILER}
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/test-native-aggregate-parallel-graph.sh)

add_test(NAME flowmini_support_boundary
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWCORE_ROOT=${CMAKE_CURRENT_SOURCE_DIR}
        FLOWMINI_BIN=$<TARGET_FILE:flowmini>
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/test-support-boundary.sh)
