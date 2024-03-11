add_executable(kernel main.c boot.S)
target_link_libraries(kernel PRIVATE drivers)
add_custom_command(
    TARGET kernel POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} ${CMAKE_CURRENT_BINARY_DIR}/kernel${CMAKE_EXECUTABLE_SUFFIX} -O binary ${CMAKE_CURRENT_BINARY_DIR}/kernel.img
    COMMAND ${CMAKE_OBJDUMP} -l -S -D ${CMAKE_CURRENT_BINARY_DIR}/kernel${CMAKE_EXECUTABLE_SUFFIX} > ${CMAKE_CURRENT_BINARY_DIR}/kernel.asm
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Convert the ELF output file to a binary image" )

add_custom_target(make_card ALL DEPENDS make_card)
add_custom_command(
  TARGET make_card POST_BUILD
  DEPENDS kernel
  COMMAND ${CMAKE_SOURCE_DIR}/scripts/make_card.sh ${CMAKE_CURRENT_BINARY_DIR}/kernel.img ${RPIOS_IMG_DIR}
)

