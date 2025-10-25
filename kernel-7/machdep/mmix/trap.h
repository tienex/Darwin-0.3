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
 * distributed on an "AS IS" BASIS, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON- INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */

#ifndef	_MMIX_TRAP_H_
#define	_MMIX_TRAP_H_

/*
 * Hardware exception vectors for MMIX are in exception.h
 * MMIX uses rK register for interrupt control
 */

#ifndef	__ASSEMBLER__

#include <mach/thread_status.h>
#include <mach/boolean.h>
#include <mmix/thread.h>

extern void			doexception(int exc, int code, int sub, thread_t th);

extern void			thread_exception_return(void);

extern boolean_t 		alignment(unsigned long long addr,
					  unsigned long long inst,
					  struct mmix_saved_state *ssp);

extern struct mmix_saved_state*	trap(int trapno,
				     struct mmix_saved_state *ss,
				     unsigned long long rW,
				     unsigned long long rX);

extern struct mmix_saved_state* interrupt(int intno,
					 struct mmix_saved_state *ss,
					 unsigned long long rW,
					 unsigned long long rX);

extern int			syscall_error(int exception,
					      int code,
					      int subcode,
					      struct mmix_saved_state *ss);

#endif	/* __ASSEMBLER__ */

#endif	/* _MMIX_TRAP_H_ */
