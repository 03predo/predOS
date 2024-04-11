# Debug Interface

The goal of the debug interface is to provide a way to set breakpoints to halt the cpu to monitor the operating system state.

We can accomplish this through the use of the BKPT instruction which acts as though a prefetch abort has occured. On a prefetch abort our handler must check the IFSR(instruction fault status register) to see if it indicates an instruction debug event fault and call the debug event handler with the breakpoint number as the argument.

## Implementation

We need to configure the CPU for monitor debug-mode, this can be done by setting the bit in the DSCR(debug status control register).

```
mrc p14, #0, r0, c0, c1, #0
orr r0, r0, #(0x1 << MONITOR_DEBUG_MODE)
mcr p14, #0, r0, c0, c1, #0
```

Then we can define a macro to call the breakpoint instruction with an associated argument. This will trigger the prefetch abort handler, where we check the IFSR to see if the abort was caused by a breakpoint. In that case we will call the debug_handler with the associated breakpoint handler.

```
// load IFSR into r0
mrc p15, #0, r0, c5, c0, #1
and r0, r0, #0b1111 // the status is in the bottom 4 bits
cmp r0, #(IFSR_DEBUG_EVENT)
bne prefetch_abort_handler

// the link register holds the address of the instruction after the breakpoint
mov r0, lr;
bl debug_event_handler

bx lr
```

