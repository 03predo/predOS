#include <stdint.h>
#include "bcm2835.h"
#include "emmc.h"
#include "sys_log.h"
#include "gpio.h"

typedef union {
  struct {
    uint32_t reserved0 : 1;
    uint32_t data_width_4bit_enable : 1;
    uint32_t reserved1 : 2;
    uint32_t data_width_8bit_enable : 1;
    uint32_t reserved2 : 10;
    uint32_t gap_stop : 1;
    uint32_t gap_restart : 1;
    uint32_t read_wait_enable : 1;
    uint32_t gap_interrupt_enable : 1;
    uint32_t spi_mode : 1;
    uint32_t boot_mode_access : 1;
    uint32_t alternate_boot_mode_access : 1;
    uint32_t reserved3 : 9;
  } fields;
  uint32_t raw;
}emmc_control0_t;

typedef union {
  struct {
    uint32_t clk_internal_clock_enable : 1;
    uint32_t clk_stable : 1;
    uint32_t clk_enable : 1;
    uint32_t reserved0 : 2;
    uint32_t clk_generation_select : 1;
    uint32_t clk_freq_msb : 2;
    uint32_t clk_freq_lsb : 8;
    uint32_t data_timeout_unit_exponent : 4;
    uint32_t reserved1 : 4;
    uint32_t reset_host_circuit : 1;
    uint32_t reset_command_circuit : 1;
    uint32_t reset_data_circuit : 1;
    uint32_t reserved2 : 4;
  } fields;
  uint32_t raw;
}emmc_control1_t;

typedef union {
  struct {  
    uint32_t auto_command_not_executed : 1;
    uint32_t auto_command_timeout : 1;
    uint32_t auto_command_crc_error : 1;
    uint32_t auto_command_end_bit_error : 1;
    uint32_t auto_command_index_error : 1;
    uint32_t reserved0 : 2;
    uint32_t auto_command12_error : 1;
    uint32_t reserved1 : 8;
    uint32_t uhs_mode : 3;
    uint32_t reserved2 : 2;
    uint32_t tune_on : 1;
    uint32_t tuned : 1;
    uint32_t reserved3 : 8;
  } fields;
  uint32_t raw;
}emmc_control2_t;

typedef union {
  struct {
    uint16_t command_response_type : 2;
    uint16_t reserved0 : 1;
    uint16_t response_crc_check_enable : 1;
    uint16_t response_index_check_enable : 1;
    uint16_t command_is_data_transfer : 1;
    uint16_t command_type : 2;
    uint16_t command_index : 6;
    uint16_t reserved1 : 2;
  } fields;
  uint16_t raw;
}emmc_command_t;

typedef union {
  struct {
    uint16_t reserved0 : 1;
    uint16_t block_count_enable : 1;
    uint16_t auto_command_enable : 2;
    uint16_t data_transfer_direction : 1;
    uint16_t multi_block_transfer : 1;
    uint16_t reserved1 : 10;
  } fields;
  uint16_t raw;
}emmc_transfer_mode_t;

status_t emmc_init(void){
  SYS_LOG("reseting host circuit");
  uint32_t c1 = EMMC->CONTROL1;
  // reset host circuit
  c1 |= (1 << 24);
  // disable clock
  c1 &= ~(1 << 2);
  c1 &= ~(1 << 0);
  EMMC->CONTROL1 = c1;
  sys_timer_sleep(1000000);
  if((EMMC->CONTROL1 & (1 << 24)) != 0){
    SYS_LOG("failed to reset properly"); 
    return STATUS_ERR;
  }
  SYS_LOG("reset successful");

  if((EMMC->STATUS & (1 << 16)) == 0){
    SYS_LOG("no card insterted");
  }

  SYS_LOG("status: %#x", EMMC->STATUS);

  EMMC->CONTROL2 = 0;

  c1 = EMMC->CONTROL1;
  c1 |= 1; // enable internal clock
  c1 |= (0x80 << 8); // set clock divder to 1/(2*128) to get close to 400kHz id frequency
  c1 |= (7 << 16); // timeout unit exponent
  EMMC->CONTROL1 = c1;

  sys_timer_sleep(1000000);
  if((EMMC->CONTROL1 & 0x2) == 0){
    SYS_LOG("clock failed to stabilize");
  }
  SYS_LOG("clock stabilized");

  EMMC->CONTROL1 |= 0x4; // enable SD clock

  sys_timer_sleep(2000);

  EMMC->INTERRUPT_EN = 0;
  EMMC->INTERRUPT |= 0xffffffff;
  EMMC->INTERRUPT_MASK |= 0xffffffff;

  sys_timer_sleep(2000);

  EMMC->ARGUMENT1 = 0;
  EMMC->BLOCK_SIZE_COUNT = 0x200;
  emmc_command_t cmd = {0};
  cmd.fields.command_index = 0;
  cmd.fields.command_response_type = 0b00;

  emmc_transfer_mode_t tm = {0};
  tm.fields.data_transfer_direction = 0;
  EMMC->INTERRUPT |= 0xffffffff;
  sys_timer_sleep(2000);
  SYS_LOG("int: %#x, resp: %#x, status: %#x, cmdtm: %#x", EMMC->INTERRUPT, EMMC->RESPONSE0, EMMC->STATUS, (cmd.raw << 16) + tm.raw);
  EMMC->COMMAND_TRANSFER_MODE = (cmd.raw << 16) + tm.raw;
  sys_timer_sleep(1000000);
  SYS_LOG("int: %#x, resp: %#x, status: %#x", EMMC->INTERRUPT, EMMC->RESPONSE0, EMMC->STATUS);

  EMMC->INTERRUPT |= 0xffffffff;
  EMMC->ARGUMENT1 = 0x1aa;
  cmd.fields.command_index = 8;
  cmd.fields.command_response_type = 0b10;
  cmd.fields.response_crc_check_enable = 1;
  tm.fields.data_transfer_direction = 0;
  sys_timer_sleep(2000);
  EMMC->COMMAND_TRANSFER_MODE = (cmd.raw << 16) + tm.raw;
  sys_timer_sleep(1000000);
  SYS_LOG("int: %#x, cmdtm: %#x, resp0: %#x, resp1: %#x, resp2: %#x, resp3: %#x", EMMC->INTERRUPT, (cmd.raw << 16) + tm.raw, EMMC->RESPONSE0, EMMC->RESPONSE1, EMMC->RESPONSE2, EMMC->RESPONSE3);

  EMMC->ARGUMENT1 = 0x0;
  EMMC->INTERRUPT |= 0xffffffff;
  cmd.fields.command_index = 55;
  cmd.fields.command_response_type = 0b10;
  cmd.fields.response_crc_check_enable = 1;
  tm.fields.data_transfer_direction = 0;
  sys_timer_sleep(2000);
  EMMC->COMMAND_TRANSFER_MODE = (cmd.raw << 16) + tm.raw;
  sys_timer_sleep(1000000);
  SYS_LOG("int: %#x, cmdtm: %#x, resp0: %#x, resp1: %#x, resp2: %#x, resp3: %#x", EMMC->INTERRUPT, (cmd.raw << 16) + tm.raw, EMMC->RESPONSE0, EMMC->RESPONSE1, EMMC->RESPONSE2, EMMC->RESPONSE3);

  EMMC->INTERRUPT |= 0xffffffff;
  cmd.fields.command_index = 41;
  cmd.fields.command_response_type = 0b10;
  cmd.fields.response_crc_check_enable = 0;
  tm.fields.data_transfer_direction = 0;
  sys_timer_sleep(2000);
  EMMC->COMMAND_TRANSFER_MODE = (cmd.raw << 16) + tm.raw;
  sys_timer_sleep(1000000);
  SYS_LOG("int: %#x, cmdtm: %#x, resp0: %#x, resp1: %#x, resp2: %#x, resp3: %#x", EMMC->INTERRUPT, (cmd.raw << 16) + tm.raw, EMMC->RESPONSE0, EMMC->RESPONSE1, EMMC->RESPONSE2, EMMC->RESPONSE3);
for(int i = 0; i < 10; ++i){
  EMMC->ARGUMENT1 = 0x0;
  EMMC->INTERRUPT |= 0xffffffff;
  cmd.fields.command_index = 55;
  cmd.fields.command_response_type = 0b10;
  cmd.fields.response_crc_check_enable = 1;
  tm.fields.data_transfer_direction = 0;
  sys_timer_sleep(2000);
  EMMC->COMMAND_TRANSFER_MODE = (cmd.raw << 16) + tm.raw;
  sys_timer_sleep(1000000);
  SYS_LOG("int: %#x, cmdtm: %#x, resp0: %#x, resp1: %#x, resp2: %#x, resp3: %#x", EMMC->INTERRUPT, (cmd.raw << 16) + tm.raw, EMMC->RESPONSE0, EMMC->RESPONSE1, EMMC->RESPONSE2, EMMC->RESPONSE3);
  
  EMMC->INTERRUPT |= 0xffffffff;
  EMMC->ARGUMENT1 = 0x00ff8000 | (1 << 30);
  cmd.fields.command_index = 41;
  cmd.fields.command_response_type = 0b10;
  cmd.fields.response_crc_check_enable = 0;
  tm.fields.data_transfer_direction = 0;
  sys_timer_sleep(2000);
  EMMC->COMMAND_TRANSFER_MODE = (cmd.raw << 16) + tm.raw;
  sys_timer_sleep(1000000);
  SYS_LOG("int: %#x, cmdtm: %#x, resp0: %#x, resp1: %#x, resp2: %#x, resp3: %#x", EMMC->INTERRUPT, (cmd.raw << 16) + tm.raw, EMMC->RESPONSE0, EMMC->RESPONSE1, EMMC->RESPONSE2, EMMC->RESPONSE3);
  if(EMMC->RESPONSE0 & (1 << 31)) break;
}

  EMMC->INTERRUPT |= 0xffffffff;
  EMMC->ARGUMENT1 = 0;
  cmd.fields.command_index = 2;
  cmd.fields.command_response_type = 0b01;
  cmd.fields.response_crc_check_enable = 1;
  tm.fields.data_transfer_direction = 0;
  sys_timer_sleep(2000);
  EMMC->COMMAND_TRANSFER_MODE = (cmd.raw << 16) + tm.raw;
  sys_timer_sleep(1000000);
  SYS_LOG("int: %#8x, cmdtm: %#8x, resp0: %#8x, resp1: %#8x, resp2: %#8x, resp3: %#8x", EMMC->INTERRUPT, (cmd.raw << 16) + tm.raw, EMMC->RESPONSE0, EMMC->RESPONSE1, EMMC->RESPONSE2, EMMC->RESPONSE3);
  // card initialization and identification process p. 21 physical layer simplified spec
    return STATUS_OK;
}

