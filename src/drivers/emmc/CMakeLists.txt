add_library(emmc STATIC emmc.c)
target_include_directories(emmc PUBLIC ${DRIVER_INTERFACE_DIR} ${SOC_DIR} ${COMMON_DIR})
target_link_libraries(emmc PRIVATE bcm2835)
