
.macro FUNC name colon
.global \name
.type \name , %function
\name \colon
.endm

.macro END_FUNC name
.size \name , .-\name
.endm

    .globl tee_smc_call

FUNC tee_smc_call, :
	push {r0-r12, lr}
	mov r4, r0
	ldr r0, [r4]
	ldr r1, [r4,#4]
	ldr r2, [r4,#8]
	ldr r3, [r4,#12]
	.arch_extension sec
	smc #0
	str r0, [r4]
	pop {r0-r12, pc}
END_FUNC tee_smc_call


