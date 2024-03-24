add_library(gpio STATIC gpio.c)
target_include_directories(gpio PUBLIC ${DRIVER_INTERFACE_DIR} ${SOC_DIR} ${COMMON_DIR})
target_link_libraries(gpio PRIVATE bcm2835)
