set confirm off
source scripts/gef.py
set architecture riscv:rv64
file build/kernel
#target extended-remote 127.0.0.1:3333
gef-remote --qemu-user --qemu-binary build/kernel 127.0.0.1 3333
b *0x80200000
