add_library(power_management STATIC power_management.c)
target_include_directories(power_management PUBLIC ${DRIVER_INTERFACE_DIR} ${SOC_DIR} ${COMMON_DIR})
target_link_libraries(power_management PRIVATE bcm2835)
