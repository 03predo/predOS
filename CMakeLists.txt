cmake_minimum_required( VERSION 3.22.1 )

project(rpiOS C ASM)

set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostartfiles" )
set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mfloat-abi=hard" )
set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0" )

add_executable(kernel main.c)

add_custom_command(
    TARGET kernel POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} ./kernel${CMAKE_EXECUTABLE_SUFFIX} -O binary ./kernel.img
    COMMAND ${CMAKE_OBJDUMP} -l -S -D ./kernel${CMAKE_EXECUTABLE_SUFFIX} > ./kernel.asm
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Convert the ELF output file to a binary image" )
