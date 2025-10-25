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
 * Natural alignment of shorts and longs (for MIPS)
 *
 * MIPS architecture requires natural alignment for most data types.
 * Unaligned access may cause exceptions or be handled in software.
 */

/*
 * Helper functions for aligned short access
 */
__inline__ static unsigned short
get_align_short(void *ivalue)
{
    unsigned char *cp = (unsigned char *) ivalue;
    unsigned short result;

    result = ((unsigned short)cp[0] << 8) | (unsigned short)cp[1];
    return result;
}

__inline__ static unsigned short
put_align_short(unsigned short ivalue, void *ovalue)
{
    unsigned char *cp = (unsigned char *) ovalue;

    cp[0] = (unsigned char)(ivalue >> 8);
    cp[1] = (unsigned char)(ivalue);
    return ivalue;
}

/*
 * Helper functions for aligned long access
 */
__inline__ static unsigned long
get_align_long(void *ivalue)
{
    unsigned char *cp = (unsigned char *) ivalue;
    unsigned long result;

    result = ((unsigned long)cp[0] << 24) |
             ((unsigned long)cp[1] << 16) |
             ((unsigned long)cp[2] << 8) |
             (unsigned long)cp[3];
    return result;
}

__inline__ static unsigned long
put_align_long(unsigned long ivalue, void *ovalue)
{
    unsigned char *cp = (unsigned char *) ovalue;

    cp[0] = (unsigned char)(ivalue >> 24);
    cp[1] = (unsigned char)(ivalue >> 16);
    cp[2] = (unsigned char)(ivalue >> 8);
    cp[3] = (unsigned char)(ivalue);
    return ivalue;
}

/*
 * Helper functions for aligned long long access (MIPS64)
 */
__inline__ static unsigned long long
get_align_long_long(void *ivalue)
{
    unsigned char *cp = (unsigned char *) ivalue;
    unsigned long long result;

    result = ((unsigned long long)cp[0] << 56) |
             ((unsigned long long)cp[1] << 48) |
             ((unsigned long long)cp[2] << 40) |
             ((unsigned long long)cp[3] << 32) |
             ((unsigned long long)cp[4] << 24) |
             ((unsigned long long)cp[5] << 16) |
             ((unsigned long long)cp[6] << 8) |
             (unsigned long long)cp[7];
    return result;
}

__inline__ static unsigned long long
put_align_long_long(unsigned long long ivalue, void *ovalue)
{
    unsigned char *cp = (unsigned char *) ovalue;

    cp[0] = (unsigned char)(ivalue >> 56);
    cp[1] = (unsigned char)(ivalue >> 48);
    cp[2] = (unsigned char)(ivalue >> 40);
    cp[3] = (unsigned char)(ivalue >> 32);
    cp[4] = (unsigned char)(ivalue >> 24);
    cp[5] = (unsigned char)(ivalue >> 16);
    cp[6] = (unsigned char)(ivalue >> 8);
    cp[7] = (unsigned char)(ivalue);
    return ivalue;
}
