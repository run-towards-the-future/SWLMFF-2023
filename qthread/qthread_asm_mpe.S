	.set noreorder
	.set noat
    .section .text
	.comm	__qstat_addr, 8, 8
	.globl slave_qthread_init
#include "regdef.h"
	.align 4
	.ent	qthread_wait_cpe_init
	.globl	qthread_wait_cpe_init
qthread_wait_cpe_init:
	.prologue
	ldih gp, 0(t12) !gpdisp!1
	ldi  gp, 0(gp) !gpdisp!1
	ldl t0, __qstat_addr(gp) !literal
wait_cpe_ready:
	ldl t1,0(t0)
	beq t1, wait_cpe_ready
	ret
	.end qthread_wait_cpe_init

	.align 4
	.ent	qthread_spawn
	.globl	qthread_spawn
qthread_spawn:
	.prologue
	ldih gp, 0(t12) !gpdisp!3
	ldi  gp, 0(gp) !gpdisp!3
	ldl t0, __qstat_addr(gp) !literal
	ldl t0, 0(t0)
	stl a1, 8(t0)
	stl a0, 0(t0)
	ldi v0, 0(t0)
	ret
	.end qthread_spawn

	.align 4
	.ent	qthread_join
	.globl	qthread_join
qthread_join:
	.prologue
	ldih gp, 0(t12) !gpdisp!2
	ldi  gp, 0(gp)  !gpdisp!2
	ldl t0, __qstat_addr(gp) !literal
	ldl t0, 0(t0)
wait_cpe_done:
	ldl t1, 0(t0)
        memb
	bne t1, wait_cpe_done
	ret
	.end qthread_join
