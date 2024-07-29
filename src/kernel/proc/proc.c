#include "proc.h"
#include "mmu.h"
#include "sys_log.h"

status_t proc_create(process_control_block_t* pcb){
  if(mmu_allocate_frame(&pcb->text_frame) != STATUS_OK){
    SYS_LOGE("failed to allocate frame");
    return STATUS_ERR;
  }
  SYS_LOGI("got frame: %#x", pcb->text_frame);

  if(mmu_allocate_frame(&pcb->stack_frame) != STATUS_OK){
    SYS_LOGE("failed to allocate frame");
    return STATUS_ERR;
  }
  SYS_LOGI("got frame: %#x", pcb->stack_frame);
  pcb->virtual_stack_frame = pcb->stack_frame;

  pcb->stack_pointer = (uint32_t*)(pcb->stack_frame + SECTION_SIZE);

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


