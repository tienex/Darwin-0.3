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
 * Byte ordering conversion (for RISC-V).
 */

static __inline__
unsigned short
NXSwapShort(
    unsigned short	inv
)
{
    unsigned short	outv;

    /* RISC-V byte swap for 16-bit */
    __asm__ volatile(
        "slli %0, %1, 8\n\t"
        "srli %1, %1, 8\n\t"
        "or %0, %0, %1"
        : "=r" (outv)
        : "r" (inv));

    return (outv);
}

static __inline__
unsigned int
NXSwapInt(
    unsigned int	inv
)
{
    unsigned int	outv;

    /* RISC-V byte swap for 32-bit using shifts and masks */
    outv = ((inv & 0x000000FF) << 24) |
           ((inv & 0x0000FF00) <<  8) |
           ((inv & 0x00FF0000) >>  8) |
           ((inv & 0xFF000000) >> 24);

    return (outv);
}

static __inline__
unsigned long
NXSwapLong(
    unsigned long	inv
)
{
#if defined(__riscv_xlen) && __riscv_xlen == 64
    unsigned long	outv;

    /* 64-bit byte swap */
    outv = ((inv & 0x00000000000000FFUL) << 56) |
           ((inv & 0x000000000000FF00UL) << 40) |
           ((inv & 0x0000000000FF0000UL) << 24) |
           ((inv & 0x00000000FF000000UL) <<  8) |
           ((inv & 0x000000FF00000000UL) >>  8) |
           ((inv & 0x0000FF0000000000UL) >> 24) |
           ((inv & 0x00FF000000000000UL) >> 40) |
           ((inv & 0xFF00000000000000UL) >> 56);

    return (outv);
#else
    return NXSwapInt(inv);
#endif
}

static __inline__
unsigned long long
NXSwapLongLong(
    unsigned long long	inv
)
{
    union llconv {
	unsigned long long	ull;
	unsigned int		ui[2];
    } *inp, outv;

    inp = (union llconv *)&inv;

    outv.ui[0] = NXSwapInt(inp->ui[1]);
    outv.ui[1] = NXSwapInt(inp->ui[0]);

    return (outv.ull);
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
