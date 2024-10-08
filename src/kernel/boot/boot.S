.section ".text.boot"

#include "arm_defines.S"

.global _start
.global kernel_init

_start:
  ldr pc,  _reset_vector
  ldr pc, _undefined_instruction_vector
  ldr pc, _software_interrupt_vector
  ldr pc, _prefetch_abort_vector
  ldr pc, _data_abort_vector
  ldr pc, _unused_vector
  ldr pc, _interrupt_vector
  ldr pc, _fast_interrupt_vector

_reset_vector:                  .word _reset_handler
_undefined_instruction_vector:  .word _undefined_instruction_handler
_software_interrupt_vector:     .word _software_interrupt_handler
_prefetch_abort_vector:         .word prefetch_abort_handler
_data_abort_vector:             .word _data_abort_handler
_unused_vector:                 .word _reset_handler
_interrupt_vector:              .word interrupt_handler
_fast_interrupt_vector:         .word fast_interrupt_handler

_enable_interrupts:
  // clear bit 0x80 of the cpsr reg
  mrs     r0, cpsr
  bic     r0, r0, #0x80
  msr     cpsr_c, r0
  cpsie   i
  mov     pc, lr

_reset_handler:
  mov r0, #0x8000
  mov r1, #0x0000

  // load vectors and constant addresses into registers, and store at 0x0000
  ldmia r0!, {r2, r3, r4, r5, r6, r7, r8, r9}
  stmia r1!, {r2, r3, r4, r5, r6, r7, r8, r9}
  ldmia r0!, {r2, r3, r4, r5, r6, r7, r8, r9}
  stmia r1!, {r2, r3, r4, r5, r6, r7, r8, r9}

  // set stack pointers
  mov r0, #(CPSR_MODE_FIQ | CPSR_IRQ_INHIBIT | CPSR_FIQ_INHIBIT )
  msr cpsr_c, r0
  ldr sp, =FIQ_STACK_ADDR

  mov r0, #(CPSR_MODE_IRQ | CPSR_IRQ_INHIBIT | CPSR_FIQ_INHIBIT )
  msr cpsr_c, r0
  ldr sp, =IRQ_STACK_ADDR

  mov r0, #(CPSR_MODE_SVR | CPSR_IRQ_INHIBIT | CPSR_FIQ_INHIBIT )
  msr cpsr_c, r0
  ldr sp, =SVR_STACK_ADDR
  cpsie   i

  mov r0, #(CPSR_MODE_ABORT | CPSR_IRQ_INHIBIT | CPSR_FIQ_INHIBIT )
  msr cpsr_c, r0
  ldr sp, =ABORT_STACK_ADDR
  cpsie   i

  mov r0, #(CPSR_MODE_HYPERVISOR | CPSR_IRQ_INHIBIT | CPSR_FIQ_INHIBIT )
  msr cpsr_c, r0
  ldr sp, =HYPERVISOR_STACK_ADDR
  cpsie   i

  mov r0, #(CPSR_MODE_UNDEFINED | CPSR_IRQ_INHIBIT | CPSR_FIQ_INHIBIT )
  msr cpsr_c, r0
  ldr sp, =UNDEFINED_STACK_ADDR
  cpsie   i

  mov r0, #(CPSR_MODE_SYSTEM | CPSR_IRQ_INHIBIT | CPSR_FIQ_INHIBIT )
  msr cpsr_c, r0
  ldr sp, =SYSTEM_STACK_ADDR
  cpsie   i

  // r1 = Access Control Register
  mrc p15, #0, r1, c1, c0, #2
  // enable full access for p10,11
  orr r1, r1, #(0xf << 20)
  // ccess Control Register = r1
  mcr p15, #0, r1, c1, c0, #2
  mov r1, #0
  // flush prefetch buffer because of FMXR below
  mcr p15, #0, r1, c7, c5, #4
  // and CP 10 & 11 were only just enabled
  // Enable VFP itself
  mov r0,#0x40000000
  // FPEXC = r0
  fmxr fpexc, r0

  // set monitor debug mode bit in DCSR
  mrc p14, #0, r0, c0, c1, #0
  orr r0, r0, #(0x1 << DSCR_MONITOR_DEBUG_ENABLE)
  and r0, r0, #(~(0x1 << DSCR_MODE_SELECT))
  mcr p14, #0, r0, c0, c1, #0
  mrc p14, #0, r0, c0, c1, #0

  bl _enable_interrupts
  b kernel_init

_inf_loop:
  b _inf_loop

