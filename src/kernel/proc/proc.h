#include <stdint.h>
#include "status.h"

typedef enum {
  UNUSED = 0,
  READY = 1,
  RUNNING = 2,
  BLOCKED = 3
}process_state_t;

typedef struct {
  uint32_t id;
  process_state_t state;
  char** argv;
  uint32_t text_frame;
  uint32_t stack_frame;
  uint32_t virtual_stack_frame;
  uint32_t* stack_pointer;
  uint64_t timestamp;
}process_control_block_t;

status_t proc_create(process_control_block_t* pcb);
status_t proc_destroy(process_control_block_t* pcb);
