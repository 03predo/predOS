set(SOC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bcm2835)

add_subdirectory(bcm2835)
add_subdirectory(gpio)
add_subdirectory(uart)
add_subdirectory(sys_timer)
add_subdirectory(arm_timer)
add_subdirectory(power_management)
add_subdirectory(emmc)

add_library(drivers INTERFACE)
target_link_libraries(drivers INTERFACE gpio uart sys_timer arm_timer power_management emmc)
