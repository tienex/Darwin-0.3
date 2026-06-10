/*
 * MMIX system call handling
 */

#include <mach/mach_types.h>
#include <machdep/mmix/thread.h>

void
unix_syscall(struct mmix_saved_state *regs)
{
	/* Handle Unix system call */
	/* System call number in $0, args in $1-$6 */
	/* TODO: Dispatch to appropriate syscall handler */
}

void
mach_syscall(struct mmix_saved_state *regs)
{
	/* Handle Mach system call */
	/* TODO: Dispatch to Mach trap handler */
}
