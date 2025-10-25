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
 * RISC-V KDP (Kernel Debugging Protocol) machine-dependent code
 */

#include <mach/mach_types.h>

/*
 * KDP initialization
 */
void
kdp_machine_init(void)
{
	/* Initialize KDP for RISC-V */
}

/*
 * Get register state
 */
void
kdp_machine_get_breakinsn(unsigned char *bytes, int *size)
{
	/* RISC-V ebreak instruction: 0x00100073 */
	static unsigned char breakinsn[] = {0x73, 0x00, 0x10, 0x00};
	*size = 4;
	bytes[0] = breakinsn[0];
	bytes[1] = breakinsn[1];
	bytes[2] = breakinsn[2];
	bytes[3] = breakinsn[3];
}
