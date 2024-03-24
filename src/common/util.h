#pragma once

#define GET_LR(lr) asm inline ("mov %0, lr" : "=r" (lr) : : )
#define GET_SP(sp) asm inline ("mov %0, sp" : "=r" (sp) : : )
#define GET_CPSR(cpsr) asm inline ("mrs %0, cpsr" : "=r" (cpsr) : : )
