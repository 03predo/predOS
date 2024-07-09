add_executable(test_fat test_fat.c)

target_link_libraries(test_fat Unity)

add_test(NAME test_runner_fat COMMAND test_fat)
