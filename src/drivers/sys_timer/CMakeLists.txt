add_library(sys_timer STATIC sys_timer.c)
target_include_directories(sys_timer PUBLIC ${DRIVER_INTERFACE_DIR} ${SOC_DIR} ${COMMON_DIR})
target_link_libraries(sys_timer PRIVATE bcm2835)
