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

    infof("boot cpu loops");

    while (1) {
    }

    assert("boot cpu loop returns");

    __builtin_unreachable();
}