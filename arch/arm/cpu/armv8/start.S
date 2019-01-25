/*
 * (C) Copyright 2013
 * David Feng <fenghua@phytium.com.cn>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm-offsets.h>
#include <config.h>
#include <version.h>
#include <linux/linkage.h>
#include <asm/macro.h>
#include <asm/armv8/mmu.h>

/*************************************************************************
 *
 * Startup Code (reset vector)
 *
 *************************************************************************/

.globl	_start
_start:
	b	reset

	.align 3

.globl	_TEXT_BASE
_TEXT_BASE:
	.quad	CONFIG_SYS_TEXT_BASE

/*
 * These are defined in the linker script.
 */
.globl	_end_ofs
_end_ofs:
	.quad	_end - _start

.globl	_bss_start_ofs
_bss_start_ofs:
	.quad	__bss_start - _start

.globl	_bss_end_ofs
_bss_end_ofs:
	.quad	__bss_end - _start

reset:
	switch_el x1, 5f, 4f, 4f	/* cfg in ATF: ACTRL_EL3 disable low level access CPUECTRL*/
5:
	mrs     x0, S3_1_C15_C2_1
	orr     x0, x0, #(1 << 6)
	msr     S3_1_C15_C2_1, x0
4:
	/*
	 * Could be EL3/EL2/EL1, Initial State:
	 * Little Endian, MMU Disabled, i/dCache Disabled
	 */
	adr	x0, vectors
	switch_el x1, 3f, 2f, 1f
3:	msr	vbar_el3, x0
	msr	cptr_el3, xzr			/* Enable FP/SIMD */
	ldr	x0, =COUNTER_FREQUENCY
	msr	cntfrq_el0, x0			/* Initialize CNTFRQ */
	b	0f
2:	msr	vbar_el2, x0
	mov	x0, #0x33ff
	msr	cptr_el2, x0			/* Enable FP/SIMD */
	b	0f
1:	msr	vbar_el1, x0
	mov	x0, #3 << 20
	msr	cpacr_el1, x0			/* Enable FP/SIMD */
0:

	/* Cache/BPB/TLB Invalidate */
	bl	__asm_flush_dcache_all		/* dCache clean&invalidate */
	bl	__asm_invalidate_icache_all	/* iCache invalidate */
	bl	__asm_invalidate_tlb_all	/* invalidate TLBs */

	/* Processor specific initialization */
	bl	lowlevel_init

	branch_if_master x0, x1, master_cpu

	/*
	 * Slave CPUs
	 */
slave_cpu:
	wfe
	ldr	x1, =CPU_RELEASE_ADDR
	ldr	x0, [x1]
	cbz	x0, slave_cpu
	br	x0			/* branch to the given address */

	/*
	 * Master CPU
	 */
master_cpu:
	bl	_main

/*-----------------------------------------------------------------------*/

WEAK(lowlevel_init)
	/* Initialize GIC Secure Bank Status */
	mov	x29, lr			/* Save LR */
	bl	gic_init

	branch_if_master x0, x1, 1f

	/* enable irq in EL3 */
	mrs	x0, scr_el3
	mov	x1, #(7 << 1)
	orr	x0, x0, x1
	msr	scr_el3, x0
	isb	sy

	/*
	 * Slave should wait for master clearing spin table.
	 * This sync prevent salves observing incorrect
	 * value of spin table and jumping to wrong place.
	 */
	bl	wait_for_wakeup

	/* disable irq in EL3 */
	mrs	x0, scr_el3
	mov	x1, #~(7<<1)
	and	x0, x0, x1
	msr	scr_el3, x0
	isb	sy

	/*
	 * All processors will enter EL2 and optionally EL1.
	 */
	bl	armv8_switch_to_el2
#ifdef CONFIG_ARMV8_SWITCH_TO_EL1
	bl	armv8_switch_to_el1
#endif

1:
	mov	lr, x29			/* Restore LR */
	ret
ENDPROC(lowlevel_init)

/*-----------------------------------------------------------------------*/

ENTRY(c_runtime_cpu_setup)
	/* If I-cache is enabled invalidate it */
#ifndef CONFIG_SYS_ICACHE_OFF
	ic	iallu			/* I+BTB cache invalidate */
	isb	sy
#endif

#ifndef CONFIG_SYS_DCACHE_OFF
	/*
	 * Setup MAIR and TCR.
	 */
	ldr	x0, =MEMORY_ATTRIBUTES
	ldr	x1, =TCR_FLAGS

	switch_el x2, 3f, 2f, 1f
3:	orr	x1, x1, TCR_EL3_IPS_BITS
	msr	mair_el3, x0
	msr	tcr_el3, x1
	b	0f
2:	orr	x1, x1, TCR_EL2_IPS_BITS
	msr	mair_el2, x0
	msr	tcr_el2, x1
	b	0f
1:	orr	x1, x1, TCR_EL1_IPS_BITS
	msr	mair_el1, x0
	msr	tcr_el1, x1
0:
#endif

	/* Relocate vBAR */
	adr	x0, vectors
	switch_el x1, 3f, 2f, 1f
3:	msr	vbar_el3, x0
	b	0f
2:	msr	vbar_el2, x0
	b	0f
1:	msr	vbar_el1, x0
0:

	ret
ENDPROC(c_runtime_cpu_setup)
