add_library(syscall OBJECT syscall.c)
target_link_libraries(syscall PRIVATE fat drivers)
target_include_directories(syscall PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})


