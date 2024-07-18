set(COMMON_DIR ${CMAKE_CURRENT_SOURCE_DIR}/common)
set(APPS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/apps)
set(DRIVER_INTERFACE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/drivers/interfaces)

if(NOT PREDOS_TEST)
  set(CMAKE_C_FLAGS "${KERNEL_C_FLAGS}" ) 
  set(CMAKE_EXE_LINKER_FLAGS "${KERNEL_LINKER_FLAGS}")
  add_subdirectory(drivers)
endif()

add_subdirectory(kernel)

if(DEFINED APPS)
  set(CMAKE_C_FLAGS "${APP_C_FLAGS}") 
  set(CMAKE_EXE_LINKER_FLAGS "${APP_LINKER_FLAGS}")
  add_subdirectory(apps)
endif()

