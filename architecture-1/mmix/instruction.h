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
 * MMIX Instruction Format Definitions
 *
 * MMIX instructions are always 32 bits (4 bytes) wide.
 * The instruction format is:
 *
 *   Opcode (8 bits) | X (8 bits) | Y (8 bits) | Z (8 bits)
 *
 * Where:
 *   Opcode: Operation code
 *   X: Destination register (or other use)
 *   Y: First operand register
 *   Z: Second operand (register or immediate)
 */

#ifndef _ARCHITECTURE_MMIX_INSTRUCTION_H_
#define _ARCHITECTURE_MMIX_INSTRUCTION_H_

/*
 * MMIX instruction structure
 */
typedef struct {
    unsigned char opcode;	/* Operation code */
    unsigned char x;		/* X field (usually destination) */
    unsigned char y;		/* Y field (usually first operand) */
    unsigned char z;		/* Z field (usually second operand) */
} mmix_instruction_t;

/*
 * MMIX Opcode Groups
 */

/* Load and Store Instructions (0x80-0x9F) */
#define MMIX_OP_LDB		0x80	/* Load Byte */
#define MMIX_OP_LDBU		0x82	/* Load Byte Unsigned */
#define MMIX_OP_LDW		0x84	/* Load Wyde (16-bit) */
#define MMIX_OP_LDWU		0x86	/* Load Wyde Unsigned */
#define MMIX_OP_LDT		0x88	/* Load Tetra (32-bit) */
#define MMIX_OP_LDTU		0x8A	/* Load Tetra Unsigned */
#define MMIX_OP_LDO		0x8C	/* Load Octa (64-bit) */
#define MMIX_OP_LDOU		0x8E	/* Load Octa Unsigned */
#define MMIX_OP_STB		0xA0	/* Store Byte */
#define MMIX_OP_STBU		0xA2	/* Store Byte Unsigned */
#define MMIX_OP_STW		0xA4	/* Store Wyde */
#define MMIX_OP_STWU		0xA6	/* Store Wyde Unsigned */
#define MMIX_OP_STT		0xA8	/* Store Tetra */
#define MMIX_OP_STTU		0xAA	/* Store Tetra Unsigned */
#define MMIX_OP_STO		0xAC	/* Store Octa */
#define MMIX_OP_STOU		0xAE	/* Store Octa Unsigned */

/* Arithmetic Instructions (0x20-0x3F) */
#define MMIX_OP_ADD		0x20	/* Add */
#define MMIX_OP_SUB		0x22	/* Subtract */
#define MMIX_OP_MUL		0x18	/* Multiply */
#define MMIX_OP_DIV		0x1A	/* Divide */
#define MMIX_OP_ADDU		0x28	/* Add Unsigned */
#define MMIX_OP_SUBU		0x2A	/* Subtract Unsigned */
#define MMIX_OP_MULU		0x1C	/* Multiply Unsigned */
#define MMIX_OP_DIVU		0x1E	/* Divide Unsigned */

/* Bitwise Instructions (0x38-0x4F) */
#define MMIX_OP_AND		0xC8	/* Bitwise AND */
#define MMIX_OP_OR		0xC0	/* Bitwise OR */
#define MMIX_OP_XOR		0xC6	/* Bitwise XOR */
#define MMIX_OP_ANDN		0xCA	/* Bitwise AND NOT */
#define MMIX_OP_ORN		0xC2	/* Bitwise OR NOT */
#define MMIX_OP_NAND		0xCC	/* Bitwise NAND */
#define MMIX_OP_NOR		0xC4	/* Bitwise NOR */
#define MMIX_OP_NXOR		0xCE	/* Bitwise NXOR */

/* Shift Instructions */
#define MMIX_OP_SL		0x38	/* Shift Left */
#define MMIX_OP_SLU		0x3A	/* Shift Left Unsigned */
#define MMIX_OP_SR		0x3C	/* Shift Right */
#define MMIX_OP_SRU		0x3E	/* Shift Right Unsigned */

/* Comparison Instructions (0x30-0x37) */
#define MMIX_OP_CMP		0x30	/* Compare */
#define MMIX_OP_CMPU		0x31	/* Compare Unsigned */

/* Branch Instructions (0xF0-0xFF) */
#define MMIX_OP_BN		0x40	/* Branch if Negative */
#define MMIX_OP_BZ		0x42	/* Branch if Zero */
#define MMIX_OP_BP		0x44	/* Branch if Positive */
#define MMIX_OP_BOD		0x46	/* Branch if Odd */
#define MMIX_OP_BNN		0x48	/* Branch if Non-Negative */
#define MMIX_OP_BNZ		0x4A	/* Branch if Non-Zero */
#define MMIX_OP_BNP		0x4C	/* Branch if Non-Positive */
#define MMIX_OP_BEV		0x4E	/* Branch if Even */

/* Jump Instructions */
#define MMIX_OP_JMP		0xF0	/* Jump */
#define MMIX_OP_PUSHJ		0xF2	/* Push and Jump */
#define MMIX_OP_GETA		0xF4	/* Get Address */
#define MMIX_OP_PUT		0xF6	/* Put to special register */
#define MMIX_OP_GET		0xFE	/* Get from special register */
#define MMIX_OP_POP		0xF8	/* Pop */

/* Floating Point Instructions (0x10-0x17) */
#define MMIX_OP_FADD		0x06	/* Floating Add */
#define MMIX_OP_FSUB		0x08	/* Floating Subtract */
#define MMIX_OP_FMUL		0x10	/* Floating Multiply */
#define MMIX_OP_FDIV		0x12	/* Floating Divide */
#define MMIX_OP_FREM		0x14	/* Floating Remainder */
#define MMIX_OP_FSQRT		0x15	/* Floating Square Root */
#define MMIX_OP_FINT		0x16	/* Floating to Integer */
#define MMIX_OP_FCMP		0x01	/* Floating Compare */

/* Special Instructions */
#define MMIX_OP_TRAP		0x00	/* Trap (System call) */
#define MMIX_OP_TRIP		0xFF	/* Trip (Forced trap) */
#define MMIX_OP_SYNC		0xFC	/* Synchronize */
#define MMIX_OP_LDVTS		0xF9	/* Load Virtual Translation */

/*
 * Instruction field extraction macros
 */
#define MMIX_OPCODE(i)		(((i) >> 24) & 0xFF)
#define MMIX_X(i)		(((i) >> 16) & 0xFF)
#define MMIX_Y(i)		(((i) >> 8) & 0xFF)
#define MMIX_Z(i)		((i) & 0xFF)
#define MMIX_YZ(i)		((i) & 0xFFFF)
#define MMIX_XYZ(i)		((i) & 0xFFFFFF)

/*
 * Instruction construction macros
 */
#define MMIX_INST(op, x, y, z)	\
	(((op) << 24) | ((x) << 16) | ((y) << 8) | (z))

#define MMIX_INST_YZ(op, x, yz)	\
	(((op) << 24) | ((x) << 16) | ((yz) & 0xFFFF))

#define MMIX_INST_XYZ(op, xyz)	\
	(((op) << 24) | ((xyz) & 0xFFFFFF))

#endif /* _ARCHITECTURE_MMIX_INSTRUCTION_H_ */
