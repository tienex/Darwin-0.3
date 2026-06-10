/*
 * MMIX Unix signal delivery
 */

#include <sys/param.h>
#include <sys/signal.h>
#include <sys/proc.h>

void
sendsig(sig_t catcher, int sig, int mask, unsigned code)
{
	/* Deliver signal to user process */
	/* TODO: Set up signal trampoline on user stack */
}
