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
 * x86-64 Kernel startup routine.
 *
 * Entry point for the x86-64 kernel.
 * Assumes bootloader has set up basic 64-bit environment.
 */

#include <architecture/x86_64/asm_help.h>

	.text
	.code64

	/*
	 * Kernel entry point for x86-64.
	 * Control is transferred here from the boot loader.
	 * Machine is in long mode (64-bit), with basic paging enabled.
	 *
	 * The bootloader should have:
	 * - Enabled long mode (EFER.LME = 1)
	 * - Set up identity-mapped pages for the kernel
	 * - Loaded the kernel at the appropriate address
	 */

	.globl _start
	.globl start
_start:
start:
	/* Clear direction flag (C calling convention) */
	cld

	/* Save bootloader parameters if any */
	/* rdi, rsi, rdx may contain boot parameters */
	movq	%rdi, %r12	/* Save first parameter */
	movq	%rsi, %r13	/* Save second parameter */
	movq	%rdx, %r14	/* Save third parameter */

	/* Initialize stack pointer to a safe initial stack */
	leaq	_intstack_top(%rip), %rsp

	/* Ensure we're running with interrupts disabled */
	cli

	/* Initialize GDT for 64-bit mode */
	call	_gdt_init

	/* Load new GDT */
	leaq	_gdt_desc(%rip), %rax
	lgdt	(%rax)

	/* Reload segment registers */
	movw	$0x10, %ax	/* Kernel data segment selector */
	movw	%ax, %ds
	movw	%ax, %es
	movw	%ax, %ss
	xorw	%ax, %ax	/* FS and GS not used initially */
	movw	%ax, %fs
	movw	%ax, %gs

	/* Far jump to reload CS with kernel code selector */
	pushq	$0x08		/* Kernel code segment selector */
	leaq	start64(%rip), %rax
	pushq	%rax
	lretq

start64:
	/* Now running with proper 64-bit segments */

	/* Initialize IDT */
	call	_idt_init

	/* Load IDT */
	leaq	_idt_desc(%rip), %rax
	lidt	(%rax)

	/* Restore boot parameters before calling C code */
	movq	%r12, %rdi
	movq	%r13, %rsi
	movq	%r14, %rdx

	/* Call x86-64 initialization routine */
	call	_x86_64_init

	/* Should never return, but just in case... */
halt_loop:
	hlt
	jmp	halt_loop

	/*
	 * GDT and IDT descriptors
	 */
	.data
	.align	8

	.globl _gdt_desc
_gdt_desc:
	.word	_gdt_end - _gdt - 1	/* Limit */
	.quad	_gdt			/* Base address */

	.globl _idt_desc
_idt_desc:
	.word	_idt_end - _idt - 1	/* Limit */
	.quad	_idt			/* Base address */

	/*
	 * Initial interrupt stack
	 */
	.section .bss
	.align	16
	.globl _intstack
_intstack:
	.space	16384			/* 16KB initial stack */
	.globl _intstack_top
_intstack_top:

	.text

	/*
	 * Enter 64-bit long mode from 32-bit compatibility mode
	 * This is used if the bootloader doesn't set up long mode
	 * (Currently not called, but provided for completeness)
	 */
	.code32
	.globl _enter_long_mode
_enter_long_mode:
	/* Enable PAE (Physical Address Extension) */
	movl	%cr4, %eax
	orl	$0x20, %eax		/* CR4.PAE = 1 */
	movl	%eax, %cr4

	/* Load CR3 with PML4 base */
	movl	$_pml4_base, %eax
	movl	%eax, %cr3

	/* Enable long mode in EFER MSR */
	movl	$0xC0000080, %ecx	/* EFER MSR */
	rdmsr
	orl	$0x100, %eax		/* EFER.LME = 1 */
	wrmsr

	/* Enable paging and protection */
	movl	%cr0, %eax
	orl	$0x80000001, %eax	/* CR0.PG = 1, CR0.PE = 1 */
	movl	%eax, %cr0

	/* Now in compatibility mode, need to jump to 64-bit code */
	ljmp	$0x08, $_start

	.code64
