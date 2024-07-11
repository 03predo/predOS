add_library(boot OBJECT boot.S)
target_include_directories(boot PUBLIC ${COMMON_DIR})
