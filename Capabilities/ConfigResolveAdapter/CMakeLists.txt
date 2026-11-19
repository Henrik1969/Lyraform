set(FRANKENCORE_CONFIGRESOLVE_ROOT "" CACHE PATH
    "ConfigResolve installation or build root for the optional adapter")

if (FRANKENCORE_CONFIGRESOLVE_ROOT)
    find_path(FRANKENCORE_CONFIGRESOLVE_INCLUDE
        NAMES configresolve/configresolve.h
        PATHS "${FRANKENCORE_CONFIGRESOLVE_ROOT}/include"
        NO_DEFAULT_PATH)
    find_library(FRANKENCORE_CONFIGRESOLVE_LIBRARY
        NAMES configresolve_shared configresolve
        PATHS "${FRANKENCORE_CONFIGRESOLVE_ROOT}/lib"
              "${FRANKENCORE_CONFIGRESOLVE_ROOT}/cmake-build-release"
              "${FRANKENCORE_CONFIGRESOLVE_ROOT}/build-release"
              "${FRANKENCORE_CONFIGRESOLVE_ROOT}"
        NO_DEFAULT_PATH)
    if (NOT FRANKENCORE_CONFIGRESOLVE_INCLUDE OR NOT FRANKENCORE_CONFIGRESOLVE_LIBRARY)
        message(FATAL_ERROR "FRANKENCORE_CONFIGRESOLVE_ROOT does not contain ConfigResolve headers and library")
    endif()

    add_library(frankencore_policy_configresolve STATIC configresolve_adapter.cpp)
    target_include_directories(frankencore_policy_configresolve
        PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include
        PRIVATE "${FRANKENCORE_CONFIGRESOLVE_INCLUDE}")
    target_link_libraries(frankencore_policy_configresolve
        PUBLIC Frankencore::Contracts
        PRIVATE "${FRANKENCORE_CONFIGRESOLVE_LIBRARY}")
    target_compile_features(frankencore_policy_configresolve PUBLIC cxx_std_20)
    target_compile_options(frankencore_policy_configresolve PRIVATE -Wall -Wextra -Wpedantic)
    add_library(Frankencore::PolicyConfigResolve ALIAS frankencore_policy_configresolve)
else()
    message(STATUS "ConfigResolve adapter disabled; set FRANKENCORE_CONFIGRESOLVE_ROOT to enable it")
endif()
