#include <stdint.h>
#include "bcm2835.h"
#include "emmc.h"
#include "sys_log.h"
#include "gpio.h"
#include "emmc_def.h"

status_t emmc_command_response_type(emmc_command_index_t command_index, emmc_response_type_t* response_type){
  switch(command_index){
    case GO_IDLE_STATE:
      *response_type = RESPONSE_NONE;      
      break;
    case ALL_SEND_CID:
      *response_type = RESPONSE_136_BIT;
      break;
    case SEND_RELATIVE_ADDR:
      *response_type = RESPONSE_48_BIT;
      break;
    case SET_DSR:
      *response_type = RESPONSE_NONE;
      break;
    case SELECT_DESELECT_CARD:
      *response_type = RESPONSE_48_BIT_BUSY;
      break;
    case SEND_IF_COND:
      *response_type = RESPONSE_48_BIT;
      break;
    case APP_CMD:
      *response_type = RESPONSE_48_BIT;
      break;
    default:
      *response_type = RESPONSE_NONE;
      return STATUS_ERR;
  }
  return STATUS_OK;
}

status_t emmc_command_data_direction(emmc_command_index_t command_index, emmc_data_direction_t* data_dir){
  switch(command_index){
    case GO_IDLE_STATE:
      *data_dir = HOST_TO_CARD;
      break;
    case ALL_SEND_CID:
      *data_dir = HOST_TO_CARD;
      break;
    case SEND_RELATIVE_ADDR:
      *data_dir = HOST_TO_CARD;
      break;
    case SET_DSR:
      *data_dir = HOST_TO_CARD;
      break;
    case SELECT_DESELECT_CARD:
      *data_dir = HOST_TO_CARD;
      break;
    case SEND_IF_COND:
      *data_dir = HOST_TO_CARD;
      break;
    case APP_CMD:
      *data_dir = HOST_TO_CARD;
      break;
    default:
      *data_dir = HOST_TO_CARD;
      return STATUS_ERR;
  }
  return STATUS_OK;
}

status_t emmc_app_command_response_type(emmc_app_command_t app_command, emmc_response_type_t* response_type){
  switch(app_command){
    case SD_SEND_OP_COND:
      *response_type = RESPONSE_48_BIT;
      break;
    default:
      *response_type = RESPONSE_NONE;
      return STATUS_ERR;
  }
  return STATUS_OK;
}

status_t emmc_app_command_data_direction(emmc_app_command_t app_command, emmc_data_direction_t* data_dir){
  switch(app_command){
    case SD_SEND_OP_COND:
      *data_dir = HOST_TO_CARD;
      break;
    default:
      *data_dir = HOST_TO_CARD;
      return STATUS_ERR;
  }
  return STATUS_OK;
}

status_t emmc_reset_host_circuit(){
  emmc_control1_t c1;
  c1.raw = EMMC->CONTROL1;
  // reset host circuit
  c1.fields.reset_host_circuit = 1;
  // disable clock
  c1.fields.clk_enable = 0;
  c1.fields.clk_internal_clock_enable = 0;
  EMMC->CONTROL1 = c1.raw;
  sys_timer_sleep(1000000);
  c1.raw = EMMC->CONTROL1;
  if(c1.fields.reset_host_circuit != 0){
    SYS_LOG("failed to reset properly"); 
    return STATUS_ERR;
  }
  SYS_LOG("reset successful");
  return STATUS_OK;
}

status_t emmc_enable_clock(){
  EMMC->CONTROL2 = 0;

  emmc_control1_t c1;
  c1.raw = EMMC->CONTROL1;
  c1.fields.clk_internal_clock_enable = 1; // enable internal clock
  c1.fields.clk_freq_lsb = 0x80; // set clock divder to 1/(2*128) to get close to 400kHz id frequency
  c1.fields.data_timeout_unit_exponent = 7; // timeout unit exponent
  EMMC->CONTROL1 = c1.raw;

  sys_timer_sleep(1000000);
  c1.raw = EMMC->CONTROL1;
  if(c1.fields.clk_stable == 0){
    SYS_LOG("clock failed to stabilize");
    return STATUS_ERR;
  }
  SYS_LOG("clock stabilized");

  c1.fields.clk_enable = 1; // enable SD clock
  EMMC->CONTROL1 = c1.raw;

  sys_timer_sleep(2000);
  return STATUS_OK;
}

status_t emmc_issue_command(emmc_command_t command, emmc_transfer_mode_t transfer_mode,
                            emmc_argument_t argument, emmc_response_t* response){
  EMMC->ARGUMENT1 = argument.argument1;
  EMMC->ARGUMENT2 = argument.argument2;

  EMMC->INTERRUPT |= 0xffffffff;
  sys_timer_sleep(2000);
  EMMC->COMMAND_TRANSFER_MODE = (command.raw << 16) + transfer_mode.raw;

  while(EMMC->INTERRUPT == 0);

  // this needs to be improved to account for diff commands (reads/writes)
  if(!(EMMC->INTERRUPT & 0x1)){
    SYS_LOG("int: %#8x", EMMC->INTERRUPT);
    return STATUS_ERR;
  }

  response->resp0 = EMMC->RESPONSE0;
  response->resp1 = EMMC->RESPONSE1;
  response->resp2 = EMMC->RESPONSE2;
  response->resp3 = EMMC->RESPONSE3;
  return STATUS_OK;
}

status_t emmc_send_command(emmc_command_index_t command_index, emmc_argument_t argument, emmc_response_t* response){
  emmc_response_type_t response_type;
  if(emmc_command_response_type(command_index, &response_type) != STATUS_OK){
    SYS_LOG("failed to get response type");
    return STATUS_ERR;
  }

  emmc_data_direction_t data_dir;
  if(emmc_command_data_direction(command_index, &data_dir) != STATUS_OK){
    SYS_LOG("failed to get data direction");
    return STATUS_ERR;
  }

  emmc_command_t cmd = {0};
  cmd.fields.command_index = command_index;
  cmd.fields.command_response_type = response_type;

  emmc_transfer_mode_t tm = {0};
  tm.fields.data_transfer_direction = data_dir;

  if(emmc_issue_command(cmd, tm, argument, response) != STATUS_OK) return STATUS_ERR;

  return STATUS_OK;
}

status_t emmc_send_app_command(emmc_app_command_t app_command, emmc_argument_t argument, emmc_response_t* response){
  emmc_argument_t command_arg;
  command_arg.argument1 = 0;
  command_arg.argument2 = 0;
  if(emmc_send_command(APP_CMD, command_arg, response) != STATUS_OK) return STATUS_ERR;


  emmc_response_type_t response_type;
  if(emmc_app_command_response_type(app_command, &response_type) != STATUS_OK){
    SYS_LOG("failed to get response type");
    return STATUS_ERR;
  }

  emmc_data_direction_t data_dir;
  if(emmc_app_command_data_direction(app_command, &data_dir) != STATUS_OK){
    SYS_LOG("failed to get data direction");
    return STATUS_ERR;
  }

  emmc_command_t cmd = {0};
  cmd.fields.command_index = app_command;
  cmd.fields.command_response_type = response_type;

  emmc_transfer_mode_t tm = {0};
  tm.fields.data_transfer_direction = data_dir;

  if(emmc_issue_command(cmd, tm, argument, response) != STATUS_OK) return STATUS_ERR;

  return STATUS_OK;
}

status_t emmc_init(void){
  if(emmc_reset_host_circuit() != STATUS_OK) return STATUS_ERR;

  emmc_status_t status;
  status.raw = EMMC->STATUS;
  if(status.fields.card_inserted == 0){
    SYS_LOG("no card insterted");
  }

  if(emmc_enable_clock() != STATUS_OK) return STATUS_ERR; 

  EMMC->INTERRUPT_EN = 0;
  EMMC->INTERRUPT |= 0xffffffff;
  EMMC->INTERRUPT_MASK |= 0xffffffff;
  EMMC->BLOCK_SIZE_COUNT = 0x200;

  sys_timer_sleep(2000);

  emmc_command_t cmd = {0};
  emmc_transfer_mode_t tm = {0};

  emmc_response_t resp;
  emmc_argument_t arg;
  arg.argument1 = 0;
  arg.argument2 = 0;
  if(emmc_send_command(GO_IDLE_STATE, arg, &resp) != STATUS_OK) return STATUS_ERR;


  arg.argument1 = 0x1aa;
  if(emmc_send_command(SEND_IF_COND, arg, &resp) != STATUS_OK) return STATUS_ERR;
  
  SYS_LOG("int: %#x, cmdtm: %#x, resp0: %#x, resp1: %#x, resp2: %#x, resp3: %#x", EMMC->INTERRUPT, (cmd.raw << 16) + tm.raw, EMMC->RESPONSE0, EMMC->RESPONSE1, EMMC->RESPONSE2, EMMC->RESPONSE3);

  arg.argument1 = 0;
  arg.argument2 = 0;
  if(emmc_send_app_command(SD_SEND_OP_COND, arg, &resp) != STATUS_OK) return STATUS_ERR;
/*
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
  */
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

