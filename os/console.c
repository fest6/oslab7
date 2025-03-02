#include "console.h"

#include "debug.h"
#include "defs.h"
#include "sbi.h"

static int uart_inited = false;
static void uart_putchar(int);

static struct spinlock uart_tx_lock;
volatile int panicked = 0;

#define BACKSPACE 0x100
#define C(x)      ((x) - '@')  // Control-x

static void uart_putchar(int ch) {
    int intr = intr_off();
    while ((read_reg(LSR) & LSR_TX_IDLE) == 0);

    set_reg(THR, ch);
    if (intr)
        intr_on();
}

static int uartgetc(void) {
    if (read_reg(LSR) & 0x01) {
        // input data is ready.
        return read_reg(RHR);
    } else {
        return -1;
    }
}

void consputc(int c) {
    if (!uart_inited || panicked)  // when panicked, use SBI output
        sbi_putchar(c);
    else {
        if (c == BACKSPACE) {
            uart_putchar('\b');
            uart_putchar(' ');
            uart_putchar('\b');
        } else if (c == '\n') {
            uart_putchar('\r');
            uart_putchar('\n');
        } else {
            uart_putchar(c);
        }
    }
}


void console_init() {
    assert(!uart_inited);
    spinlock_init(&uart_tx_lock, "uart_tx");

    // no need to init uart8250, they are already inited by OpenSBI.

    // disable interrupts.
    set_reg(IER, 0x00);

    // reset and enable FIFOs.
    set_reg(FCR, FCR_FIFO_ENABLE | FCR_FIFO_CLEAR);

    // enable receive interrupts.
    set_reg(IER, IER_RX_ENABLE | IER_TX_ENABLE);
    uart_inited = true;
}

static void consintr(int c) {
    switch (c) {
        case '\x7f':  // Delete key
            consputc(BACKSPACE);
            break;
        default:
            if (c != 0) {
                c = (c == '\r') ? '\n' : c;

                // echo back to the serial.
                consputc(c);
            }
            break;
    }
}

void uart_intr() {
    while (1) {
        int c = uartgetc();
        if (c == -1)
            break;
        // infof("uart: %c", c);
        consintr(c);
    }
}
