#ifndef __alpha_regdef_h__
#define __alpha_regdef_h__

#define v0	$0	/* function return value */

#define t0	$1	/* temporary registers (caller-saved) */
#define t1	$2
#define t2	$3
#define t3	$4
#define t4	$5
#define t5	$6
#define t6	$7
#define t7	$8

#define	s0	$9	/* saved-registers (callee-saved registers) */
#define	s1	$10
#define	s2	$11
#define	s3	$12
#define	s4	$13
#define	s5	$14
#define	s6	$15
#define	fp	s6	/* frame-pointer (s6 in frame-less procedures) */

#define a0	$16	/* argument registers (caller-saved) */
#define a1	$17
#define a2	$18
#define a3	$19
#define a4	$20
#define a5	$21

#define t8	$22	/* more temps (caller-saved) */
#define t9	$23
#define t10	$24
#define t11	$25
#define ra	$26	/* return address register */
#define t12	$27

#define pv	t12	/* procedure-variable register */
#define AT	$at	/* assembler temporary */
#define gp	$29	/* global pointer */
#define sp	$30	/* stack pointer */
#define zero	$31	/* reads as zero, writes are noops */

#define vt0	$33	/* temporary registers (caller-saved) */
#define vt1	$34
#define vt2	$35
#define vt3	$36
#define vt4	$37
#define vt5	$38
#define vt6	$39
#define vt7	$40

#define	vs0	$41	/* saved-registers (callee-saved registers) */
#define	vs1	$42
#define	vs2	$43
#define	vs3	$44
#define	vs4	$45
#define	vs5	$46
#define	vs6	$47

#define vt8	 $54	/* more temps (caller-saved) */
#define vt9	 $55
#define vt10 $56
#define vt11 $57
#define vt12 $59
#endif /* __alpha_regdef_h__ */
