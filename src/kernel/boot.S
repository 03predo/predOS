.section ".text.boot"

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
_undefined_instruction_vector:  .word undefined_instruction_handler
_software_interrupt_vector:     .word software_interrupt_handler
_prefetch_abort_vector:         .word prefetch_abort_handler
_data_abort_vector:             .word data_abort_handler
_unused_vector:                 .word _reset_handler
_interrupt_vector:              .word interrupt_handler
_fast_interrupt_vector:         .word fast_interrupt_handler

_reset_handler:
  mov r0, #0x8000
  mov r1, #0x0000
  ldmia r0!, {r2, r3, r4, r5, r6, r7, r8, r9}
  stmia r1!, {r2, r3, r4, r5, r6, r7, r8, r9}
  ldmia r0!, {r2, r3, r4, r5, r6, r7, r8, r9}
  stmia r1!, {r2, r3, r4, r5, r6, r7, r8, r9}

  ldr sp, =0x8000

  b kernel_init

_inf_loop:
  b _inf_loop
