# DLX ISA Extensions - Comprehensive Instruction Set Additions

## Overview

This document specifies major ISA extensions to DLX including:
- **Compressed Instruction Mode** (Thumb-2 style)
- **4-Operand Instructions** (FMA and others)
- **Extended FPU** (SPARC64-style with 128-bit, VAX compatibility)
- **Advanced Bit Manipulation** (PA-RISC and Alpha inspired)
- **Polynomial Instructions** (VAX POLY)
- **PC-Relative Addressing** (SPARC/RISC-V/LoongArch inspired)
- **Bytecode Execution Mode** (Jazelle-style for Java/CLI)
- **Managed Code Mode** (ThumbEE-style for JIT/dynamic recompilers)

## 1. Compressed Instruction Mode (DLX-C)

### ARM Thumb-2 Inspired Mode

Unlike the earlier plan for 16-bit compressed instructions mixed with 32-bit, DLX-C implements a **mode-based approach** like ARM Thumb-2.

```c
/* Status Register Compressed Mode Bit */
#define DLX_STATUS_COMPRESSED   0x00100000  /* Bit 20: Compressed mode */

/* Mode Switching */
#define IS_COMPRESSED_MODE(status) ((status) & DLX_STATUS_COMPRESSED)
```

### Mode Switching Instructions

```assembly
# SETC - Switch to Compressed mode
SETC                        # Enter 16-bit compressed mode
                           # Sets STATUS.COMPRESSED bit
                           # PC must be 2-byte aligned

# SETW - Switch to Wide (32-bit) mode
SETW                        # Enter 32-bit normal mode
                           # Clears STATUS.COMPRESSED bit
                           # PC must be 4-byte aligned

# BXC - Branch and eXchange to Compressed
BXC  rs                     # Branch to address in rs, enter compressed mode
                           # if (rs & 1): compressed mode, PC = rs & ~1
                           # else: normal mode, PC = rs

# BLXC - Branch Link eXchange to Compressed
BLXC rs                     # Like BXC but saves return address
```

### Compressed Instruction Format (16-bit)

```
Format C1: Register-Register ALU
┌────┬────────┬────┬────┬────┐
│ Op │ Funct  │ Rd │ Rs │ Rt │
│ 4  │   4    │ 3  │ 3  │ 2  │
└────┴────────┴────┴────┴────┘

Format C2: Immediate
┌────┬────┬────┬────────────┐
│ Op │ Rd │ Rs │    Imm8    │
│ 4  │ 3  │ 3  │     6      │
└────┴────┴────┴────────────┘

Format C3: Load/Store
┌────┬────┬────┬────┬───────┐
│ Op │ Rd │ Rs │ Rb │ Offset│
│ 4  │ 3  │ 3  │ 3  │   3   │
└────┴────┴────┴────┴───────┘

Format C4: Branch
┌────┬────────────────────┐
│ Op │    Offset11        │
│ 5  │       11           │
└────┴────────────────────┘

Format C5: Special/Wide Prefix
┌────┬────────────────────┐
│ Op │   Varies           │
│ 5  │      11            │
└────┴────────────────────┘
```

### Compressed Instruction Set

```assembly
# Compressed ALU (Format C1)
CADD  rd, rs, rt           # rd = rs + rt (r0-r7)
CSUB  rd, rs, rt           # rd = rs - rt
CAND  rd, rs, rt           # rd = rs & rt
COR   rd, rs, rt           # rd = rs | rt
CXOR  rd, rs, rt           # rd = rs ^ rt
CSLL  rd, rs, rt           # rd = rs << rt
CSRL  rd, rs, rt           # rd = rs >> rt (logical)
CMOV  rd, rs               # rd = rs

# Compressed Immediate (Format C2)
CADDI rd, rs, #imm6        # rd = rs + sign_extend(imm6)
CANDI rd, rs, #imm6        # rd = rs & zero_extend(imm6)
CORI  rd, rs, #imm6        # rd = rs | zero_extend(imm6)
CSLLI rd, rs, #imm5        # rd = rs << imm5
CSRLI rd, rs, #imm5        # rd = rs >> imm5

# Compressed Load/Store (Format C3)
CLW   rd, offset(rs)       # rd = MEM[rs + offset*4] (word)
CSW   rt, offset(rs)       # MEM[rs + offset*4] = rt
CLH   rd, offset(rs)       # rd = MEM[rs + offset*2] (halfword)
CSH   rt, offset(rs)       # MEM[rs + offset*2] = rt
CLB   rd, offset(rs)       # rd = MEM[rs + offset] (byte)
CSB   rt, offset(rs)       # MEM[rs + offset] = rt

# Compressed Branch (Format C4)
CBEQZ rs, offset           # if (rs == 0) PC += offset*2
CBNEZ rs, offset           # if (rs != 0) PC += offset*2
CB    offset               # PC += offset*2 (unconditional)
CBAL  offset               # r31 = PC+2; PC += offset*2

# Special Compressed
CNOP                       # No operation (16-bit)
CRET                       # Return (jr r31)
CPUSH {regs}               # Push registers to stack
CPOP  {regs}               # Pop registers from stack

# Wide Prefix (32-bit in compressed mode)
# Two consecutive 16-bit instructions treated as one 32-bit
WIDE  <32-bit instruction> # Execute full 32-bit instruction
```

### Compressed Mode Calling Convention

```c
/* Registers in compressed mode (r0-r7 directly accessible) */
r0: Zero (always 0)
r1: Return address (ra)
r2: Stack pointer (sp)
r3: Frame pointer (fp)
r4-r7: Argument/temporary registers

/* Extended register access requires WIDE prefix or mode switch */

/* Function call in compressed mode */
void compressed_function(int a, int b) {
    asm("cpush {r4-r7}");           // Save used registers
    asm("caddi r2, r2, #-16");      // Allocate stack frame

    // Function body uses r4-r7 for locals
    int result = a + b;

    asm("caddi r2, r2, #16");       // Deallocate stack
    asm("cpop {r4-r7}");            // Restore registers
    asm("cret");                     // Return
}
```

## 2. Four-Operand Instructions

### 4-Register Instruction Format

```
Format 4R: Four-Operand ALU/FPU
┌────────┬────┬────┬────┬────┬──────┬────────┐
│ Opcode │ Rd │ Rs │ Rt │ Rx │ Func │Reserved│
│   6    │ 5  │ 5  │ 5  │ 5  │  4   │   2    │
└────────┴────┴────┴────┴────┴──────┴────────┘

Rd: Destination register
Rs: Source register 1
Rt: Source register 2
Rx: Source register 3
Func: Function code
```

### Fused Multiply-Add (FMA) Instructions

```assembly
# Integer FMA
FMADD  rd, rs, rt, rx      # rd = (rs * rt) + rx (signed)
FMSUB  rd, rs, rt, rx      # rd = (rs * rt) - rx (signed)
FNMADD rd, rs, rt, rx      # rd = -(rs * rt) + rx
FNMSUB rd, rs, rt, rx      # rd = -(rs * rt) - rx

FMADDU rd, rs, rt, rx      # rd = (rs * rt) + rx (unsigned)
FMSUBU rd, rs, rt, rx      # rd = (rs * rt) - rx (unsigned)

# Floating-Point FMA (IEEE 754-2008 compliant)
FMA.S  fd, fs, ft, fx      # fd = (fs * ft) + fx (single precision)
FMA.D  fd, fs, ft, fx      # fd = (fs * ft) + fx (double precision)
FMA.Q  fd, fs, ft, fx      # fd = (fs * ft) + fx (quad precision)

FMS.S  fd, fs, ft, fx      # fd = (fs * ft) - fx
FMS.D  fd, fs, ft, fx
FMS.Q  fd, fs, ft, fx

FNMA.S fd, fs, ft, fx      # fd = -(fs * ft) + fx
FNMA.D fd, fs, ft, fx
FNMA.Q fd, fs, ft, fx

FNMS.S fd, fs, ft, fx      # fd = -(fs * ft) - fx
FNMS.D fd, fs, ft, fx
FNMS.Q fd, fs, ft, fx

# Example usage:
# Compute: result = (a * b) + (c * d)
FMA.D  f0, f1, f2, f3      # f0 = (f1 * f2) + f3
```

### Four-Operand Bit Manipulation

```assembly
# Bit field operations with 4 operands
BFINS  rd, rs, rt, rx      # Insert bit field: rd = rs with bits[rt:rx] from rt
BFEXT  rd, rs, rt, rx      # Extract bit field: rd = rs[rt:rx]
BFCLR  rd, rs, rt, rx      # Clear bit field: rd = rs with bits[rt:rx] = 0

# Conditional select with 4 operands
CSEL4  rd, rs, rt, rx      # if (rd != 0) rd = rs else rd = rt, using rx as flag
```

### Four-Operand Crypto

```assembly
# AES operations (4 operands)
AESENC  rd, rs, rt, rx     # AES encrypt: rd = AES_Enc(rs, rt, rx)
AESDEC  rd, rs, rt, rx     # AES decrypt: rd = AES_Dec(rs, rt, rx)

# SHA operations
SHA256  rd, rs, rt, rx     # SHA-256 round: rd = SHA256(rs, rt, rx)
SHA512  rd, rs, rt, rx     # SHA-512 round: rd = SHA512(rs, rt, rx)
```

## 3. Extended FPU - SPARC64 Style

### 128-bit Quad Precision Support

Following SPARC64 architecture, the FPU supports register pairing for 128-bit operations:

```c
/* FPU Register File Extended */
typedef struct {
    /* 32 single-precision registers (32-bit each) */
    float f[32];            /* f0-f31 */

    /* 32 double-precision registers (64-bit each) */
    /* Overlaid: f0-f1 = d0, f2-f3 = d1, ... */
    double d[16];           /* d0, d2, d4, ..., d30 (even only) */

    /* 16 quad-precision registers (128-bit each) */
    /* Overlaid: f0-f3 = q0, f4-f7 = q4, ... */
    __float128 q[16];       /* q0, q4, q8, q12, q16, q20, q24, q28 */

    /* Only multiples of 4 are valid for quad */
} dlx_fpu_sparc64_t;

/* Register naming */
#define F(n)    fpr.f[n]        /* Single: f0-f31 */
#define D(n)    fpr.d[(n)/2]    /* Double: d0,d2,d4,...,d30 (n must be even) */
#define Q(n)    fpr.q[(n)/4]    /* Quad: q0,q4,q8,...,q28 (n must be multiple of 4) */
```

### Quad Precision Instructions

```assembly
# Quad precision load/store (128-bit)
LDQ    fq0, (rs)           # Load 128-bit quad (f0-f3 = MEM[rs])
STQ    fq0, (rs)           # Store 128-bit quad (MEM[rs] = f0-f3)
LDQA   fq4, (rs)           # Load quad alternate
STQA   fq4, (rs)           # Store quad alternate

# Quad precision arithmetic
FADD.Q fq0, fq4, fq8       # fq0 = fq4 + fq8 (128-bit)
FSUB.Q fq0, fq4, fq8       # fq0 = fq4 - fq8
FMUL.Q fq0, fq4, fq8       # fq0 = fq4 * fq8
FDIV.Q fq0, fq4, fq8       # fq0 = fq4 / fq8
FSQRT.Q fq0, fq4           # fq0 = sqrt(fq4)

# Quad precision compare
FCMP.Q fq0, fq4            # Compare fq0 and fq4, set FCC
FCMPE.Q fq0, fq4           # Compare with exception on unordered

# Quad precision conversions
FSTQ   fq0, fs1            # fq0 = (quad)fs1 (single → quad)
FDTQ   fq0, fd2            # fq0 = (quad)fd2 (double → quad)
FQTS   fs0, fq4            # fs0 = (single)fq4 (quad → single)
FQTD   fd0, fq4            # fd0 = (double)fq4 (quad → double)
FQTI   rd, fq4             # rd = (int)fq4 (quad → integer)
FITQ   fq0, rs             # fq0 = (quad)rs (integer → quad)

# Quad precision FMA
FMA.Q  fq0, fq4, fq8, fq12 # fq0 = (fq4 * fq8) + fq12
```

### SPARC64-Style FPU Status

```c
/* FSR - Floating-Point State Register (64-bit on DLX64) */
typedef struct {
    uint32_t fcc0:2;            /* FP Condition Code 0 */
    uint32_t fcc1:2;            /* FP Condition Code 1 */
    uint32_t fcc2:2;            /* FP Condition Code 2 */
    uint32_t fcc3:2;            /* FP Condition Code 3 */

    uint32_t rd:2;              /* Rounding Direction */
#define FP_RN   0               /* Round to Nearest */
#define FP_RZ   1               /* Round toward Zero */
#define FP_RP   2               /* Round toward +∞ */
#define FP_RM   3               /* Round toward -∞ */

    uint32_t tem:5;             /* Trap Enable Mask */
#define FP_TEM_NX   (1 << 0)    /* Inexact */
#define FP_TEM_DZ   (1 << 1)    /* Divide by Zero */
#define FP_TEM_UF   (1 << 2)    /* Underflow */
#define FP_TEM_OF   (1 << 3)    /* Overflow */
#define FP_TEM_NV   (1 << 4)    /* Invalid */

    uint32_t ns:1;              /* Non-Standard FP */
    uint32_t ver:3;             /* FPU Version */
    uint32_t ftt:3;             /* FP Trap Type */
    uint32_t qne:1;             /* FP Queue Not Empty */

    uint32_t aexc:5;            /* Accrued Exceptions */
    uint32_t cexc:5;            /* Current Exceptions */

    uint32_t reserved:6;
} dlx_fsr_t;
```

## 4. VAX Floating-Point Support (Alpha-style)

### VAX FP Formats

Following Digital Alpha's approach to VAX compatibility:

```c
/* VAX F_floating (32-bit, similar to IEEE single but different) */
typedef struct {
    uint32_t frac1:7;       /* Fraction bits 16-22 */
    uint32_t exp:8;         /* Exponent (excess 128) */
    uint32_t sign:1;        /* Sign bit */
    uint32_t frac2:16;      /* Fraction bits 0-15 */
} vax_f_float_t;

/* VAX D_floating (64-bit) */
typedef struct {
    uint32_t frac1:7;
    uint32_t exp:8;
    uint32_t sign:1;
    uint32_t frac2:16;
    uint32_t frac3:16;
    uint32_t frac4:16;
} vax_d_float_t;

/* VAX G_floating (64-bit, extended range) */
typedef struct {
    uint32_t frac1:4;
    uint32_t exp:11;        /* Exponent (excess 1024) */
    uint32_t sign:1;
    uint32_t frac2:16;
    uint32_t frac3:16;
    uint32_t frac4:16;
} vax_g_float_t;

/* VAX H_floating (128-bit) */
typedef struct {
    uint32_t frac1:7;
    uint32_t exp:15;        /* Exponent (excess 16384) */
    uint32_t sign:1;
    uint32_t frac2:16;
    uint32_t frac3:32;
    uint32_t frac4:32;
    uint32_t frac5:16;
} vax_h_float_t;
```

### VAX FP Instructions

```assembly
# VAX F_floating (32-bit)
ADDF   fd, fs, ft          # fd = fs + ft (VAX F format)
SUBF   fd, fs, ft          # fd = fs - ft
MULF   fd, fs, ft          # fd = fs * ft
DIVF   fd, fs, ft          # fd = fs / ft
CMPF   fs, ft              # Compare VAX F format

# VAX D_floating (64-bit)
ADDD   fd, fs, ft          # VAX D format add
SUBD   fd, fs, ft
MULD   fd, fs, ft
DIVD   fd, fs, ft
CMPD   fs, ft

# VAX G_floating (64-bit, extended)
ADDG   fd, fs, ft          # VAX G format add
SUBG   fd, fs, ft
MULG   fd, fs, ft
DIVG   fd, fs, ft
CMPG   fs, ft

# VAX H_floating (128-bit)
ADDH   fq0, fq4, fq8       # VAX H format add (128-bit)
SUBH   fq0, fq4, fq8
MULH   fq0, fq4, fq8
DIVH   fq0, fq4, fq8
CMPH   fq0, fq4

# Conversions between IEEE and VAX
CVTIF  fd, fs              # Convert IEEE single to VAX F
CVTFI  fd, fs              # Convert VAX F to IEEE single
CVTID  fd, fs              # Convert IEEE double to VAX D
CVTDI  fd, fs              # Convert VAX D to IEEE double
CVTIG  fd, fs              # Convert IEEE double to VAX G
CVTGI  fd, fs              # Convert VAX G to IEEE double

# Special VAX operations
POLYF  fd, fs, ft, rx      # Polynomial evaluation (VAX F)
POLYD  fd, fs, ft, rx      # Polynomial evaluation (VAX D)
```

### VAX FP Exception Handling

```c
/* VAX FP exceptions differ from IEEE */
#define VAX_FP_RES_OPERAND  0x01    /* Reserved operand */
#define VAX_FP_DIV_ZERO     0x02    /* Divide by zero */
#define VAX_FP_OVERFLOW     0x04    /* Overflow */
#define VAX_FP_UNDERFLOW    0x08    /* Underflow */

/* No NaN or Infinity in VAX FP - use reserved operands */
#define VAX_F_RESERVED  0x00008000  /* Sign=0, Exp=0, Frac!=0 */
#define VAX_D_RESERVED  0x0000800000000000ULL
```

## 5. PA-RISC and Alpha Bit Manipulation

### PA-RISC Inspired Instructions

```assembly
# PA-RISC deposit/extract
DEP    rd, rs, pos, len    # Deposit: rd[pos:pos+len] = rs[31-len:31]
DEPI   rd, imm, pos, len   # Deposit immediate
EXTRU  rd, rs, pos, len    # Extract unsigned: rd = rs[pos:pos+len] (zero-extend)
EXTRS  rd, rs, pos, len    # Extract signed: rd = rs[pos:pos+len] (sign-extend)

# PA-RISC shift/add
SHLADD rd, rs, sh, rt      # rd = (rs << sh) + rt (sh = 1,2,3)
SH1ADD rd, rs, rt          # rd = (rs << 1) + rt
SH2ADD rd, rs, rt          # rd = (rs << 2) + rt
SH3ADD rd, rs, rt          # rd = (rs << 3) + rt

# PA-RISC advanced shifts
SHRPD  rd, rs, rt, sh      # Shift right pair double: rd = (rs:rt) >> sh
SHRPW  rd, rs, rt, sh      # Shift right pair word

# PA-RISC bit tests
BVB    rs, bit, target     # Branch if bit set
BVBI   rs, bit, target     # Branch if bit clear

# PA-RISC unit operations
UXOR   rd, rs, rt          # Unit XOR (bytewise XOR for parallel compare)
UHADD  rd, rs, rt          # Unit halving add (SIMD within register)
UHSUB  rd, rs, rt          # Unit halving subtract
UAVG   rd, rs, rt          # Unit average
```

### Alpha Inspired Instructions

```assembly
# Alpha byte manipulation
EXTBL  rd, rs, rt          # Extract byte low
EXTWL  rd, rs, rt          # Extract word low
EXTLL  rd, rs, rt          # Extract longword low
EXTQL  rd, rs, rt          # Extract quadword low
EXTWH  rd, rs, rt          # Extract word high
EXTLH  rd, rs, rt          # Extract longword high
EXTQH  rd, rs, rt          # Extract quadword high

INSBL  rd, rs, rt          # Insert byte low
INSWL  rd, rs, rt          # Insert word low
INSLL  rd, rs, rt          # Insert longword low
INSQL  rd, rs, rt          # Insert quadword low
INSWH  rd, rs, rt          # Insert word high
INSLH  rd, rs, rt          # Insert longword high
INSQH  rd, rs, rt          # Insert quadword high

MSKBL  rd, rs, rt          # Mask byte low
MSKWL  rd, rs, rt          # Mask word low
MSKLL  rd, rs, rt          # Mask longword low
MSKQL  rd, rs, rt          # Mask quadword low

# Alpha bit counting
CTPOP  rd, rs              # Count population (number of 1 bits)
CTLZ   rd, rs              # Count leading zeros
CTTZ   rd, rs              # Count trailing zeros

# Alpha conditional moves (inspired by Alpha CIX)
CMOVEQ rd, rs, rt          # if (rt == 0) rd = rs
CMOVNE rd, rs, rt          # if (rt != 0) rd = rs
CMOVLT rd, rs, rt          # if (rt < 0) rd = rs
CMOVLE rd, rs, rt          # if (rt <= 0) rd = rs
CMOVGT rd, rs, rt          # if (rt > 0) rd = rs
CMOVGE rd, rs, rt          # if (rt >= 0) rd = rs
CMOVLBC rd, rs, rt         # if (rt[0] == 0) rd = rs (low bit clear)
CMOVLBS rd, rs, rt         # if (rt[0] == 1) rd = rs (low bit set)

# Alpha MAXMIN (from Max/BWX extensions)
MAXSB  rd, rs, rt          # rd = max(rs, rt) signed byte
MAXSW  rd, rs, rt          # rd = max(rs, rt) signed word
MAXSD  rd, rs, rt          # rd = max(rs, rt) signed dword
MAXUB  rd, rs, rt          # rd = max(rs, rt) unsigned byte
MAXUW  rd, rs, rt          # rd = max(rs, rt) unsigned word
MAXUD  rd, rs, rt          # rd = max(rs, rt) unsigned dword

MINSB  rd, rs, rt          # rd = min(rs, rt) signed byte
MINSW  rd, rs, rt          # rd = min(rs, rt) signed word
MINSD  rd, rs, rt          # rd = min(rs, rt) signed dword
MINUB  rd, rs, rt          # rd = min(rs, rt) unsigned byte
MINUW  rd, rs, rt          # rd = min(rs, rt) unsigned word
MINUD  rd, rs, rt          # rd = min(rs, rt) unsigned dword

# Alpha packed byte operations
PERR   rd, rs, rt          # Pixel error (sum of absolute differences)
PKLB   rd, rs              # Pack longwords to bytes
PKWB   rd, rs              # Pack words to bytes
UNPKBL rd, rs              # Unpack bytes to longwords
UNPKBW rd, rs              # Unpack bytes to words
```

### Combined Bit Manipulation Examples

```c
/* Example: Efficient byte swap using PA-RISC/Alpha ops */
uint32_t bswap32(uint32_t x) {
    /* Using Alpha extract/insert */
    asm("extbl %0, %1, 0, %0" : "=r"(b0) : "r"(x));
    asm("extbl %0, %1, 1, %0" : "=r"(b1) : "r"(x));
    asm("extbl %0, %1, 2, %0" : "=r"(b2) : "r"(x));
    asm("extbl %0, %1, 3, %0" : "=r"(b3) : "r"(x));

    return (b0 << 24) | (b1 << 16) | (b2 << 8) | b3;
}

/* Example: Fast string compare using PA-RISC unit ops */
int strcmp_fast(const char *s1, const char *s2) {
    uint64_t w1, w2;
    asm("ld %0, (%1)" : "=r"(w1) : "r"(s1));
    asm("ld %0, (%1)" : "=r"(w2) : "r"(s2));
    asm("uxor %0, %1, %2" : "=r"(diff) : "r"(w1), "r"(w2));
    /* diff shows which bytes differ */
}
```

## 6. VAX POLY Instruction

### Polynomial Evaluation

The VAX POLY instruction performs efficient polynomial evaluation using Horner's method, used for transcendental functions, CRC, and checksums.

```assembly
# POLY - Polynomial Evaluation (from VAX)
# Format: POLY degree, arg, table_ptr
#
# Evaluates: result = C[0] + C[1]*x + C[2]*x^2 + ... + C[n]*x^n
# Using Horner's method: (...((C[n]*x + C[n-1])*x + C[n-2])...)

POLY   fd, degree, arg, table
# fd: Result (floating-point register)
# degree: Polynomial degree (r0-r31)
# arg: Argument value (floating-point register)
# table: Pointer to coefficient table (r0-r31)

# Variants for different FP formats
POLY.S fd, degree, arg, table   # Single precision
POLY.D fd, degree, arg, table   # Double precision
POLY.Q fd, degree, arg, table   # Quad precision
POLY.F fd, degree, arg, table   # VAX F format
POLY.G fd, degree, arg, table   # VAX G format

# Integer polynomial (for CRC, hash functions)
POLYI  rd, degree, arg, table   # Integer polynomial
```

### POLY Implementation

```c
/* Hardware implementation of POLY */
double poly_eval(int degree, double x, double *coef) {
    double result = coef[degree];

    for (int i = degree - 1; i >= 0; i--) {
        result = result * x + coef[i];
    }

    return result;
}

/* Example: sin(x) approximation */
double sin_approx(double x) {
    /* sin(x) ≈ x - x³/3! + x⁵/5! - x⁷/7! + x⁹/9! */
    double coef[5] = {
        0.0,                    /* x⁰ */
        1.0,                    /* x¹ */
        0.0,                    /* x² */
        -1.0/6.0,               /* x³ */
        0.0,                    /* x⁴ */
    };

    asm("poly.d %0, $4, %1, %2"
        : "=f"(result)
        : "f"(x), "r"(coef));

    return result;
}
```

### CRC Polynomial Instructions

```assembly
# Specialized CRC operations using POLY
CRC32  rd, rs, rt          # CRC-32 polynomial (Ethernet)
CRC32C rd, rs, rt          # CRC-32C (Castagnoli, iSCSI)
CRC16  rd, rs, rt          # CRC-16 (Modbus, USB)
CRC64  rd, rs, rt          # CRC-64 (ECMA-182)

# Programmable CRC
CRCPOLY rd, rs, rt, poly   # CRC with custom polynomial
```

## 7. PC-Relative Addressing and Large Displacements

### SPARC-Inspired PC-Relative

```assembly
# Call with 30-bit PC-relative offset (SPARC-style)
CALL   offset30            # r31 = PC + 4; PC = PC + (offset30 << 2)
                          # Range: ±2GB

# PC-relative load/store (SPARC64)
LDPC   rd, offset19        # rd = MEM[PC + (offset19 << 2)]
                          # Range: ±1MB
STPC   rs, offset19        # MEM[PC + (offset19 << 2)] = rs

# PC-relative address generation
RDPC   rd, offset19        # rd = PC + (offset19 << 2)
                          # Load effective address relative to PC
```

### RISC-V Inspired Large Immediates

```assembly
# AUIPC - Add Upper Immediate to PC (RISC-V style)
AUIPC  rd, imm20           # rd = PC + (imm20 << 12)
                          # Builds 32-bit PC-relative addresses

# Combined with ADDI for full 32-bit displacement:
AUIPC  r1, %pcrel_hi(symbol)
ADDI   r1, r1, %pcrel_lo(symbol)
# r1 now contains address of symbol

# JAL - Jump and Link with 21-bit offset (RISC-V)
JAL    rd, offset21        # rd = PC + 4; PC = PC + sign_extend(offset21 << 1)
                          # Range: ±1MB

# JALR - Jump and Link Register (RISC-V)
JALR   rd, offset12(rs)    # rd = PC + 4; PC = (rs + sign_extend(offset12)) & ~1
```

### LoongArch Inspired Instructions

```assembly
# PCADDI - PC + Immediate << 2 (LoongArch)
PCADDI   rd, imm20         # rd = PC + sign_extend(imm20 << 2)

# PCADDU12I - PC + Immediate << 12 (LoongArch)
PCADDU12I rd, imm20        # rd = PC + sign_extend(imm20 << 12)

# PCADDU18I - PC + Immediate << 18 (LoongArch)
PCADDU18I rd, imm20        # rd = PC + sign_extend(imm20 << 18)

# PCALAU12I - PC + (Immediate << 12) aligned (LoongArch)
PCALAU12I rd, imm20        # rd = (PC + sign_extend(imm20 << 12)) & ~0xFFF
                          # Aligns to 4KB boundary

# Large displacement branches
BEQC   rs, rt, offset23    # if (rs == rt) PC += sign_extend(offset23 << 2)
                          # Range: ±16MB
BNEC   rs, rt, offset23    # if (rs != rt) PC += sign_extend(offset23 << 2)
BLTC   rs, rt, offset23    # if (rs < rt) PC += sign_extend(offset23 << 2)
BGEC   rs, rt, offset23    # if (rs >= rt) PC += sign_extend(offset23 << 2)
```

### Position-Independent Code (PIC) Support

```assembly
# GOT-relative addressing
GOTPC  rd, offset          # rd = GOT + PC + offset
LDGOT  rd, offset(rs)      # rd = MEM[GOT + offset]

# PLT calls
CALLPLT offset             # Call through PLT (Procedure Linkage Table)

# Example: PIC function call
get_pc:
    jal    r31, 1f          # r31 = PC + 4
1:  addi   r31, r31, (_GLOBAL_OFFSET_TABLE_ - 1b)
    # r31 now points to GOT

load_symbol:
    ldgot  r1, symbol@GOT(r31)
    jalr   r1               # Call via GOT
```

### Large Code Model Support

```assembly
# 64-bit absolute address construction (DLX64)
LUI    rd, imm20           # rd = imm20 << 12 (bits 12-31)
ORI    rd, rd, imm12       # rd |= imm12 (bits 0-11)

# For full 64-bit address (6 instructions):
LUI    rd, imm20_0         # Bits 12-31
ORI    rd, rd, imm12_0     # Bits 0-11
SLDI   rd, rd, 32          # Shift left 32
LUI    rt, imm20_1         # Bits 44-63
ORI    rt, rt, imm12_1     # Bits 32-43
OR     rd, rd, rt          # Combine

# Or using new macro ops:
LIADDR rd, imm64           # Pseudo: Load 64-bit immediate address
```

## 8. Bytecode Execution Mode (DLX-J - Jazelle-style)

### Java/CLI Bytecode Acceleration

DLX-J provides hardware-accelerated bytecode execution for Java and .NET CLI, similar to ARM Jazelle.

```c
/* Status Register Bytecode Mode Bit */
#define DLX_STATUS_BYTECODE     0x00200000  /* Bit 21: Bytecode execution mode */

/* Bytecode Configuration Register (CP0 $32, sel 0) */
typedef struct {
    uint32_t enable:1;          /* Bit 0: Enable bytecode mode */
    uint32_t bytecode_type:2;   /* Bits 1-2: Bytecode type */
#define BYTECODE_JAVA_CLASS     0   /* Java class bytecode */
#define BYTECODE_DOTNET_CLI     1   /* .NET CLI bytecode */
#define BYTECODE_PYTHON         2   /* Python bytecode */
#define BYTECODE_JAVASCRIPT     3   /* JavaScript bytecode */

    uint32_t stack_check:1;     /* Bit 3: Automatic stack overflow check */
    uint32_t gc_barrier:1;      /* Bit 4: GC write barrier support */
    uint32_t bounds_check:1;    /* Bit 5: Array bounds checking */
    uint32_t null_check:1;      /* Bit 6: Null pointer checking */
    uint32_t type_check:1;      /* Bit 7: Runtime type checking */

    uint32_t reserved:24;
} dlx_bytecode_config_t;
```

### Bytecode Execution State

```c
/* Bytecode Execution Registers (mapped to r24-r31) */
#define BC_SP   r24         /* Bytecode stack pointer */
#define BC_PC   r25         /* Bytecode PC */
#define BC_FP   r26         /* Bytecode frame pointer */
#define BC_METH r27         /* Current method pointer */
#define BC_CONST r28        /* Constant pool pointer */
#define BC_THIS r29         /* 'this' pointer (Java) / object ref */
#define BC_TEMP0 r30        /* Temporary 0 */
#define BC_TEMP1 r31        /* Temporary 1 */

/* Bytecode State Structure */
typedef struct {
    uint32_t *bytecode_pc;      /* Current bytecode instruction */
    uint32_t *operand_stack;    /* Java operand stack */
    uint32_t stack_depth;       /* Current stack depth */
    uint32_t max_stack;         /* Maximum stack depth */

    void *constant_pool;        /* Constant pool base */
    void *method_area;          /* Method area base */
    void *heap_base;            /* Managed heap base */

    /* Exception handling */
    void *exception_table;      /* Exception handlers */
    void *current_exception;    /* Current exception object */

    /* GC support */
    uint32_t gc_epoch;          /* GC epoch counter */
    void *gc_card_table;        /* Card table for write barriers */
} bytecode_state_t;
```

### Java Bytecode Instructions

```assembly
# Enter bytecode mode
BXJAVA  rs                  # Enter Java bytecode mode, rs = method pointer
                           # Hardware interprets Java bytecode

# Common Java bytecodes accelerated in hardware:
# Stack operations
BCPUSH  imm                 # Push immediate to operand stack
BCPOP   rd                  # Pop from operand stack to register
BCDUP                       # Duplicate top of stack
BCSWAP                      # Swap top two stack elements

# Load/Store local variables
BCLOAD  local_index         # Push local variable to stack
BCSTORE local_index         # Pop stack to local variable
BCALOAD                     # Array load (with bounds check)
BCASTORE                    # Array store (with bounds check)

# Arithmetic (operates on stack)
BCADD                       # Pop two, push sum
BCSUB                       # Pop two, push difference
BCMUL                       # Pop two, push product
BCDIV                       # Pop two, push quotient (with div-by-zero check)
BCREM                       # Remainder
BCNEG                       # Negate top of stack

# Control flow
BCIF_ICMPEQ target          # Pop two, branch if equal
BCIF_ICMPNE target          # Pop two, branch if not equal
BCIF_ICMPLT target          # Pop two, branch if less than
BCGOTO target               # Unconditional branch
BCRETURN                    # Return from method

# Object operations
BCNEW class_index           # Allocate new object
BCNEWARRAY type, size       # Allocate new array
BCGETFIELD field_index      # Get instance field
BCPUTFIELD field_index      # Set instance field (with GC barrier)
BCINVOKEVIRTUAL method      # Virtual method call
BCINVOKESPECIAL method      # Special method call (constructor, private)
BCINVOKEINTERFACE method    # Interface method call

# Type operations
BCCHECKCAST class           # Type check and cast
BCINSTANCEOF class          # Instance type test

# Synchronization
BCMONITORENTER              # Acquire object monitor
BCMONITOREXIT               # Release object monitor

# Exception handling
BCTHROW                     # Throw exception
```

### .NET CLI Bytecode Support

```assembly
# .NET CLI opcodes
BCLDARG  arg_num            # Load argument
BCSTARG  arg_num            # Store argument
BCLDLOC  local_num          # Load local
BCSTLOC  local_num          # Store local

# .NET specific
BCBOX    type               # Box value type
BCUNBOX  type               # Unbox to value type
BCCALL   token              # Method call via token
BCCALLVIRT token            # Virtual call
BCNEWOBJ token              # Create object
BCASTCLASS type             # Cast to type
BCISINST type               # Test instance type

# .NET exception handling
BCLEAVE  target             # Leave protected block
BCENDFINALLY                # End finally block
```

### Bytecode Translation Cache

```c
/* Hardware Translation Cache (similar to Jazelle) */
typedef struct {
    uint32_t bytecode_pc;       /* Bytecode address */
    void *native_code;          /* Translated native code */
    uint32_t hotness;           /* Execution count */
    uint32_t valid:1;
    uint32_t optimized:1;       /* JIT optimized version */
} bytecode_cache_entry_t;

typedef struct {
    bytecode_cache_entry_t entries[1024];  /* 1K entries */
    uint32_t hits;
    uint32_t misses;
} bytecode_translation_cache_t;
```

## 9. Managed Code Mode (DLX-EE - ThumbEE-style)

### Enhanced Execution Environment

DLX-EE provides optimized support for managed code runtimes, JIT compilers, and dynamic recompilers.

```c
/* Status Register Managed Code Bit */
#define DLX_STATUS_MANAGED      0x00400000  /* Bit 22: Managed code mode */

/* Managed Code Configuration (CP0 $32, sel 1) */
typedef struct {
    uint32_t enable:1;          /* Enable managed mode */
    uint32_t null_check_mode:2; /* Null check handling */
#define NULL_CHECK_TRAP         0   /* Trap on null */
#define NULL_CHECK_BRANCH       1   /* Branch on null */
#define NULL_CHECK_SKIP         2   /* Skip instruction on null */

    uint32_t handler_base:20;   /* Exception handler base (page aligned) */
    uint32_t stack_limit:9;     /* Stack limit check mode */
} dlx_managed_config_t;
```

### Managed Code Features

```assembly
# NULL pointer checking with handler
LDNCHK  rd, (rs)            # Load with null check
                           # if (rs == 0) PC = null_handler
                           # else rd = MEM[rs]

STNCHK  rt, (rs)            # Store with null check

# Array bounds checking
LDACHK  rd, (rs), rt        # Load array with bounds check
                           # if (rt >= array.length) PC = bounds_handler
                           # else rd = MEM[rs + rt*4]

STACHK  rd, (rs), rt        # Store array with bounds check

# Stack overflow checking (automatic)
# Hardware checks SP against limit on function entry
ENTERMGD frame_size         # Enter managed function
                           # if (SP - frame_size < stack_limit)
                           #     PC = stack_overflow_handler
                           # else SP -= frame_size

# Type checks (optimized)
TYPECHK rd, rs, type_id     # rd = (rs instanceof type_id)
                           # Hardware-accelerated type checking

# GC write barriers (automatic)
STGC    rt, (rs)            # Store with GC write barrier
                           # MEM[rs] = rt
                           # card_table[rs >> PAGE_SHIFT] = DIRTY

# Handler table access
LDHANDLER rd, exception_id  # rd = handler_table[exception_id]
                           # Fast exception dispatch
```

### Managed Code Register Conventions

```
r0:  Zero
r1-r8:  Callee-saved (preserved across calls)
r9-r15: Caller-saved (temporary)
r16:    GC base pointer (managed heap base)
r17:    Thread-local storage
r18:    Exception object
r19:    Type metadata pointer
r20-r23: Reserved for managed runtime
r24-r27: Bytecode mode (when in bytecode mode)
r28:    Stack limit
r29:    Frame pointer
r30:    Stack pointer
r31:    Return address
```

### JIT Compiler Support

```assembly
# Optimized for dynamic code generation
JITPATCH addr, new_code     # Hot-patch code at addr (privileged)
                           # For JIT recompilation

# Guard page support for deoptimization
GUARDCHK guard_id           # Check guard condition
                           # if (guard[guard_id] invalid)
                           #     PC = deopt_handler
                           # Used for speculative optimization

# Inline cache support
ICACHE_CHK type, target     # Inline cache check
                           # if (obj.type == type) call target
                           # else call slow_path

# Polymorphic inline cache
PIC_CHK    type_table, size # Check against polymorphic types
```

## 10. IA-64 Style NaT Bits and Speculation

### NaT (Not a Thing) Bits

Each general-purpose register has an associated NaT bit to detect uninitialized usage and support speculative loads.

```c
/* NaT Bit Register (CP0 $33, sel 0) - 32-bit bitmap */
typedef struct {
    uint32_t nat_bits;          /* One bit per GPR r0-r31 */
} dlx_nat_register_t;

/* NaT Configuration (CP0 $33, sel 1) */
typedef struct {
    uint32_t enable:1;          /* Enable NaT tracking */
    uint32_t trap_on_nat:1;     /* Trap when NaT consumed */
    uint32_t defer_exceptions:1;/* Defer exceptions (speculation) */
    uint32_t reserved:29;
} dlx_nat_config_t;
```

### NaT Semantics

```assembly
# NaT propagation rules:
# 1. Any operation with NaT input produces NaT output
# 2. Stores with NaT address raise exception
# 3. Branches with NaT condition raise exception

# Examples:
LD.S    r1, (r2)            # r1 = MEM[r2], NaT[r1] = 0
                           # If fault, NaT[r1] = 1, defer exception

ADD     r3, r1, r4          # If NaT[r1] or NaT[r4], then NaT[r3] = 1
                           # else r3 = r1 + r4, NaT[r3] = 0

ST      r3, (r5)            # If NaT[r3] or NaT[r5], raise NaT exception
                           # else MEM[r5] = r3

CHK.S   r1, recovery        # Check NaT[r1]
                           # if NaT[r1] == 1, PC = recovery
                           # Used after speculative load

# Speculative load
LD.S    r1, (r2)            # Speculative load (may fault)
                           # On fault: NaT[r1] = 1, continue
...
CHK.S   r1, .Lrecover       # Check if load faulted
# Use r1 safely here
.Lrecover:
LD.A    r1, (r2)            # Advanced load (non-speculative)
```

### Speculative Execution Support

```assembly
# Speculation instructions
LD.S    rd, (rs)            # Speculative load (set NaT on fault)
LD.A    rd, (rs)            # Advanced load (normal load, clears NaT)
LD.SA   rd, (rs)            # Speculative advanced load

CHK.S   rs, recovery        # Check speculation, branch if NaT
CHK.A   rs, recovery        # Check advanced load

# Control speculation
# Move loads before branches
.Lloop:
    LD.S    r1, (r2)        # Speculative load (may not execute)
    BEQZ    r3, .Ldone      # May skip use of r1
    CHK.S   r1, .Lrecover   # Verify load completed
    ADD     r4, r1, r5      # Use r1
    B       .Lloop

.Lrecover:
    LD.A    r1, (r2)        # Re-execute load
    ADD     r4, r1, r5
    B       .Lloop

.Ldone:
```

## 11. Predication

### Full Predication Support

Like IA-64, DLX supports predicated execution to eliminate branches and enable aggressive ILP.

```c
/* Predicate Register File (64 1-bit registers) */
typedef struct {
    uint64_t predicates;        /* p0-p63 */
} dlx_predicate_regs_t;

/* p0 is always TRUE (hardwired to 1) */
#define PRED_ALWAYS_TRUE    0
```

### Predicate Instruction Format

```
Predicated Instruction Format:
┌────────┬────┬─────────────────────┐
│ Opcode │ Qp │  Standard fields    │
│   6    │ 6  │       20            │
└────────┴────┴─────────────────────┘

Qp: Qualifying predicate (p0-p63)
If Qp == 0 (FALSE), instruction is nullified (NOP)
```

### Predicated Instructions

```assembly
# Predicated ALU
(p1) ADD    r1, r2, r3      # if (p1) r1 = r2 + r3
(p2) SUB    r4, r5, r6      # if (p2) r4 = r5 - r6
(p3) MUL    r7, r8, r9      # if (p3) r7 = r8 * r9

# Predicated load/store
(p4) LD     r1, (r2)        # if (p4) r1 = MEM[r2]
(p5) ST     r3, (r4)        # if (p5) MEM[r4] = r3

# Predicated branches
(p6) BR     target          # if (p6) PC = target

# Predicate-defining compares (two output predicates)
CMP.EQ  p1, p2, r3, r4      # if (r3 == r4) p1=1, p2=0
                           # else p1=0, p2=1

CMP.LT  p1, p2, r3, r4      # if (r3 < r4) p1=1, p2=0
                           # else p1=0, p2=1

CMP.LE  p1, p2, r3, r4      # if (r3 <= r4) p1=1, p2=0
                           # else p1=0, p2=1

# Parallel compares (set 4 predicates)
CMP4.EQ p1,p2,p3,p4, r5,r6  # Four-way compare

# Unconditional predicate sets
CMP.EQ.UNC p1, p0, r3, r4   # Unconditional (always sets p1)
```

### Predicate Logic Operations

```assembly
# Predicate AND
PAND    p1, p2, p3          # p1 = p2 & p3

# Predicate OR
POR     p1, p2, p3          # p1 = p2 | p3

# Predicate XOR
PXOR    p1, p2, p3          # p1 = p2 ^ p3

# Predicate NOT
PNOT    p1, p2              # p1 = !p2

# Predicate NAND (useful for if-then-else)
PNAND   p1, p2, p3          # p1 = !(p2 & p3)
```

### If-Then-Else with Predication

```assembly
# Traditional code with branches:
    CMP     r1, r2
    BEQ     .Lelse
.Lthen:
    ADD     r3, r4, r5
    B       .Ldone
.Lelse:
    SUB     r3, r4, r5
.Ldone:

# Predicated version (no branches):
    CMP.EQ  p1, p2, r1, r2   # p1 = (r1==r2), p2 = !(r1==r2)
(p1) ADD    r3, r4, r5       # Execute if p1
(p2) SUB    r3, r4, r5       # Execute if p2
# Both instructions issued in parallel, one nullified
```

### Loop Predication (Software Pipelining)

```assembly
# Example: Predicated loop for software pipelining
    MOV     p3 = p0          # p3 = TRUE (prolog)
    MOV     lc = count       # Loop counter

.Lloop:
(p3) LD     r1 = (r2)       # Stage 1: Load (predicated)
(p3) ADD    r3 = r1, r4     # Stage 2: Add (predicated)
(p3) ST     r3 = (r5)       # Stage 3: Store (predicated)

    ADD     r2 = r2, #4     # Update pointers
    ADD     r5 = r5, #4

    SUB     lc = lc, #1     # Decrement counter
    CMP.NE  p3, p0, lc, #0  # p3 = (lc != 0)
(p3) BR     .Lloop          # Loop if p3

# All three stages execute in parallel, predicated by p3
```

### Predication Benefits

```c
/* Performance improvements from predication */

// 1. Branch elimination
// Before: 50% branch misprediction rate = 10 cycle penalty
// After: No branches = 0 cycle penalty

// 2. Increased ILP
// Multiple predicated instructions execute in parallel
// Processor can issue 4+ instructions per cycle

// 3. Better code density
// No need for branch target code duplication

/* Example: Min/max without branches */
int min(int a, int b) {
    asm("cmp.lt p1, p2, %1, %2"
        "\n(p1) mov %0 = %1"
        "\n(p2) mov %0 = %2"
        : "=r"(result)
        : "r"(a), "r"(b)
        : "p1", "p2");
    return result;
}
```

## 12. ARM-Style Shifter Operand

### Overview

DLX supports ARM-style barrel shifter operands, allowing ALU instructions to perform a shift/rotate on the second source operand in the same cycle. This is transparent at the assembly level and can be mixed with regular instructions.

### Shifter Operand Types

```c
/* Shifter operand encoding (alternative instruction format) */
typedef enum {
    SHIFT_LSL,      /* Logical shift left */
    SHIFT_LSR,      /* Logical shift right */
    SHIFT_ASR,      /* Arithmetic shift right */
    SHIFT_ROR,      /* Rotate right */
    SHIFT_RRX,      /* Rotate right with extend (33-bit rotate through carry) */
} shift_type_t;
```

### Instruction Format with Shifter

```
Standard Format (no shift):
┌────────┬────┬────┬────┬────────┬────────┐
│ Opcode │ Rd │ Rs │ Rt │  Func  │Reserved│
│   6    │ 5  │ 5  │ 5  │   6    │   5    │
└────────┴────┴────┴────┴────────┴────────┘

Shifter Format:
┌────────┬────┬────┬────┬───┬────┬───┬────┐
│ Opcode │ Rd │ Rs │ Rt │Sh │ShAmt│I│Func│
│   6    │ 5  │ 5  │ 5  │ 2 │  5  │1│ 3  │
└────────┴────┴────┴────┴───┴────┴───┴────┘

Sh: Shift type (LSL, LSR, ASR, ROR)
ShAmt: Shift amount (5-bit: 0-31)
I: Immediate shift (1) or register shift (0)
```

### Assembly Syntax

```assembly
# Basic ALU operations with shifter operand

# Logical shift left
ADD   r1, r2, r3, LSL #5    # r1 = r2 + (r3 << 5)
SUB   r4, r5, r6, LSL #2    # r4 = r5 - (r6 << 2)
AND   r7, r8, r9, LSL r10   # r7 = r8 & (r9 << r10)

# Logical shift right
ADD   r1, r2, r3, LSR #8    # r1 = r2 + (r3 >> 8)
OR    r4, r5, r6, LSR #16   # r4 = r5 | (r6 >> 16)

# Arithmetic shift right (sign-extend)
SUB   r1, r2, r3, ASR #4    # r1 = r2 - (r3 >>> 4)
CMP   r4, r5, ASR #7        # Compare r4 with (r5 >>> 7)

# Rotate right
EOR   r1, r2, r3, ROR #12   # r1 = r2 ^ (r3 rotated right 12)
MOV   r4, r5, ROR #8        # r4 = (r5 rotated right 8)

# Rotate right through extend (33-bit rotate via carry)
ADC   r1, r2, r3, RRX       # r1 = r2 + (r3 rotated through carry)
```

### Supported Instructions with Shifter

```assembly
# All ALU instructions support shifter operand:

# Arithmetic
ADD   rd, rs, rt, <shift>
SUB   rd, rs, rt, <shift>
ADC   rd, rs, rt, <shift>   # Add with carry
SBC   rd, rs, rt, <shift>   # Subtract with carry
RSB   rd, rs, rt, <shift>   # Reverse subtract: rd = rt - rs

# Logical
AND   rd, rs, rt, <shift>
OR    rd, rs, rt, <shift>
XOR   rd, rs, rt, <shift>
BIC   rd, rs, rt, <shift>   # Bit clear: rd = rs & ~rt

# Move
MOV   rd, rt, <shift>       # rd = rt shifted
MVN   rd, rt, <shift>       # rd = ~(rt shifted)

# Compare (no destination register)
CMP   rs, rt, <shift>       # Compare rs with (rt shifted)
CMN   rs, rt, <shift>       # Compare negative
TST   rs, rt, <shift>       # Test (AND without storing)
TEQ   rs, rt, <shift>       # Test equivalence (XOR without storing)
```

### Immediate Shift vs Register Shift

```assembly
# Immediate shift amount (5-bit constant)
ADD   r1, r2, r3, LSL #5    # Shift by constant 5

# Register shift amount (use register for shift count)
ADD   r1, r2, r3, LSL r4    # Shift by value in r4
# r1 = r2 + (r3 << (r4 & 0x1F))  (only low 5 bits of r4 used)

# Examples:
MOV   r1, r2, LSL r3        # r1 = r2 << r3
SUB   r4, r5, r6, ROR r7    # r4 = r5 - (r6 rotated right by r7)
```

### Zero Shift Special Cases

```
LSL #0: No shift (passthrough)
LSR #0: Treated as LSR #32 (result = 0)
ASR #0: Treated as ASR #32 (result = all sign bits)
ROR #0: Treated as RRX (rotate through carry)
```

### Examples and Use Cases

```c
/* Example 1: Fast array indexing */
void array_access(int *array, int index) {
    int value;

    /* Traditional: */
    value = array[index];
    /* Compiles to: */
    asm("lw %0, (%1, %2)" : "=r"(value) : "r"(array), "r"(index*4));

    /* With shifter: */
    asm("add r1, %0, %1, lsl #2" :: "r"(array), "r"(index));
    asm("lw %0, (r1)" : "=r"(value));
    /* Single add instruction with built-in shift! */
}

/* Example 2: Bit manipulation */
void set_bit(uint32_t *flags, int bit_num) {
    /* Traditional: */
    *flags |= (1 << bit_num);
    /* Requires: load, shift immediate 1, or, store */

    /* With shifter: */
    asm("mov r1, #1");
    asm("lw r2, (%0)" :: "r"(flags));
    asm("or r2, r2, r1, lsl %0" :: "r"(bit_num));
    asm("sw r2, (%0)" :: "r"(flags));
    /* One less instruction! */
}

/* Example 3: Fixed-point arithmetic */
int32_t fixed_multiply(int32_t a, int32_t b, int shift) {
    /* result = (a * b) >> shift */
    int64_t product;

    /* With shifter operand: */
    asm("mul %0, %1, %2" : "=r"(product) : "r"((int64_t)a), "r"((int64_t)b));
    asm("mov %0, %1, asr %2" : "=r"(result) : "r"(product), "r"(shift));
    /* Combined multiply and shift in minimal instructions */
}

/* Example 4: Endian swap with rotates */
uint32_t bswap32(uint32_t x) {
    uint32_t result;

    /* Using ROR to swap bytes */
    asm("mov %0, %1, ror #8" : "=r"(result) : "r"(x));
    /* Plus additional byte manipulation */

    return result;
}
```

### Condition Code Updates

```assembly
# 'S' suffix updates condition codes based on result

ADDS  r1, r2, r3, LSL #4    # r1 = r2 + (r3<<4), update flags
SUBS  r4, r5, r6, ASR #8    # r4 = r5 - (r6>>8), update flags
ANDS  r7, r8, r9, ROR #12   # r7 = r8 & (r9 rot 12), update Z flag
```

### Combining with Other Features

```assembly
# Predicated execution + shifter
(p1) ADD  r1, r2, r3, LSL #2    # if (p1) r1 = r2 + (r3<<2)

# Four-operand + shifter
FMA   r1, r2, r3, LSL #1, r4    # r1 = (r2 * (r3<<1)) + r4
```

## 13. Multiple Load/Store with Register Mask

### Overview

DLX supports ARM/PowerPC-style multiple register load/store instructions with register masks for efficient function prologues/epilogues and context saving.

### Instruction Formats

```
LDM/STM Format:
┌────────┬────┬────┬──────────────────┬────┐
│ Opcode │Mode│Base│   Register Mask  │Func│
│   6    │ 2  │ 5  │       16         │ 3  │
└────────┴────┴────┴──────────────────┴────┘

Register Mask: 16-bit bitmap (r0-r15)
Mode: Addressing mode (IA, IB, DA, DB)
Base: Base register
```

### General Purpose Register Operations

```assembly
# LDM - Load Multiple
LDM     rs, {reg_list}      # Load multiple registers from memory
LDMIA   rs, {reg_list}      # Load Multiple Increment After
LDMIB   rs, {reg_list}      # Load Multiple Increment Before
LDMDA   rs, {reg_list}      # Load Multiple Decrement After
LDMDB   rs, {reg_list}      # Load Multiple Decrement Before

# STM - Store Multiple
STM     rs, {reg_list}      # Store multiple registers to memory
STMIA   rs, {reg_list}      # Store Multiple Increment After
STMIB   rs, {reg_list}      # Store Multiple Increment Before
STMDA   rs, {reg_list}      # Store Multiple Decrement After
STMDB   rs, {reg_list}      # Store Multiple Decrement Before

# Aliases for stack operations
PUSH    {reg_list}          # Alias for STMDB sp, {reg_list}
POP     {reg_list}          # Alias for LDMIA sp, {reg_list}

# Writeback variants (update base register)
LDM     rs!, {reg_list}     # Load and update base
STM     rs!, {reg_list}     # Store and update base
```

### Addressing Modes

```c
/* Addressing modes for LDM/STM */

// IA (Increment After): address starts at base, increments after each transfer
LDMIA r10, {r1-r5}
// r1 = MEM[r10 + 0]
// r2 = MEM[r10 + 4]
// r3 = MEM[r10 + 8]
// r4 = MEM[r10 + 12]
// r5 = MEM[r10 + 16]

// IB (Increment Before): increment before each transfer
LDMIB r10, {r1-r5}
// r1 = MEM[r10 + 4]
// r2 = MEM[r10 + 8]
// r3 = MEM[r10 + 12]
// r4 = MEM[r10 + 16]
// r5 = MEM[r10 + 20]

// DA (Decrement After): address starts at base, decrements after
LDMDA r10, {r1-r5}
// r5 = MEM[r10 + 0]
// r4 = MEM[r10 - 4]
// r3 = MEM[r10 - 8]
// r2 = MEM[r10 - 12]
// r1 = MEM[r10 - 16]

// DB (Decrement Before): decrement before each transfer
LDMDB r10, {r1-r5}
// r5 = MEM[r10 - 4]
// r4 = MEM[r10 - 8]
// r3 = MEM[r10 - 12]
// r2 = MEM[r10 - 16]
// r1 = MEM[r10 - 20]
```

### Register List Syntax

```assembly
# Individual registers
LDM   r10, {r1, r3, r5, r7}

# Register ranges
LDM   r10, {r1-r8}           # Load r1, r2, r3, r4, r5, r6, r7, r8

# Mixed
LDM   r10, {r1-r4, r7, r10-r15}

# With PC (for function return)
LDM   sp!, {r4-r11, pc}      # Restore registers and return

# Full context save/restore
STMDB sp!, {r0-r15}          # Save all registers
LDMIA sp!, {r0-r15}          # Restore all registers
```

### Function Prologue/Epilogue

```assembly
# Function prologue
function_entry:
    PUSH  {r4-r11, lr}       # Save callee-saved regs + return address
    # Equivalent to: STMDB sp!, {r4-r11, lr}

    # Function body
    ...

function_exit:
    POP   {r4-r11, pc}       # Restore regs and return
    # Equivalent to: LDMIA sp!, {r4-r11, pc}
```

### FPU Multiple Load/Store

```assembly
# FLDM - FPU Load Multiple
FLDM    rs, {fp_list}        # Load multiple FP registers
FLDMIA  rs, {f0-f7}          # Load f0 through f7
FLDMDB  rs!, {f16-f31}       # Load f16-f31, update base

# FSTM - FPU Store Multiple
FSTM    rs, {fp_list}        # Store multiple FP registers
FSTMIA  rs!, {f0-f15}        # Store f0-f15, update base
FSTMDB  sp!, {f0-f31}        # Push all FP registers

# Double precision (load/store pairs)
FLDMD   rs, {d0-d15}         # Load double-precision regs
FSTMD   rs!, {d0-d15}        # Store double-precision regs

# Quad precision (load/store quads)
FLDMQ   rs, {q0, q4, q8}     # Load quad-precision regs
FSTMQ   rs!, {q0-q12}        # Store quad-precision regs
```

### Vector Unit Multiple Load/Store

```assembly
# VLDM - Vector Load Multiple
VLDM    rs, {v0-v15}         # Load vector registers
VLDMIA  rs!, {v0-v31}        # Load all vectors, update base

# VSTM - Vector Store Multiple
VSTM    rs, {v0-v7}          # Store vector registers
VSTMDB  sp!, {v0-v31}        # Push all vector registers

# Predicate register save/restore
PLDM    rs, {p0-p15}         # Load predicate registers
PSTM    rs!, {p0-p63}        # Store all predicates
```

### Context Switch Example

```assembly
# Fast context switch with multiple load/store

save_context:
    # Save GPRs
    STMIA  r0!, {r1-r15}     # Save all GPRs to context block
    # Save FPRs
    FSTMIA r0!, {f0-f31}     # Save all FP registers
    # Save vector regs
    VSTMIA r0!, {v0-v31}     # Save all vector registers
    # Save predicates
    PSTMIA r0!, {p0-p63}     # Save all predicate registers

    jr     r31                # Return

restore_context:
    # Restore GPRs
    LDMIA  r0!, {r1-r15}
    # Restore FPRs
    FLDMIA r0!, {f0-f31}
    # Restore vector regs
    VLDMIA r0!, {v0-v31}
    # Restore predicates
    PLDMIA r0!, {p0-p63}

    jr     r31
```

### Interrupt Handler

```assembly
interrupt_handler:
    # Fast register save (no memory allocation needed)
    STMDB  sp!, {r0-r12, lr}  # Save context

    # Handle interrupt
    jal    interrupt_service_routine

    # Fast restore and return
    LDMIA  sp!, {r0-r12, pc}  # Restore and return from interrupt
```

### Sparse Register Mask

```assembly
# Save only specific registers
PUSH   {r4, r6, r8, r10, lr}   # Save sparse set

# Function uses only these registers
...

# Restore sparse set
POP    {r4, r6, r8, r10, pc}   # Restore and return
```

### Performance Benefits

```c
/* Compare traditional vs multiple load/store */

// Traditional: 16 instructions for context save
sw   r1, 0(r10)
sw   r2, 4(r10)
sw   r3, 8(r10)
// ... 13 more stores ...
sw   r15, 60(r10)

// With STM: 1 instruction!
asm("stmia r10, {r1-r15}");

/* Cycle counts:
 * Traditional: 16 stores × 1 cycle = 16 cycles (serial)
 * STM: 1 instruction, 16 cycles (pipelined)
 * But: reduced I-cache pressure, fewer instruction fetches
 */
```

### Alignment Requirements

```c
/* Multiple load/store requires aligned base address */

// Base must be 4-byte aligned for word transfers
LDMIA  r10, {r1-r8}     // r10 must be aligned to 4

// Base must be 8-byte aligned for double-precision
FLDMD  r10, {d0-d15}    // r10 must be aligned to 8

// Base must be 16-byte aligned for vectors
VLDM   r10, {v0-v15}    // r10 must be aligned to 16

// Misaligned access generates exception
```

### Atomic Multiple Operations

```assembly
# Atomic load-link / store-conditional multiple (advanced)
LLDM   rs, {reg_list}    # Load-linked multiple
SCDM   rs, {reg_list}    # Store-conditional multiple

# Example: Atomic structure update
retry:
    LLDM   r10, {r1-r4}   # Load 4-word structure
    # Modify r1-r4
    SCDM   r10, {r1-r4}   # Attempt atomic store
    beqz   r1, retry      # Retry if failed
```

## 14. PowerPC-Style Multiple Condition Registers

### Overview

DLX supports PowerPC-inspired Condition Registers (CR) that allow multiple comparison results to be stored simultaneously. This enables complex conditional logic without overwriting previous comparison results.

### Condition Register Architecture

```c
/* Condition Register File - 8 fields of 4 bits each */
typedef struct {
    uint32_t lt:1;    /* Less Than */
    uint32_t gt:1;    /* Greater Than */
    uint32_t eq:1;    /* Equal */
    uint32_t so:1;    /* Summary Overflow */
} cr_field_t;

/* Complete CR - 32 bits total */
typedef struct {
    cr_field_t cr0;   /* Bits 0-3: Primary comparison */
    cr_field_t cr1;   /* Bits 4-7: Secondary comparison */
    cr_field_t cr2;   /* Bits 8-11 */
    cr_field_t cr3;   /* Bits 12-15 */
    cr_field_t cr4;   /* Bits 16-19 */
    cr_field_t cr5;   /* Bits 20-23 */
    cr_field_t cr6;   /* Bits 24-27 */
    cr_field_t cr7;   /* Bits 28-31 */
} condition_register_t;

/* CR as 32-bit value */
/* Format: [CR0][CR1][CR2][CR3][CR4][CR5][CR6][CR7] */
/* Each field: [LT|GT|EQ|SO] */

/* Example CR value: 0x84200000
 * CR0 = 0b1000 (LT=1, GT=0, EQ=0, SO=0)  - result < 0
 * CR1 = 0b0100 (LT=0, GT=1, EQ=0, SO=0)  - result > 0
 * CR2 = 0b0010 (LT=0, GT=0, EQ=1, SO=0)  - result == 0
 */
```

### Special Registers

```c
/* Special Registers for CR */
#define SR_CR       0x20    /* Condition Register (32-bit) */
#define SR_XER      0x21    /* Fixed Point Exception Register */

/* XER Register bits used by CR */
#define XER_SO      (1 << 31)   /* Summary Overflow */
#define XER_OV      (1 << 30)   /* Overflow */
#define XER_CA      (1 << 29)   /* Carry */

/* Access CR */
MFCR    rd              /* Move From CR: rd = CR */
MTCR    rs              /* Move To CR: CR = rs */
MCRFS   crd, crs        /* Move CR Field: CR[crd] = CR[crs] */
MCRXR   crd             /* Move to CR from XER */
```

### Extended Compare Instructions

```assembly
# Compare with CR field specification
# Format: CMP{cond} crD, rA, rB
# Sets CR field crD based on comparison of rA and rB

# Signed integer compare
CMP     cr0, r3, r4     # CR0 = compare(r3, r4) signed
CMPI    cr1, r5, 100    # CR1 = compare(r5, 100) signed

# Unsigned integer compare
CMPU    cr2, r6, r7     # CR2 = compare(r6, r7) unsigned
CMPUI   cr3, r8, 0xFF   # CR3 = compare(r8, 255) unsigned

# Floating-point compare
FCMP    cr4, f1, f2     # CR4 = compare(f1, f2) FP
FCMPD   cr5, d1, d2     # CR5 = compare(d1, d2) double

# Default field (CR0) if not specified
CMP     r3, r4          # Same as: CMP cr0, r3, r4
```

### CR Field Encoding After Compare

```c
/* Compare result encoding in CR field */
#define CR_LT   0x8     /* 0b1000: rA < rB */
#define CR_GT   0x4     /* 0b0100: rA > rB */
#define CR_EQ   0x2     /* 0b0010: rA == rB */
#define CR_SO   0x1     /* 0b0001: Summary overflow */

/* Examples after signed compare: CMP cr1, r3, r4 */
/* If r3 < r4:  CR1 = 0b1000 (LT set) */
/* If r3 > r4:  CR1 = 0b0100 (GT set) */
/* If r3 == r4: CR1 = 0b0010 (EQ set) */
/* SO bit copied from XER.SO */
```

### Conditional Branches Using CR Fields

```assembly
# Branch on CR field condition
# Format: B{condition}{crN} target
# Tests specified CR field, branches if condition true

# Branch if CR0.LT = 1 (less than)
BLT     target          # Branch if CR0 shows LT
BLT     cr2, target     # Branch if CR2 shows LT

# Branch if CR0.GT = 1 (greater than)
BGT     target          # Branch if CR0 shows GT
BGT     cr3, target     # Branch if CR3 shows GT

# Branch if CR0.EQ = 1 (equal)
BEQ     target          # Branch if CR0 shows EQ
BEQ     cr1, target     # Branch if CR1 shows EQ

# Branch if CR0.SO = 1 (summary overflow)
BSO     target          # Branch if CR0.SO set

# Inverted conditions
BNE     cr2, target     # Branch if CR2.EQ = 0
BLE     cr3, target     # Branch if CR3.GT = 0
BGE     cr4, target     # Branch if CR4.LT = 0

# Combined conditions
BSOL    cr5, target     # Branch if CR5.SO = 1 or CR5.LT = 1
```

### CR Logical Operations

```assembly
# CR bit manipulation (PowerPC-style)
# Each CR field has 4 bits, so CR has 32 bits total
# Bit numbering: bit 0 = CR0.LT, bit 1 = CR0.GT, bit 2 = CR0.EQ, bit 3 = CR0.SO, etc.

# CR logical operations
CRAND   bt, ba, bb      # CR[bt] = CR[ba] & CR[bb]
CROR    bt, ba, bb      # CR[bt] = CR[ba] | CR[bb]
CRXOR   bt, ba, bb      # CR[bt] = CR[ba] ^ CR[bb]
CRNAND  bt, ba, bb      # CR[bt] = ~(CR[ba] & CR[bb])
CRNOR   bt, ba, bb      # CR[bt] = ~(CR[ba] | CR[bb])
CREQV   bt, ba, bb      # CR[bt] = ~(CR[ba] ^ CR[bb])
CRANDC  bt, ba, bb      # CR[bt] = CR[ba] & ~CR[bb]
CRORC   bt, ba, bb      # CR[bt] = CR[ba] | ~CR[bb]

# Examples
CRAND   0, 4, 8         # CR0.LT = CR1.LT & CR2.LT
CROR    1, 5, 9         # CR0.GT = CR1.GT | CR2.GT
CRXOR   2, 6, 10        # CR0.EQ = CR1.EQ ^ CR2.EQ

# Macro for common patterns
#define CR_BIT(field, bit)  ((field) * 4 + (bit))
#define CR_LT_BIT(field)    CR_BIT(field, 0)
#define CR_GT_BIT(field)    CR_BIT(field, 1)
#define CR_EQ_BIT(field)    CR_BIT(field, 2)
#define CR_SO_BIT(field)    CR_BIT(field, 3)

# Example: Combine CR1.LT and CR2.GT
CRAND   0, CR_LT_BIT(1), CR_GT_BIT(2)  # CR[0] = CR1.LT & CR2.GT
```

### Multi-Way Comparisons

```assembly
# Example: Multi-way comparison for sorting
# Compare three values: r3, r4, r5

compare_three_values:
    # Compare r3 vs r4
    CMP     cr0, r3, r4     # CR0 = compare(r3, r4)

    # Compare r4 vs r5
    CMP     cr1, r4, r5     # CR1 = compare(r4, r5)

    # Compare r3 vs r5
    CMP     cr2, r3, r5     # CR2 = compare(r3, r5)

    # Now have 3 independent comparisons stored
    # Can branch based on any combination

    # If r3 < r4 and r4 < r5 → ascending order
    BLT     cr0, .check_cr1
    b       .not_ascending
.check_cr1:
    BLT     cr1, .ascending
    b       .not_ascending

.ascending:
    # r3 < r4 < r5
    li      r1, 0
    jr      r31

.not_ascending:
    # Check other orderings using cr2...
```

### Complex Conditional Logic

```assembly
# Example: Complex condition - (a < b) AND (c > d) OR (e == f)
# Using CR fields to avoid repeated comparisons

complex_condition:
    # Perform all comparisons first
    CMP     cr0, r3, r4     # a < b  → CR0
    CMP     cr1, r5, r6     # c > d  → CR1
    CMP     cr2, r7, r8     # e == f → CR2

    # Combine: (CR0.LT AND CR1.GT) OR CR2.EQ
    # Use CR logical ops

    CRAND   12, 0, 5        # CR3.LT = CR0.LT & CR1.GT
    CROR    12, 12, 10      # CR3.LT = CR3.LT | CR2.EQ

    # Branch on result
    BT      12, condition_true    # Branch if bit 12 (CR3.LT) set

    # Condition false
    li      r1, 0
    jr      r31

condition_true:
    li      r1, 1
    jr      r31
```

### Integration with Predication

```assembly
# CR fields can feed predicate registers for predicated execution
# Copy CR field to predicate register

CR2PRED p0, cr0         # p0 = CR0 (4 bits → 4 predicates)
CR2PRED p4, cr1         # p4-p7 = CR1 fields
CR2PRED p8, cr2         # p8-p11 = CR2 fields

# Example: Conditional execution based on comparison
CMP     cr0, r3, r4     # Compare
CR2PRED p0, cr0         # p0=LT, p1=GT, p2=EQ, p3=SO

# Predicated instructions
(p0)    ADD  r5, r6, r7  # Execute if r3 < r4
(p1)    SUB  r5, r6, r7  # Execute if r3 > r4
(p2)    XOR  r5, r5, r5  # Execute if r3 == r4
```

### Record Bit (PowerPC-Style)

```assembly
# Instructions with "." suffix update CR0 based on result
# Similar to PowerPC record bit

# Arithmetic with CR0 update
ADD.    r3, r4, r5      # r3 = r4 + r5, update CR0
SUB.    r6, r7, r8      # r6 = r7 - r8, update CR0
AND.    r9, r10, r11    # r9 = r10 & r11, update CR0

# CR0 updated based on result:
# If result < 0:  CR0 = 0b1000 (LT)
# If result > 0:  CR0 = 0b0100 (GT)
# If result == 0: CR0 = 0b0010 (EQ)
# CR0.SO copied from XER.SO

# Example: Loop with automatic CR update
    LI      r3, 100         # counter
loop:
    SUBI.   r3, r3, 1       # r3--, update CR0
    # ... loop body ...
    BNE     loop            # Branch if CR0.EQ not set (r3 != 0)
```

### Use Cases

#### 1. Sorting Networks

```c
/* Compare-exchange using multiple CR fields */
void compare_exchange(int *a, int *b, int *c, int *d)
{
    asm volatile(
        "CMP    cr0, %0, %1    \n"  // Compare a, b
        "CMP    cr1, %2, %3    \n"  // Compare c, d
        "BLE    cr0, 1f        \n"  // Skip if a <= b
        "SWAP   %0, %1         \n"  // Swap a, b
        "1:                    \n"
        "BLE    cr1, 2f        \n"  // Skip if c <= d
        "SWAP   %2, %3         \n"  // Swap c, d
        "2:                    \n"
        : "+r"(*a), "+r"(*b), "+r"(*c), "+r"(*d)
    );
}
```

#### 2. Range Checking

```assembly
# Check if value is in range [min, max]
# Uses two comparisons without overwriting results

range_check:
    # r3 = value, r4 = min, r5 = max
    CMPU    cr0, r3, r4     # value >= min?
    CMPU    cr1, r3, r5     # value <= max?

    # Combine: BGE(cr0) AND BLE(cr1)
    CRANDC  0, 1, 4         # CR0.LT = CR0.GT & ~CR1.LT
    BT      0, in_range

out_of_range:
    li      r1, 0
    jr      r31

in_range:
    li      r1, 1
    jr      r31
```

#### 3. Three-Way String Compare

```c
/* Three-way string comparison with multiple CR fields */
int strcmp_multiway(const char *s1, const char *s2, const char *s3)
{
    while (*s1 && *s2 && *s3) {
        asm volatile(
            "lbu    $t0, 0(%0)     \n"  // Load s1[i]
            "lbu    $t1, 0(%1)     \n"  // Load s2[i]
            "lbu    $t2, 0(%2)     \n"  // Load s3[i]
            "CMP    cr0, $t0, $t1  \n"  // s1[i] vs s2[i]
            "CMP    cr1, $t1, $t2  \n"  // s2[i] vs s3[i]
            "CMP    cr2, $t0, $t2  \n"  // s1[i] vs s3[i]
            "BNE    cr0, not_equal \n"
            "BNE    cr1, not_equal \n"
            "BNE    cr2, not_equal \n"
            : : "r"(s1), "r"(s2), "r"(s3)
        );
        s1++; s2++; s3++;
    }
    return 0;
not_equal:
    /* Analyze CR0-CR2 to determine ordering */
    return analyze_cr_fields();
}
```

### Instruction Encoding

```
┌────────┬────┬────┬────┬────────────┬────┐
│ Opcode │crD │ rA │ rB │    Func    │ 0  │  Compare
│   6    │ 3  │ 5  │ 5  │     11     │ 2  │
└────────┴────┴────┴────┴────────────┴────┘

crD: Target CR field (0-7)
rA, rB: Source registers
Func: Compare type
  0x000: CMP   (signed)
  0x001: CMPU  (unsigned)
  0x002: FCMP  (floating-point)
  0x003: FCMPD (double FP)

┌────────┬────┬────┬─────────────────────┐
│ Opcode │ bt │ ba │   bb    │   Func   │  CR Logical
│   6    │ 5  │ 5  │    5    │    11    │
└────────┴────┴────┴─────────┴──────────┘

bt: Target CR bit (0-31)
ba, bb: Source CR bits (0-31)
Func: CR logical operation
  0x001: CRAND
  0x002: CROR
  0x003: CRXOR
  0x004: CRNAND
  0x005: CRNOR
  0x006: CREQV
  0x007: CRANDC
  0x008: CRORC
```

### Context Switching

```c
/* Save/restore CR in context switch */
struct task_struct {
    uint32_t cr;        /* Condition Register */
    uint32_t xer;       /* Fixed Point Exception Register */
    /* ... other state ... */
};

void save_cr_state(struct task_struct *task)
{
    asm volatile("mfcr %0" : "=r"(task->cr));
    asm volatile("mfsr %0, $XER" : "=r"(task->xer));
}

void restore_cr_state(struct task_struct *task)
{
    asm volatile("mtcr %0" :: "r"(task->cr));
    asm volatile("mtsr $XER, %0" :: "r"(task->xer));
}
```

### Performance Considerations

```c
/*
 * Multiple CR fields enable:
 *
 * 1. Parallel comparisons without dependency chains
 * 2. Complex conditional logic without repeated compares
 * 3. Efficient multi-way branches
 * 4. Reduced branch misprediction penalties
 *
 * Cost: 32 bits of state, minimal area overhead
 * Benefit: 2-5x speedup in comparison-heavy code
 */
```

### Summary

**Key Features**:
- 8 independent CR fields (CR0-CR7), 4 bits each
- Each field stores LT, GT, EQ, SO comparison result
- Compare instructions specify target CR field
- Branch instructions can test any CR field
- CR logical operations combine conditions
- Integration with predication system
- Record bit for automatic CR0 update

**Use Cases**:
- Sorting and searching algorithms
- Range checking and validation
- Multi-way comparisons
- Complex conditional logic
- Predicated execution
- Reduced branch misprediction

**PowerPC Compatibility**:
- Inspired by PowerPC CR architecture
- Similar encoding and semantics
- CR logical operations match PowerPC
- Record bit functionality

## 15. ARM64-Style Pointer Authentication (PAC)

### Overview

DLX implements ARM64-inspired Pointer Authentication Codes (PAC) to provide cryptographic protection against code reuse attacks (ROP, JOP) and memory corruption exploits. PAC adds cryptographic signatures to pointers, making them unusable if tampered with.

### Security Model

```c
/*
 * Pointer Authentication protects against:
 * - Return-Oriented Programming (ROP)
 * - Jump-Oriented Programming (JOP)
 * - Buffer overflow pointer corruption
 * - vtable hijacking
 * - Function pointer corruption
 *
 * PAC embeds a cryptographic signature in unused high bits of pointers
 */
```

### PAC Key Architecture

```c
/* Five 128-bit PAC keys stored in special registers */
typedef struct {
    uint64_t low;
    uint64_t high;
} pac_key_t;

/* PAC Key Registers (accessible only in kernel mode) */
pac_key_t APIAKey;   /* Instruction pointer A key */
pac_key_t APIBKey;   /* Instruction pointer B key */
pac_key_t APDAKey;   /* Data pointer A key */
pac_key_t APDBKey;   /* Data pointer B key */
pac_key_t APGAKey;   /* Generic authentication key */

/* Key management */
#define SR_APIAKEY_LOW   0x50   /* APIAKey[63:0] */
#define SR_APIAKEY_HIGH  0x51   /* APIAKey[127:64] */
#define SR_APIBKEY_LOW   0x52
#define SR_APIBKEY_HIGH  0x53
#define SR_APDAKEY_LOW   0x54
#define SR_APDAKEY_HIGH  0x55
#define SR_APDBKEY_LOW   0x56
#define SR_APDBKEY_HIGH  0x57
#define SR_APGAKEY_LOW   0x58
#define SR_APGAKEY_HIGH  0x59

/* Initialize keys (kernel only) */
void init_pac_keys(void)
{
    pac_key_t key;

    /* Generate random keys */
    key.low = random64();
    key.high = random64();

    asm volatile("mtsr $APIAKEY_LOW, %0" :: "r"(key.low));
    asm volatile("mtsr $APIAKEY_HIGH, %0" :: "r"(key.high));
    /* ... initialize other keys ... */
}
```

### PAC Bit Layout (32-bit DLX)

```
┌──────────────┬──────────────────────────┐
│   PAC (8)    │    Address (24)         │
│   bits 31-24 │    bits 23-0            │
└──────────────┴──────────────────────────┘

For 32-bit pointers:
- Bits 23-0: Actual address (16 MB addressable per pointer)
- Bits 31-24: PAC signature (8 bits)

For 64-bit pointers (if 64-bit extension enabled):
- Bits 47-0: Actual address (256 TB addressable)
- Bits 63-48: PAC signature (16 bits)
```

### PAC Instructions

#### Sign Instructions

```assembly
# Sign instruction pointers (for return addresses, function pointers)
PACIA   rd, rs          # Sign rd using APIAKey with context rs
PACIB   rd, rs          # Sign rd using APIBKey with context rs

# Sign data pointers (for C++ vtables, data structure pointers)
PACDA   rd, rs          # Sign rd using APDAKey with context rs
PACDB   rd, rs          # Sign rd using APDBKey with context rs

# Generic authentication
PACGA   rd, rn, rm      # rd = PAC(rn, rm) using APGAKey

# Sign with SP as context (common case)
PACIASP                 # PACIA lr, sp (sign return address)
PACIBSP                 # PACIB lr, sp

# Examples
    # Sign return address before call
    PACIASP             # Sign lr with sp as context
    JAL     function    # Call function

    # Sign a function pointer
    PACIA   r3, r4      # Sign r3 with r4 as context

    # Sign a vtable pointer
    PACDA   r5, r6      # Sign r5 with r6 as context
```

#### Authenticate Instructions

```assembly
# Authenticate and strip PAC from instruction pointers
AUTIA   rd, rs          # Authenticate rd using APIAKey with context rs
AUTIB   rd, rs          # Authenticate rd using APIBKey with context rs

# Authenticate and strip PAC from data pointers
AUTDA   rd, rs          # Authenticate rd using APDAKey with context rs
AUTDB   rd, rs          # Authenticate rd using APDBKey with context rs

# Authenticate with SP as context
AUTIASP                 # AUTIA lr, sp (authenticate return address)
AUTIBSP                 # AUTIB lr, sp

# Examples
    # Authenticate return address after return
    AUTIASP             # Authenticate lr with sp as context
    JR      lr          # Return to authenticated address

    # Authenticate before indirect call
    AUTIA   r3, r4      # Authenticate r3 with r4 as context
    JALR    r3          # Call authenticated function

    # Authenticate vtable pointer before use
    AUTDA   r5, r6      # Authenticate r5 with r6 as context
    LW      r7, 0(r5)   # Load from authenticated vtable
```

#### Strip Instructions (No Authentication)

```assembly
# Strip PAC without authenticating (for debugging, special cases)
XPACI   rd              # Strip PAC from instruction pointer
XPACD   rd              # Strip PAC from data pointer

# These do NOT verify authenticity, only remove the PAC bits
# Use with caution - only for debugging or controlled scenarios
```

### PAC Algorithm

```c
/* Simplified PAC algorithm (actual implementation is hardware-specific) */
uint32_t compute_pac(uint32_t pointer, uint64_t context, pac_key_t key)
{
    uint64_t input[3];
    uint64_t hash;

    /* Prepare input for hash */
    input[0] = pointer;
    input[1] = context;
    input[2] = key.low ^ key.high;

    /* Compute PAC using QARMA or PRINCE cipher (ARM uses QARMA) */
    hash = qarma64(input, key);

    /* Extract PAC bits (high 8 bits for 32-bit pointers) */
    uint32_t pac = (hash >> 56) & 0xFF;

    /* Insert PAC into high bits of pointer */
    return (pointer & 0x00FFFFFF) | (pac << 24);
}

uint32_t authenticate_pac(uint32_t signed_ptr, uint64_t context, pac_key_t key)
{
    uint32_t ptr = signed_ptr & 0x00FFFFFF;  /* Extract address */
    uint32_t pac = (signed_ptr >> 24) & 0xFF; /* Extract PAC */

    /* Recompute expected PAC */
    uint32_t expected_pac = (compute_pac(ptr, context, key) >> 24) & 0xFF;

    if (pac != expected_pac) {
        /* Authentication failure - poison pointer or trap */
        return poison_pointer(signed_ptr);  /* Sets bit 23 */
    }

    return ptr;  /* Return clean pointer */
}

uint32_t poison_pointer(uint32_t ptr)
{
    /* ARM64 strategy: flip bit in address to create invalid pointer */
    return ptr | 0x00800000;  /* Set bit 23 - creates invalid address */
}
```

### Protected Function Calls

```assembly
# Standard function call with PAC
function_call_protected:
    # Before call
    PACIASP             # Sign return address (lr) with sp
    JAL     target      # Call function

    # After return
    AUTIASP             # Authenticate return address
    # If authentication fails, lr is poisoned → crash on use
    JR      lr          # Return (will fault if lr was corrupted)

# Tail call with PAC
tail_call_protected:
    AUTIASP             # Authenticate current lr
    PACIASP             # Re-sign for tail call
    J       target      # Tail call
```

### Protected Indirect Calls

```assembly
# Function pointer call with PAC
indirect_call_protected:
    # r3 = function pointer (signed)
    # r4 = context (e.g., object pointer for C++)

    PACIA   lr, sp      # Sign our return address
    AUTIA   r3, r4      # Authenticate function pointer
    JALR    r3          # Call authenticated function
    AUTIA   lr, sp      # Authenticate return address
    JR      lr          # Return

# vtable call (C++) with PAC
vtable_call_protected:
    # r5 = object pointer
    # r6 = vtable pointer (signed with object as context)

    AUTDA   r6, r5      # Authenticate vtable pointer
    LW      r7, 8(r6)   # Load method pointer from vtable
    AUTIA   r7, r6      # Authenticate method pointer
    JALR    r7          # Call authenticated method
```

### Context Switching with PAC

```c
struct task_struct {
    pac_key_t apikey_a; /* Per-process keys */
    pac_key_t apikey_b;
    pac_key_t apdkey_a;
    pac_key_t apdkey_b;
    pac_key_t apgkey;
};

void switch_pac_keys(struct task_struct *next)
{
    /* Load process-specific PAC keys */
    asm volatile(
        "mtsr $APIAKEY_LOW, %0\n"
        "mtsr $APIAKEY_HIGH, %1\n"
        "mtsr $APIBKEY_LOW, %2\n"
        "mtsr $APIBKEY_HIGH, %3\n"
        :: "r"(next->apikey_a.low), "r"(next->apikey_a.high),
           "r"(next->apikey_b.low), "r"(next->apikey_b.high)
    );
    /* ... load other keys ... */
}
```

### PAC in System Calls

```assembly
# System call with PAC protection
syscall_entry:
    # User mode → Kernel mode
    # lr contains signed return address

    AUTIASP             # Authenticate user return address

    # Save authenticated lr
    SW      lr, saved_lr(sp)

    # Process syscall
    # ...

    # Return to user mode
    LW      lr, saved_lr(sp)
    PACIASP             # Re-sign return address for user
    ERET                # Return to user mode
```

### PAC Exception Handling

```c
/* PAC fault occurs when authenticated pointer is used */
void handle_pac_fault(struct pt_regs *regs)
{
    /* PAC authentication failure detected */
    printk(KERN_ALERT "PAC authentication failure at PC=%08x\n",
           regs->pc);

    /* This is likely a security violation */
    /* Terminate the process */
    do_exit(SIGSEGV);
}
```

### Compiler Integration

```c
/* GCC/Clang attributes for PAC */

/* Enable PAC for all functions in this file */
#pragma GCC target "+pac"

/* Function-specific PAC control */
__attribute__((ptrauth_returns))        /* Sign return addresses */
__attribute__((ptrauth_calls))          /* Sign function pointers */
__attribute__((ptrauth_vtables))        /* Sign C++ vtables */

/* Example */
__attribute__((ptrauth_returns))
int secure_function(int x)
{
    /* Return address automatically signed/authenticated */
    return x * 2;
}

/* Disable PAC for specific function (e.g., interrupt handlers) */
__attribute__((no_ptrauth))
void interrupt_handler(void)
{
    /* No PAC overhead in time-critical code */
}
```

### PAC Control Register

```c
/* CP0 PAC Configuration Register */
typedef struct {
    uint32_t enable:1;          /* Bit 0: Enable PAC */
    uint32_t kernel_enable:1;   /* Bit 1: Enable PAC in kernel */
    uint32_t user_enable:1;     /* Bit 2: Enable PAC in user mode */
    uint32_t trap_on_fail:1;    /* Bit 3: Trap on auth failure */
    uint32_t poison_on_fail:1;  /* Bit 4: Poison pointer on failure */
    uint32_t key_per_process:1; /* Bit 5: Per-process keys */
    uint32_t reserved:26;
} pac_config_t;

#define SR_PAC_CONFIG   0x5A

/* Enable PAC */
void enable_pac(void)
{
    uint32_t config = 0;
    config |= (1 << 0);  /* Enable PAC */
    config |= (1 << 1);  /* Enable in kernel */
    config |= (1 << 2);  /* Enable in user mode */
    config |= (1 << 3);  /* Trap on failure */

    asm volatile("mtsr $PAC_CONFIG, %0" :: "r"(config));
}
```

### Use Cases

#### 1. Return Address Protection

```c
/* Automatic return address protection */
int protected_function(int arg)
{
    /* Compiler inserts PACIASP at function entry */

    char buffer[64];
    /* Even if buffer overflow occurs, cannot corrupt return address */
    gets(buffer);  /* Dangerous but return address is protected */

    /* Compiler inserts AUTIASP before return */
    return arg;
}
```

#### 2. C++ Virtual Function Protection

```cpp
class Base {
public:
    virtual void method() {
        /* vtable pointer is PAC-signed */
    }
};

void call_virtual(Base *obj) {
    /* vtable pointer authenticated before dereference */
    obj->method();  /* Safe from vtable hijacking */
}
```

#### 3. Function Pointer Protection

```c
typedef int (*callback_t)(int);

void register_callback(callback_t func, void *context) {
    /* Sign function pointer with context */
    asm volatile("pacia %0, %1" : "+r"(func) : "r"(context));

    /* Store signed pointer */
    callbacks[num_callbacks++] = func;
}

void invoke_callback(int index, int arg) {
    callback_t func = callbacks[index];

    /* Authenticate before calling */
    asm volatile("autia %0, %1" : "+r"(func) : "r"(context));

    /* Call authenticated function */
    func(arg);  /* Safe from function pointer corruption */
}
```

### Performance Impact

```c
/*
 * PAC overhead:
 * - Sign: 1-3 cycles (hardware accelerated)
 * - Authenticate: 1-3 cycles
 * - Typical function call overhead: 2-6 cycles
 * - Minimal impact: ~2-5% on most workloads
 *
 * Security benefit:
 * - Stops ~90% of code reuse attacks
 * - Makes ROP chains nearly impossible
 * - Protects critical pointers at low cost
 */
```

### Instruction Encoding

```
┌────────┬────┬────┬─────┬───────────────┐
│ Opcode │ rd │ rs │Func │    Reserved   │  PAC Sign
│   6    │ 5  │ 5  │ 3   │      13       │
└────────┴────┴────┴─────┴───────────────┘

Func:
  0x0: PACIA  (sign instruction pointer, A key)
  0x1: PACIB  (sign instruction pointer, B key)
  0x2: PACDA  (sign data pointer, A key)
  0x3: PACDB  (sign data pointer, B key)
  0x4: AUTIA  (authenticate instruction pointer, A key)
  0x5: AUTIB  (authenticate instruction pointer, B key)
  0x6: AUTDA  (authenticate data pointer, A key)
  0x7: AUTDB  (authenticate data pointer, B key)

┌────────┬────┬───────────────────────────┐
│ Opcode │ rd │         Func              │  PAC Strip/SP variants
│   6    │ 5  │           21              │
└────────┴────┴───────────────────────────┘

Func:
  0x000: XPACI   (strip instruction PAC)
  0x001: XPACD   (strip data PAC)
  0x010: PACIASP (sign lr with sp)
  0x011: PACIBSP
  0x012: AUTIASP (authenticate lr with sp)
  0x013: AUTIBSP
```

### Boot-Time PAC Initialization

```c
/* Initialize PAC system at boot */
void __init pac_init(void)
{
    pac_key_t keys[5];
    int i;

    /* Generate cryptographically random keys */
    for (i = 0; i < 5; i++) {
        get_random_bytes(&keys[i], sizeof(pac_key_t));
    }

    /* Load keys into hardware */
    load_pac_keys(keys);

    /* Enable PAC */
    enable_pac();

    printk(KERN_INFO "PAC initialized with random keys\n");
}
```

### Summary

**Key Features**:
- Five 128-bit PAC keys (APIA, APIB, APDA, APDB, APGA)
- Sign/authenticate instruction and data pointers
- 8-bit PAC in high bits of 32-bit pointers (16-bit on 64-bit)
- Hardware-accelerated QARMA cipher
- Trap or poison on authentication failure
- Compiler integration for automatic protection
- Per-process keys for isolation

**Security Benefits**:
- Prevents ROP/JOP attacks
- Protects return addresses from corruption
- Secures C++ vtables
- Validates function pointers
- Minimal performance overhead (2-5%)

**ARM64 Compatibility**:
- Based on ARMv8.3-A PAC extension
- Similar instruction naming and semantics
- QARMA-based PAC algorithm
- Pointer poisoning strategy

## 16. Endian-Swapping Load/Store and Auto-Increment/Decrement

### Overview

DLX provides PowerPC-style byte-reversed load/store instructions for cross-endian data access, combined with ARM-style pre/post-increment addressing modes for efficient sequential access patterns.

### Endian-Swapping Load/Store Instructions

#### Byte-Reversed Load Instructions

```assembly
# Load with automatic byte reversal (PowerPC LWBRX style)
LWBRX   rd, (rs)        # Load word byte-reversed indexed
LHBRX   rd, (rs)        # Load halfword byte-reversed indexed
LDBRX   rd, (rs)        # Load doubleword byte-reversed (64-bit)

# With offset
LWBRX   rd, offset(rs)  # Load word byte-reversed with offset
LHBRX   rd, offset(rs)  # Load halfword byte-reversed with offset

# Examples
    # Load big-endian word from network packet
    LWBRX   r3, 0(r4)   # r3 = byte_swap32(mem[r4])

    # Load big-endian halfword (e.g., port number)
    LHBRX   r5, 2(r4)   # r5 = byte_swap16(mem[r4+2])

    # Load little-endian data on big-endian system
    LI      r10, buffer
    LWBRX   r11, 0(r10) # Auto-swap for cross-endian compatibility
```

#### Byte-Reversed Store Instructions

```assembly
# Store with automatic byte reversal
STWBRX  rs, (rd)        # Store word byte-reversed indexed
STHBRX  rs, (rd)        # Store halfword byte-reversed indexed
STDBRX  rs, (rd)        # Store doubleword byte-reversed (64-bit)

# With offset
STWBRX  rs, offset(rd)  # Store word byte-reversed with offset
STHBRX  rs, offset(rd)  # Store halfword byte-reversed with offset

# Examples
    # Store word to network packet (convert to big-endian)
    STWBRX  r3, 0(r4)   # mem[r4] = byte_swap32(r3)

    # Store port number (convert to network byte order)
    STHBRX  r5, 2(r4)   # mem[r4+2] = byte_swap16(r5)
```

#### Endian Swap Semantics

```c
/* LWBRX: Load Word Byte-Reversed */
uint32_t lwbrx(uint32_t *addr)
{
    uint32_t val = *addr;
    return ((val & 0x000000FF) << 24) |
           ((val & 0x0000FF00) << 8)  |
           ((val & 0x00FF0000) >> 8)  |
           ((val & 0xFF000000) >> 24);
}

/* LHBRX: Load Halfword Byte-Reversed */
uint16_t lhbrx(uint16_t *addr)
{
    uint16_t val = *addr;
    return ((val & 0x00FF) << 8) |
           ((val & 0xFF00) >> 8);
}

/* STWBRX: Store Word Byte-Reversed */
void stwbrx(uint32_t val, uint32_t *addr)
{
    *addr = ((val & 0x000000FF) << 24) |
            ((val & 0x0000FF00) << 8)  |
            ((val & 0x00FF0000) >> 8)  |
            ((val & 0xFF000000) >> 24);
}
```

### Auto-Increment/Decrement Addressing Modes

#### Post-Increment Load/Store

```assembly
# Load/Store with post-increment (ARM-style)
LW      rd, (rs)+       # rd = mem[rs], rs = rs + 4
LH      rd, (rs)+       # rd = mem[rs], rs = rs + 2
LB      rd, (rs)+       # rd = mem[rs], rs = rs + 1
LD      rd, (rs)+       # rd = mem[rs], rs = rs + 8 (64-bit)

SW      rt, (rs)+       # mem[rs] = rt, rs = rs + 4
SH      rt, (rs)+       # mem[rs] = rt, rs = rs + 2
SB      rt, (rs)+       # mem[rs] = rt, rs = rs + 1
SD      rt, (rs)+       # mem[rs] = rt, rs = rs + 8 (64-bit)

# Examples
    # Copy array elements
    LI      r10, src_array
    LI      r11, dst_array
    LI      r12, 100        # count

.loop:
    LW      r13, (r10)+     # Load and increment src
    SW      r13, (r11)+     # Store and increment dst
    SUBI    r12, r12, 1
    BNEZ    r12, .loop
```

#### Pre-Increment Load/Store

```assembly
# Load/Store with pre-increment
LW      rd, +(rs)       # rs = rs + 4, rd = mem[rs]
LH      rd, +(rs)       # rs = rs + 2, rd = mem[rs]
LB      rd, +(rs)       # rs = rs + 1, rd = mem[rs]

SW      rt, +(rs)       # rs = rs + 4, mem[rs] = rt
SH      rt, +(rs)       # rs = rs + 2, mem[rs] = rt
SB      rt, +(rs)       # rs = rs + 1, mem[rs] = rt

# Examples
    # Push to stack (grows upward)
    SW      r3, +(sp)   # sp += 4, then store

    # String processing
    LI      r10, string - 1
.scan:
    LB      r11, +(r10) # Increment, then load next char
    BEQZ    r11, .done
    # Process character
    J       .scan
```

#### Post-Decrement Load/Store

```assembly
# Load/Store with post-decrement
LW      rd, (rs)-       # rd = mem[rs], rs = rs - 4
LH      rd, (rs)-       # rd = mem[rs], rs = rs - 2
LB      rd, (rs)-       # rd = mem[rs], rs = rs - 1

SW      rt, (rs)-       # mem[rs] = rt, rs = rs - 4
SH      rt, (rs)-       # mem[rs] = rt, rs = rs - 2
SB      rt, (rs)-       # mem[rs] = rt, rs = rs - 1

# Examples
    # Reverse copy
    LI      r10, src_end
    LI      r11, dst_end
    LI      r12, 100

.loop:
    LW      r13, (r10)-     # Load from end, decrement
    SW      r13, (r11)-     # Store to end, decrement
    SUBI    r12, r12, 1
    BNEZ    r12, .loop
```

#### Pre-Decrement Load/Store

```assembly
# Load/Store with pre-decrement
LW      rd, -(rs)       # rs = rs - 4, rd = mem[rs]
LH      rd, -(rs)       # rs = rs - 2, rd = mem[rs]
LB      rd, -(rs)       # rs = rs - 1, rd = mem[rs]

SW      rt, -(rs)       # rs = rs - 4, mem[rs] = rt
SH      rt, -(rs)       # rs = rs - 2, mem[rs] = rt
SB      rt, -(rs)       # rs = rs - 1, mem[rs] = rt

# Examples
    # Push to stack (downward growing - typical)
    SW      r3, -(sp)   # sp -= 4, then store

    # Pop from stack
    LW      r4, (sp)+   # Load, then sp += 4
```

### Combined: Endian-Swapping with Auto-Increment/Decrement

```assembly
# Byte-reversed load/store with post-increment
LWBRX   rd, (rs)+       # rd = byte_swap(mem[rs]), rs += 4
LHBRX   rd, (rs)+       # rd = byte_swap(mem[rs]), rs += 2
STWBRX  rt, (rs)+       # mem[rs] = byte_swap(rt), rs += 4
STHBRX  rt, (rs)+       # mem[rs] = byte_swap(rt), rs += 2

# Byte-reversed with pre-increment
LWBRX   rd, +(rs)       # rs += 4, rd = byte_swap(mem[rs])
STWBRX  rt, +(rs)       # rs += 4, mem[rs] = byte_swap(rt)

# Byte-reversed with post-decrement
LWBRX   rd, (rs)-       # rd = byte_swap(mem[rs]), rs -= 4
STWBRX  rt, (rs)-       # mem[rs] = byte_swap(rt), rs -= 4

# Byte-reversed with pre-decrement
LWBRX   rd, -(rs)       # rs -= 4, rd = byte_swap(mem[rs])
STWBRX  rt, -(rs)       # rs -= 4, mem[rs] = byte_swap(rt)

# Example: Convert network packet from big-endian
    LI      r10, packet_buf
    LI      r11, 10         # 10 words to convert

.convert_loop:
    LWBRX   r12, (r10)+     # Load big-endian, convert, increment
    SW      r12, local_buf(r11)
    SUBI    r11, r11, 1
    BNEZ    r11, .convert_loop
```

### Use Cases

#### 1. Network Protocol Processing

```c
/* Parse TCP header with automatic endian conversion */
struct tcp_header {
    uint16_t src_port;      /* Big-endian */
    uint16_t dst_port;      /* Big-endian */
    uint32_t seq_num;       /* Big-endian */
    uint32_t ack_num;       /* Big-endian */
};

void parse_tcp_header(uint8_t *packet, struct tcp_header *hdr)
{
    uint16_t *p16 = (uint16_t *)packet;
    uint32_t *p32 = (uint32_t *)packet;

    asm volatile(
        "lhbrx  %0, 0(%2) \n"   /* src_port */
        "lhbrx  %1, 2(%2) \n"   /* dst_port */
        : "=r"(hdr->src_port), "=r"(hdr->dst_port)
        : "r"(packet)
    );

    asm volatile(
        "lwbrx  %0, 4(%2) \n"   /* seq_num */
        "lwbrx  %1, 8(%2) \n"   /* ack_num */
        : "=r"(hdr->seq_num), "=r"(hdr->ack_num)
        : "r"(packet)
    );
}
```

#### 2. Optimized memcpy

```assembly
# Fast memcpy with post-increment
# r3 = dst, r4 = src, r5 = count (bytes)

optimized_memcpy:
    SRLI    r6, r5, 2       # word count = bytes / 4
    ANDI    r7, r5, 3       # remaining bytes

    # Copy words with post-increment
.word_loop:
    BEQZ    r6, .byte_copy
    LW      r8, (r4)+       # Load word, src += 4
    SW      r8, (r3)+       # Store word, dst += 4
    SUBI    r6, r6, 1
    J       .word_loop

    # Copy remaining bytes
.byte_copy:
    BEQZ    r7, .done
    LB      r8, (r4)+       # Load byte, src++
    SB      r8, (r3)+       # Store byte, dst++
    SUBI    r7, r7, 1
    J       .byte_copy

.done:
    JR      r31
```

#### 3. String Operations

```assembly
# strlen with post-increment
# r3 = string pointer, returns length in r1

strlen:
    MV      r10, r3         # Save original pointer
    LI      r11, 0          # length = 0

.loop:
    LB      r12, (r10)+     # Load byte, pointer++
    BEQZ    r12, .done      # Break if null terminator
    ADDI    r11, r11, 1     # length++
    J       .loop

.done:
    MV      r1, r11         # Return length
    JR      r31

# strcpy with post-increment
# r3 = dst, r4 = src

strcpy:
.copy_loop:
    LB      r10, (r4)+      # Load from src, src++
    SB      r10, (r3)+      # Store to dst, dst++
    BNEZ    r10, .copy_loop # Continue until null
    JR      r31
```

#### 4. Stack Frame Management

```assembly
# Function prologue with pre-decrement
function_entry:
    SW      lr, -(sp)       # Push return address
    SW      r31, -(sp)      # Push frame pointer
    MV      r31, sp         # Set frame pointer

    # Allocate local variables
    SW      r4, -(sp)       # Save callee-saved registers
    SW      r5, -(sp)
    SW      r6, -(sp)

    # Function body
    # ...

# Function epilogue with post-increment
function_exit:
    LW      r6, (sp)+       # Restore callee-saved registers
    LW      r5, (sp)+
    LW      r4, (sp)+

    MV      sp, r31         # Restore stack pointer
    LW      r31, (sp)+      # Pop frame pointer
    LW      lr, (sp)+       # Pop return address
    JR      lr
```

#### 5. Circular Buffer Operations

```assembly
# Circular buffer write with auto-increment and wraparound
# r3 = buffer base, r4 = buffer size, r5 = current_pos, r6 = value

circular_buffer_write:
    # Calculate address
    ADD     r10, r3, r5     # addr = base + pos

    # Write value
    SW      r6, (r10)       # Store value

    # Increment position with wraparound
    ADDI    r5, r5, 4       # pos += 4
    BGE     r5, r4, .wrap   # if pos >= size, wrap

    JR      r31

.wrap:
    LI      r5, 0           # pos = 0
    JR      r31
```

#### 6. Little-Endian File Format on Big-Endian System

```c
/* Read little-endian file format */
struct file_header {
    uint32_t magic;         /* Little-endian */
    uint32_t version;       /* Little-endian */
    uint32_t offset;        /* Little-endian */
    uint32_t size;          /* Little-endian */
};

int read_file_header(int fd, struct file_header *hdr)
{
    uint32_t buf[4];
    read(fd, buf, sizeof(buf));

    /* If system is big-endian, need byte swap */
    #if __BYTE_ORDER == __BIG_ENDIAN
    asm volatile(
        "lwbrx  %0, 0(%4) \n"
        "lwbrx  %1, 4(%4) \n"
        "lwbrx  %2, 8(%4) \n"
        "lwbrx  %3, 12(%4) \n"
        : "=r"(hdr->magic), "=r"(hdr->version),
          "=r"(hdr->offset), "=r"(hdr->size)
        : "r"(buf)
    );
    #else
    memcpy(hdr, buf, sizeof(*hdr));
    #endif

    return 0;
}
```

### Instruction Encoding

```
┌────────┬────┬────┬────┬────┬─────────┐
│ Opcode │ rd │ rs │off │mode│  Func   │  Load/Store variants
│   6    │ 5  │ 5  │ 8  │ 2  │    6    │
└────────┴────┴────┴────┴────┴─────────┘

mode: Addressing mode
  0b00: Normal (no auto-increment)
  0b01: Post-increment (addr, then addr += size)
  0b10: Pre-increment (addr += size, then addr)
  0b11: Post-decrement (addr, then addr -= size)
  0b11: Pre-decrement (addr -= size, then addr)  [use sign of offset]

Func: Operation
  0x00: LW     (load word)
  0x01: LH     (load halfword)
  0x02: LB     (load byte)
  0x03: LD     (load doubleword)
  0x08: SW     (store word)
  0x09: SH     (store halfword)
  0x0A: SB     (store byte)
  0x0B: SD     (store doubleword)
  0x10: LWBRX  (load word byte-reversed)
  0x11: LHBRX  (load halfword byte-reversed)
  0x12: LDBRX  (load doubleword byte-reversed)
  0x18: STWBRX (store word byte-reversed)
  0x19: STHBRX (store halfword byte-reversed)
  0x1A: STDBRX (store doubleword byte-reversed)
```

### Performance Benefits

```c
/*
 * Auto-increment addressing:
 * - Reduces register pressure (no separate ADD instruction)
 * - Improves code density (1 instruction vs 2)
 * - Enables better loop optimization
 * - Faster memcpy/memset/strcpy (30-50% speedup)
 *
 * Endian-swapping loads/stores:
 * - Eliminates separate byte-swap instructions
 * - Single-cycle operation (vs 3-5 for manual swap)
 * - Cleaner code for network/file I/O
 * - 2-4x speedup for endian conversion
 */
```

### Alignment Requirements

```c
/* Auto-increment maintains natural alignment */
LW      r3, (r10)+      /* r10 must be 4-byte aligned, increments by 4 */
LH      r4, (r11)+      /* r11 must be 2-byte aligned, increments by 2 */
LB      r5, (r12)+      /* r12 can be any alignment, increments by 1 */

/* Byte-reversed loads also require natural alignment */
LWBRX   r6, (r13)       /* r13 must be 4-byte aligned */
LHBRX   r7, (r14)       /* r14 must be 2-byte aligned */

/* Unaligned access generates exception */
```

### Comparison with Other Architectures

```
DLX          ARM          PowerPC       x86
────────────────────────────────────────────────
LW (r3)+     LDR r3,[r4]! LWZU r3,4(r4) MOV EAX,[EDI] ; INC EDI
LWBRX r3,(r4) REV r3,r3   LWBRX r3,0,r4 BSWAP EAX
SW -(sp)     STR r3,[sp,#-4]! STWU r3,-4(sp) PUSH EAX
```

### Summary

**Endian-Swapping Load/Store**:
- LWBRX/LHBRX/LDBRX: Load with byte reversal
- STWBRX/STHBRX/STDBRX: Store with byte reversal
- PowerPC-compatible instruction naming
- Single-cycle operation
- Network and file I/O optimization

**Auto-Increment/Decrement**:
- Post-increment: (rs)+ - use address, then increment
- Pre-increment: +(rs) - increment, then use address
- Post-decrement: (rs)- - use address, then decrement
- Pre-decrement: -(rs) - decrement, then use address
- ARM-compatible syntax
- Reduced code size and register pressure

**Combined Benefits**:
- All instructions can combine endian swap with auto-inc/dec
- Efficient network packet processing
- Fast string and memory operations
- Optimized stack frame management
- Cross-platform data structure handling

## 17. Quantum Computing Emulation Acceleration

### Overview

DLX provides hardware acceleration for quantum computing emulation, enabling efficient simulation of quantum algorithms on classical hardware. These instructions accelerate qubit state manipulation, quantum gate operations, and measurement.

### Quantum State Representation

```c
/* Qubit state (complex amplitude) */
typedef struct {
    double real;        /* Real component */
    double imag;        /* Imaginary component */
} complex_t;

/* Single qubit state (2 complex amplitudes) */
typedef struct {
    complex_t alpha;    /* Amplitude for |0⟩ */
    complex_t beta;     /* Amplitude for |1⟩ */
} qubit_t;

/* N-qubit state (2^N complex amplitudes) */
typedef struct {
    uint32_t num_qubits;        /* Number of qubits */
    uint32_t num_amplitudes;    /* 2^num_qubits */
    complex_t *amplitudes;      /* State vector */
} quantum_state_t;

/* Example: 3-qubit state has 2^3 = 8 complex amplitudes */
/* Representing superposition of |000⟩, |001⟩, |010⟩, ..., |111⟩ */
```

### Quantum Register File

```c
/* Quantum accelerator registers */
typedef struct {
    /* Quantum state vector pointer */
    complex_t *qstate;          /* QR0: State vector base address */
    uint32_t qsize;             /* QR1: Number of qubits */
    uint32_t qmask;             /* QR2: Qubit mask for operations */

    /* Gate matrices (2x2 complex for single-qubit gates) */
    complex_t gate[4];          /* QR3-QR6: Gate matrix elements */

    /* Measurement results */
    uint32_t measured_state;    /* QR7: Last measurement result */
    double probability;         /* QR8: Measurement probability */

    /* Control register */
    uint32_t qcontrol;          /* QR9: Control bits and flags */
} quantum_regs_t;

/* Access quantum registers */
#define SR_QSTATE       0x70    /* Quantum state vector pointer */
#define SR_QSIZE        0x71    /* Number of qubits */
#define SR_QMASK        0x72    /* Qubit operation mask */
#define SR_QGATE0       0x73    /* Gate matrix element [0,0] */
#define SR_QGATE1       0x74    /* Gate matrix element [0,1] */
#define SR_QGATE2       0x75    /* Gate matrix element [1,0] */
#define SR_QGATE3       0x76    /* Gate matrix element [1,1] */
#define SR_QMEASURE     0x77    /* Measurement result */
#define SR_QPROB        0x78    /* Measurement probability */
#define SR_QCONTROL     0x79    /* Control register */
```

### Quantum Instructions

#### State Initialization

```assembly
# Initialize quantum state
QINIT   qubits          # Initialize |00...0⟩ state for n qubits
QLOAD   addr, qubits    # Load quantum state from memory
QSTORE  addr            # Store quantum state to memory

# Examples
    QINIT   3           # Initialize 3-qubit system to |000⟩
    # State vector: [1+0i, 0+0i, 0+0i, 0+0i, 0+0i, 0+0i, 0+0i, 0+0i]

    LI      r10, state_buffer
    QLOAD   (r10), 4    # Load 4-qubit state from memory

    QSTORE  (r10)       # Save current quantum state
```

#### Single-Qubit Gates

```assembly
# Pauli gates
QX      qubit           # Pauli-X (NOT gate): |0⟩ ↔ |1⟩
QY      qubit           # Pauli-Y gate
QZ      qubit           # Pauli-Z gate: phase flip

# Hadamard gate (creates superposition)
QH      qubit           # Hadamard: |0⟩ → (|0⟩+|1⟩)/√2, |1⟩ → (|0⟩-|1⟩)/√2

# Phase gates
QS      qubit           # S gate: phase shift π/2
QT      qubit           # T gate: phase shift π/4
QPHASE  qubit, angle    # Arbitrary phase shift

# Rotation gates
QRX     qubit, angle    # Rotation around X-axis
QRY     qubit, angle    # Rotation around Y-axis
QRZ     qubit, angle    # Rotation around Z-axis

# Examples
    # Apply Hadamard to qubit 0 (create superposition)
    QH      0           # |0⟩ → (|0⟩+|1⟩)/√2

    # Apply Pauli-X to qubit 1 (flip)
    QX      1           # |0⟩ → |1⟩, |1⟩ → |0⟩

    # Rotate qubit 2 around Y-axis by π/4
    LI      r3, 0x3F490FDB  # π/4 in float
    QRY     2, r3
```

#### Two-Qubit Gates

```assembly
# CNOT gate (Controlled-NOT)
QCNOT   control, target     # If control=|1⟩, flip target

# Controlled-Z gate
QCZ     control, target     # If control=|1⟩, apply Z to target

# SWAP gate
QSWAP   qubit1, qubit2      # Swap states of two qubits

# Controlled phase
QCPHASE control, target, angle  # Controlled phase shift

# Examples
    # Create Bell state: (|00⟩+|11⟩)/√2
    QH      0           # Hadamard on qubit 0
    QCNOT   0, 1        # CNOT with control=0, target=1

    # SWAP qubits 2 and 3
    QSWAP   2, 3
```

#### Three-Qubit Gates

```assembly
# Toffoli gate (CCNOT - Controlled-Controlled-NOT)
QTOFFOLI c1, c2, target     # If c1=|1⟩ AND c2=|1⟩, flip target

# Fredkin gate (CSWAP - Controlled-SWAP)
QFREDKIN c, q1, q2          # If c=|1⟩, swap q1 and q2

# Examples
    # Toffoli gate for 3-bit AND
    QTOFFOLI 0, 1, 2    # If qubits 0 and 1 are |1⟩, flip qubit 2
```

#### Custom Gates

```assembly
# Apply arbitrary single-qubit gate
QGATE   qubit           # Apply gate from QR3-QR6 registers

# Load gate matrix (2x2 complex matrix)
QLOADGATE addr          # Load 4 complex numbers from memory

# Examples
    # Custom gate: Load and apply
    LI      r10, my_gate_matrix
    QLOADGATE (r10)     # Load matrix into gate registers
    QGATE   0           # Apply to qubit 0
```

#### Measurement

```assembly
# Measure qubit(s)
QMEASURE qubit          # Measure single qubit, collapse state
QMEASURE_ALL            # Measure all qubits
QPROB    qubit          # Get probability of measuring |1⟩ (no collapse)

# Examples
    # Measure qubit 0
    QMEASURE 0          # Result in SR_QMEASURE (0 or 1)
    MFSR    r3, $QMEASURE  # r3 = measurement result

    # Get probability without collapsing
    QPROB   1
    MFSR    r4, $QPROB  # r4 = P(qubit 1 = |1⟩)
```

#### Entanglement Operations

```assembly
# Check entanglement
QENTANGLED q1, q2, rd   # rd = 1 if q1 and q2 are entangled

# Partial trace (reduce to subsystem)
QTRACE  qubits_to_keep_mask  # Trace out other qubits

# Examples
    # Check if qubits 0 and 1 are entangled
    QENTANGLED 0, 1, r5
    BNEZ    r5, .is_entangled
```

### Quantum Algorithm Implementations

#### Deutsch-Jozsa Algorithm

```assembly
# Deutsch-Jozsa algorithm: Determine if function is constant or balanced
# Input: n qubits (input) + 1 qubit (output)
# Oracle: black box implementing f(x)

deutsch_jozsa:
    # Initialize: |0⟩^n|1⟩
    QINIT   4               # 3 input + 1 output qubit
    QX      3               # Set output qubit to |1⟩

    # Apply Hadamard to all qubits
    QH      0
    QH      1
    QH      2
    QH      3               # Now in superposition

    # Apply oracle (function-specific)
    JAL     oracle_function

    # Apply Hadamard to input qubits
    QH      0
    QH      1
    QH      2

    # Measure input qubits
    QMEASURE 0
    MFSR    r3, $QMEASURE
    QMEASURE 1
    MFSR    r4, $QMEASURE
    QMEASURE 2
    MFSR    r5, $QMEASURE

    # If all measurements = 0, function is constant
    # Otherwise, function is balanced
    OR      r6, r3, r4
    OR      r6, r6, r5
    BEQZ    r6, .constant_function

.balanced_function:
    LI      r1, 1           # Return 1 (balanced)
    JR      r31

.constant_function:
    LI      r1, 0           # Return 0 (constant)
    JR      r31
```

#### Grover's Search Algorithm

```assembly
# Grover's algorithm: Search unsorted database
# Find item where f(x) = 1 in O(√N) time

grovers_search:
    # Initialize n qubits
    LI      r10, 4          # Search space of 2^4 = 16 items
    QINIT   r10

    # Apply Hadamard to all qubits (equal superposition)
    LI      r11, 0
.hadamard_loop:
    QH      r11
    ADDI    r11, r11, 1
    BGE     r11, r10, .hadamard_done
    J       .hadamard_loop

.hadamard_done:
    # Calculate number of iterations: π/4 * √(2^n) ≈ √N
    LI      r12, 3          # For 16 items, ~3 iterations

.grover_iteration:
    # Oracle: Mark the target state
    JAL     grover_oracle

    # Grover diffusion operator
    JAL     grover_diffusion

    # Decrement iteration counter
    SUBI    r12, r12, 1
    BNEZ    r12, .grover_iteration

    # Measure all qubits
    QMEASURE_ALL
    MFSR    r1, $QMEASURE   # Result is the target item

    JR      r31

# Grover diffusion operator: 2|ψ⟩⟨ψ| - I
grover_diffusion:
    # Apply H to all qubits
    LI      r13, 0
.diff_h1:
    QH      r13
    ADDI    r13, r13, 1
    BGE     r13, r10, .diff_h1_done
    J       .diff_h1

.diff_h1_done:
    # Apply X to all qubits
    LI      r13, 0
.diff_x:
    QX      r13
    ADDI    r13, r13, 1
    BGE     r13, r10, .diff_x_done
    J       .diff_x

.diff_x_done:
    # Multi-controlled Z gate
    # (Simplified: use Toffoli chain)
    QTOFFOLI 0, 1, 2
    QCZ     2, 3

    # Apply X to all qubits
    LI      r13, 0
.diff_x2:
    QX      r13
    ADDI    r13, r13, 1
    BGE     r13, r10, .diff_x2_done
    J       .diff_x2

.diff_x2_done:
    # Apply H to all qubits
    LI      r13, 0
.diff_h2:
    QH      r13
    ADDI    r13, r13, 1
    BGE     r13, r10, .diff_h2_done
    J       .diff_h2

.diff_h2_done:
    JR      r31
```

#### Quantum Fourier Transform (QFT)

```assembly
# Quantum Fourier Transform
# Essential for Shor's algorithm and phase estimation

quantum_fourier_transform:
    # Input: n qubits
    # r10 = number of qubits
    MFSR    r10, $QSIZE

    LI      r11, 0          # i = 0

.qft_outer_loop:
    BGE     r11, r10, .qft_done

    # Apply Hadamard to qubit i
    QH      r11

    # Apply controlled phase rotations
    ADDI    r12, r11, 1     # j = i + 1

.qft_inner_loop:
    BGE     r12, r10, .qft_inner_done

    # Calculate phase: 2π / 2^(j-i+1)
    SUB     r13, r12, r11   # j - i
    ADDI    r13, r13, 1     # j - i + 1
    LI      r14, 1
    SLL     r14, r14, r13   # 2^(j-i+1)

    # phase = 2π / r14
    # (Floating point calculation omitted for brevity)

    QCPHASE r12, r11, r15   # Controlled phase rotation

    ADDI    r12, r12, 1
    J       .qft_inner_loop

.qft_inner_done:
    ADDI    r11, r11, 1
    J       .qft_outer_loop

.qft_done:
    # Reverse qubit order (swap operations)
    LI      r11, 0
    SUBI    r12, r10, 1

.qft_swap_loop:
    BGE     r11, r12, .qft_swap_done
    QSWAP   r11, r12
    ADDI    r11, r11, 1
    SUBI    r12, r12, 1
    J       .qft_swap_loop

.qft_swap_done:
    JR      r31
```

### Hardware Implementation Details

#### State Vector Operations

```c
/* Apply single-qubit gate to state vector */
void apply_single_qubit_gate(quantum_state_t *state,
                             int qubit,
                             complex_t gate[4])
{
    uint32_t num_amplitudes = state->num_amplitudes;
    uint32_t stride = 1 << qubit;  /* 2^qubit */

    /* Iterate over state vector */
    for (uint32_t i = 0; i < num_amplitudes; i += stride * 2) {
        for (uint32_t j = 0; j < stride; j++) {
            uint32_t idx0 = i + j;           /* |...0...⟩ */
            uint32_t idx1 = i + j + stride;  /* |...1...⟩ */

            complex_t a0 = state->amplitudes[idx0];
            complex_t a1 = state->amplitudes[idx1];

            /* Matrix multiplication: [gate] * [a0; a1] */
            state->amplitudes[idx0] = complex_add(
                complex_mul(gate[0], a0),
                complex_mul(gate[1], a1)
            );
            state->amplitudes[idx1] = complex_add(
                complex_mul(gate[2], a0),
                complex_mul(gate[3], a1)
            );
        }
    }
}

/* Apply CNOT gate */
void apply_cnot(quantum_state_t *state, int control, int target)
{
    uint32_t control_bit = 1 << control;
    uint32_t target_bit = 1 << target;
    uint32_t num_amplitudes = state->num_amplitudes;

    for (uint32_t i = 0; i < num_amplitudes; i++) {
        /* Only flip target if control bit is 1 */
        if (i & control_bit) {
            uint32_t j = i ^ target_bit;  /* Flip target bit */
            if (j > i) {
                /* Swap amplitudes[i] and amplitudes[j] */
                complex_t temp = state->amplitudes[i];
                state->amplitudes[i] = state->amplitudes[j];
                state->amplitudes[j] = temp;
            }
        }
    }
}
```

#### Measurement

```c
/* Measure qubit and collapse state */
int measure_qubit(quantum_state_t *state, int qubit)
{
    uint32_t qubit_bit = 1 << qubit;
    double prob_0 = 0.0;
    double prob_1 = 0.0;

    /* Calculate probabilities */
    for (uint32_t i = 0; i < state->num_amplitudes; i++) {
        double amp_squared = complex_magnitude_squared(state->amplitudes[i]);
        if (i & qubit_bit)
            prob_1 += amp_squared;
        else
            prob_0 += amp_squared;
    }

    /* Generate random number */
    double rand_val = random_double();

    int result;
    if (rand_val < prob_0) {
        result = 0;
        /* Collapse to |0⟩: zero out |1⟩ amplitudes, renormalize */
        for (uint32_t i = 0; i < state->num_amplitudes; i++) {
            if (i & qubit_bit)
                state->amplitudes[i] = (complex_t){0.0, 0.0};
            else
                state->amplitudes[i] = complex_scale(state->amplitudes[i],
                                                     1.0 / sqrt(prob_0));
        }
    } else {
        result = 1;
        /* Collapse to |1⟩: zero out |0⟩ amplitudes, renormalize */
        for (uint32_t i = 0; i < state->num_amplitudes; i++) {
            if (i & qubit_bit)
                state->amplitudes[i] = complex_scale(state->amplitudes[i],
                                                     1.0 / sqrt(prob_1));
            else
                state->amplitudes[i] = (complex_t){0.0, 0.0};
        }
    }

    return result;
}
```

### Complex Number Operations

```assembly
# Complex arithmetic (double precision)
CADD    rd, rs, rt          # rd = rs + rt (complex addition)
CSUB    rd, rs, rt          # rd = rs - rt (complex subtraction)
CMUL    rd, rs, rt          # rd = rs * rt (complex multiplication)
CDIV    rd, rs, rt          # rd = rs / rt (complex division)
CMAG    rd, rs              # rd = |rs| (magnitude)
CCONJ   rd, rs              # rd = rs* (complex conjugate)
CPHASE  rd, rs              # rd = arg(rs) (phase angle)

# Complex load/store (16 bytes: 2 doubles)
LDC     rd, (rs)            # Load complex from memory
STC     rt, (rs)            # Store complex to memory

# Examples
    # Load two complex numbers
    LI      r10, complex1
    LI      r11, complex2
    LDC     c0, (r10)       # c0 = complex1
    LDC     c1, (r11)       # c1 = complex2

    # Multiply complex numbers
    CMUL    c2, c0, c1      # c2 = c0 * c1

    # Get magnitude
    CMAG    r12, c2         # r12 = |c2|
```

### Performance Considerations

```c
/*
 * Quantum Emulation Performance:
 *
 * State vector size: 2^n complex numbers (n = number of qubits)
 * - 1 qubit: 2 * 16 bytes = 32 bytes
 * - 10 qubits: 1024 * 16 bytes = 16 KB
 * - 20 qubits: 1M * 16 bytes = 16 MB
 * - 30 qubits: 1G * 16 bytes = 16 GB
 * - 40 qubits: 1T * 16 bytes = 16 TB (impractical)
 *
 * Hardware acceleration benefits:
 * - Single-qubit gate: 2^n operations → 10-100x speedup
 * - Two-qubit gate: 2^n operations → 10-100x speedup
 * - Measurement: O(2^n) → 5-10x speedup
 * - Complex arithmetic: 4-8x speedup
 *
 * Practical limit: ~25-30 qubits on consumer hardware
 * Specialized hardware (GPU, FPGA): ~40-45 qubits
 */
```

### Quantum Gate Library

```c
/* Standard quantum gates (2x2 complex matrices) */

/* Pauli-X (NOT gate) */
const complex_t GATE_X[4] = {
    {0, 0}, {1, 0},    /* [0  1] */
    {1, 0}, {0, 0}     /* [1  0] */
};

/* Pauli-Y gate */
const complex_t GATE_Y[4] = {
    {0, 0}, {0, -1},   /* [0  -i] */
    {0, 1}, {0, 0}     /* [i   0] */
};

/* Pauli-Z gate */
const complex_t GATE_Z[4] = {
    {1, 0}, {0, 0},    /* [1   0] */
    {0, 0}, {-1, 0}    /* [0  -1] */
};

/* Hadamard gate */
const complex_t GATE_H[4] = {
    {M_SQRT1_2, 0}, {M_SQRT1_2, 0},      /* [1/√2  1/√2] */
    {M_SQRT1_2, 0}, {-M_SQRT1_2, 0}      /* [1/√2 -1/√2] */
};

/* S gate (phase π/2) */
const complex_t GATE_S[4] = {
    {1, 0}, {0, 0},    /* [1  0] */
    {0, 0}, {0, 1}     /* [0  i] */
};

/* T gate (phase π/4) */
const complex_t GATE_T[4] = {
    {1, 0}, {0, 0},                    /* [1    0  ] */
    {0, 0}, {M_SQRT1_2, M_SQRT1_2}     /* [0  e^(iπ/4)] */
};
```

### Use Cases

#### Quantum Chemistry Simulation

```c
/* Simulate molecular Hamiltonian */
void simulate_molecule(int num_qubits)
{
    asm volatile(
        "qinit  %0          \n"  /* Initialize qubits */
        "qh     0           \n"  /* Prepare superposition */
        /* Apply Trotter decomposition */
        "qrx    0, %1       \n"  /* Rotation gates */
        "qry    1, %2       \n"
        "qcnot  0, 1        \n"  /* Entangling gates */
        /* ... more gates ... */
        "qmeasure_all       \n"  /* Measure final state */
        : : "r"(num_qubits), "r"(angle1), "r"(angle2)
    );
}
```

#### Quantum Machine Learning

```c
/* Variational quantum eigensolver (VQE) */
double vqe_iteration(double *params, int num_params)
{
    /* Prepare parameterized quantum circuit */
    quantum_circuit_prepare(params, num_params);

    /* Measure expectation value of Hamiltonian */
    double energy = measure_hamiltonian();

    /* Classical optimization updates params */
    return energy;
}
```

### Instruction Encoding

```
┌────────┬────┬────┬─────┬───────────────┐
│ Opcode │qubit│func│angle│   Reserved   │  Single-qubit gates
│   6    │ 5  │ 6  │  8  │      7        │
└────────┴────┴────┴─────┴───────────────┘

Func:
  0x00: QX     (Pauli-X)
  0x01: QY     (Pauli-Y)
  0x02: QZ     (Pauli-Z)
  0x03: QH     (Hadamard)
  0x04: QS     (S gate)
  0x05: QT     (T gate)
  0x08: QRX    (Rotate X)
  0x09: QRY    (Rotate Y)
  0x0A: QRZ    (Rotate Z)
  0x0B: QPHASE (Phase shift)

┌────────┬────┬────┬────┬───────────────┐
│ Opcode │ q1 │ q2 │func│   Reserved    │  Two-qubit gates
│   6    │ 5  │ 5  │ 6  │      10       │
└────────┴────┴────┴────┴───────────────┘

Func:
  0x00: QCNOT  (Controlled-NOT)
  0x01: QCZ    (Controlled-Z)
  0x02: QSWAP  (SWAP)
  0x03: QCPHASE (Controlled phase)
```

### Summary

**Key Features**:
- Hardware-accelerated quantum gate operations
- Support for 1-qubit, 2-qubit, and 3-qubit gates
- State vector emulation (up to ~30 qubits practical)
- Complex number arithmetic instructions
- Measurement and state collapse
- Standard gate library (Pauli, Hadamard, phase, rotation)
- Entanglement operations

**Supported Algorithms**:
- Deutsch-Jozsa algorithm
- Grover's search algorithm
- Quantum Fourier Transform (QFT)
- Shor's factoring algorithm (via QFT)
- Variational Quantum Eigensolver (VQE)
- Quantum chemistry simulation

**Performance**:
- 10-100x speedup vs software emulation
- Complex arithmetic: 4-8x faster
- Single-qubit gates: O(2^n) operations accelerated
- Practical limit: 25-30 qubits on consumer hardware

**Applications**:
- Quantum algorithm research and development
- Quantum chemistry and materials science
- Cryptography and security research
- Machine learning (quantum neural networks)
- Optimization problems

## 18. Extended Numeric Formats (Decimal FP, BCD, bfloat16, FP8)

### Overview

DLX provides comprehensive support for alternative numeric formats beyond IEEE 754 binary floating-point, including decimal floating-point (IEEE 754-2008), Binary Coded Decimal (BCD), bfloat16 (ML/AI), and FP8 (deep learning). All formats integrate with the vector unit.

### IEEE 754-2008 Decimal Floating-Point

#### Decimal Format Specifications

```c
/* IEEE 754-2008 decimal floating-point formats */

/* Decimal32 (32-bit) */
typedef struct {
    uint32_t bits;      /* Encoded using DPD or BID */
    /* 7 decimal digits precision */
    /* Exponent range: -95 to +96 */
} decimal32_t;

/* Decimal64 (64-bit) */
typedef struct {
    uint64_t bits;      /* Encoded using DPD or BID */
    /* 16 decimal digits precision */
    /* Exponent range: -383 to +384 */
} decimal64_t;

/* Decimal128 (128-bit) */
typedef struct {
    uint64_t high;
    uint64_t low;
    /* 34 decimal digits precision */
    /* Exponent range: -6143 to +6144 */
} decimal128_t;

/* Example values in decimal64:
 * 1.234567890123456 × 10^0  = exact representation
 * 0.1                        = exact (unlike binary FP!)
 * 1.0 / 3.0                  = 0.3333333333333333...
 */
```

#### Encoding Schemes

```c
/*
 * Two encoding schemes for decimal FP:
 *
 * 1. DPD (Densely Packed Decimal):
 *    - Groups of 3 decimal digits packed into 10 bits
 *    - Hardware-friendly decoding
 *    - Used by IBM POWER processors
 *
 * 2. BID (Binary Integer Decimal):
 *    - Coefficient stored as binary integer
 *    - Simpler arithmetic operations
 *    - Used by Intel x86 processors
 *
 * DLX supports both, selectable via control register
 */

/* Decimal32 format (DPD encoding) */
┌─┬─────────┬──────────────────────────────┐
│S│Combination│     Trailing significand   │
│1│   5      │           20                │
└─┴─────────┴──────────────────────────────┘

/* Decimal64 format (DPD encoding) */
┌─┬─────────┬──────────────────────────────────────────────────────┐
│S│Combination│              Trailing significand                  │
│1│    5     │                    50                               │
└─┴─────────┴──────────────────────────────────────────────────────┘

/* Decimal128 format (DPD encoding) */
┌─┬─────────┬────────────────────────────────────────────────────────────────────────────┐
│S│Combination│                    Trailing significand                                  │
│1│    5     │                            110                                            │
└─┴─────────┴────────────────────────────────────────────────────────────────────────────┘
```

#### Decimal FP Control Register

```c
/* Decimal FP configuration */
typedef struct {
    uint32_t enable:1;          /* Bit 0: Enable decimal FP */
    uint32_t encoding:1;        /* Bit 1: 0=DPD, 1=BID */
    uint32_t rounding:3;        /* Bits 2-4: Rounding mode */
    uint32_t exception_mask:5;  /* Bits 5-9: Exception masks */
    uint32_t reserved:22;
} decimal_fp_config_t;

#define SR_DFP_CONFIG   0x80

/* Rounding modes (IEEE 754-2008) */
#define DFP_ROUND_HALF_EVEN         0  /* Round to nearest, ties to even */
#define DFP_ROUND_HALF_UP           1  /* Round to nearest, ties away from 0 */
#define DFP_ROUND_HALF_DOWN         2  /* Round to nearest, ties toward 0 */
#define DFP_ROUND_UP                3  /* Round toward +∞ */
#define DFP_ROUND_DOWN              4  /* Round toward -∞ */
#define DFP_ROUND_TOWARD_ZERO       5  /* Round toward 0 */
#define DFP_ROUND_HALF_AWAY_ZERO    6  /* Round to nearest, ties away from 0 */
```

#### Decimal FP Instructions

```assembly
# Decimal32 operations (32-bit, 7 digits)
DADD.32   fd, fs, ft        # fd = fs + ft (decimal32)
DSUB.32   fd, fs, ft        # fd = fs - ft
DMUL.32   fd, fs, ft        # fd = fs * ft
DDIV.32   fd, fs, ft        # fd = fs / ft
DSQRT.32  fd, fs            # fd = √fs
DFMA.32   fd, fs, ft, fa    # fd = fs * ft + fa

# Decimal64 operations (64-bit, 16 digits)
DADD.64   fd, fs, ft        # fd = fs + ft (decimal64)
DSUB.64   fd, fs, ft        # fd = fs - ft
DMUL.64   fd, fs, ft        # fd = fs * ft
DDIV.64   fd, fs, ft        # fd = fs / ft
DSQRT.64  fd, fs            # fd = √fs
DFMA.64   fd, fs, ft, fa    # fd = fs * ft + fa

# Decimal128 operations (128-bit, 34 digits)
DADD.128  fd, fs, ft        # fd = fs + ft (decimal128)
DSUB.128  fd, fs, ft        # fd = fs - ft
DMUL.128  fd, fs, ft        # fd = fs * ft
DDIV.128  fd, fs, ft        # fd = fs / ft
DSQRT.128 fd, fs            # fd = √fs
DFMA.128  fd, fs, ft, fa    # fd = fs * ft + fa

# Comparison
DCMP.32   fd, fs, ft        # Compare decimal32
DCMP.64   fd, fs, ft        # Compare decimal64
DCMP.128  fd, fs, ft        # Compare decimal128

# Conversion between decimal formats
DCVT.32.64   fd, fs         # Convert decimal64 → decimal32
DCVT.64.32   fd, fs         # Convert decimal32 → decimal64
DCVT.128.64  fd, fs         # Convert decimal64 → decimal128

# Conversion to/from binary FP
DCVT.D.F     fd, fs         # Binary float → decimal64
DCVT.F.D     fd, fs         # Decimal64 → binary float
DCVT.Q.D     fd, fs         # Decimal64 → binary double

# Conversion to/from integer
DCVT.W.D     rd, fs         # Decimal64 → int32
DCVT.D.W     fd, rs         # Int32 → decimal64
DCVT.L.D     rd, fs         # Decimal64 → int64
DCVT.D.L     fd, rs         # Int64 → decimal64

# Load/Store decimal FP
LDD.32    fd, offset(rs)    # Load decimal32
LDD.64    fd, offset(rs)    # Load decimal64
LDD.128   fd, offset(rs)    # Load decimal128
STD.32    fs, offset(rd)    # Store decimal32
STD.64    fs, offset(rd)    # Store decimal64
STD.128   fs, offset(rd)    # Store decimal128

# Examples
    # Financial calculation: 0.1 + 0.2 = 0.3 (exact!)
    LDD.64  f1, const_0_1   # f1 = 0.1 (exact in decimal)
    LDD.64  f2, const_0_2   # f2 = 0.2 (exact)
    DADD.64 f3, f1, f2      # f3 = 0.3 (exact!)

    # Compare with binary FP (would have rounding error)
    LWC1    f4, const_0_1_binary  # f4 = 0.1000000014...
    LWC1    f5, const_0_2_binary  # f5 = 0.2000000029...
    ADD.D   f6, f4, f5            # f6 = 0.3000000043... (error!)
```

### Binary Coded Decimal (BCD)

#### BCD Formats

```c
/* Packed BCD: 2 digits per byte */
typedef struct {
    uint8_t digit1:4;   /* High nibble: first digit (0-9) */
    uint8_t digit0:4;   /* Low nibble: second digit (0-9) */
} packed_bcd_t;

/* Unpacked BCD: 1 digit per byte */
typedef struct {
    uint8_t digit:4;    /* Low nibble: digit (0-9) */
    uint8_t unused:4;   /* High nibble: unused (usually 0) */
} unpacked_bcd_t;

/* BCD string (variable length) */
typedef struct {
    uint8_t length;         /* Number of digits */
    uint8_t sign:1;         /* 0=positive, 1=negative */
    uint8_t decimal_pos:7;  /* Position of decimal point */
    uint8_t digits[];       /* Packed BCD digits */
} bcd_string_t;

/* Example: 12345 in packed BCD */
/* bytes: 0x01 0x23 0x45 */
/*         ^     ^     ^
 *        1    23    45
 */
```

#### BCD Instructions

```assembly
# BCD arithmetic (packed format)
BCDADD  rd, rs, rt          # BCD addition
BCDSUB  rd, rs, rt          # BCD subtraction
BCDMUL  rd, rs, rt          # BCD multiplication
BCDDIV  rd, rs, rt          # BCD division

# BCD arithmetic with carry/borrow
BCDADDC rd, rs, rt          # BCD add with carry
BCDSUBB rd, rs, rt          # BCD subtract with borrow

# BCD comparison
BCDCMP  rs, rt              # Compare BCD values

# BCD conversion
BCD2BIN rd, rs              # Packed BCD → binary
BIN2BCD rd, rs              # Binary → packed BCD
BCDPACK rd, rs              # Unpacked → packed BCD
BCDUNPACK rd, rs            # Packed → unpacked BCD

# BCD string operations
BCDLEN  rd, (rs)            # Get BCD string length
BCDNEG  rd, rs              # Negate BCD value
BCDABS  rd, rs              # Absolute value

# Examples
    # Add two BCD numbers: 1234 + 5678
    LI      r10, 0x1234     # r10 = 1234 in packed BCD
    LI      r11, 0x5678     # r11 = 5678 in packed BCD
    BCDADD  r12, r10, r11   # r12 = 0x6912 (6912 in BCD)

    # Convert binary to BCD for display
    LI      r13, 12345      # r13 = 12345 (binary)
    BIN2BCD r14, r13        # r14 = 0x12345 (packed BCD)

    # Multi-precision BCD addition (128-bit example)
    # Add 16-digit BCD numbers
    LW      r10, bcd1_low
    LW      r11, bcd1_high
    LW      r12, bcd2_low
    LW      r13, bcd2_high

    BCDADD  r14, r10, r12   # Add low parts
    BCDADDC r15, r11, r13   # Add high parts with carry
```

#### BCD Decimal Adjust

```assembly
# Decimal adjust after binary arithmetic
DAA     rd, rs              # Decimal adjust after addition
DAS     rd, rs              # Decimal adjust after subtraction

# Example: x86-style BCD arithmetic
    LI      r10, 0x19       # 19 in BCD
    LI      r11, 0x28       # 28 in BCD
    ADD     r12, r10, r11   # r12 = 0x41 (binary addition)
    DAA     r12, r12        # r12 = 0x47 (adjusted to BCD)
```

### bfloat16 (Brain Floating Point)

#### bfloat16 Format

```c
/* bfloat16: Truncated IEEE 754 single precision */
typedef struct {
    uint16_t sign:1;        /* Sign bit */
    uint16_t exponent:8;    /* Exponent (same as float32) */
    uint16_t mantissa:7;    /* Mantissa (truncated from 23 to 7 bits) */
} bfloat16_t;

/*
 * bfloat16 vs other 16-bit formats:
 *
 * Format      Sign  Exp  Mantissa  Range           Precision
 * ────────────────────────────────────────────────────────────
 * bfloat16     1    8      7       ~10^-38..10^38  ~3 decimal digits
 * IEEE fp16    1    5     10       ~10^-8..65504   ~3 decimal digits
 * TensorFloat  1    8     10       ~10^-38..10^38  ~3 decimal digits
 *
 * bfloat16 advantages:
 * - Same exponent range as float32 (no overflow in ML training)
 * - Simple conversion: truncate float32 mantissa
 * - Hardware-friendly for ML accelerators
 */

/* Conversion examples */
float32:   0x3F800000  (1.0)
bfloat16:  0x3F80      (1.0 - just drop low 16 bits!)

float32:   0x40490FDB  (π)
bfloat16:  0x4049      (~3.140625)
```

#### bfloat16 Instructions

```assembly
# bfloat16 arithmetic
BADD.H    fd, fs, ft        # bfloat16 addition
BSUB.H    fd, fs, ft        # bfloat16 subtraction
BMUL.H    fd, fs, ft        # bfloat16 multiplication
BDIV.H    fd, fs, ft        # bfloat16 division
BSQRT.H   fd, fs            # bfloat16 square root
BFMA.H    fd, fs, ft, fa    # bfloat16 FMA

# Comparison
BCMP.H    fs, ft            # Compare bfloat16

# Conversion
BCVT.H.S  fd, fs            # float32 → bfloat16
BCVT.S.H  fd, fs            # bfloat16 → float32
BCVT.H.D  fd, fs            # float64 → bfloat16
BCVT.D.H  fd, fs            # bfloat16 → float64
BCVT.H.W  fd, rs            # int32 → bfloat16
BCVT.W.H  rd, fs            # bfloat16 → int32

# Load/Store
LH.BF16   fd, offset(rs)    # Load bfloat16
SH.BF16   fs, offset(rd)    # Store bfloat16

# Vector operations (8 bfloat16 in 128-bit vector)
VBADD.H   vd, vs, vt        # Vector bfloat16 add (8 elements)
VBMUL.H   vd, vs, vt        # Vector bfloat16 multiply
VBFMA.H   vd, vs, vt, va    # Vector bfloat16 FMA

# Examples
    # ML forward pass with bfloat16
    LH.BF16  f1, weights(r10)
    LH.BF16  f2, inputs(r11)
    BMUL.H   f3, f1, f2      # w * x
    BADD.H   f4, f3, f0      # + bias

    # Convert float32 activations to bfloat16 for next layer
    BCVT.H.S f5, f4          # Truncate to bfloat16
    SH.BF16  f5, outputs(r12)
```

### FP8 (8-bit Floating Point)

#### FP8 Formats

```c
/* FP8 E4M3 (4-bit exponent, 3-bit mantissa) */
typedef struct {
    uint8_t sign:1;         /* Sign bit */
    uint8_t exponent:4;     /* Exponent (bias 7) */
    uint8_t mantissa:3;     /* Mantissa */
} fp8_e4m3_t;

/* FP8 E5M2 (5-bit exponent, 2-bit mantissa) */
typedef struct {
    uint8_t sign:1;         /* Sign bit */
    uint8_t exponent:5;     /* Exponent (bias 15) */
    uint8_t mantissa:2;     /* Mantissa */
} fp8_e5m2_t;

/*
 * FP8 format comparison:
 *
 * Format    Sign  Exp  Mant  Bias  Range           Use Case
 * ──────────────────────────────────────────────────────────
 * E4M3       1    4    3     7     -448..448       Forward pass
 * E5M2       1    5    2    15     -57344..57344   Backward pass (gradients)
 *
 * E4M3: Better precision, smaller range (weights, activations)
 * E5M2: Worse precision, larger range (gradients)
 */

/* Example values in E4M3 */
0x00: 0.0
0x38: 1.0
0x3C: 1.5
0x3F: 1.75
0x78: 448 (max normal)
0xFF: -448

/* Example values in E5M2 */
0x00: 0.0
0x3C: 1.0
0x3E: 1.5
0x3F: 1.75
0x7B: 57344 (max normal)
```

#### FP8 Instructions

```assembly
# FP8 E4M3 arithmetic
F8ADD.E4M3   fd, fs, ft     # FP8 E4M3 addition
F8SUB.E4M3   fd, fs, ft     # FP8 E4M3 subtraction
F8MUL.E4M3   fd, fs, ft     # FP8 E4M3 multiplication

# FP8 E5M2 arithmetic
F8ADD.E5M2   fd, fs, ft     # FP8 E5M2 addition
F8SUB.E5M2   fd, fs, ft     # FP8 E5M2 subtraction
F8MUL.E5M2   fd, fs, ft     # FP8 E5M2 multiplication

# Comparison
F8CMP.E4M3   fs, ft         # Compare FP8 E4M3
F8CMP.E5M2   fs, ft         # Compare FP8 E5M2

# Conversion to/from other formats
F8CVT.E4M3.S fd, fs         # float32 → FP8 E4M3
F8CVT.S.E4M3 fd, fs         # FP8 E4M3 → float32
F8CVT.E5M2.S fd, fs         # float32 → FP8 E5M2
F8CVT.S.E5M2 fd, fs         # FP8 E5M2 → float32
F8CVT.E4M3.H fd, fs         # bfloat16 → FP8 E4M3
F8CVT.H.E4M3 fd, fs         # FP8 E4M3 → bfloat16

# Load/Store
LB.FP8    fd, offset(rs)    # Load FP8 (8 bits)
SB.FP8    fs, offset(rd)    # Store FP8

# Vector operations (16 FP8 in 128-bit vector)
VF8ADD.E4M3 vd, vs, vt      # Vector FP8 E4M3 add (16 elements)
VF8MUL.E4M3 vd, vs, vt      # Vector FP8 E4M3 multiply

# Mixed precision matrix multiply (common pattern)
# A(FP8) × B(FP8) → C(float32)
VDOT.FP8.S  fd, vs, vt      # Dot product: FP8 inputs, float32 output

# Examples
    # Deep learning inference with FP8
    LB.FP8   f1, weights(r10)     # Load FP8 weight
    LB.FP8   f2, activations(r11) # Load FP8 activation
    F8MUL.E4M3 f3, f1, f2         # Multiply in FP8
    F8CVT.S.E4M3 f4, f3           # Convert to float32 for accumulation
    ADD.S    f5, f5, f4           # Accumulate in higher precision
```

### Vector Operations with All Formats

#### Vector Format Configuration

```c
/* Vector unit supports all numeric formats */
typedef enum {
    VEC_FORMAT_FP32,        /* IEEE 754 single precision */
    VEC_FORMAT_FP64,        /* IEEE 754 double precision */
    VEC_FORMAT_FP16,        /* IEEE 754 half precision */
    VEC_FORMAT_BFLOAT16,    /* bfloat16 */
    VEC_FORMAT_FP8_E4M3,    /* FP8 E4M3 */
    VEC_FORMAT_FP8_E5M2,    /* FP8 E5M2 */
    VEC_FORMAT_DECIMAL32,   /* IEEE 754-2008 decimal32 */
    VEC_FORMAT_DECIMAL64,   /* IEEE 754-2008 decimal64 */
    VEC_FORMAT_DECIMAL128,  /* IEEE 754-2008 decimal128 */
    VEC_FORMAT_BCD_PACKED,  /* Packed BCD */
    VEC_FORMAT_VAX_F,       /* VAX F_floating */
    VEC_FORMAT_VAX_D,       /* VAX D_floating */
    VEC_FORMAT_VAX_G,       /* VAX G_floating */
    VEC_FORMAT_VAX_H,       /* VAX H_floating */
} vector_format_t;

/* Vector configuration register */
#define SR_VEC_FORMAT   0x85

/* Set vector format */
VSETFMT format              # Set active vector format

/* Examples */
    VSETFMT VEC_FORMAT_BFLOAT16     # Switch to bfloat16
    VADD    v0, v1, v2              # Add 8×bfloat16

    VSETFMT VEC_FORMAT_FP8_E4M3     # Switch to FP8
    VMUL    v0, v1, v2              # Multiply 16×FP8

    VSETFMT VEC_FORMAT_DECIMAL64    # Switch to decimal64
    VADD    v0, v1, v2              # Add 2×decimal64
```

#### Vector Element Counts by Format

```
Format          Bits/Element    Elements per 128-bit vector
────────────────────────────────────────────────────────────
FP8 E4M3             8                    16
FP8 E5M2             8                    16
FP16                16                     8
bfloat16            16                     8
FP32                32                     4
FP64                64                     2
Decimal32           32                     4
Decimal64           64                     2
Decimal128         128                     1
VAX F               32                     4
VAX D               64                     2
VAX G               64                     2
VAX H              128                     1
BCD (packed)         8                    16  (32 digits)
```

#### Vector Arithmetic (Format-Aware)

```assembly
# Generic vector operations (format set by VSETFMT)
VADD    vd, vs, vt          # Vector add (any format)
VSUB    vd, vs, vt          # Vector subtract
VMUL    vd, vs, vt          # Vector multiply
VDIV    vd, vs, vt          # Vector divide
VSQRT   vd, vs              # Vector square root
VFMA    vd, vs, vt, va      # Vector FMA
VABS    vd, vs              # Vector absolute value
VNEG    vd, vs              # Vector negate
VMIN    vd, vs, vt          # Vector minimum
VMAX    vd, vs, vt          # Vector maximum

# Vector dot product (mixed precision support)
VDOT.FP8.S   fd, vs, vt     # Dot product: FP8→float32
VDOT.H.S     fd, vs, vt     # Dot product: bfloat16→float32
VDOT.S.D     fd, vs, vt     # Dot product: float32→float64
VDOT.D32.D64 fd, vs, vt     # Dot product: decimal32→decimal64

# Examples - Neural Network Layer (FP8)
neural_net_layer_fp8:
    VSETFMT VEC_FORMAT_FP8_E4M3

    LI      r10, 0          # i = 0
    LI      r11, 128        # num_neurons
    LI      r12, weights
    LI      r13, inputs
    LI      r14, outputs

.loop:
    # Load 16 FP8 weights
    VLB.FP8 v0, 0(r12)

    # Load 16 FP8 inputs
    VLB.FP8 v1, 0(r13)

    # Multiply and accumulate (convert to float32 for accumulation)
    VDOT.FP8.S f0, v0, v1

    # Add bias and apply activation
    ADD.S   f0, f0, f_bias
    # ReLU activation
    MAX.S   f0, f0, f_zero

    # Convert back to FP8 for next layer
    F8CVT.E4M3.S f1, f0
    SB.FP8  f1, 0(r14)

    ADDI    r10, r10, 16
    ADDI    r12, r12, 16
    ADDI    r13, r13, 16
    ADDI    r14, r14, 1
    BLT     r10, r11, .loop

# Financial Calculation (Decimal64)
financial_sum_decimal:
    VSETFMT VEC_FORMAT_DECIMAL64

    # Sum array of monetary values (exact decimal arithmetic)
    LI      r10, amounts_array
    LI      r11, 1000           # 1000 amounts
    VZERO.D v0                  # accumulator = 0.0

.sum_loop:
    VLD.64  v1, 0(r10)          # Load 2 decimal64 values
    VADD    v0, v0, v1          # Add to accumulator
    ADDI    r10, r10, 16        # Next 2 values
    SUBI    r11, r11, 2
    BGTZ    r11, .sum_loop

    # Extract final sum
    VEXTRACT.D64 f0, v0, 0      # Get first element
    VEXTRACT.D64 f1, v0, 1      # Get second element
    DADD.64 f0, f0, f1          # Final sum (exact!)
```

#### Mixed-Precision Vector Operations

```assembly
# Convert between formats in vectors
VCVT.FP8.BF16  vd, vs       # 16×FP8 → 8×bfloat16 (pairwise avg)
VCVT.BF16.FP32 vd, vs       # 8×bfloat16 → 4×float32
VCVT.FP32.FP64 vd, vs       # 4×float32 → 2×float64
VCVT.D32.D64   vd, vs       # 4×decimal32 → 2×decimal64

# Widen operations (compute in higher precision)
VWADD.FP8.FP32  vd, vs, vt  # Add 16×FP8, produce 16×float32
VWMUL.BF16.FP32 vd, vs, vt  # Mul 8×bfloat16, produce 8×float32

# Narrow operations (round to lower precision)
VNADD.FP32.BF16 vd, vs, vt  # Add as float32, round to bfloat16
VNMUL.FP64.FP32 vd, vs, vt  # Mul as float64, round to float32

# Examples - Transformer Model (Mixed Precision)
transformer_attention:
    # Query, Key, Value in bfloat16
    VSETFMT VEC_FORMAT_BFLOAT16

    VLD.H   v_q, 0(r_query)     # Load query (8×bf16)
    VLD.H   v_k, 0(r_key)       # Load key (8×bf16)

    # Compute attention scores in float32 (higher precision)
    VDOT.H.S f_score, v_q, v_k  # Q·K in float32

    # Scale and softmax (float32)
    MUL.S   f_score, f_score, f_scale
    JAL     softmax_fp32

    # Apply to values (mixed precision)
    VLD.H   v_v, 0(r_value)     # Load value (8×bf16)

    # Multiply: float32 score × bfloat16 value → bfloat16 output
    VSCALE.H v_out, v_v, f_score

    VST.H   v_out, 0(r_output)
```

### VAX Floating-Point with Vectors

#### VAX FP Vector Operations

```assembly
# Set vector unit to VAX format
VSETFMT VEC_FORMAT_VAX_F        # VAX F_floating (32-bit)
VSETFMT VEC_FORMAT_VAX_D        # VAX D_floating (64-bit)
VSETFMT VEC_FORMAT_VAX_G        # VAX G_floating (64-bit)
VSETFMT VEC_FORMAT_VAX_H        # VAX H_floating (128-bit)

# Vector operations in VAX format
VADD    vd, vs, vt              # Vector add (VAX FP)
VMUL    vd, vs, vt              # Vector multiply (VAX FP)
VPOLY   vd, vs, vt, degree      # Vector polynomial (VAX POLY)

# Example: Legacy VAX code acceleration
vax_vector_computation:
    VSETFMT VEC_FORMAT_VAX_G    # VAX G_floating (64-bit)

    # Load VAX format data
    VLD.VAX v0, data_array(r10)
    VLD.VAX v1, coeffs(r11)

    # Compute with VAX semantics (no gradual underflow)
    VMUL    v2, v0, v1
    VADD    v3, v2, v_bias

    # Store VAX format results
    VST.VAX v3, results(r12)
```

### Format-Specific Optimizations

#### Hardware Support

```c
/*
 * Hardware acceleration for numeric formats:
 *
 * Format          Hardware Unit       Throughput (ops/cycle)
 * ─────────────────────────────────────────────────────────
 * FP32            FPU (standard)      4 (SIMD)
 * FP64            FPU (standard)      2 (SIMD)
 * FP16            FPU (dedicated)     8 (SIMD)
 * bfloat16        Tensor cores        8 (SIMD)
 * FP8 E4M3        Tensor cores        16 (SIMD)
 * FP8 E5M2        Tensor cores        16 (SIMD)
 * Decimal32       DFP unit            4 (SIMD)
 * Decimal64       DFP unit            2 (SIMD)
 * Decimal128      DFP unit            1
 * BCD             BCD ALU             16 digits/cycle
 * VAX FP          FPU (emulated)      2-4 (SIMD)
 *
 * Total die area: ~15% of chip for all FP/numeric units
 */
```

#### Performance Examples

```c
/* Benchmark: Matrix multiplication (1024×1024) */

/* FP32 baseline */
gemm_fp32();        // 100% performance, 100% accuracy

/* bfloat16 (ML training) */
gemm_bf16();        // 200% performance, 99.5% accuracy

/* FP8 E4M3 (ML inference) */
gemm_fp8();         // 400% performance, 98% accuracy

/* Decimal64 (financial) */
gemm_decimal64();   // 30% performance, 100% decimal accuracy

/* Mixed precision (best of both) */
gemm_mixed();       // 150% performance, 99.9% accuracy
                    // FP8 multiply, FP32 accumulate
```

### Use Cases by Format

```c
/*
 * Format selection guide:
 *
 * IEEE 754-2008 Decimal FP:
 * - Financial applications (exact decimal representation)
 * - Currency calculations (0.1 + 0.2 = 0.3, exactly!)
 * - Legal/regulatory compliance (exact decimal rounding)
 * - Accounting systems
 * - Tax calculations
 *
 * BCD (Binary Coded Decimal):
 * - Display formatting (easy digit extraction)
 * - Embedded systems (calculator chips)
 * - Legacy system compatibility (COBOL, mainframes)
 * - Phone numbers, ZIP codes, SKUs
 * - Credit card numbers
 *
 * bfloat16:
 * - Machine learning training (gradients, weights)
 * - Neural network forward/backward pass
 * - Transformers, LLMs (GPT, BERT)
 * - Mixed precision training
 * - Scientific computing (where range > precision)
 *
 * FP8:
 * - Deep learning inference (maximum throughput)
 * - Quantized neural networks
 * - Edge AI deployment
 * - Real-time inference (video, audio)
 * - Model compression (4× smaller than fp32)
 *
 * VAX FP (with vectors):
 * - Legacy VAX/VMS application acceleration
 * - Porting old scientific codes
 * - Compatibility with historical data
 */
```

## 19. Instruction Encoding Summary

### Opcode Space Allocation

```
0x00-0x0F: Basic ALU (existing DLX)
0x10-0x1F: FPU (extended with VAX support)
0x20-0x2F: Load/Store (extended with PC-relative)
0x30-0x33: Branch (standard)
0x34-0x37: Branch extended (large displacement)
0x38-0x3B: Compressed mode and mode control
0x3C: Four-operand instructions
0x3D: PA-RISC style bit manipulation
0x3E: Alpha style bit manipulation
0x3F: Special (POLY, CRC, etc.)
```

### Feature Summary

| Feature | Encoding | Complexity | LOC Estimate |
|---------|----------|------------|--------------|
| Compressed Mode | Mode bit + 16-bit encoding | High | 5,000 |
| 4-Operand FMA | 4R format | Medium | 2,000 |
| SPARC64 FPU | Extended FP regs | High | 3,000 |
| VAX FP | Alternative FP format | High | 4,000 |
| PA-RISC Bit Ops | New opcodes | Medium | 2,000 |
| Alpha Bit Ops | New opcodes | Medium | 2,000 |
| POLY | Special instruction | Medium | 1,500 |
| PC-Relative | Extended addressing | Medium | 2,500 |

**Total Estimated LOC**: ~22,000 lines

---

**Document Status**: Specification Complete
**Implementation Status**: Not Started
**Target**: DLX ISA 2.0
**Complexity**: Very High - major ISA revision
**Compatibility**: Backward compatible when features disabled
**Performance Impact**: +20-50% for code density (compressed), +10-30% for FP (quad precision), +5-15% for bit manipulation
