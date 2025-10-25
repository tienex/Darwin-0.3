/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * Alpha Architecture Register Definitions
 */

#ifndef _ARCH_ALPHA_REG_H_
#define _ARCH_ALPHA_REG_H_

/*
 * Alpha Register Set
 */

/* Integer Registers */
#define ALPHA_NREGS		32	/* Number of integer registers */
#define ALPHA_FREGS		32	/* Number of floating-point registers */

/* Special Integer Registers */
#define ALPHA_REG_V0		0	/* Return value */
#define ALPHA_REG_T0		1	/* Temporary registers */
#define ALPHA_REG_T1		2
#define ALPHA_REG_T2		3
#define ALPHA_REG_T3		4
#define ALPHA_REG_T4		5
#define ALPHA_REG_T5		6
#define ALPHA_REG_T6		7
#define ALPHA_REG_T7		8
#define ALPHA_REG_S0		9	/* Saved registers */
#define ALPHA_REG_S1		10
#define ALPHA_REG_S2		11
#define ALPHA_REG_S3		12
#define ALPHA_REG_S4		13
#define ALPHA_REG_S5		14
#define ALPHA_REG_FP		15	/* Frame pointer */
#define ALPHA_REG_A0		16	/* Argument registers */
#define ALPHA_REG_A1		17
#define ALPHA_REG_A2		18
#define ALPHA_REG_A3		19
#define ALPHA_REG_A4		20
#define ALPHA_REG_A5		21
#define ALPHA_REG_T8		22	/* Temporary registers */
#define ALPHA_REG_T9		23
#define ALPHA_REG_T10		24
#define ALPHA_REG_T11		25
#define ALPHA_REG_RA		26	/* Return address */
#define ALPHA_REG_T12		27	/* Procedure value */
#define ALPHA_REG_AT		28	/* Assembler temporary */
#define ALPHA_REG_GP		29	/* Global pointer */
#define ALPHA_REG_SP		30	/* Stack pointer */
#define ALPHA_REG_ZERO		31	/* Always zero */

/*
 * Processor Status Register (PS)
 */
#define ALPHA_PS_CM0		0x0001	/* Current mode bit 0 */
#define ALPHA_PS_CM1		0x0002	/* Current mode bit 1 */
#define ALPHA_PS_IPL		0x001F	/* Interrupt priority level */
#define ALPHA_PS_IPL_SHIFT	0

/* Current mode values */
#define ALPHA_CM_KERNEL		0	/* Kernel mode */
#define ALPHA_CM_EXEC		1	/* Executive mode */
#define ALPHA_CM_SUPER		2	/* Supervisor mode */
#define ALPHA_CM_USER		3	/* User mode */

/*
 * Floating Point Control Register (FPCR)
 */
#define ALPHA_FPCR_INVD		0x0000020000000000UL	/* Invalid operation disable */
#define ALPHA_FPCR_DZED		0x0000040000000000UL	/* Divide by zero disable */
#define ALPHA_FPCR_OVFD		0x0000080000000000UL	/* Overflow disable */
#define ALPHA_FPCR_INV		0x0020000000000000UL	/* Invalid operation */
#define ALPHA_FPCR_DZE		0x0040000000000000UL	/* Divide by zero */
#define ALPHA_FPCR_OVF		0x0080000000000000UL	/* Overflow */
#define ALPHA_FPCR_UNF		0x0100000000000000UL	/* Underflow */
#define ALPHA_FPCR_INE		0x0200000000000000UL	/* Inexact */
#define ALPHA_FPCR_IOV		0x0400000000000000UL	/* Integer overflow */
#define ALPHA_FPCR_DYN		0x0C00000000000000UL	/* Dynamic rounding mode */

/* Rounding modes */
#define ALPHA_FPCR_DYN_CHOPPED	0x0000000000000000UL	/* Round toward zero */
#define ALPHA_FPCR_DYN_MINUS	0x0400000000000000UL	/* Round toward -infinity */
#define ALPHA_FPCR_DYN_NORMAL	0x0800000000000000UL	/* Round to nearest */
#define ALPHA_FPCR_DYN_PLUS	0x0C00000000000000UL	/* Round toward +infinity */

#ifndef __ASSEMBLER__

/*
 * Alpha saved state structure
 */
struct alpha_saved_state {
	unsigned long	v0;		/* r0: return value */
	unsigned long	t0;		/* r1-r8: temporaries */
	unsigned long	t1;
	unsigned long	t2;
	unsigned long	t3;
	unsigned long	t4;
	unsigned long	t5;
	unsigned long	t6;
	unsigned long	t7;
	unsigned long	s0;		/* r9-r14: saved registers */
	unsigned long	s1;
	unsigned long	s2;
	unsigned long	s3;
	unsigned long	s4;
	unsigned long	s5;
	unsigned long	fp;		/* r15: frame pointer */
	unsigned long	a0;		/* r16-r21: arguments */
	unsigned long	a1;
	unsigned long	a2;
	unsigned long	a3;
	unsigned long	a4;
	unsigned long	a5;
	unsigned long	t8;		/* r22-r25: temporaries */
	unsigned long	t9;
	unsigned long	t10;
	unsigned long	t11;
	unsigned long	ra;		/* r26: return address */
	unsigned long	t12;		/* r27: procedure value */
	unsigned long	at;		/* r28: assembler temp */
	unsigned long	gp;		/* r29: global pointer */
	unsigned long	sp;		/* r30: stack pointer */
	unsigned long	pc;		/* Program counter */
	unsigned long	ps;		/* Processor status */
};

/*
 * Alpha floating point saved state
 */
struct alpha_float_state {
	unsigned long	fpregs[ALPHA_FREGS];	/* f0-f31 */
	unsigned long	fpcr;			/* FP control register */
};

typedef struct alpha_saved_state	alpha_saved_state_t;
typedef struct alpha_float_state	alpha_float_state_t;

#endif /* !__ASSEMBLER__ */

#endif /* _ARCH_ALPHA_REG_H_ */
