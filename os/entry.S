    .section .text.entry
    .globl _entry
_entry:
    lla sp, boot_stack_top
    call bootcpu_entry

    .globl _entry_secondary_cpu
_entry_secondary_cpu:
    # RISC-V SBI Spec: Ch.9 Hart State Management Extension
    #    sbi_hart_start: a0: hartid, a1: opaque
    lla sp, boot_stack_top
    call secondarycpu_entry

    .section .bss.stack
    .globl boot_stack
boot_stack:
    .space 4096
    .globl boot_stack_top
boot_stack_top:
