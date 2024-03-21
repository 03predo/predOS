.section ".text.boot"

.global _start
.global kernel_init

_start:
  ldr sp, =0x8000

  b kernel_init

_inf_loop:
  b _inf_loop
