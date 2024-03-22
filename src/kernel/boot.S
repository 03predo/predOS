.section ".text.boot"

.equ    CPSR_MODE_USER,         0x10
.equ    CPSR_MODE_FIQ,          0x11
.equ    CPSR_MODE_IRQ,          0x12
.equ    CPSR_MODE_SVR,          0x13
.equ    CPSR_MODE_ABORT,        0x17
.equ    CPSR_MODE_HYPERVISOR,   0x1A
.equ    CPSR_MODE_UNDEFINED,    0x1B
.equ    CPSR_MODE_SYSTEM,       0x1F
.equ    CPSR_MODE_MASK,         0x1F

.equ    CPSR_IRQ_INHIBIT,       0x80
.equ    CPSR_FIQ_INHIBIT,       0x40
.equ    CPSR_THUMB,             0x20

.global _start
.global kernel_init
.global _enable_interrupts

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
  // load vectors and constant addresses into registers, and store at 0x0000
  ldmia r0!, {r2, r3, r4, r5, r6, r7, r8, r9}
  stmia r1!, {r2, r3, r4, r5, r6, r7, r8, r9}
  ldmia r0!, {r2, r3, r4, r5, r6, r7, r8, r9}
  stmia r1!, {r2, r3, r4, r5, r6, r7, r8, r9}

  mov r0, #(CPSR_MODE_IRQ | CPSR_IRQ_INHIBIT | CPSR_FIQ_INHIBIT )
  msr cpsr_c, r0
  ldr sp, =0x7000


  // set stack pointer to below text and data segment
  mov r0, #(CPSR_MODE_SVR | CPSR_IRQ_INHIBIT | CPSR_FIQ_INHIBIT )
  msr cpsr_c, r0
  ldr sp, =0x8000


  // r1 = Access Control Register
  MRC p15, #0, r1, c1, c0, #2
  // enable full access for p10,11
  ORR r1, r1, #(0xf << 20)
  // ccess Control Register = r1
  MCR p15, #0, r1, c1, c0, #2
  MOV r1, #0
  // flush prefetch buffer because of FMXR below
  MCR p15, #0, r1, c7, c5, #4
  // and CP 10 & 11 were only just enabled
  // Enable VFP itself
  MOV r0,#0x40000000
  // FPEXC = r0
  FMXR FPEXC, r0

  b kernel_init

_enable_interrupts:
  // clear bit 0x80 of the cpsr reg
  mrs     r0, cpsr
  bic     r0, r0, #0x80
  msr     cpsr_c, r0
  // this command flushes changes to processor
  cpsie   i
  mov     pc, lr

_inf_loop:
  b _inf_loop
