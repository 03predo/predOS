ENTRY(_start)
OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")

SDRAM_SIZE = 0x20000000;
VC_SDRAM_SIZE = 0x8000;
ARM_SDRAM_SIZE = SDRAM_SIZE - VC_SDRAM_SIZE;
KERNEL_SIZE = 0x2000000;
BOOTLOADER_SIZE = 0x800;

BOOT_ADDR = 0x8000;
KERNEL_LOAD_ADDR = BOOT_ADDR + BOOTLOADER_SIZE;
KERNEL_VIRTUAL_OFFSET = 0xC0000000;
KERNEL_VIRTUAL_ADDR = KERNEL_LOAD_ADDR + KERNEL_VIRTUAL_OFFSET;

SECTIONS {
  .boot BOOT_ADDR :
  {
    *(.text.boot)
    *(.text.boot.*)
  }  

  .kernel_text KERNEL_VIRTUAL_ADDR :
  AT( KERNEL_LOAD_ADDR )
  {
    *(.text)
  }

  .kernel_data . :
  {
    *(.rodata)
    *(.data .data.* .gnu.linkonce.d.*)
  }
  
  .kernel_bss :
  {
    __bss_start__ = .;
    *(.dynbss)
    *(.bss .bss.* .gnu.linkonce.b.*)
    *(COMMON)
    __bss_end__ = .;

  } 

  _end = .; 
}
