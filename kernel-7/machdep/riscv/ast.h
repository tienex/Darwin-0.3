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
 * RISC-V Asynchronous System Trap (AST) support
 */

#ifndef _MACHDEP_RISCV_AST_H_
#define _MACHDEP_RISCV_AST_H_

#include <kern/ast.h>

/*
 * AST checking on RISC-V
 * Called before returning to user mode
 */
#ifndef __ASSEMBLER__

#define aston(mycpu)
#define astoff(mycpu)

/*
 * Check for pending ASTs
 */
#define ast_needed(thread)      ((thread)->ast)

/*
 * AST context structure
 */
#define ast_context(thread, pc) /* Nothing needed for RISC-V */

#endif /* !__ASSEMBLER__ */

#endif /* _MACHDEP_RISCV_AST_H_ */
