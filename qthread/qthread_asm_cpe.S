	.set noreorder
	.set volatile
	.set noat
#ifdef __sw_thl__
#define ARCH sw3
#define VALIGN 5
#define VSIZE 32
#else
#define ARCH sws
#define VALIGN 6
#define VSIZE 64
#endif
	.arch ARCH
#include "regdef.h"
	.section .ldm
	.org 0x20
	.align	VALIGN
	.globl	__qstat
	.type	__qstat, @object
	.size	__qstat, VSIZE
    #.globl slave_qthread_init
__qstat:	# 0x20
	.skip VSIZE

	.section .text1, "ax"
	.align 4
	.ent	qthread_boot_from_athread
	.globl	qthread_boot_from_athread
qthread_boot_from_athread:
    .prologue
    #ldi   sp, 0xff($31)
#ifdef __sw_thl__
    sll   sp, 8, sp
#endif
#ifdef __sw_ocn__
    #sll   sp, 10, sp
#endif
    ldl   t12, qthread_init(gp) !literal!1
    call  ra, (t12), 0x0 !lituse_jsr!1
    // .cfi_endproc
    .end qthread_boot_from_athread

	.ent	qthread_waiting_for_task
	.globl	qthread_waiting_for_task
#ifdef __sw_thl__
	.ent	qthread_waiting_for_task
	.globl	qthread_waiting_for_task
qthread_waiting_for_task:
    ldih    s0,__qstat(zero) !ldmhigh
    ldi     s0,__qstat(s0)   !ldmlow
    ldi     s2,0xff(zero)
    rcsr    s1, 0
    and     s1, 63, s1
    and     s1, 7, s3
qthread_next_task:
    bne     s1, qthread_row_master
qthread_wait_pc:
    vldd    t12, 0(s0)
    beq     t12, qthread_wait_pc
    putc    t12, 8
    putr    t12, 8
    br      $31, qthread_run_task
qthread_row_master:
    bne     s3, qthread_row_recv
    getc    t12
    putr    t12, 8
    br      $31, qthread_run_task
qthread_row_recv:
    getr    t12
qthread_run_task:
    vextf   t12, 1, a0
    rcsr    s4, 4
    synr    s2
    sync    s2
    call    ra, (t12), 0
    synr    s2
    sync    s2
    rcsr    s5, 4
    subl    s5, s4, s5
    bne     s1, qthread_fini_task
    stl     s5, 8(s0)
    stl     $31, 0(s0)
qthread_fini_task:
    br      $31, qthread_next_task
    ret
#elif defined(__sw_ocn__)
qthread_waiting_for_task:
	#s0: where shall I store/read vt12? reply: 16(s0)
	#s1: _MYID
	#s2: sync mask
    mov     sp, s5
    ldi     t0, 0x3f
    not     t0, t0
    and     sp, t0, sp
    ldi     sp, -64(sp)	
    ldih    s0,__qstat(zero) !ldmhigh
    ldi     s0,__qstat(s0)   !ldmlow
    ldi     s2,0xff(zero)
    stl     $31, 16(s0)
    sync    s2
    synr    s2
    rcsr    s1, 0
    and     s1, 63, s1
    beq     s1, qthread_init_desc
    br      qthread_recv
qthread_init_desc:	
    ldi     $32, 16(s0)
    vinsw   $32, $32, 3, $32
    ldih    $32, -256($32)
    vinsw   $32, $32, 2, $32
    ldi     $32, 1($31)
    sll     $32, 60, $32
    or      $32, 16, $32
    vstd    $32, 0(sp)
qthread_wait_pc:
    vldd    vt12, 0(s0)
    vldd    $32, 0(sp)
    beq     vt12, qthread_wait_pc
    rma     $32, s0, s0
qthread_recv:
    ldw     t0, 16(s0)
    ldl     t12, 0(s0)
    ldl     a0, 8(s0)
    beq     t0, qthread_recv
qthread_run_task:
    call    ra, (t12), 0
    stl     $31, 16(s0)
    synr    s2
    sync    s2
    bne     s1, qthread_recv
    stl     $31, 0(s0)
qthread_fini_task:
    br      $31, qthread_wait_pc
    mov     s5, sp
    ldi     sp, 64(sp)
    ret
#endif
    .end    qthread_waiting_for_task
	.section .text1
	.align 4
	.ent	qthread_dummy
	.globl	qthread_dummy
qthread_dummy:
    ret
    .end qthread_dummy
