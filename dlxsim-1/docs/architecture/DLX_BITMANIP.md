# DLX Bit Manipulation Extensions (DLX-B)

## Overview

DLX-B adds comprehensive bit manipulation instructions inspired by ARM64 (LSE, FEAT_FlagM), Intel BMI/BMI2/ABM, and AMD TBM extensions. These instructions accelerate cryptography, compression, parsing, and general-purpose computing.

---

## 1. Bit Counting and Finding

### Count Leading/Trailing Zeros

```asm
CLZ  rd, rs        ; Count leading zeros
                   ; rd = number of leading 0 bits in rs
                   ; If rs = 0, rd = 32 (or 64 for DLX64)

CTZ  rd, rs        ; Count trailing zeros
                   ; rd = number of trailing 0 bits in rs
                   ; If rs = 0, rd = 32 (or 64 for DLX64)

CLS  rd, rs        ; Count leading sign bits (ones or zeros)
                   ; rd = CLZ(rs XOR (rs << 1))
```

**Example**:
```asm
li   r1, 0x00FF0000
clz  r2, r1         ; r2 = 8 (eight leading zeros)
ctz  r3, r1         ; r3 = 16 (sixteen trailing zeros)
```

**Use Cases**:
- Finding first set bit
- Integer log2 calculation
- Normalization in floating-point
- Priority encoding

### Population Count

```asm
POPCNT  rd, rs     ; Count number of 1 bits
                   ; rd = number of set bits in rs

PARITY  rd, rs     ; Calculate parity
                   ; rd = (POPCNT(rs) & 1)
```

**Example**:
```asm
li      r1, 0x12345678
popcnt  r2, r1         ; r2 = 13 (thirteen 1 bits)
parity  r3, r1         ; r3 = 1 (odd parity)
```

**Use Cases**:
- Hamming distance calculation
- Error detection/correction
- Chess bitboard operations
- Set cardinality

---

## 2. Bit Field Extraction and Insertion

### Bit Field Extract (ARM64 UBFX/SBFX-inspired)

```asm
BFEXT  rd, rs, pos, width    ; Extract unsigned bit field
                             ; rd = (rs >> pos) & ((1 << width) - 1)

BFEXTS rd, rs, pos, width    ; Extract signed bit field (sign-extended)
                             ; temp = (rs >> pos) & ((1 << width) - 1)
                             ; rd = sign_extend(temp, width)
```

**Example**:
```asm
li    r1, 0x12345678
bfext r2, r1, 8, 8      ; r2 = 0x56 (extract bits 8-15)
bfext r3, r1, 0, 16     ; r3 = 0x5678 (extract bits 0-15)
```

### Bit Field Insert (ARM64 BFI-inspired)

```asm
BFINS  rd, rs, pos, width    ; Insert bit field
                             ; mask = ((1 << width) - 1) << pos
                             ; rd = (rd & ~mask) | ((rs << pos) & mask)

BFINSU rd, rs, pos, width    ; Insert with update of source
                             ; Same as BFINS but also updates rs
```

**Example**:
```asm
li    r1, 0xFFFF0000
li    r2, 0xAB
bfins r1, r2, 8, 8      ; r1 = 0xFFFF AB 00 (insert 0xAB at bits 8-15)
```

### Bit Field Clear (ARM64 BFC-inspired)

```asm
BFC  rd, pos, width         ; Clear bit field
                            ; mask = ((1 << width) - 1) << pos
                            ; rd = rd & ~mask
```

**Use Cases**:
- Packed structure manipulation
- Protocol parsing (extract fields)
- Image processing (pixel components)
- Compression algorithms

---

## 3. Bit Manipulation (Intel BMI-inspired)

### Bit Extraction and Manipulation

```asm
BEXTR  rd, rs, control      ; Bit extract (Intel BMI)
                            ; start = control & 0xFF
                            ; len = (control >> 8) & 0xFF
                            ; rd = (rs >> start) & ((1 << len) - 1)

BLSI   rd, rs               ; Extract lowest set bit
                            ; rd = rs & (-rs)

BLSMSK rd, rs               ; Get mask up to lowest set bit
                            ; rd = rs ^ (rs - 1)

BLSR   rd, rs               ; Reset lowest set bit
                            ; rd = rs & (rs - 1)
```

**Examples**:
```asm
; Extract lowest set bit
li    r1, 0b10110000
blsi  r2, r1               ; r2 = 0b00010000

; Get mask up to lowest set bit
li    r1, 0b10110000
blsmsk r2, r1              ; r2 = 0b00011111

; Reset lowest set bit
li    r1, 0b10110000
blsr  r2, r1               ; r2 = 0b10100000
```

**Use Cases**:
- Iterating through set bits
- Sparse set operations
- Bitboard manipulation in games

### Bitwise Ternary Logic (Intel AVX512-inspired)

```asm
BITSEL rd, rs1, rs2, rs3    ; Bitwise select
                            ; rd = (rs1 & rs2) | (~rs1 & rs3)

BITTRI rd, rs1, rs2, imm8   ; Bitwise ternary logic
                            ; Compute based on 8-bit truth table in imm8
```

**Example**:
```asm
; Conditional bitwise select
li      r1, 0xFF00FF00   ; mask
li      r2, 0xAAAAAAAA   ; value A
li      r3, 0x55555555   ; value B
bitsel  r4, r1, r2, r3   ; r4 = 0xAA00AA00 (select from A or B)
```

---

## 4. Bit Deposit and Extract (Intel BMI2-inspired)

### Parallel Bit Deposit

```asm
PDEP  rd, rs, mask         ; Parallel bit deposit
                           ; Deposit contiguous low bits of rs
                           ; to positions indicated by 1s in mask
```

**Example**:
```asm
li   r1, 0b00001111        ; source
li   r2, 0b10101010        ; mask
pdep r3, r1, r2            ; r3 = 0b10001000
```

**Visualization**:
```
Source: 0000 1111
Mask:   1010 1010
         ↓ ↓ ↓ ↓
Result: 1000 1000
```

### Parallel Bit Extract

```asm
PEXT  rd, rs, mask         ; Parallel bit extract
                           ; Extract bits from rs at positions
                           ; indicated by 1s in mask, compact to low bits
```

**Example**:
```asm
li   r1, 0b10110011        ; source
li   r2, 0b11001100        ; mask
pext r3, r1, r2            ; r3 = 0b00001011
```

**Visualization**:
```
Source: 1011 0011
Mask:   1100 1100
         ↓↓   ↓↓
Result: 0000 1011
```

**Use Cases**:
- Bit permutations
- Compression algorithms
- Chess move generation
- Hash functions

---

## 5. Byte and Bit Reversal

### Byte Reverse (ARM64 REV-inspired)

```asm
REV    rd, rs              ; Reverse bytes in word
                           ; REV32: swap bytes in 32-bit word
                           ; REV64: swap bytes in 64-bit word (DLX64)

REV16  rd, rs              ; Reverse bytes in halfwords
                           ; Swap bytes in each 16-bit halfword

RBIT   rd, rs              ; Reverse bits
                           ; Reverse order of all bits in word
```

**Examples**:
```asm
; Byte reverse
li   r1, 0x12345678
rev  r2, r1               ; r2 = 0x78563412

; Reverse bytes in halfwords
li   r1, 0x12345678
rev16 r2, r1              ; r2 = 0x34127856

; Bit reverse
li   r1, 0b10110010
rbit r2, r1               ; r2 = 0b01001101 (reversed)
```

**Use Cases**:
- Endianness conversion
- Network byte order
- Hashing and checksums
- Cryptography (AES)

---

## 6. Rotation and Funnel Shifts

### Rotate (ARM64 ROR-inspired)

```asm
ROL   rd, rs, rt           ; Rotate left (reg)
                           ; rd = (rs << (rt & 31)) | (rs >> (32 - (rt & 31)))

ROR   rd, rs, rt           ; Rotate right (reg)
                           ; rd = (rs >> (rt & 31)) | (rs << (32 - (rt & 31)))

ROLI  rd, rs, imm          ; Rotate left (immediate)
RORI  rd, rs, imm          ; Rotate right (immediate)
```

**Example**:
```asm
li   r1, 0x12345678
li   r2, 8
rol  r3, r1, r2           ; r3 = 0x34567812
ror  r4, r1, r2           ; r4 = 0x78123456
```

### Funnel Shift (Intel BMI2 SHRD/SHLD-inspired)

```asm
FSHL  rd, rs1, rs2, rt    ; Funnel shift left
                          ; temp = (rs1 << 32) | rs2
                          ; rd = temp << rt

FSHR  rd, rs1, rs2, rt    ; Funnel shift right
                          ; temp = (rs1 << 32) | rs2
                          ; rd = temp >> rt
```

**Example**:
```asm
li   r1, 0xAAAA0000
li   r2, 0x0000BBBB
li   r3, 16
fshl r4, r1, r2, r3       ; r4 = 0x0000AAAA (shift across boundary)
```

**Use Cases**:
- Cryptographic operations (DES, AES)
- CRC calculations
- Multi-precision arithmetic
- Stream ciphers

---

## 7. Conditional Bit Operations

### Conditional Select (ARM64 CSEL-inspired)

```asm
CSEL  rd, rs1, rs2, cond  ; Conditional select
                          ; if (condition) rd = rs1 else rd = rs2

CSINV rd, rs1, rs2, cond  ; Conditional select inverted
                          ; if (condition) rd = rs1 else rd = ~rs2

CSINC rd, rs1, rs2, cond  ; Conditional select increment
                          ; if (condition) rd = rs1 else rd = rs2 + 1

CSNEG rd, rs1, rs2, cond  ; Conditional select negate
                          ; if (condition) rd = rs1 else rd = -rs2
```

**Condition Codes**:
- EQ: Equal (Z=1)
- NE: Not equal (Z=0)
- LT: Less than (N=1)
- GE: Greater or equal (N=0)
- GT: Greater than (Z=0 && N=0)
- LE: Less or equal (Z=1 || N=1)

**Example**:
```asm
; Compute absolute value without branches
slt   r1, r2, r0         ; r1 = (r2 < 0)
csneg r3, r2, r2, eq     ; if (r2 >= 0) r3 = r2 else r3 = -r2
```

**Use Cases**:
- Branchless programming
- Performance optimization
- Constant-time crypto operations

---

## 8. Advanced Bit Operations

### Bit Gather and Scatter

```asm
BGATHR rd, rs, index_mask  ; Gather bits based on index mask
BSCATR rd, rs, index_mask  ; Scatter bits based on index mask
```

### Bit Permutation

```asm
BPERM  rd, rs, perm        ; Permute bits according to permutation vector
```

### Carry-less Multiply (CLMUL for AES-GCM)

```asm
CLMUL   rd, rs1, rs2       ; Carry-less multiply (polynomial multiply)
                           ; Used in AES-GCM and CRC

CLMULH  rd, rs1, rs2       ; Carry-less multiply high
                           ; High 32 bits of 64-bit result
```

**Example (CRC calculation)**:
```asm
li    r1, data_word
li    r2, crc_polynomial
clmul r3, r1, r2           ; r3 = carry-less product (low)
clmulh r4, r1, r2          ; r4 = carry-less product (high)
```

**Use Cases**:
- AES-GCM authentication
- CRC-32C calculation
- Reed-Solomon codes
- Error correction

---

## 9. Atomic Bit Operations

### Atomic Bit Set/Clear (ARM64 LSE-inspired)

```asm
LDBTS  rd, rs, (rt)        ; Load and bit test-and-set
                           ; Atomically: temp = mem[rt]; mem[rt] |= (1 << rs); rd = temp

LDBTR  rd, rs, (rt)        ; Load and bit test-and-reset
                           ; Atomically: temp = mem[rt]; mem[rt] &= ~(1 << rs); rd = temp

LDBTC  rd, rs, (rt)        ; Load and bit test-and-complement
                           ; Atomically: temp = mem[rt]; mem[rt] ^= (1 << rs); rd = temp
```

**Use Cases**:
- Lock-free data structures
- Bit vectors
- Resource allocation bitmaps
- Concurrent algorithms

---

## 10. Instruction Encoding

### R-Type (3 registers)
```
31    26 25  21 20  16 15  11 10   6 5    0
+-------+------+------+------+------+------+
| 0x00  | rs2  | rs1  | rd   | 0x00 | func |
+-------+------+------+------+------+------+
Opcode: SPECIAL (0x00)
func: Bit manipulation function code
```

### I-Type (immediate)
```
31    26 25  21 20  16 15             0
+-------+------+------+----------------+
| 0x3F  | rs   | rd   |    immediate   |
+-------+------+------+----------------+
Opcode: BITMANIP (0x3F)
```

### Bit Manipulation Function Codes
```
0x100: CLZ      0x110: BLSI     0x120: PDEP     0x130: ROL
0x101: CTZ      0x111: BLSMSK   0x121: PEXT     0x131: ROR
0x102: CLS      0x112: BLSR     0x122: REV      0x132: FSHL
0x103: POPCNT   0x113: BEXTR    0x123: REV16    0x133: FSHR
0x104: PARITY   0x114: BITSEL   0x124: RBIT     0x134: CLMUL
0x105: BFEXT    0x115: CSEL     0x125: BGATHR   0x135: CLMULH
0x106: BFEXTS   0x116: CSINV    0x126: BSCATR
0x107: BFINS    0x117: CSINC    0x127: BPERM
0x108: BFC      0x118: CSNEG    0x128: LDBTS
                                0x129: LDBTR
                                0x12A: LDBTC
```

---

## 11. Performance and Use Cases

### Cryptography
- **AES**: CLMUL for GCM mode, RBIT for bit permutations
- **SHA**: ROL/ROR for message scheduling, BFEXT for word extraction
- **DES**: Funnel shifts (FSHL/FSHR) for permutations

### Compression
- **DEFLATE**: BFEXT for code extraction, PDEP/PEXT for bit packing
- **LZ77**: CLZ/CTZ for match finding, POPCNT for similarity

### Parsing
- **JSON/XML**: BFEXT for field extraction, CLMUL for hashing
- **UTF-8**: BFINS for character assembly, BFC for masking

### Chess Engines
- **Bitboards**: POPCNT for piece counting, BLSI for piece iteration
- **Move Generation**: PDEP/PEXT for attack generation

### Performance Impact
- **Without BMI**: 5-20 instructions per operation
- **With BMI**: 1-3 instructions per operation
- **Speedup**: 2-10x for bit-heavy algorithms

---

## 12. Code Examples

### Example 1: Extract RGB from 32-bit Color

```asm
; Input: r1 = 0xAARRGGBB (Alpha, Red, Green, Blue)
; Extract components

bfext r2, r1, 16, 8    ; r2 = RR (red)
bfext r3, r1, 8, 8     ; r3 = GG (green)
bfext r4, r1, 0, 8     ; r4 = BB (blue)
bfext r5, r1, 24, 8    ; r5 = AA (alpha)
```

### Example 2: Swap Endianness

```asm
; Convert between big-endian and little-endian
; Input: r1 = 0x12345678

rev r2, r1             ; r2 = 0x78563412 (byte swap)
```

### Example 3: Calculate Integer Log2

```asm
; Calculate floor(log2(n))
; Input: r1 = n (must be > 0)

clz  r2, r1            ; Count leading zeros
li   r3, 31
sub  r4, r3, r2        ; r4 = 31 - clz = floor(log2(n))
```

### Example 4: Iterate Through Set Bits

```asm
; Process each set bit in r1
li   r1, 0b10110100   ; Input bitmap

loop:
  beqz r1, done        ; Exit if no bits left
  blsi r2, r1          ; r2 = isolated lowest bit

  ; Process bit in r2 here
  ; ...

  blsr r1, r1          ; Remove lowest bit
  j    loop

done:
```

### Example 5: Constant-Time Absolute Value

```asm
; Compute abs(r1) without branches (constant-time for crypto)

srai r2, r1, 31        ; r2 = (r1 < 0) ? -1 : 0 (sign extension)
xor  r3, r1, r2        ; r3 = r1 ^ r2
sub  r4, r3, r2        ; r4 = |r1|
```

---

## 13. Compatibility and Detection

### Feature Detection

```asm
; Read capabilities register
mfc0  r1, $caps        ; Capabilities register

; Check for bit manipulation support
andi  r2, r1, 0x0080   ; Bit 7 = BMI support
bnez  r2, has_bmi

; Fallback to software implementation
jal   software_clz
j     continue

has_bmi:
; Use hardware instruction
clz   r3, r4

continue:
```

### Version Flags

**Capabilities Register (CP0)**:
- Bit 7: BMI (Basic Bit Manipulation)
- Bit 8: BMI2 (Advanced: PDEP/PEXT)
- Bit 9: CLMUL (Carry-less multiply)
- Bit 10: LSE (Atomic bit operations)

---

## 14. Summary

DLX-B adds 40+ bit manipulation instructions organized into:

1. **Counting**: CLZ, CTZ, CLS, POPCNT, PARITY
2. **Extraction/Insertion**: BFEXT, BFEXTS, BFINS, BFC
3. **Manipulation**: BLSI, BLSMSK, BLSR, BEXTR, BITSEL
4. **Deposit/Extract**: PDEP, PEXT (Intel BMI2)
5. **Reversal**: REV, REV16, RBIT
6. **Rotation**: ROL, ROR, FSHL, FSHR
7. **Conditional**: CSEL, CSINV, CSINC, CSNEG
8. **Advanced**: CLMUL, BPERM, BGATHR, BSCATR
9. **Atomic**: LDBTS, LDBTR, LDBTC

**Benefits**:
- 2-10x speedup for bit-intensive code
- Enables branchless algorithms
- Critical for crypto, compression, parsing
- Reduces code size and complexity

**Implementation Status**: 📋 PLANNED (Architecture specified)

---

**Document Version**: 1.0
**Last Updated**: October 2024
**Status**: Architecture Specification
