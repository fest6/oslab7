#include "plic.h"

#include "defs.h"
#include "trap.h"
#include "console.h"

//
// the riscv Platform Level Interrupt Controller (PLIC).
//
// see docs: https://github.com/riscv/riscv-plic-spec/blob/master/riscv-plic.adoc

void plicinit(void)
{
	// set desired IRQ priorities non-zero (otherwise disabled).
	// Interrupt source: UART0 - 10

	*(uint32 *)(KERNEL_PLIC_BASE + QEMU_UART0_IRQ * 4) = 1;
}

void plicinithart(void)
{
	const int mhartid = 0;
	const int ctx = mhartid * 2 + 1;

	// base + 0x002000 + 0x80 + 0x100*hart
	// Assumption: Each hart has two context, we use the last one referring to the S-mode context.
	//	hart 0: context 0 -> M mode
	//			context 1 -> S mode
	//	hart 1: context 2 -> M mode
	//			context 3 -> S mode

	// set enable bits for this hart's S-mode for the uart.
	uint32 off = QEMU_UART0_IRQ / 32;
	uint32 bit = QEMU_UART0_IRQ % 32;
	*(uint32 *)(PLIC_SENABLE(ctx) + off * 4) |= (1 << bit);

	// set this hart's S-mode priority threshold to 0.
	*(uint32 *)PLIC_SPRIORITY(ctx) = 0;

	w_sie(r_sie() | SIE_SEIE);	// enable External Interrupt
}

// ask the PLIC what interrupt we should serve.
int plic_claim(void)
{
	const int mhartid = 0;
	const int ctx = mhartid * 2 + 1;

	int irq = *(uint32 *)PLIC_SCLAIM(ctx);
	return irq;
}

// tell the PLIC we've served this IRQ.
void plic_complete(int irq)
{
	const int mhartid = 0;
	const int ctx = mhartid * 2 + 1;

	*(uint32 *)PLIC_SCLAIM(ctx) = irq;
}
