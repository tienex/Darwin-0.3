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

#ifndef	_MACH_MMIX_NDR_H_
#define _MACH_MMIX_NDR_H_

/* NDR record for MMIX */

#import <mach/ndr.h>

NDR_record_t NDR_record = {
	0,			/* mig_reserved */
	0,			/* mig_reserved */
	0,			/* mig_reserved */
	NDR_PROTOCOL_2_0,
	NDR_INT_BIG_ENDIAN,
	NDR_CHAR_ASCII,
	NDR_FLOAT_IEEE,
	0,
};

#endif	/* _MACH_MMIX_NDR_H_ */
