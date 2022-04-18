	.set noreorder
	.set volatile
	.set noat
	.set nomacro
	.arch sws
	.set sws_start
### Optimization level and target	: -O2 -mslave  -msimd 
	.section	.text1.slave_dummy,"ax",@progbits
	.align 2
	.align 4
	.globl slave_dummy
	.ent slave_dummy
slave_dummy:
	.frame $30,0,$26,0
$LFB246:
	.cfi_startproc
	ldih $29,0($27)		!gpdisp!1
	ldi $29,0($29)		!gpdisp!1
$slave_dummy..ng:
	.prologue 1
	ldl $1,v8($29)		!literal
	vldw_u $34,0($16)
	vldw_u $35,64+0($16)
	ldi $2,0($16)
	vconw $35,$34,$2,$34
	vstd $34,0($1)
	ret $31,($26),1
	.cfi_endproc
$LFE246:
	.end slave_dummy
	.comm	v8,64,64
	.ident	"GCC: (GNU) 7.1.0 20170502 (swgcc-1079 Committer: qianh <qianh@example.com> by comp9 at localhost.localdomain on Thu Dec 24 21:36:33 CST 2020)"
	.section	.note.GNU-stack,"",@progbits
