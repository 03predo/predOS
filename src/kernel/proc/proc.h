#include <stdint.h>
#include <unistd.h>
#include "status.h"
#include "mmu.h"

typedef enum {
  UNUSED = 0,
  READY = 1,
  RUNNING = 2,
  BLOCKED = 3,
  SLEEP = 4,
} process_state_t;

typedef enum {
  NONE = 0,
  WAIT,
  READ,
} process_blocked_t;

typedef struct {
  pid_t pid;
  pid_t parent_pid;
  process_state_t state;
  char** argv;
  uint32_t text_frame;
  uint32_t stack_frame;
  uint32_t virtual_stack_frame;
  uint32_t* stack_pointer;
  uint64_t timestamp;
  mmu_memory_attributes_t attributes;
  mmu_access_permissions_t permissions;
  process_blocked_t blocked;

} process_control_block_t;

status_t proc_create(process_control_block_t* pcb);
status_t proc_destroy(process_control_block_t* pcb);
status_t proc_frame_write_disable(process_control_block_t* pcb);
status_t proc_frame_write_enable(process_control_block_t* pcb);
status_t proc_frame_map(process_control_block_t* pcb);
