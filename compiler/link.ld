ENTRY(_start)
OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")

SDRAM_SIZE = 0x20000000;
KERNEL_SIZE = 0x2000000;
BOOTLOADER_SIZE = 0x200;

BOOT_ADDR = 0x8000;
KERNEL_LOAD_ADDR = BOOT_ADDR + BOOTLOADER_SIZE;
KERNEL_VIRTUAL_ADDR = BOOT_ADDR + BOOTLOADER_SIZE; /* should eventually be virtual SDRAM_SIZE - KERNEL_SIZE */


SECTIONS {
  .boot BOOT_ADDR:
  {
    *(.text.boot)
    *(.text.kernel_init)
  }


  .kernel KERNEL_VIRTUAL_ADDR :
  AT(KERNEL_LOAD_ADDR)
  {
    *(.text)
    *(.rodata)
    *(.data)
    _bss_start = .;
    *(.bss)
    *(COMMON)
    _bss_end = .;
  }
}
