add_library(frankencore_core_packages STATIC packages.cpp)
target_include_directories(frankencore_core_packages
    PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
target_compile_features(frankencore_core_packages PUBLIC cxx_std_20)
target_compile_options(frankencore_core_packages PRIVATE -Wall -Wextra -Wpedantic)

add_library(Frankencore::Packages ALIAS frankencore_core_packages)

# Test-only library variant: inject allocation exhaustion inside the checked
# public JSON projection and verify its noexcept refusal contract.
add_library(frankencore_core_packages_fault STATIC packages.cpp)
target_include_directories(frankencore_core_packages_fault
    PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
target_compile_features(frankencore_core_packages_fault PUBLIC cxx_std_20)
target_compile_options(frankencore_core_packages_fault PRIVATE -Wall -Wextra -Wpedantic)
target_compile_definitions(frankencore_core_packages_fault PRIVATE
    FRANKENCORE_PACKAGES_TEST_ALLOCATION_FAILURE
    FRANKENCORE_PACKAGES_TEST_READ_ALLOCATION_FAILURE
    FRANKENCORE_PACKAGES_TEST_APT_INDEX_ALLOCATION_FAILURE)
add_library(Frankencore::PackagesFault ALIAS frankencore_core_packages_fault)
