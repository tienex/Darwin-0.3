# Chapter 12: Floating-Point Extension (F/D/Q)

## 12.1 Floating-Point Registers

32 floating-point registers (f0-f31), each capable of holding:
- **Single precision** (32-bit, F extension)
- **Double precision** (64-bit, D extension)
- **Quad precision** (128-bit, Q extension)
- **Half precision** (16-bit)
- **BFloat16** (16-bit, ML format)

## 12.2 Floating-Point Control and Status Register (FCSR)

```c
struct fcsr {
    uint32_t rm      : 3;    /* Rounding mode */
    uint32_t flags   : 5;    /* Accrued exceptions */
    uint32_t reserved: 24;
};

/* Rounding modes */
#define FP_RNE  0    /* Round to nearest, ties to even */
#define FP_RTZ  1    /* Round towards zero */
#define FP_RDN  2    /* Round down (towards -∞) */
#define FP_RUP  3    /* Round up (towards +∞) */
#define FP_RMM  4    /* Round to nearest, ties to max magnitude */

/* Exception flags */
#define FP_NV   0x10 /* Invalid operation */
#define FP_DZ   0x08 /* Divide by zero */
#define FP_OF   0x04 /* Overflow */
#define FP_UF   0x02 /* Underflow */
#define FP_NX   0x01 /* Inexact */
```

## 12.3 Single Precision (F Extension)

```assembly
# Arithmetic
fadd.s      fd, fs1, fs2        # fd ← fs1 + fs2
fsub.s      fd, fs1, fs2        # fd ← fs1 - fs2
fmul.s      fd, fs1, fs2        # fd ← fs1 × fs2
fdiv.s      fd, fs1, fs2        # fd ← fs1 ÷ fs2
fsqrt.s     fd, fs              # fd ← √fs

# Fused multiply-add
fmadd.s     fd, fs1, fs2, fs3   # fd ← (fs1 × fs2) + fs3
fmsub.s     fd, fs1, fs2, fs3   # fd ← (fs1 × fs2) - fs3
fnmadd.s    fd, fs1, fs2, fs3   # fd ← -(fs1 × fs2) + fs3
fnmsub.s    fd, fs1, fs2, fs3   # fd ← -(fs1 × fs2) - fs3

# Comparison
feq.s       rd, fs1, fs2        # rd ← (fs1 == fs2)
flt.s       rd, fs1, fs2        # rd ← (fs1 < fs2)
fle.s       rd, fs1, fs2        # rd ← (fs1 ≤ fs2)

# Min/Max
fmin.s      fd, fs1, fs2        # fd ← min(fs1, fs2)
fmax.s      fd, fs1, fs2        # fd ← max(fs1, fs2)

# Sign injection
fsgnj.s     fd, fs1, fs2        # fd ← |fs1| × sign(fs2)
fsgnjn.s    fd, fs1, fs2        # fd ← |fs1| × -sign(fs2)
fsgnjx.s    fd, fs1, fs2        # fd ← |fs1| × (sign(fs1) ⊕ sign(fs2))

# Conversions
fcvt.w.s    rd, fs              # Convert float to int32
fcvt.wu.s   rd, fs              # Convert float to uint32
fcvt.l.s    rd, fs              # Convert float to int64
fcvt.lu.s   rd, fs              # Convert float to uint64
fcvt.s.w    fd, rs              # Convert int32 to float
fcvt.s.wu   fd, rs              # Convert uint32 to float
fcvt.s.l    fd, rs              # Convert int64 to float
fcvt.s.lu   fd, rs              # Convert uint64 to float

# Move
fmv.w.x     fd, rs              # Move int to FP reg (no conversion)
fmv.x.w     rd, fs              # Move FP reg to int (no conversion)
```

## 12.4 Double Precision (D Extension)

```assembly
# Arithmetic
fadd.d      fd, fs1, fs2        # 64-bit FP add
fsub.d      fd, fs1, fs2        # 64-bit FP subtract
fmul.d      fd, fs1, fs2        # 64-bit FP multiply
fdiv.d      fd, fs1, fs2        # 64-bit FP divide
fsqrt.d     fd, fs              # 64-bit FP square root

# Fused multiply-add
fmadd.d     fd, fs1, fs2, fs3
fmsub.d     fd, fs1, fs2, fs3
fnmadd.d    fd, fs1, fs2, fs3
fnmsub.d    fd, fs1, fs2, fs3

# Conversions
fcvt.d.s    fd, fs              # Convert single to double
fcvt.s.d    fd, fs              # Convert double to single
fcvt.d.w    fd, rs              # Convert int32 to double
fcvt.d.l    fd, rs              # Convert int64 to double
fcvt.l.d    rd, fs              # Convert double to int64
```

## 12.5 Quad Precision (Q Extension)

```assembly
# Arithmetic (128-bit)
fadd.q      fd, fs1, fs2        # Quad precision add
fsub.q      fd, fs1, fs2        # Quad precision subtract
fmul.q      fd, fs1, fs2        # Quad precision multiply
fdiv.q      fd, fs1, fs2        # Quad precision divide
fsqrt.q     fd, fs              # Quad precision square root

# Conversions
fcvt.q.d    fd, fs              # Convert double to quad
fcvt.d.q    fd, fs              # Convert quad to double
fcvt.q.l    fd, rs              # Convert int64 to quad
fcvt.l.q    rd, fs              # Convert quad to int64
```

## 12.6 Half Precision and BFloat16

```assembly
# Half precision (16-bit IEEE 754)
fadd.h      fd, fs1, fs2        # Half precision add
fmul.h      fd, fs1, fs2        # Half precision multiply
fcvt.h.s    fd, fs              # Convert single to half
fcvt.s.h    fd, fs              # Convert half to single

# BFloat16 (Brain Float, 16-bit)
fadd.bf     fd, fs1, fs2        # BFloat16 add
fmul.bf     fd, fs1, fs2        # BFloat16 multiply
fcvt.bf.s   fd, fs              # Convert single to BFloat16
fcvt.s.bf   fd, fs              # Convert BFloat16 to single
```

## 12.7 Denormal Handling

DLX FPU supports:
- **IEEE 754 compliant**: Full denormal support
- **Flush-to-zero (FTZ)**: Treat denormals as zero (performance mode)
- **Denormals-are-zero (DAZ)**: Treat input denormals as zero

Configuration via FCSR:
```c
#define FCSR_FTZ    (1 << 8)    /* Flush to zero */
#define FCSR_DAZ    (1 << 9)    /* Denormals are zero */
```

## 12.8 NaN Handling

- **Quiet NaN (qNaN)**: Propagates through operations
- **Signaling NaN (sNaN)**: Raises invalid operation exception
- **NaN boxing**: Single precision values in double registers are NaN-boxed

## 12.9 Performance

Typical latencies (cycles):
- FADD/FSUB: 3-4 cycles
- FMUL: 4-5 cycles
- FDIV.S: 10-15 cycles
- FDIV.D: 15-25 cycles
- FSQRT.S: 12-18 cycles
- FSQRT.D: 20-30 cycles
- FMA: 4-5 cycles (pipelined)
