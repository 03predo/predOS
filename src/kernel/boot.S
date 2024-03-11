.section ".text.boot"

.global _start

_start:
  bl kernel_init
