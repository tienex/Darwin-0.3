/*
 * x86-64 Machine-Dependent System Calls
 *
 * This file implements machine-specific system calls for x86-64
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/proc.h>
#include <mach/machine.h>
#include <mach/machine/thread_status.h>

/*
 * External functions
 */
extern unsigned long rdmsr64(unsigned long msr);
extern void wrmsr64(unsigned long msr, unsigned long val);

/*
 * Machine-dependent system call dispatcher
 */
long
machdep_call(int call_number, void *args)
{
	switch (call_number) {
	case 0:  /* Get CPU features */
		return machdep_get_cpu_features();

	case 1:  /* Set thread area (TLS) */
		return machdep_set_thread_area((unsigned long)args);

	case 2:  /* Get thread area */
		return machdep_get_thread_area();

	case 3:  /* Get CPU info */
		return machdep_get_cpu_info(args);

	case 4:  /* Set I/O permission level */
		return machdep_set_iopl(*(int *)args);

	default:
		return -1;  /* ENOSYS */
	}
}

/*
 * Get CPU features
 */
long
machdep_get_cpu_features(void)
{
	unsigned int cpuid_result[4];
	unsigned long features = 0;

	/* Get feature flags from CPUID */
	cpuid(1, 0, (unsigned long)cpuid_result);

	/* Build feature mask from EDX and ECX */
	features = cpuid_result[3];  /* EDX */
	features |= ((unsigned long)cpuid_result[2] << 32);  /* ECX */

	return features;
}

/*
 * Set thread area for thread-local storage
 * Uses FS.base MSR (0xC0000100)
 */
long
machdep_set_thread_area(unsigned long addr)
{
	/* Validate address */
	if (addr >= VM_MAX_ADDRESS)
		return -1;  /* EINVAL */

	/* Set FS.base MSR */
	wrmsr64(0xC0000100, addr);

	return 0;
}

/*
 * Get thread area
 */
long
machdep_get_thread_area(void)
{
	return rdmsr64(0xC0000100);
}

/*
 * Get CPU information
 */
long
machdep_get_cpu_info(void *info_ptr)
{
	struct cpu_info {
		unsigned int family;
		unsigned int model;
		unsigned int stepping;
		char vendor[13];
	} info;
	unsigned int cpuid_result[4];

	/* Get vendor */
	cpuid(0, 0, (unsigned long)cpuid_result);
	*(unsigned int *)(info.vendor + 0) = cpuid_result[1];
	*(unsigned int *)(info.vendor + 4) = cpuid_result[3];
	*(unsigned int *)(info.vendor + 8) = cpuid_result[2];
	info.vendor[12] = '\0';

	/* Get family/model/stepping */
	cpuid(1, 0, (unsigned long)cpuid_result);
	info.family = ((cpuid_result[0] >> 8) & 0xF) +
	              ((cpuid_result[0] >> 20) & 0xFF);
	info.model = ((cpuid_result[0] >> 4) & 0xF) |
	             ((cpuid_result[0] >> 12) & 0xF0);
	info.stepping = cpuid_result[0] & 0xF;

	/* Copy to user space */
	if (copyout(&info, info_ptr, sizeof(info)) != 0)
		return -1;  /* EFAULT */

	return 0;
}

/*
 * Set I/O privilege level
 */
long
machdep_set_iopl(int level)
{
	/* Check permission */
	/* if (!suser()) return -EPERM; */

	if (level < 0 || level > 3)
		return -1;  /* EINVAL */

	/* Modify IOPL field in RFLAGS */
	unsigned long rflags;
	__asm__ volatile("pushfq; popq %0" : "=r" (rflags));
	rflags &= ~0x3000;  /* Clear IOPL */
	rflags |= (level << 12);  /* Set new IOPL */
	__asm__ volatile("pushq %0; popfq" :: "r" (rflags));

	return 0;
}

/*
 * Control I/O permission bitmap
 */
long
machdep_ioperm(unsigned long from, unsigned long num, int turn_on)
{
	/* Check permission */
	/* if (!suser()) return -EPERM; */

	/* Stub implementation */
	/* Would modify I/O permission bitmap in TSS */

	return 0;
}

/*
 * Read MSR (Model Specific Register)
 * Privileged operation
 */
long
machdep_read_msr(unsigned long msr, unsigned long *value)
{
	/* Check permission */
	/* if (!suser()) return -EPERM; */

	/* Check MSR validity */
	if (msr > 0xC0001FFF)
		return -1;  /* EINVAL */

	*value = rdmsr64(msr);
	return 0;
}

/*
 * Write MSR
 * Privileged operation
 */
long
machdep_write_msr(unsigned long msr, unsigned long value)
{
	/* Check permission */
	/* if (!suser()) return -EPERM; */

	/* Check MSR validity */
	if (msr > 0xC0001FFF)
		return -1;  /* EINVAL */

	wrmsr64(msr, value);
	return 0;
}

/*
 * Get processor name string
 */
long
machdep_get_processor_name(char *buf, size_t len)
{
	unsigned int cpuid_result[4];
	char name[49];
	int i;

	/* CPUID 0x80000002-0x80000004 returns processor brand string */
	for (i = 0; i < 3; i++) {
		cpuid(0x80000002 + i, 0, (unsigned long)cpuid_result);
		*(unsigned int *)(name + i * 16 + 0) = cpuid_result[0];
		*(unsigned int *)(name + i * 16 + 4) = cpuid_result[1];
		*(unsigned int *)(name + i * 16 + 8) = cpuid_result[2];
		*(unsigned int *)(name + i * 16 + 12) = cpuid_result[3];
	}
	name[48] = '\0';

	/* Copy to user buffer */
	if (copyout(name, buf, len < 49 ? len : 49) != 0)
		return -1;  /* EFAULT */

	return 0;
}

/*
 * Flush TLB entry
 */
long
machdep_flush_tlb(unsigned long addr)
{
	__asm__ volatile("invlpg (%0)" :: "r" (addr) : "memory");
	return 0;
}

/*
 * Get CPU ID
 */
long
machdep_get_cpuid(unsigned long leaf, unsigned long *result)
{
	unsigned int cpuid_result[4];

	cpuid(leaf, 0, (unsigned long)cpuid_result);

	/* Copy to user space */
	if (copyout(cpuid_result, result, sizeof(cpuid_result)) != 0)
		return -1;  /* EFAULT */

	return 0;
}
