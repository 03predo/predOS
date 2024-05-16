#include <stdint.h>

/*
 * SD Memory Card Bus Commands (Physical Layer Spec 4.7)
 */

typedef enum {
  GO_IDLE_STATE = 0,
  ALL_SEND_CID = 2,
  SEND_RELATIVE_ADDR = 3,
  SET_DSR = 4,
  SELECT_DESELECT_CARD = 7,
  SEND_IF_COND = 8,
  SEND_CSD = 9,
  READ_SINGLE_BLOCK = 17,
  WRITE_SINGLE_BLOCK = 24,
  APP_CMD = 55
} emmc_command_index_t;

typedef enum {
  SD_SEND_OP_COND = 41,
} emmc_app_command_t;

/*
 * SD Card States (Physical Layer Table 4-36)
 */

typedef enum {
  IDLE = 0,
  READY,
  IDENTIFICATION,
  STANDBY,
  TRANSMIT,
  DATA,
  RECEIVE,
  PRG,
  DIS,
} emmc_card_state_t;

/*
 * Response and Argument Types (Physical Layer Spec 4.7 & 4.9)
 */

typedef union {
  struct {
    uint32_t check_pattern : 8;
    uint32_t voltage_supplied : 4; // see Physical Layer Spec 4.3.13
    uint32_t reserved0 : 20;
  } fields;
  uint32_t raw;
} emmc_send_if_cond_arg_t;

typedef union {
  struct {
    uint32_t reserved0 : 15;
    uint32_t voltage_window : 9;
    uint32_t request_1_8v : 1;
    uint32_t reserved1 : 3;
    uint32_t xpc : 1;
    uint32_t reserved2 : 1;
    uint32_t host_capacity_support : 1;
    uint32_t reserved3 : 1;
  } fields;
  uint32_t raw;
} emmc_sd_send_op_cond_arg_t;

// sent as response R1 and R1b
typedef union {
  struct{
    uint32_t reserved0 : 3;
    uint32_t authentication_error : 1;
    uint32_t reserved1 : 1;
    uint32_t app_cmd : 1;
    uint32_t reserved2 : 2;
    uint32_t ready_for_data : 1;
    uint32_t current_state : 4;
    uint32_t error : 1;
    uint32_t illegal_command : 1;
    uint32_t crc_error : 1;
    uint32_t relative_card_address : 16;
  } fields;
  uint32_t raw;
} emmc_card_status_t;

/*
 * SD Card Register Types (Physical Layer Spec Sec 5)
 */

// Operating Conditions Register
typedef union {
  struct {
    uint32_t reserved0 : 15;
    uint32_t voltage_window : 9;
    uint32_t switch_to_1_8v_accepted : 1;
    uint32_t reserved1 : 5;
    uint32_t card_capacity_status : 1;
    uint32_t card_power_up_status : 1; // bit is low when power up is not complete
  } fields;
  uint32_t raw;
} emmc_ocr_t; 

// Card Identification Register
typedef union {
  struct {
    uint8_t manufacturing_date[2];
    uint8_t product_serial_number[4];
    uint8_t product_revision;
    uint8_t product_name[5];
    uint8_t oem_id[2];
    uint8_t manufacturer_id;
    uint8_t reserved;
  } fields;
  uint32_t raw[4];
} emmc_cid_t;

// Card-Specific Data
typedef union {
  struct {
    uint32_t reserved0 : 2;
    uint32_t file_format : 2;
    uint32_t temp_write_protection : 1;
    uint32_t permanent_write_protection : 1;
    uint32_t copy_flag : 1;
    uint32_t file_format_group : 1;
    uint32_t reserved1 : 5;
    uint32_t write_block_partial : 1;
    uint32_t max_write_block_length : 4;
    uint32_t write_speed_factor : 3;
    uint32_t reserved2 : 2;
    uint32_t write_protect_group_enable : 1;
    uint32_t write_protect_group_size : 7;
    uint32_t erase_sector_size_lsb : 1;
    // 32
    uint32_t erase_sector_size_msb : 6;
    uint32_t erase_single_block_enable : 1;
    uint32_t device_size_multiplier : 3;
    uint32_t vdd_write_current_max : 3;
    uint32_t vdd_write_current_min : 3;
    uint32_t vdd_read_current_max : 3;
    uint32_t vdd_read_current_min : 3;
    uint32_t device_size_lsb : 10;
    // 32
    uint32_t device_size_msb : 2;
    uint32_t reserved3 : 2;
    uint32_t dsr_implemented : 1;
    uint32_t read_block_misalignment : 1;
    uint32_t write_block_misalignment : 1;
    uint32_t read_block_partial : 1;
    uint32_t max_read_block_length : 4;
    uint32_t card_command_classes : 12;
    uint32_t max_transfer_rate : 8;
    // 32
    uint32_t nsac : 8;
    uint32_t taac : 8;
    uint32_t reserved4 : 6;
    uint32_t csd_structure : 2;
  } fields;
  uint32_t raw[4];
} emmc_csd_t;

/*
 * EMMC Register Types (BCM2835 Peripherals p.65)
 */

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

typedef union {
  struct {
    uint32_t command_done : 1;
    uint32_t data_done : 1;
    uint32_t block_gap : 1;
    uint32_t reserved0 : 1;
    uint32_t write_ready : 1;
    uint32_t read_ready : 1;
    uint32_t card_insertion : 1;
    uint32_t card_removal : 1;
    uint32_t card_interrupt : 1;
    uint32_t reserved1 : 3;
    uint32_t retune : 1;
    uint32_t boot_acknowledge : 1;
    uint32_t end_boot : 1;
    uint32_t reserved2 : 1;
    uint32_t command_timeout : 1;
    uint32_t command_crc_error : 1;
    uint32_t command_end_bit_error : 1;
    uint32_t command_incorrect_error : 1;
    uint32_t data_timeout : 1;
    uint32_t data_crc_error : 1;
    uint32_t data_end_bit_error : 1;
    uint32_t reserved3 : 1;
    uint32_t auto_command_error : 1;
    uint32_t reserved4 : 7;
  } fields;
  uint32_t raw;
} emmc_interrupt_t;

typedef struct {
   uint32_t response0;
   uint32_t response1;
   uint32_t response2;
   uint32_t response3;
}emmc_response_t;

typedef struct {
  uint32_t argument1;
  uint32_t argument2;
}emmc_argument_t;


