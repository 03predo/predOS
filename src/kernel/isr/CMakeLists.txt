add_library(isr OBJECT isr.c isr.S)
target_link_libraries(isr PRIVATE drivers)
