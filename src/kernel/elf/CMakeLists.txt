add_library(elf OBJECT elf.c)
target_link_libraries(elf PRIVATE drivers)
target_include_directories(elf PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})


