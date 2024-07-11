.section ".text.boot"

#include "arm_defines.S"

.global _mmu_enable
.global _mmu_disable
.global _mmu_invalidate_tlb

_mmu_enable:
  // set TTBR0 to r0
  mcr p15, #0, r0, c2, c0, #0

  // set Domain Access Control
  mrc p15, #0, r0, c3, c0, #0 
  mov r0, #0xffffffff
  mcr p15, #0, r0, c3, c0, #0

  // invalidate data and instruction cache
  mov r0, #0x0
  mcr p15, #0, r0, c7, c7, #0
  mcr p15, #0, r0, c8, c7, 0
  mcr p15, #0, r0, c7, c10, 4

  // disable cache and subpages
  mrc p15, #0, r0, c1, c0, #0 
  and r0, r0, #(~(SUBPAGES_ENABLE))
  and r0, r0, #(~(INSTRUCTION_CACHE_ENABLE))
  and r0, r0, #(~(DATA_CACHE_ENABLE))
  orr r0, r0, #(MMU_ENABLE)
  mcr p15, #0, r0, c1, c0, #0

  bx lr

_mmu_disable:
  // enable the MMU
  mrc p15, #0, r0, c1, c0, #0 
  and r0, r0, #(~(MMU_ENABLE))
  mcr p15, #0, r0, c1, c0, #0
  bx lr

.section ".text"

_mmu_invalidate_tlb:
  mov r0, #0
  mcr p15, #0, r0, c8, c7, #0
  bx lr
