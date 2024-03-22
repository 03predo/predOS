add_library(uart STATIC uart.c)
target_include_directories(uart PUBLIC ${DRIVER_INTERFACE_DIR} ${SOC_DIR} ${COMMON_DIR})
target_link_libraries(uart PRIVATE bcm2835)
