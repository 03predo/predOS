#include <stdint.h>
#include "sys_log.h"
#include "mmu.h"
#include "bcm2835.h"
#include "util.h"

#define KERNEL_VIRTUAL_ADDR      0xC0000000
#define KERNEL_STACK_ADDR        0x1ff00000
#define PERIPHERAL_PHYSICAL_ADDR 0x20000000
#define PERIPHERAL_VIRTUAL_ADDR  PERIPHERAL_BASE

#define KERNEL_LOAD_ADDR              0x8000
#define SYSTEM_PAGE_TABLE_ADDR        0x4000
#define ROOT_COARSE_PAGE_TABLE_ADDR   0x400

#define FIRST_LEVEL_PAGE_TABLE_SIZE 1024 // in 32bit words
#define SECOND_LEVEL_PAGE_TABLE_SIZE 256 // in 32bit words

#define SECTION_BASE(X) (X >> 20)
#define COARSE_PAGE_TABLE_BASE(X) (X >> 10)
#define SMALL_PAGE_BASE(X) (X >> 12)

extern void _mmu_enable(uint32_t* page_table_base);
extern void _mmu_invalidate_tlb();
status_t mmu_init(void) __attribute__((section(".text.boot.kernel")));

static uint32_t* system_page_table = (uint32_t*) SYSTEM_PAGE_TABLE_ADDR;
static uint32_t* root_coarse_page_table = (uint32_t*) ROOT_COARSE_PAGE_TABLE_ADDR; 

status_t mmu_init(){
  for(uint32_t i = 0; i < FIRST_LEVEL_PAGE_TABLE_SIZE; ++i){
    system_page_table[i] = 0;
  }

  // setup the kernel and peripheral sections in the system page table
  mmu_section_descriptor_t section;
  for(uint32_t i = 0; i < (FIRST_LEVEL_PAGE_TABLE_SIZE - SECTION_BASE(PERIPHERAL_VIRTUAL_ADDR)); ++i){
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
        .section_base = SECTION_BASE(PERIPHERAL_PHYSICAL_ADDR) + i,
      }
    };
    system_page_table[SECTION_BASE(PERIPHERAL_VIRTUAL_ADDR) + i] = section.raw;
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
      .section_base = 0x000,
    }
  };
  system_page_table[SECTION_BASE(KERNEL_VIRTUAL_ADDR)] = section.raw;

  section.fields.section_base = SECTION_BASE(KERNEL_STACK_ADDR);
  system_page_table[SECTION_BASE(KERNEL_STACK_ADDR)] = section.raw; 
 
  // setup the root coarse page table, this will be the page table
  // responsible mapping the addresses from 0x00000000 - 0x00100000
  mmu_coarse_page_table_descriptor_t coarse_page_table = {
    .fields = {
      .descriptor_type = COARSE_PAGE_TABLE,
      .domain = 0,
      .coarse_page_table_base = COARSE_PAGE_TABLE_BASE((uint32_t)root_coarse_page_table),
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
      .small_page_base = SMALL_PAGE_BASE(ROOT_COARSE_PAGE_TABLE_ADDR),
    }
  };

  // mapping for the root coarse page table
  root_coarse_page_table[SMALL_PAGE_BASE(ROOT_COARSE_PAGE_TABLE_ADDR)] = small_page.raw;

  // mapping for the 
  small_page.fields.small_page_base = SMALL_PAGE_BASE(SYSTEM_PAGE_TABLE_ADDR);
  root_coarse_page_table[SMALL_PAGE_BASE(SYSTEM_PAGE_TABLE_ADDR)] = small_page.raw;

  small_page.fields.small_page_base = SMALL_PAGE_BASE(KERNEL_LOAD_ADDR);
  root_coarse_page_table[SMALL_PAGE_BASE(KERNEL_LOAD_ADDR)] = small_page.raw;

  _mmu_enable(system_page_table);
  return STATUS_OK;
}

status_t mmu_root_coarse_page_table_clear_entry(uint8_t coarse_page_table_index){
  root_coarse_page_table[coarse_page_table_index] = 0;
  _mmu_invalidate_tlb();
  return STATUS_OK;
}

status_t mmu_section_set_base(mmu_section_descriptor_t* section, uint32_t section_base){
  if(section_base > 0xfff){
    SYS_LOG("invalid section base");
    return STATUS_ERR;
  }
  section->fields.section_base = section_base;
  return STATUS_OK;
}

status_t mmu_section_set_attributes(mmu_section_descriptor_t* section, mmu_memory_attributes_t attributes){
  switch(attributes.memory_type){
    case STRONGLY_ORDERED:
      section->fields.bufferable = 0;
      section->fields.cacheable = 0;
      section->fields.type_extension = 0;
      section->fields.shareable = 0;
      break;

    case DEVICE:
      if(attributes.shareable == 1){
        section->fields.bufferable = 1;
        section->fields.cacheable = 0;
        section->fields.type_extension = 0; 
        section->fields.shareable = 1;
      }else{
        section->fields.bufferable = 0;
        section->fields.cacheable = 0;
        section->fields.type_extension = 0b010;
        section->fields.shareable = 0;
      }
      break;

    case NORMAL:
      if(attributes.inner_cache_policy == attributes.outer_cache_policy){
        switch(attributes.inner_cache_policy){
          case NON_CACHEABLE:
            section->fields.bufferable = 0;
            section->fields.cacheable = 0;
            section->fields.type_extension = 0b001;
            break;

          case WRITE_BACK_WRITE_ALLOCATE:
            section->fields.bufferable = 1;
            section->fields.cacheable = 1;
            section->fields.type_extension = 0b001;
            break;

          case WRITE_THROUGH_NO_WRITE_ALLOCATE:
            section->fields.bufferable = 0;
            section->fields.cacheable = 1;
            section->fields.type_extension = 0;
            break;

          case WRITE_BACK_NO_WRITE_ALLOCATE:
            section->fields.bufferable = 1;
            section->fields.cacheable = 1;
            section->fields.type_extension = 0;
            break;
          default:
            SYS_LOG("undefined cache policy");
            return STATUS_ERR;
            break;
        }
      }else{
        section->fields.bufferable = attributes.inner_cache_policy & 0b01;
        section->fields.cacheable = attributes.inner_cache_policy >> 1;
        section->fields.type_extension = 0b100 + attributes.outer_cache_policy;
      }

      if(attributes.shareable){
        section->fields.shareable = 1;
      }
      break;

    default:
      SYS_LOG("undefined memory type");
      return STATUS_ERR;
  }
  return STATUS_OK;
}

status_t mmu_section_set_permissions(mmu_section_descriptor_t* section, mmu_access_permissions_t permissions){
  switch(permissions.kernel_permission){
    case NO_ACCESS:
      if(permissions.user_permission != NO_ACCESS){
        SYS_LOG("invalid user permission");
        return STATUS_ERR;
      }
      section->fields.access_permission = 0b00;
      section->fields.access_permission_extension = 0;
      break;

    case READ_ONLY:
      if(permissions.user_permission == NO_ACCESS){
        section->fields.access_permission = 0b01;
        section->fields.access_permission_extension = 1;
      }else if(permissions.user_permission == READ_ONLY){
        section->fields.access_permission = 0b10;
        section->fields.access_permission_extension = 1;
      }else if(permissions.user_permission == READ_WRITE){
        SYS_LOG("invalid user permission");
        return STATUS_ERR;
      }else{
        SYS_LOG("undefined user permission");
        return STATUS_ERR;
      }
      break;

    case READ_WRITE:
      if(permissions.user_permission == NO_ACCESS){
        section->fields.access_permission = 0b01;
        section->fields.access_permission_extension = 0;
      }else if(permissions.user_permission == READ_ONLY){
        section->fields.access_permission = 0b10;
        section->fields.access_permission_extension = 0;
      }else if(permissions.user_permission == READ_WRITE){
        section->fields.access_permission = 0b11;
        section->fields.access_permission_extension = 0;
      }else{
        SYS_LOG("undefined use permission");
        return STATUS_ERR;
      }
      break;
    
    default:
      SYS_LOG("undefined kernel permission type");
      return STATUS_ERR;
      break;
  }
  return STATUS_OK;
}


status_t mmu_section_set_domain(mmu_section_descriptor_t* section, uint8_t domain){
  if(domain > 0b1111){
    SYS_LOG("undefined domain");
    return STATUS_ERR; 
  }
  section->fields.domain = domain;
  return STATUS_OK;
}

status_t mmu_section_set_executable(mmu_section_descriptor_t* section, bool is_executable){
  section->fields.execute_never = is_executable ? 0 : 1;
}

status_t mmu_section_set_global(mmu_section_descriptor_t* section, bool is_global){
  section->fields.not_global = is_global ? 0 : 1;
}

status_t mmu_section_init(mmu_section_descriptor_t* section){
  section->fields.descriptor_type = SECTION;
  section->fields.supersection = 0;

  STATUS_OK_OR_RETURN(mmu_section_set_base(section, 0xfff));

  mmu_memory_attributes_t attributes = {
    .memory_type = STRONGLY_ORDERED,
    .inner_cache_policy = NON_CACHEABLE,
    .outer_cache_policy = NON_CACHEABLE,
    .shareable = 0,
  };
  STATUS_OK_OR_RETURN(mmu_section_set_attributes(section, attributes));

  mmu_access_permissions_t permissions = {
    .user_permission = NO_ACCESS,
    .kernel_permission = NO_ACCESS,
  };
  STATUS_OK_OR_RETURN(mmu_section_set_permissions(section, permissions));

  STATUS_OK_OR_RETURN(mmu_section_set_domain(section, 0));

  STATUS_OK_OR_RETURN(mmu_section_set_executable(section, false));

  STATUS_OK_OR_RETURN(mmu_section_set_global(section, false));

  return STATUS_OK;
}

status_t mmu_coarse_page_table_set_base(mmu_coarse_page_table_descriptor_t* coarse_page_table, uint32_t coarse_page_table_base){
  if(coarse_page_table_base > 0x3fffff){
    SYS_LOG("invalid section base");
    return STATUS_ERR;
  }
  coarse_page_table->fields.coarse_page_table_base = coarse_page_table_base;
  return STATUS_OK;
}

status_t mmu_coarse_page_table_set_domain(mmu_coarse_page_table_descriptor_t* coarse_page_table, uint8_t domain){
  mmu_section_descriptor_t section;
  STATUS_OK_OR_RETURN(mmu_section_set_domain(&section, domain));
  coarse_page_table->fields.domain = section.fields.domain;
}

status_t mmu_coarse_page_table_init(mmu_coarse_page_table_descriptor_t* coarse_page_table){
  coarse_page_table->fields.descriptor_type = COARSE_PAGE_TABLE;
  STATUS_OK_OR_RETURN(mmu_coarse_page_table_set_base(coarse_page_table, 0x3fffff));
  STATUS_OK_OR_RETURN(mmu_coarse_page_table_set_domain(coarse_page_table, 0x0));
  return STATUS_OK;
}

status_t mmu_small_page_set_base(mmu_small_page_descriptor_t* small_page, uint32_t small_page_base){
  if(small_page_base > 0xfffff){
    SYS_LOG("invalid section base");
    return STATUS_ERR;
  }
  small_page->fields.small_page_base = small_page_base;
  return STATUS_OK;
}

status_t mmu_small_page_set_attributes(mmu_small_page_descriptor_t* small_page, mmu_memory_attributes_t attributes){
  mmu_section_descriptor_t section;
  STATUS_OK_OR_RETURN(mmu_section_set_attributes(&section, attributes));
  small_page->fields.bufferable = section.fields.bufferable;
  small_page->fields.cacheable = section.fields.cacheable;
  small_page->fields.type_extension = section.fields.type_extension;
  small_page->fields.shareable = section.fields.shareable;
  return STATUS_OK;
}

status_t mmu_small_page_set_permissions(mmu_small_page_descriptor_t* small_page, mmu_access_permissions_t permissions){
  mmu_section_descriptor_t section;
  STATUS_OK_OR_RETURN(mmu_section_set_permissions(&section, permissions));
  small_page->fields.access_permission = section.fields.access_permission;
  small_page->fields.access_permission_extension = section.fields.access_permission_extension;
  return STATUS_OK;
}

status_t mmu_small_page_set_executable(mmu_small_page_descriptor_t* small_page, bool is_executable){
  small_page->fields.descriptor_type = is_executable ? SMALL_PAGE_EXECUTABLE : SMALL_PAGE_NOT_EXECUTABLE;
  return STATUS_OK;
}

status_t mmu_small_page_set_global(mmu_small_page_descriptor_t* small_page, bool is_global){
  mmu_section_descriptor_t section;
  STATUS_OK_OR_RETURN(mmu_section_set_global(&section, is_global));
  small_page->fields.not_global = section.fields.not_global;
  return STATUS_OK;
}

status_t mmu_small_page_init(mmu_small_page_descriptor_t* small_page){
  STATUS_OK_OR_RETURN(mmu_small_page_set_base(small_page, 0xfffff));
  mmu_memory_attributes_t attributes = {
    .memory_type = STRONGLY_ORDERED,
    .inner_cache_policy = NON_CACHEABLE,
    .outer_cache_policy = NON_CACHEABLE,
    .shareable = 0,
  };
  STATUS_OK_OR_RETURN(mmu_small_page_set_attributes(small_page, attributes));
  mmu_access_permissions_t permissions = {
    .user_permission = NO_ACCESS,
    .kernel_permission = NO_ACCESS,
  };
  STATUS_OK_OR_RETURN(mmu_small_page_set_permissions(small_page, permissions));
  STATUS_OK_OR_RETURN(mmu_small_page_set_executable(small_page, false)); 
  STATUS_OK_OR_RETURN(mmu_small_page_set_global(small_page, false));
  return STATUS_OK;
}


