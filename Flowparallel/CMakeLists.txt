cmake_minimum_required(VERSION 3.22)
project(flowparallel LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
add_executable(flowparallel src/main.cpp)
target_include_directories(flowparallel PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_options(flowparallel PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(flowparallel PRIVATE flowcontracts)
add_executable(flowparallel_allocation_fault src/main.cpp)
target_compile_definitions(flowparallel_allocation_fault PRIVATE FLOWPARALLEL_TEST_ALLOCATION_FAILURE)
target_include_directories(flowparallel_allocation_fault PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_options(flowparallel_allocation_fault PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(flowparallel_allocation_fault PRIVATE flowcontracts)

add_executable(flowparallel_smoketest tests/parallel_smoketest.cpp)
target_compile_options(flowparallel_smoketest PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(flowparallel_smoketest PRIVATE Threads::Threads)

add_executable(flowparallel_cuda_resource_cleanup_test tests/cuda_resource_cleanup.cpp)
target_include_directories(flowparallel_cuda_resource_cleanup_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_options(flowparallel_cuda_resource_cleanup_test PRIVATE -Wall -Wextra -Wpedantic)

add_executable(flowparallel_cpu src/cpu_provider.cpp)
target_include_directories(flowparallel_cpu PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_options(flowparallel_cpu PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(flowparallel_cpu PRIVATE flowcontracts)
add_executable(flowparallel_cpu_allocation_fault src/cpu_provider.cpp)
target_compile_definitions(flowparallel_cpu_allocation_fault PRIVATE FLOWPARALLEL_CPU_TEST_ALLOCATION_FAILURE)
target_include_directories(flowparallel_cpu_allocation_fault PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_options(flowparallel_cpu_allocation_fault PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(flowparallel_cpu_allocation_fault PRIVATE flowcontracts)

add_executable(flowparallel_cuda src/cuda_provider.cpp)
target_include_directories(flowparallel_cuda PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_options(flowparallel_cuda PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(flowparallel_cuda PRIVATE flowcontracts ${CMAKE_DL_LIBS})
add_executable(flowparallel_cuda_allocation_fault src/cuda_provider.cpp)
target_compile_definitions(flowparallel_cuda_allocation_fault PRIVATE FLOWPARALLEL_CUDA_TEST_ALLOCATION_FAILURE)
target_include_directories(flowparallel_cuda_allocation_fault PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_options(flowparallel_cuda_allocation_fault PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(flowparallel_cuda_allocation_fault PRIVATE flowcontracts ${CMAKE_DL_LIBS})

add_executable(flowparallel_cuda_execute src/cuda_execute.cpp)
target_include_directories(flowparallel_cuda_execute PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_options(flowparallel_cuda_execute PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(flowparallel_cuda_execute PRIVATE ${CMAKE_DL_LIBS})

add_executable(flowparallel_matrix_benchmark src/matrix_benchmark.cpp)
target_include_directories(flowparallel_matrix_benchmark PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_options(flowparallel_matrix_benchmark PRIVATE -Wall -Wextra -Wpedantic -O3)
target_link_libraries(flowparallel_matrix_benchmark PRIVATE ${CMAKE_DL_LIBS})

add_executable(flowparallel_runtime_planner src/runtime_planner.cpp)
target_include_directories(flowparallel_runtime_planner PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_options(flowparallel_runtime_planner PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(flowparallel_runtime_planner PRIVATE flowcontracts)
add_executable(flowparallel_runtime_planner_allocation_fault src/runtime_planner.cpp)
target_compile_definitions(flowparallel_runtime_planner_allocation_fault PRIVATE FLOWPARALLEL_RUNTIME_PLANNER_TEST_ALLOCATION_FAILURE)
target_include_directories(flowparallel_runtime_planner_allocation_fault PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_options(flowparallel_runtime_planner_allocation_fault PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(flowparallel_runtime_planner_allocation_fault PRIVATE flowcontracts)

add_executable(flowparallel_graph_reference src/graph_reference.cpp)
target_include_directories(flowparallel_graph_reference PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_options(flowparallel_graph_reference PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(flowparallel_graph_reference PRIVATE flowcontracts)
add_executable(flowparallel_graph_reference_allocation_fault src/graph_reference.cpp)
target_compile_definitions(flowparallel_graph_reference_allocation_fault PRIVATE FLOWPARALLEL_GRAPH_REFERENCE_TEST_ALLOCATION_FAILURE)
target_include_directories(flowparallel_graph_reference_allocation_fault PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_options(flowparallel_graph_reference_allocation_fault PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(flowparallel_graph_reference_allocation_fault PRIVATE flowcontracts)

add_executable(flowparallel_graph_cuda src/graph_cuda.cpp)
target_include_directories(flowparallel_graph_cuda PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_options(flowparallel_graph_cuda PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(flowparallel_graph_cuda PRIVATE flowcontracts ${CMAKE_DL_LIBS})
add_executable(flowparallel_graph_cuda_allocation_fault src/graph_cuda.cpp)
target_compile_definitions(flowparallel_graph_cuda_allocation_fault PRIVATE FLOWPARALLEL_GRAPH_CUDA_TEST_ALLOCATION_FAILURE)
target_include_directories(flowparallel_graph_cuda_allocation_fault PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_options(flowparallel_graph_cuda_allocation_fault PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(flowparallel_graph_cuda_allocation_fault PRIVATE flowcontracts ${CMAKE_DL_LIBS})

add_executable(flowparallel_graph_planner src/graph_planner.cpp)
target_include_directories(flowparallel_graph_planner PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_options(flowparallel_graph_planner PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(flowparallel_graph_planner PRIVATE flowcontracts)
add_executable(flowparallel_graph_planner_allocation_fault src/graph_planner.cpp)
target_compile_definitions(flowparallel_graph_planner_allocation_fault PRIVATE FLOWPARALLEL_GRAPH_PLANNER_TEST_ALLOCATION_FAILURE)
target_include_directories(flowparallel_graph_planner_allocation_fault PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_options(flowparallel_graph_planner_allocation_fault PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(flowparallel_graph_planner_allocation_fault PRIVATE flowcontracts)
enable_testing()
find_package(Threads REQUIRED)

add_test(NAME flowparallel_matrix_benchmark_diagnostics
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWPARALLEL_MATRIX_BENCHMARK_BIN=$<TARGET_FILE:flowparallel_matrix_benchmark>
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/run-matrix-benchmark-tests.sh)
add_test(NAME flowparallel_exception_containment
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWPARALLEL_BIN=$<TARGET_FILE:flowparallel>
        FLOWPARALLEL_CPU_BIN=$<TARGET_FILE:flowparallel_cpu>
        FLOWPARALLEL_CUDA_BIN=$<TARGET_FILE:flowparallel_cuda>
        FLOWPARALLEL_RUNTIME_PLANNER_BIN=$<TARGET_FILE:flowparallel_runtime_planner>
        FLOWPARALLEL_GRAPH_REFERENCE_BIN=$<TARGET_FILE:flowparallel_graph_reference>
        FLOWPARALLEL_GRAPH_PLANNER_BIN=$<TARGET_FILE:flowparallel_graph_planner>
        FLOWPARALLEL_GRAPH_CUDA_BIN=$<TARGET_FILE:flowparallel_graph_cuda>
        FLOWPARALLEL_CUDA_EXECUTE_BIN=$<TARGET_FILE:flowparallel_cuda_execute>
        FLOWPARALLEL_MATRIX_BENCHMARK_BIN=$<TARGET_FILE:flowparallel_matrix_benchmark>
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/run-exception-containment-tests.sh)
add_test(NAME flowparallel_allocation_fault
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWPARALLEL_ALLOCATION_FAULT_BIN=$<TARGET_FILE:flowparallel_allocation_fault>
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/run-flowparallel-allocation-fault.sh)

add_library(flowparallel_cpu_runtime STATIC src/cpu_execution.cpp)
target_include_directories(flowparallel_cpu_runtime PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(flowparallel_cpu_runtime PUBLIC cxx_std_20)
target_compile_options(flowparallel_cpu_runtime PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(flowparallel_cpu_runtime PUBLIC Threads::Threads)

add_executable(flowparallel_execution_smoketest tests/execution_smoketest.cpp)
target_compile_options(flowparallel_execution_smoketest PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(flowparallel_execution_smoketest PRIVATE flowparallel_cpu_runtime)

add_test(NAME flowparallel_pipeline
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWPARALLEL_BIN=$<TARGET_FILE:flowparallel>
        FLOWMINI_BIN=$<TARGET_FILE:flowmini>
        FLOWANALYST_BIN=$<TARGET_FILE:flowanalyst>
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/run-flowparallel-tests.sh)
set(flowparallel_smoke_memory_kb 1048576)
if(CMAKE_CXX_FLAGS MATCHES "-fsanitize=[^ ]*address")
    # ASan reserves a large virtual shadow map; the input and timeout still
    # bound the workload, but a 1 GiB address-space cap prevents startup.
    set(flowparallel_smoke_memory_kb 0)
endif()
add_test(NAME flowparallel_smoketest
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWPARALLEL_SMOKETEST_BIN=$<TARGET_FILE:flowparallel_smoketest>
        FLOWPARALLEL_SMOKETEST_MEMORY_KB=${flowparallel_smoke_memory_kb}
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/run-parallel-smoketest.sh)
add_test(NAME flowparallel_cuda_resource_cleanup
    COMMAND flowparallel_cuda_resource_cleanup_test)
add_test(NAME flowparallel_cpu_provider
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWPARALLEL_CPU_BIN=$<TARGET_FILE:flowparallel_cpu>
        FLOWPARALLEL_BIN=$<TARGET_FILE:flowparallel>
        FLOWMINI_BIN=$<TARGET_FILE:flowmini>
        FLOWANALYST_BIN=$<TARGET_FILE:flowanalyst>
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/run-cpu-provider-tests.sh)
add_test(NAME flowparallel_cpu_allocation_fault
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWPARALLEL_CPU_ALLOCATION_FAULT_BIN=$<TARGET_FILE:flowparallel_cpu_allocation_fault>
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/run-cpu-allocation-fault.sh)
add_test(NAME flowparallel_cpu_execution
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWPARALLEL_EXECUTION_SMOKETEST_BIN=$<TARGET_FILE:flowparallel_execution_smoketest>
        FLOWPARALLEL_BIN=$<TARGET_FILE:flowparallel>
        FLOWMINI_BIN=$<TARGET_FILE:flowmini>
        FLOWANALYST_BIN=$<TARGET_FILE:flowanalyst>
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/run-cpu-execution-tests.sh)
add_test(NAME flowparallel_cuda_provider
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWPARALLEL_CUDA_BIN=$<TARGET_FILE:flowparallel_cuda>
        FLOWPARALLEL_BIN=$<TARGET_FILE:flowparallel>
        FLOWMINI_BIN=$<TARGET_FILE:flowmini>
        FLOWANALYST_BIN=$<TARGET_FILE:flowanalyst>
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/run-cuda-provider-tests.sh)
add_test(NAME flowparallel_cuda_allocation_fault
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWPARALLEL_CUDA_ALLOCATION_FAULT_BIN=$<TARGET_FILE:flowparallel_cuda_allocation_fault>
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/run-cuda-allocation-fault.sh)
add_test(NAME flowparallel_runtime_planner
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWPARALLEL_RUNTIME_PLANNER_BIN=$<TARGET_FILE:flowparallel_runtime_planner>
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/run-runtime-planner-tests.sh)
add_test(NAME flowparallel_runtime_planner_allocation_fault
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWPARALLEL_RUNTIME_PLANNER_ALLOCATION_FAULT_BIN=$<TARGET_FILE:flowparallel_runtime_planner_allocation_fault>
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/run-runtime-planner-allocation-fault.sh)
add_test(NAME flowparallel_graph_reference
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWPARALLEL_GRAPH_REFERENCE_BIN=$<TARGET_FILE:flowparallel_graph_reference>
        FLOWMINI_BIN=$<TARGET_FILE:flowmini>
        FLOWANALYST_BIN=$<TARGET_FILE:flowanalyst>
        FLOW_GRAPH_FIXTURE=${CMAKE_CURRENT_SOURCE_DIR}/../Lyraform/compiler/examples/ast/parallel_independence_probe.flow
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/run-graph-reference-tests.sh)
add_test(NAME flowparallel_graph_reference_allocation_fault
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWPARALLEL_GRAPH_REFERENCE_ALLOCATION_FAULT_BIN=$<TARGET_FILE:flowparallel_graph_reference_allocation_fault>
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/run-graph-reference-allocation-fault.sh)
add_test(NAME flowparallel_graph_planner
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWPARALLEL_GRAPH_PLANNER_BIN=$<TARGET_FILE:flowparallel_graph_planner>
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/run-graph-planner-tests.sh)
add_test(NAME flowparallel_graph_cuda_allocation_fault
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWPARALLEL_GRAPH_CUDA_ALLOCATION_FAULT_BIN=$<TARGET_FILE:flowparallel_graph_cuda_allocation_fault>
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/run-graph-cuda-allocation-fault.sh)
add_test(NAME flowparallel_graph_planner_allocation_fault
    COMMAND ${CMAKE_COMMAND} -E env
        FLOWPARALLEL_GRAPH_PLANNER_ALLOCATION_FAULT_BIN=$<TARGET_FILE:flowparallel_graph_planner_allocation_fault>
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/run-graph-planner-allocation-fault.sh)
