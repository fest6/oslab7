// init: The initial user-level program

#include "../lib/user.h"

int main(void) {
    printf("init: I am the first user program!\n");

    // asm volatile(" csrw stvec, %0" : : "r"(0x80000000));
    char buf[200];
    memset(buf, 'A', sizeof(buf));

    while(1) {
        printf("input> ");
        stdout_flush();
        int bytes = read(0, buf, sizeof(buf));
        buf[bytes] = '\0';
        printf("init: read %d bytes: %s", bytes, buf);
        // no \n here, it's included in buf.
    }
}
