#include <stdint.h>
#include "bcm2835.h"
#include "emmc.h"
#include "sys_log.h"
#include "gpio.h"
#include "emmc_def.h"

#define BASE_CLOCK_FREQ 100000000 // assume base clock is 100MHz
#define IDENTIFICATION_MODE_CLOCK_FREQ 400000 // 400kHz
#define DATA_TRANSFER_MODE_CLOCK_FREQ 25000000 // 25MHz

typedef struct {
  uint16_t relative_card_address;
} emmc_state_t;

static emmc_state_t emmc_state;

status_t emmc_command_fields(emmc_command_index_t command_index, emmc_command_t* command,
                             emmc_transfer_mode_t* transfer_mode){
  switch(command_index){
    case GO_IDLE_STATE:
      command->fields.command_index = GO_IDLE_STATE;
      command->fields.command_response_type = RESPONSE_NONE;
      transfer_mode->fields.data_transfer_direction = HOST_TO_CARD;
      break;
    case ALL_SEND_CID:
      command->fields.command_index = ALL_SEND_CID;
      command->fields.command_response_type = RESPONSE_136_BIT;
      command->fields.response_crc_check_enable = 1;
      transfer_mode->fields.data_transfer_direction = HOST_TO_CARD;
      break;
    case SEND_RELATIVE_ADDR:
      command->fields.command_index = SEND_RELATIVE_ADDR;
      command->fields.command_response_type = RESPONSE_48_BIT;
      transfer_mode->fields.data_transfer_direction = HOST_TO_CARD;
      break;
    case SET_DSR:
      command->fields.command_index = SET_DSR;
      command->fields.command_response_type = RESPONSE_NONE;
      transfer_mode->fields.data_transfer_direction = HOST_TO_CARD;
      break;
    case SELECT_DESELECT_CARD:
      command->fields.command_index = SELECT_DESELECT_CARD;
      command->fields.command_response_type = RESPONSE_48_BIT_BUSY;
      command->fields.response_crc_check_enable = 1;
      transfer_mode->fields.data_transfer_direction = HOST_TO_CARD;
      break;
    case SEND_IF_COND:
      command->fields.command_index = SEND_IF_COND;
      command->fields.command_response_type = RESPONSE_48_BIT;
      transfer_mode->fields.data_transfer_direction = HOST_TO_CARD;
      break;
    case SEND_CSD:
      command->fields.command_index = SEND_CSD;
      command->fields.command_response_type = RESPONSE_136_BIT;
      command->fields.response_crc_check_enable = 1;
      transfer_mode->fields.data_transfer_direction = HOST_TO_CARD;
      break;
    case READ_SINGLE_BLOCK:
      command->fields.command_index = READ_SINGLE_BLOCK;
      command->fields.command_response_type = RESPONSE_48_BIT;
      command->fields.command_is_data_transfer = 1;
      transfer_mode->fields.data_transfer_direction = CARD_TO_HOST;
      break;
    case WRITE_SINGLE_BLOCK:
      command->fields.command_index = WRITE_SINGLE_BLOCK;
      command->fields.command_response_type = RESPONSE_48_BIT;
      command->fields.command_is_data_transfer = 1;
      transfer_mode->fields.data_transfer_direction = HOST_TO_CARD;
      break;
    case APP_CMD:
      command->fields.command_index = APP_CMD;
      command->fields.command_response_type = RESPONSE_48_BIT;
      transfer_mode->fields.data_transfer_direction = HOST_TO_CARD;
      break;
    default:
      command->raw = 0;
      transfer_mode->raw = 0;
      return STATUS_ERR;
  }
  return STATUS_OK;
  return STATUS_OK;
}

status_t emmc_app_command_fields(emmc_app_command_t app_command, emmc_command_t* command,
                             emmc_transfer_mode_t* transfer_mode){
  switch(app_command){
    case SD_SEND_OP_COND: 
      command->fields.command_index = SD_SEND_OP_COND;
      command->fields.command_response_type = RESPONSE_48_BIT;
      transfer_mode->fields.data_transfer_direction = HOST_TO_CARD;
      break;
    default:
      command->raw = 0;
      transfer_mode->raw = 0;
      return STATUS_ERR;
  }
  return STATUS_OK;
}

status_t emmc_reset_host_circuit(){
  emmc_control1_t c1;
  c1.raw = EMMC->CONTROL1;

  // reset host circuit and disable clock
  c1.fields.reset_host_circuit = 1;
  c1.fields.clk_enable = 0;
  c1.fields.clk_internal_clock_enable = 0;
  EMMC->CONTROL1 = c1.raw;

  sys_timer_sleep(1000000);

  //check to make sure reset occured
  c1.raw = EMMC->CONTROL1;
  if(c1.fields.reset_host_circuit != 0){
    SYS_LOG("failed to reset properly"); 
    return STATUS_ERR;
  }
  SYS_LOG("reset successful");

  return STATUS_OK;
}

status_t emmc_set_clock(uint32_t frequency_hz){

  // Clock Supply Sequence (Host Controller Spec 3.2.1)
  // enable internal clock and set divider such that frequency is 400kHz
  // on reset all cards are set to 400kHz frequence (Physical Layer Spec 4.2.1)
  
  if(frequency_hz > BASE_CLOCK_FREQ){
    SYS_LOG("invalid clock freq: %d", frequency_hz);
    return STATUS_ERR;
  }

  uint8_t divider = BASE_CLOCK_FREQ / frequency_hz;
  emmc_control1_t c1;
  c1.raw = EMMC->CONTROL1;
  c1.fields.clk_enable = 0;
  c1.fields.clk_internal_clock_enable = 1; 
  c1.fields.clk_freq_lsb = divider; 
  c1.fields.data_timeout_unit_exponent = 7; // command timeout
  EMMC->CONTROL1 = c1.raw;

  sys_timer_sleep(1000000);

  // check to make sure clock stabilized with new settings
  c1.raw = EMMC->CONTROL1;
  if(c1.fields.clk_stable == 0){
    SYS_LOG("clock failed to stabilize");
    return STATUS_ERR;
  }
  SYS_LOG("clock stabilized");

  // enable SD clock
  c1.fields.clk_enable = 1; 
  EMMC->CONTROL1 = c1.raw;

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

  response->response0 = EMMC->RESPONSE0;
  response->response1 = EMMC->RESPONSE1;
  response->response2 = EMMC->RESPONSE2;
  response->response3 = EMMC->RESPONSE3;
  return STATUS_OK;
}

status_t emmc_send_command(emmc_command_index_t command_index, emmc_argument_t argument, emmc_response_t* response){
  emmc_command_t cmd = {0};
  emmc_transfer_mode_t tm = {0};
  if(emmc_command_fields(command_index, &cmd, &tm) != STATUS_OK) return STATUS_ERR;
  if(emmc_issue_command(cmd, tm, argument, response) != STATUS_OK) return STATUS_ERR;

  return STATUS_OK;
}

status_t emmc_send_app_command(emmc_app_command_t app_command, emmc_argument_t argument, emmc_response_t* response){
  emmc_argument_t command_arg;
  command_arg.argument1 = 0;
  command_arg.argument2 = 0;
  if(emmc_send_command(APP_CMD, command_arg, response) != STATUS_OK) return STATUS_ERR;
 
  emmc_command_t cmd = {0};
  emmc_transfer_mode_t tm = {0};
  if(emmc_app_command_fields(app_command, &cmd, &tm) != STATUS_OK) return STATUS_ERR;
  if(emmc_issue_command(cmd, tm, argument, response) != STATUS_OK) return STATUS_ERR;

  return STATUS_OK;
}

status_t emmc_read_block(uint32_t block_address, emmc_block_t* block){
  emmc_response_t resp;
  emmc_argument_t arg;

  arg.argument1 = block_address;
  if(emmc_send_command(READ_SINGLE_BLOCK, arg, &resp) != STATUS_OK) return STATUS_ERR;

  emmc_interrupt_t interrupt = {0};
  interrupt.raw = EMMC->INTERRUPT;
  while(!interrupt.fields.read_ready) interrupt.raw = EMMC->INTERRUPT;
  uint32_t i = 0;
  while(!interrupt.fields.data_done){
    uint32_t read = EMMC->DATA;
    block->buf[i] = read & 0xff;
    block->buf[i+1] = (read & 0xff00) >> 8;
    block->buf[i+2] = (read & 0xff0000) >> 16;
    block->buf[i+3] = (read & 0xff000000) >> 24;
    interrupt.raw = EMMC->INTERRUPT;
    i += 4;
  }
  SYS_LOG("read complete, %d bytes read", i); 

  return STATUS_OK;
}

status_t emmc_write_block(uint32_t block_address, emmc_block_t* block){
  emmc_response_t resp;
  emmc_argument_t arg;

  arg.argument1 = block_address;
  if(emmc_send_command(WRITE_SINGLE_BLOCK, arg, &resp) != STATUS_OK) return STATUS_ERR;

  emmc_interrupt_t interrupt = {0};
  interrupt.raw = EMMC->INTERRUPT;
  while(!interrupt.fields.write_ready) interrupt.raw = EMMC->INTERRUPT;
  EMMC->INTERRUPT |= 0xffffffff;
  for(uint32_t i = 0; i < EMMC_BLOCK_SIZE; i += 4){
    EMMC->DATA = (block->buf[i] + (block->buf[i+1] << 8) + (block->buf[i+2] << 16) + (block->buf[i+3] << 24));
  }

  for(uint32_t i = 0; i < 100000; ++i){
    interrupt.raw = EMMC->INTERRUPT;
    if(interrupt.fields.data_done) break;
    sys_timer_sleep(5);
  }

  if(!interrupt.fields.data_done){
    SYS_LOG("data not done");
    return STATUS_ERR;
  }

  SYS_LOG("write complete");
  return STATUS_OK;
}

status_t emmc_init(void){
  
  if(emmc_reset_host_circuit() != STATUS_OK) return STATUS_ERR;

  // Stops interrupt handler being called
  EMMC->INTERRUPT_EN = 0;

  // Interrupts need to be cleared by writing 1 to register
  EMMC->INTERRUPT |= 0xffffffff;

  // Unmask all interrupts
  EMMC->INTERRUPT_MASK |= 0xffffffff;

  // This card detect and clock setup process is documented in
  // SD Host Controller Spec, Sec 3 (p. 92)
  emmc_status_t status;
  status.raw = EMMC->STATUS;
  if(status.fields.card_inserted == 0){
    SYS_LOG("no card insterted");
  }
 
  if(emmc_set_clock(IDENTIFICATION_MODE_CLOCK_FREQ) != STATUS_OK) return STATUS_ERR; 

  sys_timer_sleep(2000); 

  // Card Initialization Procedure, see Physical Layer Spec Sec 4
  
  // Send GO_IDLE_STATE to reset the card to idle state
  emmc_command_t cmd = {0};
  emmc_transfer_mode_t tm = {0};
  emmc_response_t resp = {0};
  emmc_argument_t arg = {0};
  if(emmc_send_command(GO_IDLE_STATE, arg, &resp) != STATUS_OK) return STATUS_ERR;

  // SEND_IF_COND to verify card is responding, should echo back the argument
  emmc_send_if_cond_arg_t send_if_cond_arg = {0};
  send_if_cond_arg.fields.check_pattern = 0xaa;
  send_if_cond_arg.fields.voltage_supplied = 0x1; // corresponds to 2.7-3.6v
  arg.argument1 = send_if_cond_arg.raw;
  if(emmc_send_command(SEND_IF_COND, arg, &resp) != STATUS_OK) return STATUS_ERR;
  if(resp.response0 != arg.argument1){
    SYS_LOG("SEND_IF_COND failed: arg1: %#x, resp0: %#x", arg.argument1, resp.response0);
    return STATUS_ERR;
  }
  
  // Card Initialization
  emmc_ocr_t ocr = {0};
  emmc_sd_send_op_cond_arg_t sd_send_op_cond_arg = {0};
  sd_send_op_cond_arg.fields.voltage_window = 0x1ff; // 2.6-3.9v;
  sd_send_op_cond_arg.fields.host_capacity_support = 1; // support SDHC and SDXC
  for(int i = 0; i < 10; ++i){
    // repeat command until power up is finished
    arg.argument1 = sd_send_op_cond_arg.raw;
    arg.argument2 = 0;
    if(emmc_send_app_command(SD_SEND_OP_COND, arg, &resp) != STATUS_OK) return STATUS_ERR;

    ocr.raw = resp.response0;
    if(ocr.fields.card_power_up_status) break;
  }

  if(!(ocr.fields.card_power_up_status)){
    SYS_LOG("card failed to finish power up");
    return STATUS_ERR;
  }

  if(ocr.fields.card_capacity_status) SYS_LOG("card is SDHC or SDXC");

  // get the card identification register
  arg.argument1 = 0;
  arg.argument2 = 0;
  if(emmc_send_command(ALL_SEND_CID, arg, &resp) != STATUS_OK) return STATUS_ERR;

  emmc_cid_t cid = {0};
  cid.raw[0] = resp.response0;
  cid.raw[1] = resp.response1;
  cid.raw[2] = resp.response2;
  cid.raw[3] = resp.response3;
  SYS_LOG("MID: %#x, OID: %c%c, PNM: %c%c%c%c%c", 
          cid.fields.manufacturer_id,
          cid.fields.oem_id[1], cid.fields.oem_id[0],
          cid.fields.product_name[4], cid.fields.product_name[3], cid.fields.product_name[2],
          cid.fields.product_name[1], cid.fields.product_name[0]);

  // get the relative card address (RCA)
  arg.argument1 = 0;
  arg.argument2 = 0;
  if(emmc_send_command(SEND_RELATIVE_ADDR, arg, &resp) != STATUS_OK) return STATUS_ERR;
 
  emmc_card_status_t card_status = {0};
  card_status.raw = resp.response0;  
  SYS_LOG("RCA: %#x, current_state: %#x, error: %#x", card_status.fields.relative_card_address, card_status.fields.current_state, card_status.fields.error);

  emmc_state.relative_card_address = card_status.fields.relative_card_address;

  // set clock to 25 MHz which is max freq for microSDXC
  if(emmc_set_clock(DATA_TRANSFER_MODE_CLOCK_FREQ) != STATUS_OK) return STATUS_ERR; 

  // Must be set to 512, see Host Controller Spec 1.7.2
  EMMC->BLOCK_SIZE_COUNT = 0x200;

  // put the card into transfer mode
  arg.argument1 = emmc_state.relative_card_address << 16;
  if(emmc_send_command(SELECT_DESELECT_CARD, arg, &resp) != STATUS_OK) return STATUS_ERR;
  return STATUS_OK;
}

