#include "proc.h"
#include "sys_log.h"

status_t proc_create(process_control_block_t* pcb){
  if(mmu_allocate_frame(&pcb->text_frame) != STATUS_OK){
    SYS_LOGE("failed to allocate frame");
    return STATUS_ERR;
  }
  SYS_LOGD("got frame: %#x", pcb->text_frame);

  if(mmu_allocate_frame(&pcb->stack_frame) != STATUS_OK){
    SYS_LOGE("failed to allocate frame");
    return STATUS_ERR;
  }
  SYS_LOGD("got frame: %#x", pcb->stack_frame);
  pcb->virtual_stack_frame = pcb->stack_frame;

  pcb->stack_pointer = (uint32_t*)(pcb->stack_frame + SECTION_SIZE);

  pcb->attributes = (mmu_memory_attributes_t) {
    .memory_type = NORMAL,
    .inner_cache_policy = WRITE_BACK_NO_WRITE_ALLOCATE,
    .outer_cache_policy = WRITE_BACK_NO_WRITE_ALLOCATE,
    .shareable = false,
  };

  pcb->permissions = (mmu_access_permissions_t) {
    .user_permission = READ_WRITE,
    .kernel_permission = READ_WRITE,
  };

  return STATUS_OK;
}

status_t proc_destroy(process_control_block_t* pcb){
  if(mmu_deallocate_frame(pcb->text_frame) != STATUS_OK){
    SYS_LOGE("failed to deallocate frame");
    return STATUS_ERR;
  }
  pcb->text_frame = 0;

  if(mmu_deallocate_frame(pcb->stack_frame) != STATUS_OK){
    SYS_LOGE("failed to deallocate frame");
    return STATUS_ERR;
  }
  pcb->stack_frame = 0;
  pcb->stack_pointer = NULL;
  
  return STATUS_OK;
}

status_t proc_frame_write_enable(process_control_block_t* pcb){
  mmu_memory_attributes_t attributes = {
    .memory_type = NORMAL,
    .inner_cache_policy = WRITE_BACK_NO_WRITE_ALLOCATE,
    .outer_cache_policy = WRITE_BACK_NO_WRITE_ALLOCATE,
    .shareable = false,
  };

  mmu_access_permissions_t permissions = {
    .user_permission = NO_ACCESS,
    .kernel_permission = READ_WRITE,
  };

  mmu_section_descriptor_t section;
  STATUS_OK_OR_RETURN(mmu_section_init(&section));
  STATUS_OK_OR_RETURN(mmu_section_set_attributes(&section, pcb->attributes));
  STATUS_OK_OR_RETURN(mmu_section_set_permissions(&section, pcb->permissions));
  STATUS_OK_OR_RETURN(mmu_section_set_domain(&section, 0));

  STATUS_OK_OR_RETURN(mmu_section_set_base(&section, SECTION_BASE(pcb->stack_frame)));
  STATUS_OK_OR_RETURN(mmu_system_page_table_set_entry(SECTION_BASE(pcb->stack_frame), section.raw));
  STATUS_OK_OR_RETURN(mmu_section_set_base(&section, SECTION_BASE(pcb->text_frame)));
  STATUS_OK_OR_RETURN(mmu_system_page_table_set_entry(SECTION_BASE(pcb->text_frame), section.raw));
  return STATUS_OK;
}

status_t proc_frame_write_disable(process_control_block_t* pcb){
  STATUS_OK_OR_RETURN(mmu_system_page_table_set_entry(SECTION_BASE(pcb->stack_frame), 0));
  STATUS_OK_OR_RETURN(mmu_system_page_table_set_entry(SECTION_BASE(pcb->text_frame), 0));
  return STATUS_OK;
}

status_t proc_frame_map(process_control_block_t* pcb){
  STATUS_OK_OR_RETURN(proc_frame_write_disable(pcb));
  mmu_section_descriptor_t section;
  STATUS_OK_OR_RETURN(mmu_section_init(&section));
  STATUS_OK_OR_RETURN(mmu_section_set_attributes(&section, pcb->attributes));
  STATUS_OK_OR_RETURN(mmu_section_set_permissions(&section, pcb->permissions));
  STATUS_OK_OR_RETURN(mmu_section_set_domain(&section, 0));
  STATUS_OK_OR_RETURN(mmu_section_set_base(&section, SECTION_BASE(pcb->stack_frame)));
  STATUS_OK_OR_RETURN(mmu_system_page_table_set_entry(SECTION_BASE(pcb->virtual_stack_frame), section.raw));

  mmu_small_page_descriptor_t small_page;
  STATUS_OK_OR_RETURN(mmu_small_page_init(&small_page));
  STATUS_OK_OR_RETURN(mmu_small_page_set_attributes(&small_page, pcb->attributes));
  STATUS_OK_OR_RETURN(mmu_small_page_set_permissions(&small_page, pcb->permissions));
  STATUS_OK_OR_RETURN(mmu_small_page_set_executable(&small_page, true));

  for(int i = 0; i < (256 - SMALL_PAGE_BASE(0x8000)); ++i){
    STATUS_OK_OR_RETURN(mmu_small_page_set_base(&small_page, SMALL_PAGE_BASE(pcb->text_frame + 0x8000) + i));
    STATUS_OK_OR_RETURN(mmu_root_coarse_page_table_set_entry(SMALL_PAGE_BASE(0x8000) + i, small_page));
  }
  return STATUS_OK;
}

