#include "elf.h"
#include "sys_log.h"

status_t elf_print_header(elf_header_t eh){
  SYS_LOGI(
    "elf header:\n"
    "   signature: %#x %#x %#x %#x\n"
    "   word_size: %#x\n"
    "   endianness: %#x\n"
    "   elf_version0: %#x\n"
    "   operating_system: %#x\n"
    "   abi_version: %#x\n"
    "   object_file_type: %#x\n"
    "   machine: %#x\n"
    "   elf_version1: %#x\n"
    "   entry_point: %#x\n"
    "   program_header_offset: %#x\n"
    "   section_header_offset: %#x\n"
    "   flags: %#x\n"
    "   elf_header_size: %#x\n"
    "   program_header_size: %#x\n"
    "   program_header_num: %#x\n"
    "   section_header_size: %#x\n"
    "   section_header_num: %#x\n"
    "   section_header_index: %#x\n",
    eh.signature[0], eh.signature[1], eh.signature[2], eh.signature[3],
    eh.word_size,
    eh.endianness,
    eh.elf_version0,
    eh.operating_system,
    eh.abi_version,
    eh.object_file_type,
    eh.machine,
    eh.elf_version1,
    eh.entry_point,
    eh.program_header_offset,
    eh.section_header_offset,
    eh.flags,
    eh.elf_header_size,
    eh.program_header_size,
    eh.program_header_num,
    eh.section_header_size,
    eh.section_header_num,
    eh.section_header_index
  );

  return STATUS_OK;
}

status_t elf_validate_header(elf_header_t eh){ 
  uint8_t elf_signature[] = {0x7f, 0x45, 0x4c, 0x46};
  for(uint32_t i = 0; i < sizeof(elf_signature); ++i){
    if(eh.signature[i] != elf_signature[i]){
      SYS_LOGE("invalid elf signature: %#x, %#x, %#x, %#x", eh.signature[0], eh.signature[1], eh.signature[2], eh.signature[3]);
      return STATUS_ERR;
    }
  }

  if(eh.word_size != ELF_32_BIT){
    SYS_LOGE("invalid word size: %#x", eh.word_size);
    return STATUS_ERR;
  }

  if(eh.endianness != ELF_LITTLE_ENDIAN){
    SYS_LOGE("invalid endianness: %#x", eh.endianness);
    return STATUS_ERR;
  }

  if(eh.elf_version0 != 1){
    SYS_LOGE("invalid elf version: %#x", eh.elf_version0);
    return STATUS_ERR;
  }
  elf_print_header(eh); 
  return STATUS_OK;
}
