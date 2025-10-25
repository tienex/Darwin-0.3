/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * Alpha VM Machine-Dependent Functions
 */

#include <mach/mach_types.h>
#include <kern/thread.h>

void
thread_setstatus(thread_t thread, int flavor, void *tstate, unsigned int count)
{
	/* Set thread state */
}

void
thread_getstatus(thread_t thread, int flavor, void *tstate, unsigned int *count)
{
	/* Get thread state */
}
