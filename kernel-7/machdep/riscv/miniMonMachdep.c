/*
 * Copyright (c) 1999 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * Portions Copyright (c) 1999 Apple Computer, Inc.  All Rights
 * Reserved.  This file contains Original Code and/or Modifications of
 * Original Code as defined in and that are subject to the Apple Public
 * Source License Version 1.1 (the "License").  You may not use this file
 * except in compliance with the License.  Please obtain a copy of the
 * License at http://www.apple.com/publicsource and read it before using
 * this file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON- INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */

/*
 * RISC-V miniMonMachdep.c -- Machine-dependent code for mini-monitor
 */

#include <sys/reboot.h>
#include <kern/miniMonPrivate.h>
#include <kern/thread.h>
#include <machdep/riscv/trap.h>

#define N_BACKTRACE     6       /* Number of backtrace frames to print */

/*
 * Perform a stack backtrace
 */
static boolean_t
miniMonBacktrace(char *line)
{
	extern void *miniMonState;
	struct trapframe *tf;
	unsigned long *fp, pc, ra;
	int count = N_BACKTRACE;
	int i;

	/* Parse count from command line if provided */
	while (*line && (*line != ' ' && *line != '\t'))
		line++;
	while (*line && (*line == ' ' || *line == '\t'))
		line++;
	if (*line >= '0' && *line <= '9') {
		count = 0;
		while (*line >= '0' && *line <= '9') {
			count = count * 10 + (*line - '0');
			line++;
		}
	}

	printf("State pointer = 0x%lx\n", (unsigned long)miniMonState);

	if (miniMonState) {
		tf = (struct trapframe *)miniMonState;
		pc = tf->pc;
		fp = (unsigned long *)tf->s0;  /* Frame pointer (s0/x8) */

		printf("Backtrace:\n");
		printf("  PC: 0x%016lx\n", pc);

		/* Walk the stack frames */
		for (i = 0; i < count && fp != NULL; i++) {
			ra = *(fp - 1);  /* Return address is at fp[-1] */
			printf("  [%d] FP: 0x%016lx  RA: 0x%016lx\n",
			       i, (unsigned long)fp, ra);

			/* Move to previous frame */
			fp = (unsigned long *)*(fp - 2);  /* Previous FP is at fp[-2] */
		}
	} else {
		printf("No saved state available\n");
	}

	return TRUE;
}

/*
 * Helper: check if character is hex digit
 */
static boolean_t
ishex(char c)
{
	return ((c >= '0' && c <= '9') ||
	        (c >= 'a' && c <= 'f') ||
	        (c >= 'A' && c <= 'F'));
}

/*
 * Helper: convert hex character to value
 */
static int
xx(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	return 0;
}

/*
 * Helper: convert hex string to integer
 */
static unsigned long
xtoi(char *str)
{
	unsigned long num = 0;

	while (*str && ishex(*str)) {
		num = (16 * num) + xx(*str);
		str++;
	}
	return num;
}

/*
 * Dump memory contents
 */
static boolean_t
miniMonDump(char *line)
{
	unsigned long addr, count, i, j;
	unsigned char *p;

	/* Skip command name */
	while (*line && (*line != ' ' && *line != '\t'))
		line++;
	while (*line && (*line == ' ' || *line == '\t'))
		line++;

	/* Get address */
	if (!*line || !ishex(*line)) {
		printf("Usage: dump <address> [count]\n");
		return TRUE;
	}
	addr = xtoi(line);

	/* Skip address */
	while (*line && ishex(*line))
		line++;
	while (*line && (*line == ' ' || *line == '\t'))
		line++;

	/* Get count (default 256 bytes) */
	count = (*line && ishex(*line)) ? xtoi(line) : 256;

	/* Dump memory */
	p = (unsigned char *)addr;
	for (i = 0; i < count; i += 16) {
		printf("%016lx: ", addr + i);

		/* Hex dump */
		for (j = 0; j < 16 && (i + j) < count; j++) {
			printf("%02x ", p[i + j]);
			if (j == 7) printf(" ");
		}

		/* Padding */
		for (; j < 16; j++) {
			printf("   ");
			if (j == 7) printf(" ");
		}

		/* ASCII dump */
		printf(" |");
		for (j = 0; j < 16 && (i + j) < count; j++) {
			char c = p[i + j];
			printf("%c", (c >= 32 && c < 127) ? c : '.');
		}
		printf("|\n");
	}

	return TRUE;
}

/*
 * Display register state
 */
static boolean_t
miniMonRegisters(char *line)
{
	extern void *miniMonState;
	struct trapframe *tf;

	if (!miniMonState) {
		printf("No saved state available\n");
		return TRUE;
	}

	tf = (struct trapframe *)miniMonState;

	printf("RISC-V Register State:\n");
	printf("  pc:  0x%016lx  ra:  0x%016lx  sp:  0x%016lx\n",
	       tf->pc, tf->ra, tf->sp);
	printf("  gp:  0x%016lx  tp:  0x%016lx\n",
	       tf->gp, tf->tp);
	printf("  t0:  0x%016lx  t1:  0x%016lx  t2:  0x%016lx\n",
	       tf->t0, tf->t1, tf->t2);
	printf("  s0:  0x%016lx  s1:  0x%016lx\n",
	       tf->s0, tf->s1);
	printf("  a0:  0x%016lx  a1:  0x%016lx  a2:  0x%016lx\n",
	       tf->a0, tf->a1, tf->a2);
	printf("  a3:  0x%016lx  a4:  0x%016lx  a5:  0x%016lx\n",
	       tf->a3, tf->a4, tf->a5);
	printf("  a6:  0x%016lx  a7:  0x%016lx\n",
	       tf->a6, tf->a7);
	printf("  s2:  0x%016lx  s3:  0x%016lx  s4:  0x%016lx\n",
	       tf->s2, tf->s3, tf->s4);
	printf("  s5:  0x%016lx  s6:  0x%016lx  s7:  0x%016lx\n",
	       tf->s5, tf->s6, tf->s7);
	printf("  s8:  0x%016lx  s9:  0x%016lx  s10: 0x%016lx\n",
	       tf->s8, tf->s9, tf->s10);
	printf("  s11: 0x%016lx\n", tf->s11);
	printf("  t3:  0x%016lx  t4:  0x%016lx  t5:  0x%016lx\n",
	       tf->t3, tf->t4, tf->t5);
	printf("  t6:  0x%016lx\n", tf->t6);
	printf("\n");
	printf("  cause:  0x%016lx  tval:   0x%016lx\n",
	       tf->cause, tf->tval);
	printf("  status: 0x%016lx\n", tf->status);

	return TRUE;
}

/*
 * Machine-dependent mini-monitor commands
 */
miniMonCommandStruct machDep_cmds[] = {
	{ "bt", miniMonBacktrace, "bt [count] - stack backtrace" },
	{ "dump", miniMonDump, "dump <addr> [count] - memory dump" },
	{ "regs", miniMonRegisters, "regs - display registers" },
	{ (char *)0, (miniMonFunc)0, (char *)0 }
};
