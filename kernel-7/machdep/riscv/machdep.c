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
 * RISC-V machine-dependent code
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <mach/machine.h>

/*
 * cpu_type() - Return the CPU type for this system
 */
cpu_type_t
cpu_type(void)
{
	return CPU_TYPE_RISCV;
}

/*
 * cpu_subtype() - Return the CPU subtype for this system
 */
cpu_subtype_t
cpu_subtype(void)
{
#if defined(__riscv_xlen) && __riscv_xlen == 64
	return CPU_SUBTYPE_RISCV64_G;
#else
	return CPU_SUBTYPE_RISCV32_G;
#endif
}

/*
 * machine_slot_data() - Return machine slot data
 */
void
machine_slot_data(void)
{
	/* Stub for RISC-V */
}
