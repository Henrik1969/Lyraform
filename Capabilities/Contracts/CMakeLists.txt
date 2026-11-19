add_library(frankencore_core_contracts STATIC contracts.cpp)
target_include_directories(frankencore_core_contracts
    PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
target_compile_features(frankencore_core_contracts PUBLIC cxx_std_20)
target_compile_options(frankencore_core_contracts PRIVATE -Wall -Wextra -Wpedantic)

add_library(Frankencore::Contracts ALIAS frankencore_core_contracts)
