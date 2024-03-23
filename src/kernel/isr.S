.section ".text"

#include "cpsr_def.S"

.global _software_interrupt_handler
.global _context_switch

_software_interrupt_handler:
  // disable interrupts
  cpsid if

  // store r0 on svc stack as we use it to update cpsr
  stmdb sp!, {r0, r1}

  // switch to saved processor state to get banked registers
  mrs r0, spsr
  and r1, r0, #0x1f
  cmp r1, #(CPSR_MODE_USER)
  biceq r0, r0, #0x1f
  orreq r0, r0, #(CPSR_MODE_SYSTEM)

  orr r0, r0, #(CPSR_IRQ_INHIBIT | CPSR_FIQ_INHIBIT)
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

_context_switch:
  ldmia r0!, {r1, r2}
  msr spsr, r1
  mov lr, r2
  srsdb sp!, #(CPSR_MODE_SVR)

  and r2, r1, #0x1f
  cmp r2, #(CPSR_MODE_USER)
  biceq r1, r1, #0x1f
  orreq r1, r1, #(CPSR_MODE_SYSTEM)
  msr cpsr_c, r1

  mov sp, r0
  ldmia sp!, {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}

  msr cpsr_c, #(CPSR_MODE_SVR | CPSR_IRQ_INHIBIT | CPSR_FIQ_INHIBIT)
  rfeia sp!

