/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * Alpha Context Switching
 */

#include <architecture/alpha/asm_help.h>

	.text
	.set noreorder

/*
 * Switch to new thread context
 */
ENTRY(switch_context)
	/* Save current context */
	/* Load new context */
	/* Return */
	ret	zero, (ra), 1
END(switch_context)
