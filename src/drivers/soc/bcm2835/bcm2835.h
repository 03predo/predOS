#pragma once

#include <stdint.h>

#define PERIPHERAL_BASE 0x20000000UL

/*
 * GPIO
 */

#define GPIO_OFFSET 0x200000UL

typedef struct {
  uint32_t SEL[6];
  uint32_t res0;
  uint32_t SET[2];
  uint32_t res1;
  uint32_t CLR[2];
  uint32_t res2;
  uint32_t LEV[2];
  uint32_t res3;
  uint32_t EDS[2]; // event detect status
  uint32_t res4;
  uint32_t REN[2]; // rising edge detect enable
  uint32_t res5;
  uint32_t FEN[2];
  uint32_t res6;
  uint32_t HEN[2];
  uint32_t res7;
  uint32_t LEN[2];
  uint32_t res8;
  uint32_t AREN[2];
  uint32_t res9;
  uint32_t AFEN[2];
  uint32_t res10;
  uint32_t PUD;
  uint32_t PUDCLK[2];
}gpio_t;

extern gpio_t* GPIO;

/*
 * AUXILIARIES
 */

#define AUX_OFFSET 0x215000UL

typedef struct {
  uint32_t IRQ;
  uint32_t ENABLES;
  uint32_t MU_IO;
  uint32_t MU_IER;
  uint32_t MU_IIR;
  uint32_t MU_LCR;
  uint32_t MU_MCR;
  uint32_t MU_LSR;
  uint32_t MU_MSR;
  uint32_t MU_SCRATCH;
  uint32_t MU_CNTL;
  uint32_t MU_STAT;
  uint32_t MU_BAUD;
}aux_t;

extern aux_t* AUX;

/*
 * TIMER
 */

#define SYS_TIMER_OFFSET 0x3000UL

typedef struct {
  uint32_t CS;
  uint32_t CLO;
  uint32_t CHI;
  uint32_t C0;
  uint32_t C1;
  uint32_t C2;
  uint32_t C3;
} sys_timer_t;

extern sys_timer_t* SYS_TIMER;
