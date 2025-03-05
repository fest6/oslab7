#include "console.h"
#include "debug.h"
#include "defs.h"
#include "plic.h"
#include "sbi.h"
#include "timer.h"

void bootcpu_entry(int mhartid) {
    printf("\n\n=====\nHello World!\n=====\n\n");
    printf("Boot stack: %p\nclean bss: %p - %p\n", boot_stack, s_bss, e_bss);

    memset(s_bss, 0, e_bss - s_bss);

    printf("Boot m_hartid %d\n", mhartid);

    trap_init();
    console_init();
    printf("UART inited.\n");
    plicinit();
    timer_init();
    plicinithart();

    int labreport = 1;  // MODIFY ME: change this to 1 to work on the lab report.

    if (labreport) {
        uint64 reg;

        asm volatile("mv %0, s11" : "=r"(reg));
        printf("s11: %p\n", reg);
        printf("calling ebreak...\n");

        asm volatile("ebreak"::: "s11");

        asm volatile("mv %0, s11" : "=r"(reg));
        printf("s11: %p\n", reg);
    }

    intr_on();
    infof("boot cpu loops");

    while (1) {
    }

    assert("boot cpu loop returns");

    __builtin_unreachable();
}