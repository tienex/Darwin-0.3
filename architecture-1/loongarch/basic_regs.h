/*
 * Copyright (c) 1999 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * "Portions Copyright (c) 1999 Apple Computer, Inc.  All Rights
 * Reserved.  This file contains Original Code and/or Modifications of
 * Original Code as defined in and that are subject to the Apple Public
 * Source License Version 1.0 (the 'License').  You may not use this file
 * except in compliance with the License.  Please obtain a copy of the
 * License at http://www.apple.com/publicsource and read it before using
 * this file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License."
 *
 * @APPLE_LICENSE_HEADER_END@
 */
/*
 *	File:	architecture/loongarch/basic_regs.h
 *
 *	Basic LoongArch registers.
 */

#ifndef _ARCH_LOONGARCH_BASIC_REGS_H_
#define _ARCH_LOONGARCH_BASIC_REGS_H_

#import <architecture/loongarch/reg_help.h>
#import <architecture/loongarch/macro_help.h>

#if !defined(__ASSEMBLER__)

/*
 * Number of General Purpose registers.
 */
#define LOONGARCH_NGP_REGS	32

/*
 * Number of Floating Point registers.
 */
#define LOONGARCH_NFP_REGS	32

#endif /* ! __ASSEMBLER__ */

#endif /* _ARCH_LOONGARCH_BASIC_REGS_H_ */
