/*
 * MMIX kernel debugger support
 */

#include <mach/mach_types.h>
#include <machdep/mmix/thread.h>

#if DEBUG

void
kdp_trap(int trapno, struct mmix_saved_state *regs)
{
	/* Enter kernel debugger */
	/* TODO: Implement KDP protocol for debugging */
}

void
kdp_register_send_receive(void *send, void *receive)
{
	/* Register KDP send/receive functions */
}

#endif /* DEBUG */
