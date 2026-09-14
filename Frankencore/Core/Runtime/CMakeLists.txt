add_library(frankencore_core_runtime STATIC runtime.cpp)
target_include_directories(frankencore_core_runtime PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
target_compile_features(frankencore_core_runtime PUBLIC cxx_std_20)
target_compile_options(frankencore_core_runtime PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(frankencore_core_runtime PUBLIC ${CMAKE_DL_LIBS})
add_library(Frankencore::Runtime ALIAS frankencore_core_runtime)

add_executable(frankencore_runtime_probe probe.cpp)
target_link_libraries(frankencore_runtime_probe PRIVATE Frankencore::Runtime)
target_compile_features(frankencore_runtime_probe PRIVATE cxx_std_20)
target_compile_options(frankencore_runtime_probe PRIVATE -Wall -Wextra -Wpedantic)

# Test-only process boundary: compile a fault-injected copy of the existing
# runtime implementation so discovery throws inside its own C++ boundary.
add_library(frankencore_core_runtime_fault STATIC runtime.cpp)
target_include_directories(frankencore_core_runtime_fault PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
target_compile_features(frankencore_core_runtime_fault PUBLIC cxx_std_20)
target_compile_options(frankencore_core_runtime_fault PRIVATE -Wall -Wextra -Wpedantic)
target_compile_definitions(frankencore_core_runtime_fault PRIVATE FRANKENCORE_RUNTIME_DISCOVERY_TEST_FAULT)
target_link_libraries(frankencore_core_runtime_fault PUBLIC ${CMAKE_DL_LIBS})

add_executable(frankencore_runtime_probe_fault probe.cpp)
target_link_libraries(frankencore_runtime_probe_fault PRIVATE frankencore_core_runtime_fault)
target_compile_features(frankencore_runtime_probe_fault PRIVATE cxx_std_20)
target_compile_options(frankencore_runtime_probe_fault PRIVATE -Wall -Wextra -Wpedantic)

add_executable(frankencore_runtime_memory_test runtime-memory-test.cpp)
target_include_directories(frankencore_runtime_memory_test PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include)
target_compile_features(frankencore_runtime_memory_test PRIVATE cxx_std_20)
target_compile_options(frankencore_runtime_memory_test PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(frankencore_runtime_memory_test PRIVATE ${CMAKE_DL_LIBS})

add_test(NAME frankencore_runtime_probe
    COMMAND ${CMAKE_COMMAND} -E env
        RUNTIME_BIN=$<TARGET_FILE:frankencore_runtime_probe>
        ${CMAKE_CURRENT_LIST_DIR}/test-runtime.sh)

add_test(NAME frankencore_runtime_probe_fault
    COMMAND ${CMAKE_COMMAND} -E env
        RUNTIME_FAULT_BIN=$<TARGET_FILE:frankencore_runtime_probe_fault>
        ${CMAKE_CURRENT_LIST_DIR}/test-runtime-fault.sh)

add_test(NAME frankencore_runtime_memory
    COMMAND frankencore_runtime_memory_test)
