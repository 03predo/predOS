
add_subdirectory(fat)

if(NOT PREDOS_TEST)
  add_subdirectory(boot)
  add_subdirectory(isr)
  add_subdirectory(mmu)
  add_subdirectory(syscall)
  add_executable(
    kernel
    kernel.c
    kernel.S
  )
  target_link_libraries(kernel PRIVATE boot isr fat mmu drivers syscall)
  add_custom_command(
      TARGET kernel POST_BUILD
      COMMAND ${CMAKE_OBJCOPY} ${CMAKE_CURRENT_BINARY_DIR}/kernel${CMAKE_EXECUTABLE_SUFFIX} -O binary ${CMAKE_CURRENT_BINARY_DIR}/kernel.img
      COMMAND ${CMAKE_OBJDUMP} -l -S -D ${CMAKE_CURRENT_BINARY_DIR}/kernel${CMAKE_EXECUTABLE_SUFFIX} > ${CMAKE_CURRENT_BINARY_DIR}/kernel.asm
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
      COMMENT "Convert the ELF output file to a binary image" )


  add_custom_target(image_make ALL DEPENDS image_make)
  add_custom_command(
    TARGET image_make POST_BUILD
    DEPENDS kernel
    COMMAND ${CMAKE_SOURCE_DIR}/scripts/img_init.sh ${CMAKE_CURRENT_BINARY_DIR}/kernel.img ${IMG_DIR}
  )

  add_custom_target(image_add_big ALL DEPENDS image_add_big)
  add_custom_command(
    TARGET image_add_big POST_BUILD
    DEPENDS kernel
    COMMAND ${CMAKE_SOURCE_DIR}/scripts/img_add_file.sh ${IMG_DIR}/mbr ${IMG_DIR}/fat ${CMAKE_SOURCE_DIR}/big.txt ${IMG_DIR}/predOS.img
  )  

endif()

