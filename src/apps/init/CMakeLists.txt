add_executable(init main.c)
target_link_libraries(init PRIVATE syscall)

add_custom_command(
  TARGET init POST_BUILD
  COMMAND ${CMAKE_OBJCOPY} ${CMAKE_CURRENT_BINARY_DIR}/init${CMAKE_EXECUTABLE_SUFFIX} -O binary ${CMAKE_CURRENT_BINARY_DIR}/init
  COMMAND ${CMAKE_OBJDUMP} -l -S -D ${CMAKE_CURRENT_BINARY_DIR}/init${CMAKE_EXECUTABLE_SUFFIX} > ${CMAKE_CURRENT_BINARY_DIR}/init.asm
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

add_custom_target(image_add_init ALL DEPENDS image_add_init)
add_custom_command(
  TARGET image_add_init POST_BUILD
  DEPENDS syscall
  COMMAND ${CMAKE_SOURCE_DIR}/scripts/img_add_file.sh ${IMG_DIR}/mbr ${IMG_DIR}/fat ${CMAKE_CURRENT_BINARY_DIR}/init ${IMG_DIR}/predOS.img
  COMMAND ${CMAKE_SOURCE_DIR}/scripts/img_add_file.sh ${IMG_DIR}/mbr ${IMG_DIR}/fat ${CMAKE_CURRENT_BINARY_DIR}/init.elf ${IMG_DIR}/predOS.img
)
