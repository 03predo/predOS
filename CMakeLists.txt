cmake_minimum_required( VERSION 3.22.1 )

project(
  predOS
  VERSION 0.1
  LANGUAGES C ASM
)


set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DVERSION=\\\"${PROJECT_VERSION}\\\"")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DLOG_LEVEL=LOG_LEVEL_DEBUG")

if(PREDOS_TEST)
  set(IMG_DIR ${CMAKE_SOURCE_DIR}/build/img)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DTEST=1")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage")
  set(CMAKE_C_OUTPUT_EXTENSION_REPLACE ON)
  set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
  add_subdirectory(external)
  enable_testing()
else()
  set(IMG_DIR ${CMAKE_BINARY_DIR}/img ) 
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mfloat-abi=hard")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0")

  set(APP_C_FLAGS "${CMAKE_C_FLAGS}")
  set(APP_LINKER_FLAGS "${APP_C_FLAGS}")

  set(KERNEL_C_FLAGS "${CMAKE_C_FLAGS} -nostartfiles")
  set(KERNEL_LINKER_FLAGS "${KERNEL_C_FLAGS} -T ${CMAKE_SOURCE_DIR}/cmake/link.ld")
endif()

add_subdirectory(src)

