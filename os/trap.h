#ifndef TRAP_H
#define TRAP_H

#include "types.h"

#define SCAUSE_INTERRUPT           ((1ull) << 63)
#define SCAUSE_EXCEPTION_CODE_MASK (((1ull) << 63) - 1)

/**
 * @brief Kernel trap frame
 *
 * Used when we are in the kernel, and we meet an Exception or Interrupt to get into the trap.acquire
 *
 * This ktrapframe is stored in the kernel stack.
 *
 */
struct ktrapframe {
    uint64 x0;  // x0
    uint64 ra;
    uint64 sp;
    uint64 gp;
    uint64 tp;
    uint64 t0;
    uint64 t1;
    uint64 t2;
    uint64 s0;
    uint64 s1;
    uint64 a0;
    uint64 a1;
    uint64 a2;
    uint64 a3;
    uint64 a4;
    uint64 a5;
    uint64 a6;
    uint64 a7;
    uint64 s2;
    uint64 s3;
    uint64 s4;
    uint64 s5;
    uint64 s6;
    uint64 s7;
    uint64 s8;
    uint64 s9;
    uint64 s10;
    uint64 s11;
    uint64 t3;
    uint64 t4;
    uint64 t5;
    uint64 t6;

    // 32 * 8 bytes = 256 (0x100) bytes
};

enum Exception {
    InstructionMisaligned  = 0,
    InstructionAccessFault = 1,
    IllegalInstruction     = 2,
    Breakpoint             = 3,
    LoadMisaligned         = 4,
    LoadAccessFault        = 5,
    StoreMisaligned        = 6,
    StoreAccessFault       = 7,
    UserEnvCall            = 8,
    SupervisorEnvCall      = 9,
    MachineEnvCall         = 11,
    InstructionPageFault   = 12,
    LoadPageFault          = 13,
    StorePageFault         = 15,
};

enum Interrupt {
    UserSoft = 0,
    SupervisorSoft,
    UserTimer = 4,
    SupervisorTimer,
    UserExternal = 8,
    SupervisorExternal,
};

void trap_init();
void kerneltrap(struct ktrapframe *ktf);
void usertrapret();

#endif  // TRAP_H