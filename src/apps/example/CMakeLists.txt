add_library(example STATIC example.c)
target_include_directories(example PUBLIC ${APPS_DIR} ${DRIVER_INTERFACE_DIR} ${COMMON_DIR})
target_link_libraries(example PRIVATE drivers)
