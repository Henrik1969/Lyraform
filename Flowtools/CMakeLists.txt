cmake_minimum_required(VERSION 3.20)
project(flowcore_tools LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
enable_testing()

add_custom_target(flowcore_architecture_check
    COMMAND "${CMAKE_CURRENT_LIST_DIR}/../tools/check-frankencore-architecture.sh"
    WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/.."
    COMMENT "Checking Frankencore architectural boundaries"
    VERBATIM)

add_subdirectory(reference/clock)
add_subdirectory(reference/text)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../Frankencore/Core/Provenance
    ${CMAKE_CURRENT_BINARY_DIR}/frankencore-core-provenance)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../Frankencore/Core/Packages
    ${CMAKE_CURRENT_BINARY_DIR}/frankencore-core-packages)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../Frankencore/Core/Contracts
    ${CMAKE_CURRENT_BINARY_DIR}/frankencore-core-contracts)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../Frankencore/Core/Policy
    ${CMAKE_CURRENT_BINARY_DIR}/frankencore-core-policy)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../Frankencore/Core/Language
    ${CMAKE_CURRENT_BINARY_DIR}/frankencore-core-language)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../Frankencore/Core/Requirements
    ${CMAKE_CURRENT_BINARY_DIR}/frankencore-core-requirements)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../Frankencore/Core/Runtime
    ${CMAKE_CURRENT_BINARY_DIR}/frankencore-core-runtime)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../Flowparallel
    ${CMAKE_CURRENT_BINARY_DIR}/flowparallel)
add_subdirectory(reference/revision)
add_subdirectory(reference/packages)
add_subdirectory(reference/contracts)
add_subdirectory(reference/policy)
add_subdirectory(reference/language)
add_subdirectory(reference/requirements)
add_subdirectory(facades/ls)
add_subdirectory(examples/flowcat-clion)

add_custom_target(flowcore_conformance
    COMMAND "${CMAKE_COMMAND}" -E env
        CLOCK_BIN=$<TARGET_FILE:frankencore_clock>
        REVISION_BIN=$<TARGET_FILE:frankencore_revision_probe>
        FLOWBIND_BIN=$<TARGET_FILE:flowbind>
        "${CMAKE_CURRENT_LIST_DIR}/../tools/check-frankencore-conformance.sh"
    DEPENDS frankencore_clock frankencore_revision_probe flowcore_architecture_check
    WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/.."
    COMMENT "Running Frankencore constitutional conformance laws"
    VERBATIM)

add_test(NAME frankencore_conformance
    COMMAND "${CMAKE_COMMAND}" --build "${CMAKE_BINARY_DIR}" --target flowcore_conformance)
