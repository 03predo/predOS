#include <stdint.h>

typedef union {
  struct {
    uint32_t descriptor_type : 2;
    uint32_t reserved : 30;
  } fields;
  uint32_t raw;
} mmu_generic_descriptor_t;

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
    uint32_t section_base : 12;
  } fields;
  uint32_t raw;
} mmu_section_descriptor_t;

typedef union {
  struct {
    uint32_t descriptor_type : 2;
    uint32_t reserved0 : 3;
    uint32_t domain : 4;
    uint32_t reserved1 : 1;
    uint32_t coarse_page_table_base : 22;
  } fields;
  uint32_t raw;
} mmu_coarse_page_table_descriptor_t;

typedef union {
  struct {
    uint32_t descriptor_type : 2;
    uint32_t bufferable : 1;
    uint32_t cacheable : 1;
    uint32_t access_permission : 2;
    uint32_t type_extension : 3;
    uint32_t access_permission_extension : 1;
    uint32_t shareable : 1;
    uint32_t not_global : 1;
    uint32_t small_page_base : 20;
  } fields;
  uint32_t raw;
} mmu_small_page_descriptor_t;

