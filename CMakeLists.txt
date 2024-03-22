cmake_minimum_required( VERSION 3.22.1 )

project(predOS C ASM)

set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostartfiles" )
set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mfloat-abi=hard" )
set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0" )

set( IMG_DIR ${CMAKE_BINARY_DIR}/img )


add_subdirectory(src)
