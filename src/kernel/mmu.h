#include "status.h"

typedef enum {
  FAULT = 0b00,
  COARSE_PAGE_TABLE = 0b01,
  SECTION = 0b10
} mmu_first_level_descriptor_type_t;

typedef enum {
  LARGE_PAGE = 0b01,
  SMALL_PAGE_EXECUTABLE = 0b10,
  SMALL_PAGE_NOT_EXECUTABLE = 0b11
} mmu_second_level_descriptor_type_t;

typedef union {
  struct {
    uint32_t descriptor_type : 2;
    uint32_t bufferable : 1;
    uint32_t cacheable : 1;
    uint32_t execute_never : 1;
    uint32_t domain : 4;
    uint32_t reserved0 : 1;
    uint32_t access_permission : 2;
    uint32_t type_extension : 3;
    uint32_t access_permission_extension : 1;
    uint32_t shareable : 1;
    uint32_t not_global : 1;
    uint32_t supersection : 1;
    uint32_t reserved1 : 1;
    uint32_t section_base_address : 12;
  } fields;
  uint32_t raw;
} mmu_section_descriptor_t;

typedef union {
  struct {
    uint32_t descriptor_type : 2;
    uint32_t reserved0 : 3;
    uint32_t domain : 4;
    uint32_t reserved1 : 1;
    uint32_t coarse_page_table_addr : 22;
  } fields;
  uint32_t raw;
} mmu_coarse_page_table_descriptor_t;

typedef union {
  struct {
    uint32_t descriptor_type : 2;
    uint32_t bufferable : 1;
    uint32_t cacheable : 1;
    uint32_t access_permission : 2;
    uint32_t reserved0 : 3;
    uint32_t access_permission_extension : 1;
    uint32_t shareable : 1;
    uint32_t not_global : 1;
  } fields;
} mmu_small_page_descriptor_t;

status_t mmu_init();
status_t mmu_set_descriptor(uint16_t table_index, mmu_section_descriptor_t section);
