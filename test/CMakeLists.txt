add_executable(test1 test.c)

target_link_libraries(test1 Unity)

add_test(NAME test_runner COMMAND test1)
