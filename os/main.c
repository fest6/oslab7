#include "console.h"
#include "debug.h"
#include "defs.h"
#include "plic.h"
#include "sbi.h"
#include "timer.h"

uint64 __pa kernel_image_end_4k;
uint64 __pa kernel_image_end_2M;
uint64 __pa kpage_allocator_base;
uint64 __pa kpage_allocator_size;

static char percpu_kstack[NCPU][PGSIZE * 4] __attribute__((aligned(PGSIZE)));

__noreturn void bootcpu_entry(int mhartid);
__noreturn void secondarycpu_entry(int mhartid, int cpuid);

static void bootcpu_init();
static void secondarycpu_init();
void init();
static struct proc* initproc;

static volatile int booted_count = 0;
static volatile int halt_specific_init = 0;

/** Multiple CPU (SMP) Boot Process:
 * ------------
 * | Boot CPU |  cpuid = 0, m_hartid = random
 * ------------
 *      | OpenSBI
 * ----------------
 * |    _entry    |
 * ----------------
 *      | sp <= boot_stack (PA)
 * -------------------------
 * | _entry, bootcpu_entry |
 * -------------------------
 *      | sp <= percpu_kstack (PA)
 * ----------------
 * | bootcpu_init |
 * ----------------
 *    |                                             ------------------------
 *    | OpenSBI: HSM_Hart_Start   ---(a1)------>    | _entry_secondary_cpu |
 *    |                                             ------------------------
 *    |                                                     | sp <= percpu_kstack (PA)
 *    |                                             ----------------------
 *    |                                             | secondarycpu_entry |
 *    |                                             ----------------------
 *    |                                                     | satp <= relocate_pagetable
 *    |                                                     | sp   <= boot_stack (KIVA)
 *    |                                             ---------------------------
 *    |                                             | secondarycpu_relocating |
 *    |                                             ---------------------------
 *    |                                                     | satp <= kernel_pagetable
 *    |                                                     | sp   <= percpu_sched_stack (KVA)
 *    | wait for all cpu online                     ---------------------
 *    |                                             | secondarycpu_init |
 *    | platform level init :                       ---------------------
 *    |   console, plic, kpgmgr,                            | wait for `halt_specific_init`
 *    |   uvm, proc, loader                                 |
 *    |                                                     |
 *    | halt_init: trap, timer, plic_hart                   | halt_init: trap, timer, plic_hart
 *    |                                                     |
 * -------------                                    -------------
 * | scheduler |                                    | scheduler |
 * -------------                                    -------------
 */

void bootcpu_entry(int mhartid) {
    printf("\n\n=====\nHello World!\n=====\n\n");
    printf("Boot stack: %p\nclean bss: %p - %p\n", boot_stack, s_bss, e_bss);

    memset(s_bss, 0, e_bss - s_bss);

    printf("Boot m_hartid %d\n", mhartid);

    // the boot hart always has cpuid == 0
    w_tp(0);
    // after setup tp, we can use mycpu()
    mycpu()->cpuid = 0;
    mycpu()->mhart_id = mhartid;
    // functions in log.h (infof, errorf, etc.) requres mycpu() to be initialized.

    infof("basic smp inited, thread_id available now, we are cpu %d: %p", mhartid, mycpu());

    infof("Jump to percpu kernel stack");

    uint64 fn = (uint64)&bootcpu_init;
    uint64 sp = (uint64)&percpu_kstack[cpuid()];
    asm volatile("mv a1, %0" ::"r"(fn));
    asm volatile("mv sp, %0" ::"r"(sp));
    asm volatile("jr a1");

    __builtin_unreachable();
}

__noreturn void secondarycpu_entry(int hartid, int mycpuid) {
    printf("cpu %d (halt %d) booting. Relocating\n", mycpuid, hartid);

    // init mycpu()
    w_tp(mycpuid);
    getcpu(mycpuid)->mhart_id = hartid;
    getcpu(mycpuid)->cpuid = mycpuid;

    // jump to percpu kernel stack
    uint64 fn = (uint64)&secondarycpu_init;
    uint64 sp = (uint64)&percpu_kstack[cpuid()];
    asm volatile("mv a1, %0\n" ::"r"(fn));
    asm volatile("mv sp, %0\n" ::"r"(sp));
    asm volatile("jr a1");

    __builtin_unreachable();
}

static void bootcpu_init() {
    printf("Relocated. Boot halt sp at %p\n", r_sp());

#ifdef ENABLE_SMP
    printf("Boot another cpus.\n");

    // Attention: OpenSBI does not guarantee the boot cpu has mhartid == 0.
    // We assume NCPU == the number of cpus in the system, although spec does not guarantee this.
    {
        int cpuid = 1;
        int max_hartid = NCPU;

        for (int hartid = 0; hartid < max_hartid; hartid++) {
            if (hartid == mycpu()->mhart_id)
                continue;

            int saved_booted_cnt = booted_count;

            printf(
                "- booting hart %d: hsm_hart_start(hartid=%d, pc=_entry_sec, opaque=%d)", hartid,
                hartid, cpuid);
            int ret = sbi_hsm_hart_start(hartid, (uint64)_entry_secondary_cpu, cpuid);
            printf(" = %d.\n", ret);
            if (ret < 0) {
                printf("skipped for hart %d\n", hartid);
                continue;
            }
            while (booted_count == saved_booted_cnt);
            cpuid++;
        }
        printf("System has %d cpus online\n\n", cpuid);
    }
#endif

    trap_init();
    console_init();
    printf("UART inited.\n");
    plicinit();
    timer_init();
    plicinithart();

    MEMORY_FENCE();
    halt_specific_init = 1;
    MEMORY_FENCE();

    infof("boot cpu loops");

    while (1) {
    }

    assert("boot cpu loop returns");
}

static void secondarycpu_init() {
    printf("cpu %d (halt %d) booted. sp: %p\n", mycpu()->cpuid, mycpu()->mhart_id, r_sp());
    booted_count++;

    // wait for boot cpu initialize platform-level devices.
    while (!halt_specific_init);

    trap_init();
    timer_init();
    plicinithart();

    infof("secondary cpu loops");
    while (1) {
    }

    assert("secondary cpu loop returns");
}