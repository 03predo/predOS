add_executable(echo main.c)
target_include_directories(echo PUBLIC ${APPS_DIR} ${DRIVER_INTERFACE_DIR} ${COMMON_DIR})
target_link_libraries(echo PRIVATE syscall)
add_custom_command(
  TARGET echo POST_BUILD
  COMMAND ${CMAKE_OBJCOPY} ${CMAKE_CURRENT_BINARY_DIR}/echo${CMAKE_EXECUTABLE_SUFFIX} -O binary ${CMAKE_CURRENT_BINARY_DIR}/echo.app 
  COMMAND ${CMAKE_OBJDUMP} -l -S -D ${CMAKE_CURRENT_BINARY_DIR}/echo${CMAKE_EXECUTABLE_SUFFIX} > ${CMAKE_CURRENT_BINARY_DIR}/echo.asm
  COMMAND mv ${CMAKE_CURRENT_BINARY_DIR}/echo${CMAKE_EXECUTABLE_SUFFIX} ${CMAKE_CURRENT_BINARY_DIR}/echo.elf
  COMMAND mv ${CMAKE_CURRENT_BINARY_DIR}/echo.app ${CMAKE_CURRENT_BINARY_DIR}/echo
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

add_custom_target(image_add_echo ALL DEPENDS image_add_echo)
add_custom_command(
  TARGET image_add_echo POST_BUILD
  DEPENDS syscall
  COMMAND ${CMAKE_SOURCE_DIR}/scripts/img_add_file.sh ${IMG_DIR}/mbr ${IMG_DIR}/fat ${CMAKE_CURRENT_BINARY_DIR}/echo${CMAKE_EXECUTABLE_SUFFIX} ${IMG_DIR}/predOS.img
)

