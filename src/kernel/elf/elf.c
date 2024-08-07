#include "elf.h"
#include "sys_log.h"

#define ELF_32_BIT          0x01
#define ELF_LITTLE_ENDIAN   0x01
#define ELF_FILE_TYPE_EXEC  0x02
#define ELF_MACHINE_ARM     0x28

status_t elf_print_file_header(elf_file_header_t eh){
  SYS_LOGD(
    "elf file header:\n"
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

status_t elf_print_program_header(elf_program_header_t ph){
  SYS_LOGD(
    "elf program header:\n"
    "   type: %#x\n"
    "   offset: %#x\n"
    "   virtual_address: %#x\n"
    "   physical_address: %#x\n"
    "   size_in_file: %#x\n"
    "   size_in_memory: %#x\n"
    "   flags: %#x\n"
    "   alignment: %#x\n",
    ph.type,
    ph.offset,
    ph.virtual_address,
    ph.physical_address,
    ph.size_in_file,
    ph.size_in_memory,
    ph.flags,
    ph.alignment
  );

  return STATUS_OK;
}


status_t elf_validate_header(elf_file_header_t eh){ 
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

  if(eh.object_file_type != ELF_FILE_TYPE_EXEC){
    SYS_LOGE("invalid file type: %#x", eh.object_file_type);
    return STATUS_ERR;
  }

  if(eh.machine != ELF_MACHINE_ARM){
    SYS_LOGE("invalid machine: %#x", eh.machine);
    return STATUS_ERR;
  }

  if(eh.entry_point < 0x8000){
    SYS_LOGE("invalid entry point: %#x", eh.entry_point);
    return STATUS_ERR;
  }

  if(eh.program_header_size != sizeof(elf_program_header_t)){
    SYS_LOGE("invalid program header size: %#x", eh.program_header_size);
  }

  elf_print_file_header(eh); 
  return STATUS_OK;
}

