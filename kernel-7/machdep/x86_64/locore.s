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
 * x86-64 Low-level assembly routines
 *
 * This file contains critical low-level assembly code including:
 * - Exception and interrupt entry points
 * - System call entry
 * - Low-level utility functions
 */

#include <architecture/x86_64/asm_help.h>

	.text
	.code64

/*
 * Exception entry point macro
 * Pushes registers and calls C handler
 */
#define EXCEPTION_ENTRY(name, has_error_code) \
	.globl _##name ;\
_##name: ;\
	.if has_error_code == 0 ;\
		pushq	$0 ;\
	.endif ;\
	pushq	%rax ;\
	pushq	%rbx ;\
	pushq	%rcx ;\
	pushq	%rdx ;\
	pushq	%rsi ;\
	pushq	%rdi ;\
	pushq	%rbp ;\
	pushq	%r8 ;\
	pushq	%r9 ;\
	pushq	%r10 ;\
	pushq	%r11 ;\
	pushq	%r12 ;\
	pushq	%r13 ;\
	pushq	%r14 ;\
	pushq	%r15 ;\
	movq	%rsp, %rdi ;\
	call	_trap_handler ;\
	jmp	_exception_return

/*
 * Exception handlers
 */
EXCEPTION_ENTRY(divide_error, 0)
EXCEPTION_ENTRY(debug_trap, 0)
EXCEPTION_ENTRY(nmi, 0)
EXCEPTION_ENTRY(breakpoint, 0)
EXCEPTION_ENTRY(overflow, 0)
EXCEPTION_ENTRY(bounds_check, 0)
EXCEPTION_ENTRY(invalid_opcode, 0)
EXCEPTION_ENTRY(device_not_available, 0)
EXCEPTION_ENTRY(double_fault, 1)
EXCEPTION_ENTRY(invalid_tss, 1)
EXCEPTION_ENTRY(segment_not_present, 1)
EXCEPTION_ENTRY(stack_fault, 1)
EXCEPTION_ENTRY(general_protection, 1)
EXCEPTION_ENTRY(page_fault, 1)
EXCEPTION_ENTRY(fpu_error, 0)
EXCEPTION_ENTRY(alignment_check, 1)
EXCEPTION_ENTRY(machine_check, 0)
EXCEPTION_ENTRY(simd_error, 0)

/*
 * Return from exception
 * Restores all registers and returns to interrupted context
 */
	.globl _exception_return
_exception_return:
	popq	%r15
	popq	%r14
	popq	%r13
	popq	%r12
	popq	%r11
	popq	%r10
	popq	%r9
	popq	%r8
	popq	%rbp
	popq	%rdi
	popq	%rsi
	popq	%rdx
	popq	%rcx
	popq	%rbx
	popq	%rax
	addq	$8, %rsp	/* Skip error code */
	iretq

/*
 * System call entry point
 * Uses syscall instruction (fast system call)
 *
 * On entry:
 *   rax = syscall number
 *   rdi, rsi, rdx, r10, r8, r9 = arguments (note: rcx and r11 saved by syscall)
 *   rcx = return RIP (saved by syscall)
 *   r11 = RFLAGS (saved by syscall)
 */
	.globl _syscall_entry
_syscall_entry:
	/* Save user stack pointer */
	swapgs			/* Get kernel GS base */
	movq	%rsp, %gs:8	/* Save user RSP */
	movq	%gs:0, %rsp	/* Load kernel stack */

	/* Build trap frame */
	pushq	%r11		/* RFLAGS */
	pushq	%rcx		/* RIP */
	pushq	%rbp
	pushq	%rdi
	pushq	%rsi
	pushq	%rdx
	pushq	%r10
	pushq	%r8
	pushq	%r9
	pushq	%rax

	/* Call syscall handler */
	movq	%rsp, %rdi
	call	_syscall_handler

	/* Restore registers */
	popq	%rax		/* Return value already in rax */
	popq	%r9
	popq	%r8
	popq	%r10
	popq	%rdx
	popq	%rsi
	popq	%rdi
	popq	%rbp
	popq	%rcx		/* RIP */
	popq	%r11		/* RFLAGS */

	/* Restore user stack and return */
	movq	%gs:8, %rsp
	swapgs
	sysretq

/*
 * Load CR3 (page table base register)
 */
	.globl _load_cr3
_load_cr3:
	movq	%rdi, %cr3
	ret

/*
 * Get CR3
 */
	.globl _get_cr3
_get_cr3:
	movq	%cr3, %rax
	ret

/*
 * Get CR2 (page fault linear address)
 */
	.globl _get_cr2
_get_cr2:
	movq	%cr2, %rax
	ret

/*
 * Flush TLB (Translation Lookaside Buffer)
 */
	.globl _tlb_flush
_tlb_flush:
	movq	%cr3, %rax
	movq	%rax, %cr3
	ret

/*
 * Invalidate TLB entry for a specific address
 */
	.globl _tlb_flush_addr
_tlb_flush_addr:
	invlpg	(%rdi)
	ret

/*
 * Read MSR (Model Specific Register)
 * Input: rdi = MSR number
 * Output: rax = MSR value
 */
	.globl _rdmsr64
_rdmsr64:
	movq	%rdi, %rcx
	rdmsr
	shlq	$32, %rdx
	orq	%rdx, %rax
	ret

/*
 * Write MSR
 * Input: rdi = MSR number, rsi = value
 */
	.globl _wrmsr64
_wrmsr64:
	movq	%rdi, %rcx
	movq	%rsi, %rax
	movq	%rsi, %rdx
	shrq	$32, %rdx
	wrmsr
	ret

/*
 * Enable interrupts
 */
	.globl _sti
_sti:
	sti
	ret

/*
 * Disable interrupts
 */
	.globl _cli
_cli:
	cli
	ret

/*
 * Halt CPU
 */
	.globl _hlt
_hlt:
	hlt
	ret

/*
 * CPU pause (for spinlocks)
 */
	.globl _cpu_pause
_cpu_pause:
	pause
	ret

/*
 * Memory barrier
 */
	.globl _mfence
_mfence:
	mfence
	ret

/*
 * Load task register
 */
	.globl _ltr
_ltr:
	ltr	%di
	ret

/*
 * Store task register
 */
	.globl _str
_str:
	str	%ax
	ret

/*
 * CPUID instruction wrapper
 * Input: rdi = leaf, rsi = subleaf
 * Output: Fills structure pointed to by rdx with eax,ebx,ecx,edx
 */
	.globl _cpuid
_cpuid:
	pushq	%rbx
	movq	%rdi, %rax
	movq	%rsi, %rcx
	cpuid
	movl	%eax, 0(%rdx)
	movl	%ebx, 4(%rdx)
	movl	%ecx, 8(%rdx)
	movl	%edx, 12(%rdx)
	popq	%rbx
	ret

/*
 * Read timestamp counter
 */
	.globl _rdtsc
_rdtsc:
	rdtsc
	shlq	$32, %rdx
	orq	%rdx, %rax
	ret

/*
 * I/O port operations
 */
	.globl _inb
_inb:
	movq	%rdi, %rdx
	xorq	%rax, %rax
	inb	%dx, %al
	ret

	.globl _inw
_inw:
	movq	%rdi, %rdx
	xorq	%rax, %rax
	inw	%dx, %ax
	ret

	.globl _inl
_inl:
	movq	%rdi, %rdx
	xorq	%rax, %rax
	inl	%dx, %eax
	ret

	.globl _outb
_outb:
	movq	%rdi, %rdx
	movq	%rsi, %rax
	outb	%al, %dx
	ret

	.globl _outw
_outw:
	movq	%rdi, %rdx
	movq	%rsi, %rax
	outw	%ax, %dx
	ret

	.globl _outl
_outl:
	movq	%rdi, %rdx
	movq	%rsi, %rax
	outl	%eax, %dx
	ret

/*
 * FPU/SSE state save/restore
 */
	.globl _fxsave64
_fxsave64:
	fxsave64	(%rdi)
	ret

	.globl _fxrstor64
_fxrstor64:
	fxrstor64	(%rdi)
	ret

	.globl _xsave
_xsave:
	movq	%rsi, %rax
	movq	%rsi, %rdx
	shrq	$32, %rdx
	xsave	(%rdi)
	ret

	.globl _xrstor
_xrstor:
	movq	%rsi, %rax
	movq	%rsi, %rdx
	shrq	$32, %rdx
	xrstor	(%rdi)
	ret
