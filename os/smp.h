#ifndef __SMP_H_
#define __SMP_H_

#include "types.h"
#include "riscv.h"

struct cpu {
    int mhart_id;                  // mhartid for this cpu, passed by OpenSBI
    // struct proc *proc;             // current process
    // struct context sched_context;  // scheduler context, swtch() here to run scheduler
    int inkernel_trap;             // whether we are in a kernel trap context
    int noff;                      // how many push-off
    int interrupt_on;              // Is the interrupt Enabled before the first push-off?
    uint64 sched_kstack_top;  // top of per-cpu sheduler kernel stack
    int cpuid;  // for debug purpose
};

static inline int cpuid() {
    return r_tp();
}

// cpu.c
struct cpu *mycpu();
struct cpu *getcpu(int i);

#endif