cmake_minimum_required( VERSION 3.22.1 )

project(predOS C ASM)


set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DLOG_LEVEL=LOG_LEVEL_DEBUG" )

if(PREDOS_TEST)
  set( IMG_DIR ${CMAKE_SOURCE_DIR}/build/img )
  set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DTEST=1")
  set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
  add_subdirectory(external)
  enable_testing()
else()
  set( IMG_DIR ${CMAKE_BINARY_DIR}/img )
  set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostartfiles" )
  set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mfloat-abi=hard" )
  set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0" )
endif()

add_subdirectory(src)

