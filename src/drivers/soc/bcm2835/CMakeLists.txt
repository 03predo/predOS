add_library(bcm2835 STATIC bcm2835.c)
add_subdirectory(gpio)
add_subdirectory(uart)

add_library(drivers INTERFACE)
target_link_libraries(drivers INTERFACE gpio uart)
