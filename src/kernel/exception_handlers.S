.section ".text"

#include "arm_defines.S"

.global _undefined_instruction_handler
.global _prefetch_abort_handler
.global _software_interrupt_handler

_undefined_instruction_handler:
  mrs r0, spsr
  mov r1, lr
  b undefined_instruction_handler

_prefetch_abort_handler:
  // load IFSR into r0
  mrc p15, #0, r0, c5, c0, #1
  // the status is in the bottom 4 bits
  and r0, r0, #0b1111 
  cmp r0, #(IFSR_DEBUG_EVENT)
  bne prefetch_abort_handler
  moveq r0, lr
  beq debug_event_handler

_software_interrupt_handler:
  // disable interrupts
  cpsid iaf

  // store r0 on svc stack as we use it to update cpsr
  stmdb sp!, {r0, r1}

  // switch to saved processor state to get banked registers
  mrs r0, spsr
  bic r0, r0, #0x1f
  orr r0, r0, #(CPSR_MODE_SYSTEM | CPSR_IRQ_INHIBIT | CPSR_FIQ_INHIBIT)
  msr cpsr_c, r0

  // store general purpose and banked lr on banked stack, then store banked stack in r1
  stmdb sp!, {r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, lr} 
  mov r2, sp

  // switch back to supervisor mode
  mrs r0, cpsr
  bic r0, r0, #(0x1f) // clear last mode
  orr r0, r0, #(CPSR_MODE_SVR)
  msr cpsr_c, r0

  // place stacked r0 back on banked sp
  // now the banked stack pointer holds all of the registers corresponding to the saved mode
  ldmia sp!, {r0, r1}
  stmdb r2!, {r0, r1}

  // place saved processor mode and supervisor link register on process stack

  mrs r0, spsr
  stmdb r2!, {r0, lr}
  mov r0, r2

  b software_interrupt_handler

