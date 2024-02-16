add_subdirectory(gpio)
add_library(drivers INTERFACE)
target_link_libraries(drivers INTERFACE gpio)
