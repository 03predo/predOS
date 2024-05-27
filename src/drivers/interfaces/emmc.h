#pragma once

#include "status.h"

#define EMMC_BLOCK_SIZE 512

typedef struct{
  uint32_t buf[EMMC_BLOCK_SIZE / sizeof(uint32_t)];
} emmc_block_t;

status_t emmc_read_block(uint32_t start_block_address, uint16_t num_blocks, emmc_block_t block[]);
status_t emmc_start_write(uint32_t start_block_address, uint16_t num_blocks);
status_t emmc_write_word(uint32_t word);
status_t emmc_finish_write();
status_t emmc_write_block(uint32_t start_block_address, uint16_t num_blocks, emmc_block_t block[]);
status_t emmc_print_block(emmc_block_t);
status_t emmc_init(void);
