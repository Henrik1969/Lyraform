add_library(flowmini_testabi SHARED testabi.c)
target_include_directories(flowmini_testabi PRIVATE include)
set_target_properties(flowmini_testabi PROPERTIES
    OUTPUT_NAME "flowmini_testabi"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

add_executable(flowmini_testabi_layout layout.c)
target_include_directories(flowmini_testabi_layout PRIVATE include)

add_executable(flowmini_wideabi_layout wide_layout.c)
target_include_directories(flowmini_wideabi_layout PRIVATE include)
