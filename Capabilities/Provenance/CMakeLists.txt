add_library(frankencore_core_provenance STATIC provenance.cpp identity.cpp)
target_include_directories(frankencore_core_provenance
    PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
target_compile_features(frankencore_core_provenance PUBLIC cxx_std_20)
target_compile_options(frankencore_core_provenance PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(frankencore_core_provenance PRIVATE flowcontracts)

add_library(Frankencore::Provenance ALIAS frankencore_core_provenance)
