    .section .text.entry
    .globl _entry
_entry:
    lla sp, boot_stack_top
    call bootcpu_entry

.globl _entry_secondary_cpu
_entry_secondary_cpu:
    # RISC-V SBI Spec: Ch.9 Hart State Management Extension
    #    sbi_hart_start: a0: hartid, a1: opaque
    fence.i
    lla sp, boot_stack_top
    call secondarycpu_entry


    .globl kernel_trap_entry
    .align 2
kernel_trap_entry:
    // we store all registers in the stack
    add sp, sp, -0x100
    sd x0, 0x00(sp)
    sd x1, 0x08(sp)
    sd x2, 0x10(sp)
    sd x3, 0x18(sp)
    sd x4, 0x20(sp)
    sd x5, 0x28(sp)
    sd x6, 0x30(sp)
    sd x7, 0x38(sp)
    sd x8, 0x40(sp)
    sd x9, 0x48(sp)
    sd x10, 0x50(sp)
    sd x11, 0x58(sp)
    sd x12, 0x60(sp)
    sd x13, 0x68(sp)
    sd x14, 0x70(sp)
    sd x15, 0x78(sp)
    sd x16, 0x80(sp)
    sd x17, 0x88(sp)
    sd x18, 0x90(sp)
    sd x19, 0x98(sp)
    sd x20, 0xa0(sp)
    sd x21, 0xa8(sp)
    sd x22, 0xb0(sp)
    sd x23, 0xb8(sp)
    sd x24, 0xc0(sp)
    sd x25, 0xc8(sp)
    sd x26, 0xd0(sp)
    sd x27, 0xd8(sp)
    sd x28, 0xe0(sp)
    sd x29, 0xe8(sp)
    sd x30, 0xf0(sp)
    sd x31, 0xf8(sp)

    mv a0, sp   // make a0 point to the ktrapframe structure
    call kernel_trap

    // restore all registers
    ld x0, 0x00(sp)
    ld x1, 0x08(sp)
    ld x2, 0x10(sp)
    ld x3, 0x18(sp)
    // do not recover tp(x4), in case we have moved CPUs: ld x4, 0x20(sp)
    ld x5, 0x28(sp)
    ld x6, 0x30(sp)
    ld x7, 0x38(sp)
    ld x8, 0x40(sp)
    ld x9, 0x48(sp)
    ld x10, 0x50(sp)
    ld x11, 0x58(sp)
    ld x12, 0x60(sp)
    ld x13, 0x68(sp)
    ld x14, 0x70(sp)
    ld x15, 0x78(sp)
    ld x16, 0x80(sp)
    ld x17, 0x88(sp)
    ld x18, 0x90(sp)
    ld x19, 0x98(sp)
    ld x20, 0xa0(sp)
    ld x21, 0xa8(sp)
    ld x22, 0xb0(sp)
    ld x23, 0xb8(sp)
    ld x24, 0xc0(sp)
    ld x25, 0xc8(sp)
    ld x26, 0xd0(sp)
    ld x27, 0xd8(sp)
    ld x28, 0xe0(sp)
    ld x29, 0xe8(sp)
    ld x30, 0xf0(sp)
    ld x31, 0xf8(sp)

    // restore stack
    add sp, sp, 0x100

    // return from trap
    sret

    .section .bss.stack
    .globl boot_stack
boot_stack:
    .space 4096
    .globl boot_stack_top
boot_stack_top:
