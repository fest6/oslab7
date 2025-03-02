set confirm off
source scripts/gef.py
set architecture riscv:rv64
file build/kernel
add-symbol-file build/kernel -o -0xffffffff00000000
#target extended-remote 127.0.0.1:3333
gef-remote --qemu-user --qemu-binary build/kernel 127.0.0.1 3333
b *0x80200000
