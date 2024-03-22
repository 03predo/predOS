
extern int __bss_start__;
extern int __bss_end__;

extern int kernel_main();

int kernel_init(void) __attribute__((naked)) __attribute__((section(".text.boot.kernel")));
int kernel_init(void)
{
  int *bss = &__bss_start__;
  int* bss_end = &__bss_end__;

  while(bss < bss_end) *bss++ = 0;

  kernel_main();

  while(1);
}

