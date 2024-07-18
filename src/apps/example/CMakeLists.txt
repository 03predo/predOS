add_executable(example example.c)
target_include_directories(example PUBLIC ${APPS_DIR} ${DRIVER_INTERFACE_DIR} ${COMMON_DIR})
target_link_libraries(example PRIVATE fat syscall)
add_custom_command(
  TARGET example POST_BUILD
  COMMAND ${CMAKE_OBJDUMP} -l -S -D ${CMAKE_CURRENT_BINARY_DIR}/example${CMAKE_EXECUTABLE_SUFFIX} > ${CMAKE_CURRENT_BINARY_DIR}/example.asm
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

add_custom_target(image_add_example ALL DEPENDS image_add_example)
add_custom_command(
  TARGET image_add_example POST_BUILD
  DEPENDS kernel
  COMMAND ${CMAKE_SOURCE_DIR}/scripts/img_add_file.sh ${IMG_DIR}/mbr ${IMG_DIR}/fat ${CMAKE_CURRENT_BINARY_DIR}/example${CMAKE_EXECUTABLE_SUFFIX} ${IMG_DIR}/predOS.img
)

