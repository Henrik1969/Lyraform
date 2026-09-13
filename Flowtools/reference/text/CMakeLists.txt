add_library(flowtext_runtime SHARED text_runtime.c text_runtime.h)
target_include_directories(flowtext_runtime PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(flowtext_runtime PROPERTIES OUTPUT_NAME flowtext)
target_compile_options(flowtext_runtime PRIVATE -Wall -Wextra -Wpedantic)
