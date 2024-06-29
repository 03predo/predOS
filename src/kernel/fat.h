#pragma once

#include "status.h"
#include "emmc.h"

#define FAT_DIR_ENTRY_NAME_LEN 11

typedef enum {
  FAT12 = 0,
  FAT16,
  FAT32,
} fat_type_t;

typedef struct {
  uint8_t status;
  uint8_t chs_first_sector_address[3];
  uint8_t partition_type;
  uint8_t chs_last_sector_address[3];
  uint32_t lba_first_sector_address;
  uint32_t size_in_sectors;
} partition_entry_t;

#pragma pack(push, 1)
typedef union {
  struct {
    uint8_t bootstrap_are[440];
    uint32_t disk_signature;
    uint16_t res0;
    partition_entry_t p1; 
    partition_entry_t p2; 
    partition_entry_t p3; 
    partition_entry_t p4; 
    uint16_t boot_signature;
  } fields;
  uint8_t raw[512];
} master_boot_record_t;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct {
  uint8_t jump_boot[3];
  uint8_t oem_name[8];
  uint16_t bytes_per_sector;
  uint8_t sectors_per_cluster;
  uint16_t reserved_sector_count;
  uint8_t number_of_fats;
  uint16_t root_entry_count;
  uint16_t total_sector_count_16bit;
  uint8_t media; 
  uint16_t fat_sector_count_16bit;
  uint16_t sectors_per_track;
  uint16_t number_of_heads;
  uint32_t hidden_sector_count;
  uint32_t total_sector_count_32bit;
} bios_parameter_block_t;
#pragma pack(pop)

#pragma pack(push, 1)
typedef union {
  struct {
    bios_parameter_block_t bpb;
    uint8_t drive_number;
    uint8_t reserved1;
    uint8_t boot_signature;
    uint32_t volume_id;
    uint8_t volume_label[11];
    uint8_t file_system_type[8];
  } fields;
  uint8_t raw[62];
} boot_sector_fat16_t;
#pragma pack(pop)

#pragma pack(push, 1)
typedef union {
  struct {
    bios_parameter_block_t bpb;
    uint32_t fat_sector_count_32bit;
    uint16_t external_flags;
    uint16_t file_system_version;
    uint32_t root_cluster;
    uint16_t file_system_info;
    uint16_t backup_boot_sector;
    uint8_t reserved1[12];
    uint8_t drive_number;
    uint8_t reserved2;
    uint8_t boot_signature;
    uint32_t volume_id;
    uint8_t volume_label[11];
    uint8_t file_system_type[8];
  } fields;
  uint8_t raw[90];
} boot_sector_fat32_t;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct {
  master_boot_record_t mbr;
  fat_type_t type;
  boot_sector_fat16_t bs16;
  boot_sector_fat32_t bs32;
  uint32_t root_dir_base_sector;
  uint32_t fat_base_sector;
  uint32_t partition_base_sector;
  uint32_t data_base_sector;
  uint32_t blocks_per_cluster;
} file_allocation_table_t;
#pragma pack(pop)

typedef struct {
  uint16_t day : 5;
  uint16_t month : 4;
  uint16_t year : 7;
} fat_date_stamp_t;

typedef struct {
  uint16_t two_seconds : 5;
  uint16_t minutes : 6;
  uint16_t hours : 5;
} fat_time_stamp_t;

typedef struct {
  uint8_t name[FAT_DIR_ENTRY_NAME_LEN];
  uint8_t attribute;
  uint8_t reserved0;
  uint8_t creation_time_tenth;
  fat_time_stamp_t creation_time;
  fat_date_stamp_t creation_date;
  fat_date_stamp_t last_access_date;
  uint16_t first_cluster_high;
  fat_time_stamp_t last_write_time;
  fat_date_stamp_t last_write_date;
  uint16_t first_cluster_low;
  uint32_t file_size;
} fat_directory_entry_t;

status_t fat_init();
status_t fat_get_dir_entry(const char* file_name, fat_directory_entry_t* dir_entry);
status_t fat_print_entry(fat_directory_entry_t entry);
status_t fat_read_block(fat_directory_entry_t* dir_entry, uint32_t file_block_number, emmc_block_t* block);
status_t fat_write_block(fat_directory_entry_t* dir_entry, uint32_t file_block_number, emmc_block_t* block);
status_t fat_create_file(const char* file_name);

