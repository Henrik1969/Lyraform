cmake_minimum_required(VERSION 3.22)
project(tinyvm_recovered LANGUAGES C CXX)

include(CTest)
find_package(OpenSSL REQUIRED COMPONENTS Crypto)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

add_library(tinyvm_recovered STATIC
    src/artifact.c
    src/artifact_v2.c
    src/instructions.c
    src/isa_v1.c
    src/runtime_provider.c
    src/vm.c
    src/vm_function.c
    src/vm_switch.c
)
target_include_directories(tinyvm_recovered PUBLIC include)
target_link_libraries(tinyvm_recovered PUBLIC OpenSSL::Crypto ${CMAKE_DL_LIBS})
target_compile_options(tinyvm_recovered PRIVATE -Wall -Wextra -Wpedantic)
# vm.c deliberately uses GNU labels-as-values. Keep strict diagnostics for the
# portable handlers while acknowledging that one isolated extension boundary.
set_source_files_properties(src/vm.c src/isa_v1.c PROPERTIES COMPILE_OPTIONS -Wno-pedantic)

add_executable(tinyvm_recovered_demo src/main.c)
target_link_libraries(tinyvm_recovered_demo PRIVATE tinyvm_recovered)

add_executable(tinyvm_dispatch_benchmark benchmarks/dispatch_benchmark.c)
target_link_libraries(tinyvm_dispatch_benchmark PRIVATE tinyvm_recovered)
target_compile_options(tinyvm_dispatch_benchmark PRIVATE -Wall -Wextra -Wpedantic)

add_executable(flowtinypack-demo tools/pack_demo.c)
target_link_libraries(flowtinypack-demo PRIVATE tinyvm_recovered)
add_executable(flowtinyvalidate tools/validate.c)
target_link_libraries(flowtinyvalidate PRIVATE tinyvm_recovered)
add_executable(flowtinyrun tools/run.c)
target_link_libraries(flowtinyrun PRIVATE tinyvm_recovered)
add_executable(flowtinyv2-roundtrip tools/v2_roundtrip.c)
target_link_libraries(flowtinyv2-roundtrip PRIVATE tinyvm_recovered)
add_executable(flowtinyv2-mutate tests/artifact_v2_mutate.c)
target_link_libraries(flowtinyv2-mutate PRIVATE OpenSSL::Crypto)
add_executable(tinyvm_isa_v1_conformance tests/isa_v1_conformance.c)
target_link_libraries(tinyvm_isa_v1_conformance PRIVATE tinyvm_recovered)
add_executable(flowtinyv1-pack-demo tools/v1_pack_demo.c)
target_link_libraries(flowtinyv1-pack-demo PRIVATE tinyvm_recovered)
if(TARGET flowcontracts)
    add_executable(flowtinylower tools/lower.cpp)
    target_compile_features(flowtinylower PRIVATE cxx_std_20)
    target_compile_options(flowtinylower PRIVATE -Wall -Wextra -Wpedantic)
    target_link_libraries(flowtinylower PRIVATE tinyvm_recovered flowcontracts)
    add_executable(flowtinylower_allocation_fault tools/lower.cpp)
    target_compile_definitions(flowtinylower_allocation_fault PRIVATE FLOWTINYLLOWER_TEST_ALLOCATION_FAILURE)
    target_compile_features(flowtinylower_allocation_fault PRIVATE cxx_std_20)
    target_compile_options(flowtinylower_allocation_fault PRIVATE -Wall -Wextra -Wpedantic)
    target_link_libraries(flowtinylower_allocation_fault PRIVATE tinyvm_recovered flowcontracts)
endif()

if(BUILD_TESTING)
add_library(tinyvm_recovered_artifact_v1_allocation_fault STATIC
    src/artifact.c
    src/artifact_v2.c
    src/instructions.c
    src/isa_v1.c
    src/runtime_provider.c
    src/vm.c
    src/vm_function.c
    src/vm_switch.c
)
target_compile_definitions(tinyvm_recovered_artifact_v1_allocation_fault PRIVATE
    TINYVM_ARTIFACT_V1_TEST_ALLOCATION_FAILURE)
target_include_directories(tinyvm_recovered_artifact_v1_allocation_fault PUBLIC include)
target_link_libraries(tinyvm_recovered_artifact_v1_allocation_fault PUBLIC OpenSSL::Crypto ${CMAKE_DL_LIBS})
target_compile_options(tinyvm_recovered_artifact_v1_allocation_fault PRIVATE -Wall -Wextra -Wpedantic)
set_source_files_properties(src/vm.c src/isa_v1.c PROPERTIES COMPILE_OPTIONS -Wno-pedantic)
add_executable(flowtinyv1_allocation_fault tests/artifact-v1-allocation-fault.c)
target_link_libraries(flowtinyv1_allocation_fault PRIVATE tinyvm_recovered_artifact_v1_allocation_fault)

add_library(tinyvm_recovered_artifact_v2_allocation_fault STATIC
    src/artifact.c
    src/artifact_v2.c
    src/instructions.c
    src/isa_v1.c
    src/runtime_provider.c
    src/vm.c
    src/vm_function.c
    src/vm_switch.c
)
target_compile_definitions(tinyvm_recovered_artifact_v2_allocation_fault PRIVATE
    TINYVM_ARTIFACT_V2_TEST_ALLOCATION_FAILURE)
target_include_directories(tinyvm_recovered_artifact_v2_allocation_fault PUBLIC include)
target_link_libraries(tinyvm_recovered_artifact_v2_allocation_fault PUBLIC OpenSSL::Crypto ${CMAKE_DL_LIBS})
target_compile_options(tinyvm_recovered_artifact_v2_allocation_fault PRIVATE -Wall -Wextra -Wpedantic)
set_source_files_properties(src/vm.c src/isa_v1.c PROPERTIES COMPILE_OPTIONS -Wno-pedantic)
add_executable(flowtinyv2_allocation_fault tests/artifact-v2-allocation-fault.c)
target_link_libraries(flowtinyv2_allocation_fault PRIVATE tinyvm_recovered_artifact_v2_allocation_fault)
endif()

if(BUILD_TESTING)
    add_executable(tinyvm_isa_conformance tests/isa_conformance.c)
    target_link_libraries(tinyvm_isa_conformance PRIVATE tinyvm_recovered)
    target_compile_options(tinyvm_isa_conformance PRIVATE -Wall -Wextra -Wpedantic)

    add_test(NAME tinyvm_recovered_demo COMMAND tinyvm_recovered_demo)
    set_tests_properties(tinyvm_recovered_demo PROPERTIES
        PASS_REGULAR_EXPRESSION "result=42;halted pc=4")
    add_test(NAME tinyvm_dispatch_smoke
        COMMAND tinyvm_dispatch_benchmark --iterations 8 --words 256)
    set_tests_properties(tinyvm_dispatch_smoke PROPERTIES
        PASS_REGULAR_EXPRESSION "computed_goto.*switch.*function_pointer")
    add_test(NAME tinyvm_isa_conformance COMMAND tinyvm_isa_conformance)
    set_tests_properties(tinyvm_isa_conformance PROPERTIES
        PASS_REGULAR_EXPRESSION "TinyVM ISA conformance: 24 cases passed across 3 engines")
    add_test(NAME tinyvm_artifact_boundary
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tests/artifact-boundary.sh
                $<TARGET_FILE:flowtinypack-demo>
                $<TARGET_FILE:flowtinyvalidate>
                $<TARGET_FILE:flowtinyrun>
                ${CMAKE_CURRENT_BINARY_DIR})
    add_test(NAME tinyvm_artifact_v2_roundtrip
        COMMAND flowtinyv2-roundtrip ${CMAKE_CURRENT_BINARY_DIR}/tinyvm-v2-test.tvm)
    add_test(NAME tinyvm_artifact_v2_boundary
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tests/artifact-v2-boundary.sh
                $<TARGET_FILE:flowtinyv2-roundtrip>
                $<TARGET_FILE:flowtinyvalidate>
                $<TARGET_FILE:flowtinyrun>
                ${CMAKE_CURRENT_BINARY_DIR})
    add_test(NAME tinyvm_artifact_v2_hostile
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tests/artifact-v2-hostile.sh
                $<TARGET_FILE:flowtinyv2-roundtrip>
                $<TARGET_FILE:flowtinyv2-mutate>
                $<TARGET_FILE:flowtinyvalidate>
                ${CMAKE_CURRENT_BINARY_DIR})
    add_test(NAME tinyvm_artifact_v2_allocation_fault
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tests/artifact-v2-allocation-fault.sh
                $<TARGET_FILE:flowtinyv2-roundtrip>
                $<TARGET_FILE:flowtinyv2_allocation_fault>
                ${CMAKE_CURRENT_BINARY_DIR})
    add_test(NAME tinyvm_artifact_v1_allocation_fault
        COMMAND $<TARGET_FILE:flowtinyv1_allocation_fault>
                ${CMAKE_CURRENT_BINARY_DIR}/tinyvm-v1-allocation-fault.tvm)
    add_test(NAME tinyvm_isa_v1_conformance COMMAND tinyvm_isa_v1_conformance)
    set_tests_properties(tinyvm_isa_v1_conformance PROPERTIES
        PASS_REGULAR_EXPRESSION "TinyVM typed ISA conformance passed")
    add_test(NAME tinyvm_isa_v1_file_boundary
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tests/isa-v1-file-boundary.sh
                $<TARGET_FILE:flowtinyv1-pack-demo>
                $<TARGET_FILE:flowtinyvalidate>
                $<TARGET_FILE:flowtinyrun>
                ${CMAKE_CURRENT_BINARY_DIR})
    if(TARGET flowtinylower)
        add_test(NAME tinyvm_backend_lowering_boundary
            COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tests/backend-lowering-boundary.sh
                    $<TARGET_FILE:flowtinylower>
                    $<TARGET_FILE:flowtinyvalidate>
                    $<TARGET_FILE:flowtinyrun>
                    ${CMAKE_SOURCE_DIR}/Flowlower/tests/captured-empty-lowering.json
                    ${CMAKE_CURRENT_BINARY_DIR})
        add_test(NAME tinyvm_backend_lowering_allocation_fault
            COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tests/backend-lowering-allocation-fault.sh
                    $<TARGET_FILE:flowtinylower_allocation_fault>)
    endif()
endif()
