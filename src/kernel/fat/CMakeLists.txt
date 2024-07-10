
if(PREDOS_TEST)
  include(create_mock)
  create_mock(mock_emmc ${DRIVER_INTERFACE_DIR}/emmc.h)
  target_include_directories(mock_emmc PUBLIC ${COMMON_DIR})

  create_mock(mock_sys_timer ${DRIVER_INTERFACE_DIR}/sys_timer.h)
  target_include_directories(mock_sys_timer PUBLIC ${COMMON_DIR})

  add_library(fat OBJECT fat.c)
  target_link_libraries(fat PRIVATE mock_emmc)
  target_include_directories(fat PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

  add_subdirectory(test)
else()
  add_library(fat OBJECT fat.c)
  target_link_libraries(fat PRIVATE drivers)
  target_include_directories(fat PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
endif()
