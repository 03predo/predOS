
extern int _bss_start;
extern int _bss_end;

extern int kernel_main();

int kernel_init(void) __attribute__((naked)) __attribute__((section(".text.boot.kernel")));
int kernel_init(void)
{
  int *bss = &_bss_start;
  int* bss_end = &_bss_end;

  while(bss < bss_end) *bss++ = 0;

  kernel_main();

  while(1);
}

