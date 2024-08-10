#include <stdint.h>
#include <unistd.h>
#include <signal.h>
#include "status.h"
#include "mmu.h"

typedef enum {
  UNUSED = 0,
  READY = 1,
  RUNNING = 2,
  BLOCKED = 3,
  SLEEP = 4,
  SIGNAL = 5,
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
  process_state_t prev_state;
  char** argv;
  uint32_t text_frame;
  uint32_t stack_frame;
  uint32_t virtual_stack_frame;
  uint32_t* stack_pointer;
  uint32_t* prev_stack_pointer;
  uint64_t timestamp;
  uint64_t start_time;
  mmu_memory_attributes_t attributes;
  mmu_access_permissions_t permissions;
  process_blocked_t blocked;
  sig_t signal_handler[NSIG];
} process_control_block_t;

status_t proc_create(process_control_block_t* pcb);
status_t proc_destroy(process_control_block_t* pcb);
status_t proc_frame_write_disable(process_control_block_t* pcb);
status_t proc_frame_write_enable(process_control_block_t* pcb);
status_t proc_frame_map(process_control_block_t* pcb);
const char* proc_state_to_string(process_state_t state);
