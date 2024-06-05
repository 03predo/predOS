.section ".text"

#include "arm_defines.S"

.global _kernel_context_switch
.global _enable_interrupts

_kernel_context_switch:
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

_enable_interrupts:
  // clear bit 0x80 of the cpsr reg
  mrs     r0, cpsr
  bic     r0, r0, #0x80
  msr     cpsr_c, r0
  cpsie   i
  mov     pc, lr



