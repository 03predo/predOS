.section ".text"

#include "arm_defines.S"

.global _enable_mmu

_enable_mmu:
// set TTBR0 to r0
  mcr p15, #0, r2, c0, #0
