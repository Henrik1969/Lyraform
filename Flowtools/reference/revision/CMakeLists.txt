add_executable(frankencore_revision_probe main.cpp)
target_link_libraries(frankencore_revision_probe PRIVATE Frankencore::Provenance)
target_compile_options(frankencore_revision_probe PRIVATE -Wall -Wextra -Wpedantic)

add_executable(frankencore_provenance_api_test api_test.cpp)
target_link_libraries(frankencore_provenance_api_test PRIVATE Frankencore::Provenance)
target_compile_options(frankencore_provenance_api_test PRIVATE -Wall -Wextra -Wpedantic)

add_test(NAME frankencore_revision_contract
    COMMAND ${CMAKE_COMMAND} -E env
        REVISION_BIN=$<TARGET_FILE:frankencore_revision_probe>
        ${CMAKE_CURRENT_LIST_DIR}/../../../tools/check-frankencore-mutation-provenance.sh)
add_test(NAME frankencore_revision_rejects_non_monotonic
    COMMAND frankencore_revision_probe --old-revision 8 --new-revision 8)
set_tests_properties(frankencore_revision_rejects_non_monotonic PROPERTIES WILL_FAIL TRUE)
add_test(NAME frankencore_provenance_api
    COMMAND frankencore_provenance_api_test)
add_executable(frankencore_error_state_history_test history_test.cpp)
target_link_libraries(frankencore_error_state_history_test PRIVATE Frankencore::Provenance)
target_compile_features(frankencore_error_state_history_test PRIVATE cxx_std_20)
target_compile_options(frankencore_error_state_history_test PRIVATE -Wall -Wextra -Wpedantic)
target_link_options(frankencore_error_state_history_test PRIVATE "-Wl,--wrap=fsync")
add_test(NAME frankencore_error_state_history
    COMMAND frankencore_error_state_history_test)
add_test(NAME frankencore_reference_structured_diagnostics
    COMMAND ${CMAKE_COMMAND} -E env
        CLOCK_BIN=$<TARGET_FILE:frankencore_clock>
        REVISION_BIN=$<TARGET_FILE:frankencore_revision_probe>
        ${CMAKE_CURRENT_LIST_DIR}/../../../tools/test-reference-structured-diagnostics.sh)
