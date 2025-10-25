/*
 * MMIX fault-tolerant memory copy
 *
 * Implements copyin/copyout with page fault recovery
 */

#include <mach/mach_types.h>

int
copyin(const void *user_addr, void *kernel_addr, vm_size_t nbytes)
{
	/* Copy from user space to kernel space */
	/* TODO: Implement with fault recovery */
	bcopy(user_addr, kernel_addr, nbytes);
	return 0;
}

int
copyout(const void *kernel_addr, void *user_addr, vm_size_t nbytes)
{
	/* Copy from kernel space to user space */
	/* TODO: Implement with fault recovery */
	bcopy(kernel_addr, user_addr, nbytes);
	return 0;
}

int
copyinstr(const void *user_addr, void *kernel_addr, vm_size_t maxlen, vm_size_t *lencopied)
{
	/* Copy string from user space */
	/* TODO: Implement with fault recovery and length check */
	return 0;
}

int
copyoutstr(const void *kernel_addr, void *user_addr, vm_size_t maxlen, vm_size_t *lencopied)
{
	/* Copy string to user space */
	/* TODO: Implement with fault recovery and length check */
	return 0;
}
