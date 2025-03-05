#include "trap.h"
#include "defs.h"
#include "console.h"
#include "debug.h"
#include "plic.h"
#include "timer.h"

extern volatile int panicked;

void plic_handle() {
    int irq = plic_claim();
    if (irq == QEMU_UART0_IRQ) {
        uart_intr();
        // printf("intr %d: UART0\n", r_tp());
    }

    if (irq)
        plic_complete(irq);
}

void kernel_trap(struct ktrapframe *ktf) {
    assert(!intr_get());

    if ((r_sstatus() & SSTATUS_SPP) == 0)
        panic("kerneltrap: not from supervisor mode");

    uint64 cause          = r_scause();
    uint64 exception_code = cause & SCAUSE_EXCEPTION_CODE_MASK;
    if (cause & SCAUSE_INTERRUPT) {
        // handle interrupt
        switch (exception_code) {
            case SupervisorTimer:
                tracef("s-timer interrupt, cycle: %d", r_time());
                set_next_timer();
                // we never preempt kernel threads.
                break;
            case SupervisorExternal:
                tracef("s-external interrupt.");
                plic_handle();
                break;
            default:
                errorf("unhandled interrupt: %d", cause);
                goto kernel_panic;
        }
    } else {
        // kernel exception, unexpected.
        goto kernel_panic;
    }

    assert(!intr_get());
    return;

kernel_panic:
    panicked = 1;

    errorf("=========== Kernel Panic ===========");
    print_sysregs(true);
    print_ktrapframe(ktf);

    panic("kernel panic");
}

void set_kerneltrap() {
    assert(IS_ALIGNED((uint64)kernel_trap_entry, 4));
    w_stvec((uint64)kernel_trap_entry);  // DIRECT
}

// set up to take exceptions and traps while in the kernel.
void trap_init() {
    set_kerneltrap();
}

