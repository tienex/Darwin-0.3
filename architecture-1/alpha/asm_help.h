/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * Alpha Assembly Language Helpers
 */

#ifndef _ARCH_ALPHA_ASM_HELP_H_
#define _ARCH_ALPHA_ASM_HELP_H_

/*
 * Assembly language macros for Alpha
 */

#ifdef __ASSEMBLER__

/*
 * Procedure entry/exit macros
 */

/* Standard procedure entry */
#define ENTRY(name) \
	.text				; \
	.align 4			; \
	.globl name			; \
	.ent name			; \
name:

/* Nested procedure entry (calls other procedures) */
#define NESTED(name, framesize, ra_offset) \
	.text				; \
	.align 4			; \
	.globl name			; \
	.ent name			; \
name:					; \
	lda	sp, -framesize(sp)	; \
	stq	ra, ra_offset(sp)

/* Leaf procedure entry (doesn't call other procedures) */
#define LEAF(name) \
	.text				; \
	.align 4			; \
	.globl name			; \
	.ent name			; \
name:

/* Alternate entry point */
#define ALTENTRY(name) \
	.globl name			; \
name:

/* Procedure exit */
#define END(name) \
	.end name

/* Return from procedure */
#define RET \
	ret	zero, (ra), 1

/* Return from nested procedure */
#define RET_NESTED(framesize, ra_offset) \
	ldq	ra, ra_offset(sp)	; \
	lda	sp, framesize(sp)	; \
	ret	zero, (ra), 1

/*
 * Data definition macros
 */

/* Export a symbol */
#define EXPORT(name) \
	.globl name

/* Define a word */
#define WORD(value) \
	.long value

/* Define a quadword */
#define QUAD(value) \
	.quad value

/* ASCII string */
#define ASCIZ(string) \
	.asciz string

/*
 * Register name aliases
 */
#define v0	$0		/* Return value */
#define t0	$1		/* Temporary registers */
#define t1	$2
#define t2	$3
#define t3	$4
#define t4	$5
#define t5	$6
#define t6	$7
#define t7	$8
#define s0	$9		/* Saved registers */
#define s1	$10
#define s2	$11
#define s3	$12
#define s4	$13
#define s5	$14
#define fp	$15		/* Frame pointer */
#define a0	$16		/* Argument registers */
#define a1	$17
#define a2	$18
#define a3	$19
#define a4	$20
#define a5	$21
#define t8	$22		/* Temporary registers */
#define t9	$23
#define t10	$24
#define t11	$25
#define ra	$26		/* Return address */
#define t12	$27		/* Procedure value / temp */
#define pv	t12		/* Procedure value */
#define AT	$28		/* Assembler temporary */
#define gp	$29		/* Global pointer */
#define sp	$30		/* Stack pointer */
#define zero	$31		/* Always zero */

/*
 * Floating-point register aliases
 */
#define fv0	$f0		/* FP return value */
#define fv1	$f1
#define ft0	$f2		/* FP temporaries */
#define ft1	$f3
#define ft2	$f4
#define ft3	$f5
#define ft4	$f6
#define ft5	$f7
#define ft6	$f8
#define ft7	$f9
#define fs0	$f10		/* FP saved registers */
#define fs1	$f11
#define fs2	$f12
#define fs3	$f13
#define fs4	$f14
#define fs5	$f15
#define fa0	$f16		/* FP argument registers */
#define fa1	$f17
#define fa2	$f18
#define fa3	$f19
#define fa4	$f20
#define fa5	$f21
#define ft8	$f22		/* FP temporaries */
#define ft9	$f23
#define ft10	$f24
#define ft11	$f25
#define ft12	$f26
#define ft13	$f27
#define ft14	$f28
#define ft15	$f29
#define fzero	$f31		/* FP always zero */

/*
 * PALcode call macro
 */
#define CALL_PAL(func) \
	call_pal func

/*
 * Stack frame macros
 */
#define SAVE_REGISTERS(offset) \
	stq	s0, (0*8 + offset)(sp)	; \
	stq	s1, (1*8 + offset)(sp)	; \
	stq	s2, (2*8 + offset)(sp)	; \
	stq	s3, (3*8 + offset)(sp)	; \
	stq	s4, (4*8 + offset)(sp)	; \
	stq	s5, (5*8 + offset)(sp)	; \
	stq	fp, (6*8 + offset)(sp)

#define RESTORE_REGISTERS(offset) \
	ldq	s0, (0*8 + offset)(sp)	; \
	ldq	s1, (1*8 + offset)(sp)	; \
	ldq	s2, (2*8 + offset)(sp)	; \
	ldq	s3, (3*8 + offset)(sp)	; \
	ldq	s4, (4*8 + offset)(sp)	; \
	ldq	s5, (5*8 + offset)(sp)	; \
	ldq	fp, (6*8 + offset)(sp)

/*
 * Useful constants
 */
#define KSEG_START	0xfffffc0000000000	/* Kernel segment base */
#define K1SEG_START	0xfffffe0000000000	/* I/O segment base */

#endif /* __ASSEMBLER__ */

#endif /* _ARCH_ALPHA_ASM_HELP_H_ */
