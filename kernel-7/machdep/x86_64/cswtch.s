/*
 * x86-64 Context Switching
 *
 * This file contains the low-level context switching code for x86-64.
 * Context switching involves saving the current thread's state and
 * restoring another thread's state.
 */

#include <architecture/x86_64/asm_help.h>

	.text
	.code64

/*
 * Thread context structure (matches x86_64_thread_state_t)
 * Offsets into the thread state structure
 */
#define THREAD_RAX	0
#define THREAD_RBX	8
#define THREAD_RCX	16
#define THREAD_RDX	24
#define THREAD_RDI	32
#define THREAD_RSI	40
#define THREAD_RBP	48
#define THREAD_RSP	56
#define THREAD_R8	64
#define THREAD_R9	72
#define THREAD_R10	80
#define THREAD_R11	88
#define THREAD_R12	96
#define THREAD_R13	104
#define THREAD_R14	112
#define THREAD_R15	120
#define THREAD_RIP	128
#define THREAD_RFLAGS	136
#define THREAD_CS	144
#define THREAD_FS	152
#define THREAD_GS	160

/*
 * switch_context - Perform a context switch
 *
 * void switch_context(thread_state_t *old, thread_state_t *new)
 *
 * Saves current context to 'old' and loads context from 'new'
 *
 * Arguments:
 *   rdi = pointer to old thread state (save here)
 *   rsi = pointer to new thread state (load from here)
 */
	.globl _switch_context
_switch_context:
	/* Save old thread state */

	/* Save general purpose registers */
	movq	%rax, THREAD_RAX(%rdi)
	movq	%rbx, THREAD_RBX(%rdi)
	movq	%rcx, THREAD_RCX(%rdi)
	movq	%rdx, THREAD_RDX(%rdi)
	movq	%rsi, THREAD_RSI(%rdi)	/* Save rsi before we overwrite it */
	movq	%rbp, THREAD_RBP(%rdi)
	movq	%rsp, THREAD_RSP(%rdi)
	movq	%r8, THREAD_R8(%rdi)
	movq	%r9, THREAD_R9(%rdi)
	movq	%r10, THREAD_R10(%rdi)
	movq	%r11, THREAD_R11(%rdi)
	movq	%r12, THREAD_R12(%rdi)
	movq	%r13, THREAD_R13(%rdi)
	movq	%r14, THREAD_R14(%rdi)
	movq	%r15, THREAD_R15(%rdi)

	/* Save rdi last since we're using it as base pointer */
	movq	%rdi, THREAD_RDI(%rdi)

	/* Save return address as RIP */
	movq	(%rsp), %rax
	movq	%rax, THREAD_RIP(%rdi)

	/* Save RFLAGS */
	pushfq
	popq	%rax
	movq	%rax, THREAD_RFLAGS(%rdi)

	/* Load new thread state */

	/* Restore general purpose registers */
	movq	THREAD_RAX(%rsi), %rax
	movq	THREAD_RBX(%rsi), %rbx
	movq	THREAD_RCX(%rsi), %rcx
	movq	THREAD_RDX(%rsi), %rdx
	movq	THREAD_RBP(%rsi), %rbp
	movq	THREAD_RSP(%rsi), %rsp
	movq	THREAD_R8(%rsi), %r8
	movq	THREAD_R9(%rsi), %r9
	movq	THREAD_R10(%rsi), %r10
	movq	THREAD_R11(%rsi), %r11
	movq	THREAD_R12(%rsi), %r12
	movq	THREAD_R13(%rsi), %r13
	movq	THREAD_R14(%rsi), %r14
	movq	THREAD_R15(%rsi), %r15

	/* Restore RFLAGS */
	movq	THREAD_RFLAGS(%rsi), %r11
	pushq	%r11
	popfq

	/* Set up return address from RIP */
	movq	THREAD_RIP(%rsi), %r11
	movq	%r11, (%rsp)

	/* Restore rdi and rsi last */
	movq	THREAD_RDI(%rsi), %rdi
	movq	THREAD_RSI(%rsi), %rsi

	/* Return to new thread */
	ret

/*
 * load_context - Load a new context without saving
 *
 * void load_context(thread_state_t *new)
 *
 * Loads context from 'new' without saving current context.
 * This is used for initial thread startup.
 *
 * Arguments:
 *   rdi = pointer to new thread state
 */
	.globl _load_context
_load_context:
	/* Load all registers from thread state */
	movq	THREAD_RAX(%rdi), %rax
	movq	THREAD_RBX(%rdi), %rbx
	movq	THREAD_RCX(%rdi), %rcx
	movq	THREAD_RDX(%rdi), %rdx
	movq	THREAD_RSI(%rdi), %rsi
	movq	THREAD_RBP(%rdi), %rbp
	movq	THREAD_RSP(%rdi), %rsp
	movq	THREAD_R8(%rdi), %r8
	movq	THREAD_R9(%rdi), %r9
	movq	THREAD_R10(%rdi), %r10
	movq	THREAD_R11(%rdi), %r11
	movq	THREAD_R12(%rdi), %r12
	movq	THREAD_R13(%rdi), %r13
	movq	THREAD_R14(%rdi), %r14
	movq	THREAD_R15(%rdi), %r15

	/* Load RFLAGS */
	movq	THREAD_RFLAGS(%rdi), %r11
	pushq	%r11
	popfq

	/* Load RIP onto stack for ret */
	movq	THREAD_RIP(%rdi), %r11
	pushq	%r11

	/* Load rdi last */
	movq	THREAD_RDI(%rdi), %rdi

	/* Jump to new thread */
	ret

/*
 * save_context - Save current context
 *
 * void save_context(thread_state_t *state)
 *
 * Saves current context to 'state'
 *
 * Arguments:
 *   rdi = pointer to thread state (save here)
 */
	.globl _save_context
_save_context:
	/* Save general purpose registers */
	movq	%rax, THREAD_RAX(%rdi)
	movq	%rbx, THREAD_RBX(%rdi)
	movq	%rcx, THREAD_RCX(%rdi)
	movq	%rdx, THREAD_RDX(%rdi)
	movq	%rsi, THREAD_RSI(%rdi)
	movq	%rbp, THREAD_RBP(%rdi)
	movq	%rsp, THREAD_RSP(%rdi)
	movq	%r8, THREAD_R8(%rdi)
	movq	%r9, THREAD_R9(%rdi)
	movq	%r10, THREAD_R10(%rdi)
	movq	%r11, THREAD_R11(%rdi)
	movq	%r12, THREAD_R12(%rdi)
	movq	%r13, THREAD_R13(%rdi)
	movq	%r14, THREAD_R14(%rdi)
	movq	%r15, THREAD_R15(%rdi)
	movq	%rdi, THREAD_RDI(%rdi)

	/* Save return address as RIP */
	movq	(%rsp), %rax
	movq	%rax, THREAD_RIP(%rdi)

	/* Save RFLAGS */
	pushfq
	popq	%rax
	movq	%rax, THREAD_RFLAGS(%rdi)

	ret

/*
 * Thread entry point wrapper
 * This is called when a new thread is created
 */
	.globl _thread_bootstrap
_thread_bootstrap:
	/* Enable interrupts for user threads */
	sti

	/* Call thread function */
	/* Function pointer in r12, argument in r13 */
	movq	%r13, %rdi
	call	*%r12

	/* Thread function returned, terminate thread */
	call	_thread_terminate

	/* Should never get here */
	hlt

/*
 * Return to user mode
 * Sets up stack frame and returns to user space
 */
	.globl _return_to_user
_return_to_user:
	/* rdi points to user thread state */

	/* Push user SS and RSP */
	pushq	$0x23		/* User data selector (RPL=3) */
	movq	THREAD_RSP(%rdi), %rax
	pushq	%rax

	/* Push RFLAGS */
	movq	THREAD_RFLAGS(%rdi), %rax
	orq	$0x200, %rax	/* Ensure IF (interrupt flag) is set */
	pushq	%rax

	/* Push user CS and RIP */
	pushq	$0x1B		/* User code selector (RPL=3) */
	movq	THREAD_RIP(%rdi), %rax
	pushq	%rax

	/* Load user registers */
	movq	THREAD_RAX(%rdi), %rax
	movq	THREAD_RBX(%rdi), %rbx
	movq	THREAD_RCX(%rdi), %rcx
	movq	THREAD_RDX(%rdi), %rdx
	movq	THREAD_RSI(%rdi), %rsi
	movq	THREAD_RBP(%rdi), %rbp
	movq	THREAD_R8(%rdi), %r8
	movq	THREAD_R9(%rdi), %r9
	movq	THREAD_R10(%rdi), %r10
	movq	THREAD_R11(%rdi), %r11
	movq	THREAD_R12(%rdi), %r12
	movq	THREAD_R13(%rdi), %r13
	movq	THREAD_R14(%rdi), %r14
	movq	THREAD_R15(%rdi), %r15
	movq	THREAD_RDI(%rdi), %rdi

	/* Return to user mode */
	iretq

/*
 * FPU/SSE context switching helpers
 */
	.globl _fpu_save
_fpu_save:
	fxsave64	(%rdi)
	ret

	.globl _fpu_restore
_fpu_restore:
	fxrstor64	(%rdi)
	ret

	.globl _fpu_init
_fpu_init:
	fninit
	ret
