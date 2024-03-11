add_library(gpio STATIC gpio.c)
target_include_directories(gpio PUBLIC ${RPIOS_DRIVER_INTERFACE_DIR} ${RPIOS_SOC_DIR} ${RPIOS_COMMON_DIR})
target_link_libraries(gpio PRIVATE bcm2835)
