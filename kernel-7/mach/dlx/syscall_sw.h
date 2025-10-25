/*
 * Copyright (c) 1999 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * DLX System Call Interface
 */

#ifndef _MACH_DLX_SYSCALL_SW_H_
#define _MACH_DLX_SYSCALL_SW_H_

/*
 * System call numbers
 */
#define SYSCALL_TRAP_NUMBER	0x100

/*
 * System call macros for user space
 */
#define kernel_trap(trap_name, trap_number, arg_count) \
	.globl trap_name; \
trap_name:; \
	li r1, trap_number; \
	trap; \
	jr r31

#endif /* _MACH_DLX_SYSCALL_SW_H_ */
