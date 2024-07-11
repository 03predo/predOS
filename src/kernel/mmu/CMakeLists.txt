add_library(mmu OBJECT mmu.c mmu.S)
target_link_libraries(mmu PRIVATE drivers)
target_include_directories(mmu PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})


