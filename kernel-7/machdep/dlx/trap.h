/*
 * Copyright (c) 1999 Apple Computer, Inc. All rights reserved.
 *
 * DLX Trap Numbers and Definitions
 */

#ifndef _DLX_TRAP_H_
#define _DLX_TRAP_H_

/* Exception/trap numbers */
#define TRAP_ILLEGALINST	0x1
#define TRAP_ADDRESS		0x2
#define TRAP_ACCESS		0x3
#define TRAP_OVERFLOW		0x4
#define TRAP_DIV0		0x5
#define TRAP_PRIVILEGE		0x6
#define TRAP_FORMAT		0x7
#define TRAP_PAGEFAULT		0x20
#define TRAP_TLBFAULT		0x30
#define TRAP_TIMER		0x40
#define TRAP_KBD		0x48

/* Function prototypes */
void dlx_pagefault_handler(vm_offset_t va, int is_write);
void dlx_tlbfault_handler(vm_offset_t va, int is_write);
void dlx_exception_handler(unsigned int status, vm_offset_t fault_addr);
void syscall_handler(void);

#endif /* _DLX_TRAP_H_ */
