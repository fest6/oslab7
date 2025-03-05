    .section .text.entry
    .globl _entry
_entry:
    lla sp, boot_stack_top
    call bootcpu_entry

    .section .bss.stack
    .globl boot_stack
boot_stack:
    .space 4096
    .globl boot_stack_top
boot_stack_top:
