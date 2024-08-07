#include <stdint.h>

#include "status.h"

#define ELF_32_BIT 1
#define ELF_LITTLE_ENDIAN 1

#pragma pack(push, 1)
typedef struct {
  uint8_t signature[4];
  uint8_t word_size;
  uint8_t endianness;
  uint8_t elf_version0;
  uint8_t operating_system;
  uint8_t abi_version;
  uint8_t reserved0[7];
  uint16_t object_file_type;
  uint16_t machine; 
  uint32_t elf_version1;
  uint32_t entry_point;
  uint32_t program_header_offset;
  uint32_t section_header_offset;
  uint32_t flags;
  uint16_t elf_header_size;
  uint16_t program_header_size;
  uint16_t program_header_num;
  uint16_t section_header_size;
  uint16_t section_header_num;
  uint16_t section_header_index;
} elf_header_t;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct {
  uint32_t type;
  uint32_t offset;
  uint32_t virtual_address;
  uint32_t physical_address;
  uint32_t size_in_file;
  uint32_t size_in_memory;
  uint32_t flags;
  uint32_t alignment;
} program_header_t;
#pragma pack(pop)

status_t elf_validate_header(elf_header_t eh);
