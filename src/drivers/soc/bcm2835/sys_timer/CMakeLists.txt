add_library(sys_timer STATIC sys_timer.c)
target_include_directories(sys_timer PUBLIC ${RPIOS_DRIVER_INTERFACE_DIR} ${RPIOS_SOC_DIR} ${RPIOS_COMMON_DIR})
target_link_libraries(sys_timer PRIVATE bcm2835)
