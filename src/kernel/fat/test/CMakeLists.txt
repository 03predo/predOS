include(create_test)

create_test(test_fat test_fat.c "")
target_link_libraries(test_fat fat mock_emmc mock_sys_timer)
target_include_directories(test_fat PUBLIC ${COMMON_DIR})
target_compile_definitions(test_fat PRIVATE IMG_DIR="${IMG_DIR}")
target_compile_definitions(test_fat PRIVATE BUILD_DIR="${BUILD_DIR}")



