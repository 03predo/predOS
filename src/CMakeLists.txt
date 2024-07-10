set(COMMON_DIR ${CMAKE_CURRENT_SOURCE_DIR}/common)
set(APPS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/apps)
set(DRIVER_INTERFACE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/drivers/interfaces)

if(NOT PREDOS_TEST)
  add_subdirectory(drivers)
  add_subdirectory(apps)
endif()

add_subdirectory(kernel)

