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
 * MIPS:	Machine-dependent system calls
 *
 * Implements MIPS-specific system calls.
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/proc.h>
#include <sys/user.h>
#include <sys/errno.h>

#include <kern/thread.h>

#include <machdep/mips/pcb.h>
#include <machdep/mips/thread.h>

/*
 * Get/set thread-specific pointer (for thread-local storage)
 */

struct sysarch_args {
	int	op;
	char	*parms;
};

#define MIPS_GET_TLS	1
#define MIPS_SET_TLS	2

int
sysarch(struct proc *p, struct sysarch_args *uap, int *retval)
{
	thread_t thread = current_thread();
	pcb_t pcb = thread->pcb;
	unsigned int tls_value;

	switch (uap->op) {
	case MIPS_GET_TLS:
		/* Get thread-local storage pointer */
		tls_value = pcb->cthread_self;
		if (copyout(&tls_value, uap->parms, sizeof(unsigned int)))
			return EFAULT;
		break;

	case MIPS_SET_TLS:
		/* Set thread-local storage pointer */
		if (copyin(uap->parms, &tls_value, sizeof(unsigned int)))
			return EFAULT;
		pcb->cthread_self = tls_value;
		break;

	default:
		return EINVAL;
	}

	return 0;
}

/*
 * Cache control system call
 * Allow userspace to flush caches
 */

#define MIPS_SYNC_ICACHE	1
#define MIPS_SYNC_DCACHE	2

struct cacheflush_args {
	vm_offset_t	addr;
	vm_size_t	len;
	int		which;
};

int
cacheflush(struct proc *p, struct cacheflush_args *uap, int *retval)
{
	vm_offset_t addr = uap->addr;
	vm_size_t len = uap->len;

	/* Verify address is in user space */
	if (addr + len > VM_MAX_ADDRESS)
		return EINVAL;

	switch (uap->which) {
	case MIPS_SYNC_ICACHE:
		/* Flush instruction cache */
		__asm__ volatile(
			"	.set	push		\n"
			"	.set	noreorder	\n"
			"	sync			\n"
			"	.set	pop		\n"
		);
		break;

	case MIPS_SYNC_DCACHE:
		/* Flush data cache */
		__asm__ volatile(
			"	.set	push		\n"
			"	.set	noreorder	\n"
			"	sync			\n"
			"	.set	pop		\n"
		);
		break;

	default:
		return EINVAL;
	}

	return 0;
}

/*
 * Get CPU information
 */
struct cpuinfo_args {
	void	*info;
	size_t	len;
};

struct mips_cpu_info {
	unsigned int	cpu_prid;
	unsigned int	cpu_frequency;
	unsigned int	icache_size;
	unsigned int	dcache_size;
	unsigned int	tlb_entries;
};

int
cpuinfo(struct proc *p, struct cpuinfo_args *uap, int *retval)
{
	struct mips_cpu_info info;

	if (uap->len < sizeof(info))
		return EINVAL;

	/* Read CP0 PRId register */
	__asm__ volatile("mfc0 %0, $15" : "=r" (info.cpu_prid));

	/* Fill in CPU information */
	info.cpu_frequency = mips_get_cpu_frequency();
	info.icache_size = 16384;	/* 16KB - detect from CPU */
	info.dcache_size = 16384;	/* 16KB - detect from CPU */
	info.tlb_entries = 64;		/* Standard MIPS TLB size */

	if (copyout(&info, uap->info, sizeof(info)))
		return EFAULT;

	return 0;
}

/*
 * External declarations
 */
extern unsigned long mips_get_cpu_frequency(void);
