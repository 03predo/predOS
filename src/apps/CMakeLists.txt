add_subdirectory(example)

add_library(apps INTERFACE)
target_link_libraries(apps INTERFACE example)
