add_library(proc OBJECT proc.c)
target_link_libraries(proc PRIVATE mmu drivers)
target_include_directories(proc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})


