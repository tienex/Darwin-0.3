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
 * Byte ordering conversion for IA64 Big-Endian (Itanium)
 *
 * IA64 in big-endian mode - native byte order is big-endian
 */

static __inline__
unsigned short
NXSwapShort(
    unsigned short	inv
)
{
    unsigned short	outv;

    /* Manual byte swap for portability */
    outv = ((inv & 0xff00) >> 8) | ((inv & 0x00ff) << 8);

    return (outv);
}

static __inline__
unsigned int
NXSwapInt(
    unsigned int	inv
)
{
    unsigned int	outv;

    /* Manual byte swap for portability */
    outv = ((inv & 0xff000000) >> 24) |
           ((inv & 0x00ff0000) >> 8)  |
           ((inv & 0x0000ff00) << 8)  |
           ((inv & 0x000000ff) << 24);

    return (outv);
}

static __inline__
unsigned long
NXSwapLong(
    unsigned long	inv
)
{
    unsigned long	outv;

    /* For IA64, long is 64-bit */
    outv = ((inv & 0xff00000000000000UL) >> 56) |
           ((inv & 0x00ff000000000000UL) >> 40) |
           ((inv & 0x0000ff0000000000UL) >> 24) |
           ((inv & 0x000000ff00000000UL) >> 8)  |
           ((inv & 0x00000000ff000000UL) << 8)  |
           ((inv & 0x0000000000ff0000UL) << 24) |
           ((inv & 0x000000000000ff00UL) << 40) |
           ((inv & 0x00000000000000ffUL) << 56);

    return (outv);
}

static __inline__
unsigned long long
NXSwapLongLong(
    unsigned long long	inv
)
{
    unsigned long long	outv;

    outv = ((inv & 0xff00000000000000ULL) >> 56) |
           ((inv & 0x00ff000000000000ULL) >> 40) |
           ((inv & 0x0000ff0000000000ULL) >> 24) |
           ((inv & 0x000000ff00000000ULL) >> 8)  |
           ((inv & 0x00000000ff000000ULL) << 8)  |
           ((inv & 0x0000000000ff0000ULL) << 24) |
           ((inv & 0x000000000000ff00ULL) << 40) |
           ((inv & 0x00000000000000ffULL) << 56);

    return (outv);
}

static __inline__ NXSwappedFloat
NXConvertHostFloatToSwapped(float x)
{
    union fconv {
	float number;
	NXSwappedFloat sf;
    };
    return ((union fconv *)&x)->sf;
}

static __inline__ float
NXConvertSwappedFloatToHost(NXSwappedFloat x)
{
    union fconv {
	float number;
	NXSwappedFloat sf;
    };
    return ((union fconv *)&x)->number;
}

static __inline__ NXSwappedDouble
NXConvertHostDoubleToSwapped(double x)
{
    union dconv {
	double number;
	NXSwappedDouble sd;
    };
    return ((union dconv *)&x)->sd;
}

static __inline__ double
NXConvertSwappedDoubleToHost(NXSwappedDouble x)
{
    union dconv {
	double number;
	NXSwappedDouble sd;
    };
    return ((union dconv *)&x)->number;
}

static __inline__ NXSwappedFloat
NXSwapFloat(NXSwappedFloat x)
{
    return NXSwapInt(x);
}

static __inline__ NXSwappedDouble
NXSwapDouble(NXSwappedDouble x)
{
    return NXSwapLongLong(x);
}
