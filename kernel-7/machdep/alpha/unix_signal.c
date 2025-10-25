/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * Alpha UNIX Signal Support
 */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/signal.h>

/* Signal handling stubs */

void
sendsig(void)
{
	/* Send signal to process */
}

void
sigreturn(void)
{
	/* Return from signal handler */
}
