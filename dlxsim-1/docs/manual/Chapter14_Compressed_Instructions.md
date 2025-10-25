# Chapter 14: Compressed Instructions Extension (C)

## 14.1 Overview

The Compressed extension (C) provides 16-bit encodings for common instructions:
- **Code density**: 25-30% size reduction
- **Performance**: Reduces I-cache pressure
- **Compatibility**: Freely mixed with 32-bit instructions

## 14.2 Instruction Alignment

- 32-bit instructions: 4-byte aligned
- 16-bit instructions: 2-byte aligned
- PC can be odd multiples of 2

## 14.3 Compressed Integer Instructions

```assembly
# Load/Store
c.lw        rd', offset(rs1')   # Load word (compressed)
c.ld        rd', offset(rs1')   # Load doubleword
c.sw        rs2', offset(rs1')  # Store word
c.sd        rs2', offset(rs1')  # Store doubleword

# Arithmetic
c.add       rd, rs2             # rd ← rd + rs2
c.sub       rd', rs2'           # rd' ← rd' - rs2'
c.and       rd', rs2'           # rd' ← rd' & rs2'
c.or        rd', rs2'           # rd' ← rd' | rs2'
c.xor       rd', rs2'           # rd' ← rd' ⊕ rs2'

# Immediate
c.li        rd, imm             # rd ← sign_ext(imm)
c.lui       rd, imm             # rd ← imm << 12
c.addi      rd, imm             # rd ← rd + sign_ext(imm)
c.addiw     rd, imm             # rd ← sext((rd + imm)[31:0])
c.addi16sp  sp, imm             # sp ← sp + sign_ext(imm * 16)

# Shifts
c.slli      rd, shamt           # rd ← rd << shamt
c.srli      rd', shamt          # rd' ← rd' >> shamt (logical)
c.srai      rd', shamt          # rd' ← rd' >> shamt (arithmetic)

# Branches
c.beqz      rs1', offset        # if rs1' == 0: PC ← PC + offset
c.bnez      rs1', offset        # if rs1' != 0: PC ← PC + offset

# Jumps
c.j         offset              # PC ← PC + sign_ext(offset)
c.jr        rs1                 # PC ← rs1
c.jal       offset              # ra ← PC+2, PC ← PC + offset
c.jalr      rs1                 # ra ← PC+2, PC ← rs1

# Stack operations
c.lwsp      rd, offset(sp)      # Load word from stack
c.ldsp      rd, offset(sp)      # Load doubleword from stack
c.swsp      rs2, offset(sp)     # Store word to stack
c.sdsp      rs2, offset(sp)     # Store doubleword to stack

# Move
c.mv        rd, rs2             # rd ← rs2
```

## 14.4 Compressed Floating-Point

```assembly
c.fld       fd', offset(rs1')   # FP load double
c.fsd       fs2', offset(rs1')  # FP store double
c.fldsp     fd, offset(sp)      # FP load from stack
c.fsdsp     fs2, offset(sp)     # FP store to stack
```

## 14.5 Register Encoding

Compressed instructions use 3-bit register specifiers:
- **rd'**, **rs1'**, **rs2'**: Map to x8-x15 (s0-s1, a0-a5)
- Allows most common operations in compressed form

Mapping:
```
rd'/rs1'/rs2'   Register
000             x8  (s0/fp)
001             x9  (s1)
010             x10 (a0)
011             x11 (a1)
100             x12 (a2)
101             x13 (a3)
110             x14 (a4)
111             x15 (a5)
```

## 14.6 Pseudo-Instructions

```assembly
c.nop                   ≡ c.addi x0, 0
c.ebreak                ≡ ebreak (encoded in 16 bits)
```

## 14.7 Code Size Comparison

**Uncompressed**:
```assembly
        addi    sp, sp, -16     # 4 bytes
        sd      ra, 8(sp)       # 4 bytes
        sd      s0, 0(sp)       # 4 bytes
        call    foo             # 8 bytes (auipc + jalr)
        ld      s0, 0(sp)       # 4 bytes
        ld      ra, 8(sp)       # 4 bytes
        addi    sp, sp, 16      # 4 bytes
        ret                     # 4 bytes
                                # Total: 36 bytes
```

**Compressed**:
```assembly
        c.addi16sp sp, -16      # 2 bytes
        c.sdsp  ra, 8(sp)       # 2 bytes
        c.sdsp  s0, 0(sp)       # 2 bytes
        call    foo             # 8 bytes (no compressed form)
        c.ldsp  s0, 0(sp)       # 2 bytes
        c.ldsp  ra, 8(sp)       # 2 bytes
        c.addi16sp sp, 16       # 2 bytes
        c.jr    ra              # 2 bytes
                                # Total: 22 bytes (39% reduction)
```

## 14.8 Implementation Notes

- Decoder expands 16-bit instructions to internal 32-bit format
- No performance penalty (macro-op fusion)
- Can decode two 16-bit instructions per cycle
- Branch prediction works on compressed branches
