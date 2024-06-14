#pragma once

#define GET_LR(lr) asm inline ("mov %0, lr" : "=r" (lr) : : )
#define GET_SP(sp) asm inline ("mov %0, sp" : "=r" (sp) : : )
#define GET_CPSR(cpsr) asm inline ("mrs %0, cpsr" : "=r" (cpsr) : : )
#define GET_TTBR0(ttbr0) asm inline("mrc p15, #0, %0, c2, c0, #0" : "=r" (ttbr0) : : )
#define SET_TTBR0(ttbr0) asm inline("mcr p15, #0, %0, c2, c0, #0" : "=r" (ttbr0) : : )
#define GET_TTBR1(ttbr1) asm inline("mrc p15, #0, %0, c2, c0, #1" : "=r" (ttbr1) : : )
#define SET_TTBR1(ttbr1) asm inline("mcr p15, #0, %0, c2, c0, #1" : "=r" (ttbr1) : : )
