add_library(syscall OBJECT syscall.c)
target_include_directories(syscall PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${COMMON_DIR} ${KERNEL_DIR})


