/*
	tentry.S
*/

.macro FUNC name colon
.global \name
.type \name , %function
\name \colon
.endm

.macro DATA name colon
.global \name
.type \name , %object
\name \colon
.endm

.macro END_DATA name
.size \name , .-\name
.endm

.macro END_FUNC name
.size \name , .-\name
.endm

	.globl tos_entry

.align 2
DATA  sp_pre_ctx ,:
	.skip 12
END_DATA sp_pre_ctx

FUNC tos_entry, :
	push {r0-r12, lr}
	ldr lr, =.L_ns_ret
	mov r12, r0

	ldr   r1, =sp_pre_ctx
	mrs   r2, cpsr
	stmia r1!, {r2}
	mrs   r2, spsr
	stmia r1!, {r2}
	mov r2, sp
	stmia r1!, {r2}

	mov r0, #0
	mov r1, #0
	mov r2, #0
	mov r3, #0
	bx  r12
.L_ns_ret:
	mrc p15, 0, r12, c1, c0, 0
	bic r12, #(1<<12)
	bic r12, #(1<<2 | 1<<0)
	mcr p15, 0, r12, c1, c0, 0
	dsb
	isb

	ldr r1, =sp_pre_ctx
	ldmia r1!, {r2,r3,sp}
	msr cpsr, r2
	msr spsr, r3
	pop {r0-r12, pc}
END_FUNC tos_entry

