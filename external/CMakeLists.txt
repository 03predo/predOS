add_library(unity STATIC Unity/src/unity.c)
target_include_directories(unity PUBLIC Unity/src)

add_library(cmock CMock/src/cmock.c Unity)
target_link_libraries(cmock unity)
target_include_directories(cmock PUBLIC CMock/src)



