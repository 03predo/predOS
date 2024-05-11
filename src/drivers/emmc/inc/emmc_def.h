#include <stdint.h>

// SD Memory Card Bus Commands (p. 61 of physical layer spec)

typedef enum {
  GO_IDLE_STATE = 0,
  ALL_SEND_CID = 2,
  SEND_RELATIVE_ADDR = 3,
  SET_DSR = 4,
  SELECT_DESELECT_CARD = 7,
  SEND_IF_COND = 8
} emmc_command_index_t;

typedef enum {
  RESPONSE_NONE = 0,
  RESPONSE_136_BIT = 1,
  RESPONSE_48_BIT = 2,
  RESPONSE_48_BIT_BUSY = 3
} emmc_response_type_t;

typedef enum {
  HOST_TO_CARD = 0,
  CARD_TO_HOST = 1
} emmc_data_direction_t;

// Register Structures

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

typedef union {
  struct {
    uint32_t command_inhibit : 1;
    uint32_t data_inhibit : 1;
    uint32_t data_active : 1;
    uint32_t reserved0 : 5;
    uint32_t write_transfer : 1;
    uint32_t read_transfer : 1;
    uint32_t reserved1 : 6;
    uint32_t card_inserted : 1;
    uint32_t reserved2 : 3;
    uint32_t data_level0 : 4;
    uint32_t command_level : 1;
    uint32_t data_level1 : 4;
    uint32_t reserved3 : 3;
  } fields;
  uint32_t raw;
}emmc_status_t;

typedef struct {
   uint32_t resp0;
   uint32_t resp1;
   uint32_t resp2;
   uint32_t resp3;
}emmc_response_t;

typedef struct {
  uint32_t argument1;
  uint32_t argument2;
}emmc_argument_t;


