
void __attribute__((interrupt("UNDEF"))) undefined_instruction_handler(void){
    while(1);
}

void __attribute__((interrupt("SWI"))) software_interrupt_handler(void){
  while(1);
}

void __attribute__((interrupt("ABORT"))) prefetch_abort_handler(void){
  while(1);
}

void __attribute__((interrupt("ABORT"))) data_abort_handler(void){
  while(1);
}

void __attribute__((interrupt("IRQ"))) interrupt_handler(void){
  while(1);
}

void __attribute__((interrupt("FIQ"))) fast_interrupt_handler(void){
  while(1);
}
