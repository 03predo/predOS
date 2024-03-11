add_library(uart STATIC uart.c)
target_include_directories(uart PUBLIC ${RPIOS_DRIVER_INTERFACE_DIR} ${RPIOS_SOC_DIR} ${RPIOS_COMMON_DIR})
target_link_libraries(uart PRIVATE bcm2835)
