#pragma once

#include <stdint.h>

#define SYSFREQ (250000000UL)
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

/* AUX reg field offset */

#define AUX_IRQ_SPI2      ( 0b1U << 2 );
#define AUX_IRQ_SPI1      ( 0b1U << 1 );
#define AUX_IRQ_MINI_UART  ( 0b1U << 0 );

#define AUX_ENABLE_SPI2      ( 0b1U << 2 );
#define AUX_ENABLE_SPI1      ( 0b1U << 1 );
#define AUX_ENABLE_MINI_UART  ( 0b1U << 0 );

#define AUX_MINI_UART_IRQ_ENABLE_TRANSMIT ( 0b1U << 1 )
#define AUX_MINI_UART_IRQ_ENABLE_RECEIVE  ( 0b1U << 0 )


#define AUX_MINI_UART_IRQ_STATUS_CLEAR_FIFO   ( 0b11U  << 1 )
#define AUX_MINI_UART_IRQ_STATUS_IRQ_PENDING  ( 0b1U  << 0 )
#define AUX_MINI_UART_IRQ_STATUS_TRANSMIT_IRQ ( 0b1U  << 1 )
#define AUX_MINI_UART_IRQ_STATUS_RECEIVE_IRQ  ( 0b1U  << 2 )

#define AUX_MINI_UART_LINE_CONTROL_DLAB       ( 0b1U  << 7 )
#define AUX_MINI_UART_LINE_CONTROL_BREAK      ( 0b1U  << 6 )
#define AUX_MINI_UART_LINE_CONTROL_8BIT       ( 0b11U << 0 )

#define AUX_MINI_UART_MODEM_CONTROL_RTS ( 0b1U << 1 )

#define AUX_MINI_UART_LINE_STATUS_TX_IDLE     ( 0b1U << 6 )
#define AUX_MINI_UART_LINE_STATUS_TX_EMPTY    ( 0b1U << 5 )
#define AUX_MINI_UART_LINE_STATUS_RX_OVERRUN  ( 0b1U << 5 )
#define AUX_MINI_UART_LINE_STATUS_DATA_READY  ( 0b1U << 0 )

#define AUX_MINI_UART_MODEM_STATUS_CTS ( 0b1U << 5 )

#define AUX_MINI_UART_CONTROL_TX_ENABLE ( 0b1 << 1 )
#define AUX_MINI_UART_CONTROL_RX_ENABLE ( 0b1 << 0 )

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

/*
 * POWER MANAGEMENT
 * 
 * No documentation online about these registers, only information is form these sites
 * https://ultibo.org/wiki/Unit_BCM2835
 * https://github.com/torvalds/linux/blob/master/drivers/watchdog/bcm2835_wdt.c#L89
 * 
 */

#define PM_OFFSET     0x100000UL
#define PM_RSTC_OFFSET 0x1cUL
#define PM_WDOG_OFFSET 0x24UL

#define PM_PASSWORD 0x5a000000UL
#define PM_FULL_RESET 0x00000020UL
#define PM_WRCFG_CLR 0xffffffcfUL

extern uint32_t* RESET_CONTROLLER;
extern uint32_t* WATCHDOG;

/*
 * External Mass Media Controller
 */

#define EMMC_OFFSET 0x300000

typedef struct {
  uint32_t ARGUMENT2;
  uint32_t BLOCK_SIZE_COUNT;
  uint32_t ARGUMENT1;
  uint32_t COMMAND_TRANSFER_MODE;
  uint32_t RESPONSE0;
  uint32_t RESPONSE1;
  uint32_t RESPONSE2;
  uint32_t RESPONSE3;
  uint32_t DATA;
  uint32_t STATUS;
  uint32_t CONTROL0;
  uint32_t CONTROL1;
  uint32_t INTERRUPT;
  uint32_t INTERRUPT_MASK;
  uint32_t INTERRUPT_EN;
  uint32_t CONTROL2; // 0x3c
  uint32_t CAPABILITIES0; // 0x40
  uint32_t CAPABILITIES1; // 0x44
  uint32_t reserved0; // 0x48
  uint32_t reserved1; // 0x4c
  uint32_t FORCE_INTERRUPT;
  uint32_t BOOT_TIMEOUT;
  uint32_t DEBUG_CONFIG;
  uint32_t EXTENSION_FIFO_CONFIG;
  uint32_t EXTENSION_FIFO_ENABLE;
  uint32_t TUNE_STEP;
  uint32_t TUNE_STEPS_SDR;
  uint32_t TUNE_STEPS_DDR;
  uint32_t SPI_INTERRUPT_SELECT;
  uint32_t SLOT_INTERRUPT_STATUS_VERSION;
}emmc_t;

extern emmc_t* EMMC;

/*
 * DMA Controller
 */

#define DMA_CONTROLLER_OFFSET 0x7000
#define DMA15_OFFSET 0xE05000
#define DMA_CHANNEL_NUM 16

typedef struct {
  uint32_t CONTROL_STATUS;
  uint32_t CONTROL_BLOCK_ADDRESS;
  uint32_t reserved[6];
  uint32_t DEBUG;
} dma_channel_t;

extern dma_channel_t* DMA_CHANNEL[DMA_CHANNEL_NUM];

