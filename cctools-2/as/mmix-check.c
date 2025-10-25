/* mmix-check.c -- MMIX instruction validation
   Copyright (c) 1999-2025 Apple Computer, Inc.
   
   MMIX instruction format and operand validation
*/

#include <stdio.h>
#include "mmix-opcode.h"

/*
 * Check if a register number is valid for MMIX
 * MMIX has 256 general-purpose registers ($0-$255)
 */
int
mmix_check_register(unsigned int reg)
{
    return (reg <= 255);
}

/*
 * Check if an immediate value fits in N bits
 */
int
mmix_check_immediate(long value, int bits, int is_signed)
{
    if (is_signed) {
        long max = (1L << (bits - 1)) - 1;
        long min = -(1L << (bits - 1));
        return (value >= min && value <= max);
    } else {
        unsigned long max = (1UL << bits) - 1;
        return ((unsigned long)value <= max);
    }
}

/*
 * Check if a branch offset is valid (16-bit signed in YZ field)
 */
int
mmix_check_branch_offset(long offset)
{
    /* Branch offset is in instructions (4 bytes), 16-bit signed */
    return mmix_check_immediate(offset, 16, 1);
}

/*
 * Check if a jump offset is valid (24-bit signed in XYZ field)
 */
int
mmix_check_jump_offset(long offset)
{
    /* Jump offset is in instructions (4 bytes), 24-bit signed */
    return mmix_check_immediate(offset, 24, 1);
}

/*
 * Validate instruction operands based on format
 */
int
mmix_check_instruction(const struct mmix_opcode *op,
                        unsigned char x,
                        unsigned char y,
                        unsigned char z,
                        unsigned int imm16)
{
    const char *fmt = op->args;
    
    /* Check each operand type in format string */
    while (*fmt) {
        switch (*fmt) {
        case 'X':
        case 'Y':
        case 'Z':
            /* Register operands - all validated by caller */
            break;
            
        case 'I':
            /* 16-bit immediate */
            if (!mmix_check_immediate(imm16, 16, 0)) {
                fprintf(stderr, "Immediate value 0x%x out of range for %s\n",
                        imm16, op->name);
                return 0;
            }
            break;
            
        case 'B':
            /* Branch offset - checked during assembly */
            break;
            
        case '@':
            /* Special register - additional validation needed */
            break;
            
        default:
            /* Unknown format character */
            break;
        }
        fmt++;
    }
    
    return 1;
}
