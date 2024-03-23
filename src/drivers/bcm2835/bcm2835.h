#pragma once

#include <stdint.h>

#define PERIPHERAL_BASE 0x20000000UL

/*
 * GPIO
 */

#define GPIO_OFFSET 0x200000UL

typedef struct {
  volatile uint32_t SEL[6];
  volatile uint32_t res0;
  volatile uint32_t SET[2];
  volatile uint32_t res1;
  volatile uint32_t CLR[2];
  volatile uint32_t res2;
  volatile uint32_t LEV[2];
  volatile uint32_t res3;
  volatile uint32_t EDS[2]; // event detect status
  volatile uint32_t res4;
  volatile uint32_t REN[2]; // rising edge detect enable
  volatile uint32_t res5;
  volatile uint32_t FEN[2];
  volatile uint32_t res6;
  volatile uint32_t HEN[2];
  volatile uint32_t res7;
  volatile uint32_t LEN[2];
  volatile uint32_t res8;
  volatile uint32_t AREN[2];
  volatile uint32_t res9;
  volatile uint32_t AFEN[2];
  volatile uint32_t res10;
  volatile uint32_t PUD;
  volatile uint32_t PUDCLK[2];
}gpio_t;

extern gpio_t* GPIO;

/*
 * AUXILIARIES
 */

#define AUX_OFFSET 0x215000UL

typedef struct {
  volatile uint32_t IRQ;
  volatile uint32_t ENABLES;

  volatile uint32_t reserved1[((0x40 - 0x04) / 4) - 1];

  volatile uint32_t MINI_UART_IO;
  volatile uint32_t MINI_UART_IRQ_ENABLE;
  volatile uint32_t MINI_UART_IRQ_STATUS;
  volatile uint32_t MINI_UART_LINE_CONTROL;
  volatile uint32_t MINI_UART_MODEM_CONTROL;
  volatile uint32_t MINI_UART_LINE_STATUS;
  volatile uint32_t MINI_UART_MODEM_STATUS;
  volatile uint32_t MINI_UART_SCRATCH;
  volatile uint32_t MINI_UART_CONTROL;
  volatile uint32_t MINI_UART_STATUS;
  volatile uint32_t MINI_UART_BAUDRATE;
}aux_t;

extern aux_t* AUX;

/* AUX reg field defines */

#define AUX_IRQ_SPI2      ( 0b1U << 2);
#define AUX_IRQ_SPI1      ( 0b1U << 1);
#define AUX_IRQ_MINIUART  ( 0b1U << 0);

/*
 * SYS TIMER
 */

#define SYS_TIMER_OFFSET 0x3000UL

typedef struct {
  volatile uint32_t CS;
  volatile uint32_t CLO;
  volatile uint32_t CHI;
  volatile uint32_t C0;
  volatile uint32_t C1;
  volatile uint32_t C2;
  volatile uint32_t C3;
} sys_timer_t;

extern sys_timer_t* SYS_TIMER;

/*
 * ARM TIMER
 */

#define ARM_TIMER_OFFSET 0xB400UL

typedef struct {
  volatile uint32_t LOAD;
  volatile uint32_t VALUE;
  volatile uint32_t CONTROL;
  volatile uint32_t IRQ_CLEAR;
  volatile uint32_t RAW_IRQ;
  volatile uint32_t MASKED_IRQ;
  volatile uint32_t RELOAD;
} arm_timer_t;

extern arm_timer_t* ARM_TIMER;

/*
 * INTERRUPT CONTROLLER
 */

#define INTERRUPT_CONTROLLER_OFFSET 0xB200

typedef struct {
  volatile uint32_t IRQ_BASIC_PENDING;
  volatile uint32_t IRQ_PENDING1;
  volatile uint32_t IRQ_PENDING2;
  volatile uint32_t FIQ_CONTROL;
  volatile uint32_t ENABLE_IRQ1;
  volatile uint32_t ENABLE_IRQ2;
  volatile uint32_t ENABLE_BASIC_IRQ;
  volatile uint32_t DISABLE_IRQ1;
  volatile uint32_t DISABLE_IRQ2;
  volatile uint32_t DISABLE_BASIC_IRQ;
} irq_controller_t;

extern irq_controller_t* IRQ_CONTROLLER;
