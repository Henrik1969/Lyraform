add_library(flowtext_runtime SHARED text_runtime.c)
set_target_properties(flowtext_runtime PROPERTIES OUTPUT_NAME flowtext)
target_compile_options(flowtext_runtime PRIVATE -Wall -Wextra -Wpedantic)
