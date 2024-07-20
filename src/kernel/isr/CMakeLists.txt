add_library(isr OBJECT isr.c isr.S svc.c)
target_link_libraries(isr PRIVATE drivers fat)
target_include_directories(isr PRIVATE ${KERNEL_DIR})
