#include <stdbool.h>

#include "status.h"
#include "mmu_descriptors.h"

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

typedef enum {
  STRONGLY_ORDERED  = 0b00,
  DEVICE            = 0b01,
  NORMAL            = 0b10,
} mmu_memory_type_t;

typedef enum {
  NON_CACHEABLE = 0b00,
  WRITE_BACK_WRITE_ALLOCATE = 0b01,
  WRITE_THROUGH_NO_WRITE_ALLOCATE = 0b10,
  WRITE_BACK_NO_WRITE_ALLOCATE = 0b11,
} mmu_cache_policy_t;

typedef struct {
  mmu_memory_type_t memory_type;
  mmu_cache_policy_t inner_cache_policy;
  mmu_cache_policy_t outer_cache_policy;
  bool shareable;
} mmu_memory_attributes_t;


typedef enum {
  NO_ACCESS   = 0b00,
  READ_ONLY   = 0b01,
  READ_WRITE  = 0b10,
} mmu_access_type_t;

typedef struct {
  mmu_access_type_t user_permission;
  mmu_access_type_t kernel_permission;
} mmu_access_permissions_t;

status_t mmu_init();
status_t mmu_system_page_table_set_entry(uint16_t system_page_table_entry, mmu_generic_descriptor_t table_entry);
status_t mmu_root_coarse_page_table_clear_entry(uint8_t coarse_page_table_index);

status_t mmu_section_init(mmu_section_descriptor_t* section);
status_t mmu_section_set_base(mmu_section_descriptor_t* section, uint32_t section_base);
status_t mmu_section_set_attributes(mmu_section_descriptor_t* section, mmu_memory_attributes_t attributes);
status_t mmu_section_set_permissions(mmu_section_descriptor_t* section, mmu_access_permissions_t permissions);
status_t mmu_section_set_domain(mmu_section_descriptor_t* section, uint8_t domain);
status_t mmu_section_set_executable(mmu_section_descriptor_t* section, bool is_executable);
status_t mmu_section_set_global(mmu_section_descriptor_t* section, bool is_global);

status_t mmu_coarse_page_table_init(mmu_coarse_page_table_descriptor_t* coarse_page_table);
status_t mmu_coarse_page_table_set_base(mmu_coarse_page_table_descriptor_t* coarse_page_table, uint32_t coarse_page_table_base);
status_t mmu_coarse_page_table_set_domain(mmu_coarse_page_table_descriptor_t* coarse_page_table, uint8_t domain);

status_t mmu_small_page_init(mmu_small_page_descriptor_t* small_page);
status_t mmu_small_page_set_base(mmu_small_page_descriptor_t* small_page, uint32_t small_page_base);
status_t mmu_small_page_set_attributes(mmu_small_page_descriptor_t* small_page, mmu_memory_attributes_t attributes);
status_t mmu_small_page_set_permissions(mmu_small_page_descriptor_t* small_page, mmu_access_permissions_t permissions);
status_t mmu_small_page_set_executable(mmu_small_page_descriptor_t* small_page, bool is_executable);
status_t mmu_small_page_set_global(mmu_small_page_descriptor_t* small_page, bool is_global);

