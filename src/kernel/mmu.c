#include <stdint.h>
#include "sys_log.h"
#include "mmu.h"
#include "bcm2835.h"
#include "util.h"

#define KERNEL_VIRTUAL_OFFSET 0x10000000U
#define PERIPHERAL_PHYSICAL_BASE 0x20000000
#define PAGE_TABLE_SIZE 4096

extern void _mmu_enable(uint32_t* page_table_base);
extern void _mmu_invalidate_tlb();
status_t mmu_init(void) __attribute__((section(".text.boot.kernel")));

static uint32_t page_table[PAGE_TABLE_SIZE] __attribute__((section(".page_table")));

status_t mmu_init(){ 
  uint32_t* _page_table = (uint32_t*)(((uint32_t)page_table) - KERNEL_VIRTUAL_OFFSET);
  for(uint32_t i = 0; i < PAGE_TABLE_SIZE; ++i){
    _page_table[i] = 0;
  }

  mmu_section_descriptor_t section;
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
      .section_base_address = 0x000,
    }
  };
  _page_table[0] = section.raw;
  _page_table[0x100] = section.raw;
  
  
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

status_t mmu_set_descriptor(uint16_t table_index, mmu_section_descriptor_t section){   
  page_table[table_index] = section.raw;
  _mmu_invalidate_tlb();
  return STATUS_OK;
}
