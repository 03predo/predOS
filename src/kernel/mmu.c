#include <stdint.h>
#include "sys_log.h"
#include "mmu.h"

#define PAGE_TABLE_SIZE 4096

extern void _mmu_enable(uint32_t* page_table_base);

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

static uint32_t* page_table = (uint32_t*) 0x2000000;

status_t mmu_init(){
  SYS_LOG("sizeof descriptor: %d", sizeof(mmu_section_descriptor_t));
  SYS_LOG("page_table: %#x", page_table);
  for(uint32_t i = 0; i < PAGE_TABLE_SIZE; ++i){
    page_table[i] = 0;
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
  page_table[0] = section.raw;

  for(uint32_t i = 0x200; i < PAGE_TABLE_SIZE; ++i){
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
        .section_base_address = i,
      }
    };
    page_table[i] = section.raw;
  }
  
  _mmu_enable(page_table);
  return STATUS_OK;
}

