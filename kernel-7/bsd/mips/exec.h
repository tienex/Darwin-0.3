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
 * MIPS executable file format definitions
 */

#ifndef _BSD_MIPS_EXEC_H_
#define _BSD_MIPS_EXEC_H_

/*
 * MIPS executable header
 * Compatible with ELF format
 */

#define MIPS_EXEC_MAGIC		0x0150		/* MIPS OMAGIC */
#define MIPS_ZMAGIC		0x0413		/* MIPS ZMAGIC */

/*
 * Arguments to execve()
 */
#define EXEC_ARGCOUNT		1024		/* Max arguments */
#define EXEC_ARGSIZE		(256*1024)	/* Max argument size */

/*
 * Machine ID for MIPS
 */
#define MID_MIPS		8		/* MIPS machine ID */

#endif /* _BSD_MIPS_EXEC_H_ */
