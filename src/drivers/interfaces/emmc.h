#pragma once

#include "status.h"

#define EMMC_BLOCK_SIZE 512

typedef struct{
  uint8_t buf[EMMC_BLOCK_SIZE];
} emmc_block_t;

status_t emmc_read_block(uint32_t block_address, emmc_block_t* block);
status_t emmc_write_block(uint32_t block_address, emmc_block_t* block);
status_t emmc_init(void);
