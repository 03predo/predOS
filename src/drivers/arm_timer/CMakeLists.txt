add_library(arm_timer STATIC arm_timer.c)
target_include_directories(arm_timer PUBLIC ${DRIVER_INTERFACE_DIR} ${SOC_DIR} ${COMMON_DIR})
target_link_libraries(arm_timer PRIVATE bcm2835)
