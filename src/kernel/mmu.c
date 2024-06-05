#include <stdint.h>
#include "sys_log.h"
#include "mmu.h"
#include "bcm2835.h"
#include "util.h"

#define KERNEL_VIRTUAL_OFFSET 0x10000000U
#define PERIPHERAL_PHYSICAL_BASE 0x20000000
#define PAGE_TABLE_SIZE 4096

extern void _mmu_enable(uint32_t* page_table_base);
extern void _mmu_set_ttbr0(uint32_t* page_table);
extern void _mmu_invalidate_tlb();
status_t mmu_init(void) __attribute__((section(".text.boot.kernel")));

typedef enum {
  FAULT = 0b00,
  COARSE_PAGE_TABLE = 0b01,
  SECTION = 0b10
} mmu_first_level_descriptor_type_t;

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

static uint32_t page_table[PAGE_TABLE_SIZE] __attribute__((section(".page_table")));

status_t mmu_init(){ 
  uint32_t* _page_table = (uint32_t*)(((uint32_t)page_table) - KERNEL_VIRTUAL_OFFSET);
  for(uint32_t i = 0; i < PAGE_TABLE_SIZE; ++i){
    _page_table[i] = 0;
  }

  mmu_section_descriptor_t section = {
    .fields = {
      .descriptor_type = SECTION,
      .bufferable = 1,
      .cacheable = 1,
      .execute_never = 0,
      .domain = 0,
      .access_permission = 0b11,
      .type_extension = 0,
      .access_permission_extension = 0,
      .shareable = 0,
      .not_global = 0,
      .supersection = 0,
      .section_base_address = 0x000,
    }
  };
  _page_table[0] = section.raw;
  _page_table[0x100] = section.raw;

  for(uint32_t i = (PERIPHERAL_BASE >> 20); i < PAGE_TABLE_SIZE; ++i){
    section = (mmu_section_descriptor_t) {
      .fields = {
        .descriptor_type = SECTION,
        .bufferable = 1,
        .cacheable = 0,
        .execute_never = 1,
        .domain = 0,
        .access_permission = 0b11,
        .type_extension = 0,
        .access_permission_extension = 0,
        .shareable = 0,
        .not_global = 0,
        .supersection = 0,
        .section_base_address = ((PERIPHERAL_PHYSICAL_BASE >> 20) + i - (PERIPHERAL_BASE >> 20)),
      }
    };
    _page_table[i] = section.raw;
  }
  
  section = (mmu_section_descriptor_t) {
      .fields = {
        .descriptor_type = SECTION,
        .bufferable = 1,
        .cacheable = 1,
        .execute_never = 0,
        .domain = 0,
        .access_permission = 0b11,
        .type_extension = 0,
        .access_permission_extension = 0,
        .shareable = 0,
        .not_global = 0,
        .supersection = 0,
        .section_base_address = 0xfff,
      }
    };
  _page_table[0xfff] = section.raw;

  _mmu_enable(_page_table);
  return STATUS_OK;
}

status_t mmu_tmp(){   
  page_table[0] = 0;
  _mmu_invalidate_tlb();
  return STATUS_OK;
}
