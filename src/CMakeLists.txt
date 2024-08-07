set(COMMON_DIR ${CMAKE_CURRENT_SOURCE_DIR}/common)
set(APPS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/apps)
set(DRIVER_INTERFACE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/drivers/interfaces)
set(KERNEL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/kernel)
set(APP_C_FLAGS "${CMAKE_C_FLAGS}")

if(NOT PREDOS_TEST)
  set(CMAKE_EXE_LINKER_FLAGS "-T ${CMAKE_SOURCE_DIR}/cmake/link.ld")
  add_subdirectory(drivers)
endif()

add_subdirectory(kernel)

if(NOT PREDOS_TEST)
  set(CMAKE_EXE_LINKER_FLAGS "")
  set(CMAKE_C_FLAGS "${APP_C_FLAGS}")
  add_subdirectory(apps)
endif()

