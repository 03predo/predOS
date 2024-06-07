#include <stdint.h>
#include "sys_log.h"
#include "mmu.h"
#include "bcm2835.h"
#include "util.h"

#define PERIPHERAL_PHYSICAL_BASE 0x20000000

#define SYSTEM_PAGE_TABLE_ADDR 0x4000
#define KERNEL_COARSE_PAGE_TABLE_ADDR 0x400

#define FIRST_LEVEL_PAGE_TABLE_SIZE 1024
#define SECOND_LEVEL_PAGE_TABLE_SIZE 256

extern void _mmu_enable(uint32_t* page_table_base);
extern void _mmu_invalidate_tlb();
status_t mmu_init(void) __attribute__((section(".text.boot.kernel")));

static uint32_t* system_page_table = (uint32_t*) SYSTEM_PAGE_TABLE_ADDR;
static uint32_t* kernel_coarse_page_table = (uint32_t*) KERNEL_COARSE_PAGE_TABLE_ADDR; 

status_t mmu_init(){ 
  for(uint32_t i = 0; i < FIRST_LEVEL_PAGE_TABLE_SIZE; ++i){
    system_page_table[i] = 0;
  }

  mmu_section_descriptor_t section;
  for(uint32_t i = (PERIPHERAL_BASE >> 20); i < FIRST_LEVEL_PAGE_TABLE_SIZE; ++i){
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
    system_page_table[i] = section.raw;
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
  system_page_table[0x100] = section.raw;

  section.fields.section_base_address = 0x101;
  system_page_table[0x101] = section.raw; 
 
  mmu_coarse_page_table_descriptor_t coarse_page_table = {
    .fields = {
      .descriptor_type = COARSE_PAGE_TABLE,
      .domain = 0,
      .coarse_page_table_addr = ((uint32_t)kernel_coarse_page_table >> 10),
    }
  };
  system_page_table[0x000] = coarse_page_table.raw;
  
  mmu_small_page_descriptor_t small_page = {
    .fields = {
      .descriptor_type = SMALL_PAGE_EXECUTABLE,
      .bufferable = 1,
      .cacheable = 1,
      .access_permission = 0b11,
      .access_permission_extension = 0,
      .shareable  = 0,
      .not_global = 0,
      .small_page_addr = 0x0,
    }
  };

  kernel_coarse_page_table[0x0] = small_page.raw;

  small_page.fields.small_page_addr = 0x4;
  kernel_coarse_page_table[0x4] = small_page.raw;

  small_page.fields.small_page_addr = 0x8;
  kernel_coarse_page_table[0x8] = small_page.raw;

  _mmu_enable(system_page_table);
  return STATUS_OK;
}

status_t mmu_set_descriptor(){
  kernel_coarse_page_table[0x8] = 0;
  _mmu_invalidate_tlb();
  return STATUS_OK;
}
