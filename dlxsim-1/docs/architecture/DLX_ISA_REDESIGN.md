# DLX ISA Redesign - Modern Mnemonic System

## Overview

This document specifies a redesigned instruction set architecture for DLX with modern, consistent mnemonics inspired by:
- **PowerPC/POWER**: Clear, descriptive names with suffixes
- **OpenRISC**: Logical organization with consistent prefixes
- **ARM64**: Orthogonal design with predictable encoding

### Design Principles

1. **Consistency**: Similar operations use similar mnemonics
2. **Clarity**: Instruction names are self-documenting
3. **Suffixes**: Width/type indicated by suffix (b/h/w/d/q for 8/16/32/64/128-bit)
4. **Prefixes**: Operation class indicated by prefix (l/s for load/store, f for float, v for vector)
5. **Orthogonality**: All addressing modes work with all data types
6. **Extensibility**: Easy to add new instructions consistently

### Naming Convention

```
<prefix><operation><suffix><modifier>

Prefixes:
  l   - Load
  s   - Store
  f   - Floating-point
  v   - Vector
  m   - Matrix
  c   - Capability (CHERI)
  e   - Enclave
  nn  - Neural network
  q   - Quantum

Suffixes:
  b   - Byte (8-bit)
  h   - Halfword (16-bit)
  w   - Word (32-bit)
  d   - Doubleword (64-bit)
  q   - Quadword (128-bit)

Modifiers:
  u   - Unsigned
  i   - Immediate
  x   - Indexed
  .   - Condition/variant

Examples:
  lbz     - Load Byte Zero-extended
  stw     - Store Word
  fadd.d  - Floating Add Double
  vadd.w  - Vector Add Word
```

## General Purpose Instructions

### Load/Store Instructions

```assembly
# Load instructions (memory → register)
# Format: l<size><sign> rd, offset(rs)

lbz     rd, offset(rs)          # Load Byte Zero-extended
lbzx    rd, rs1, rs2            # Load Byte Zero-extended Indexed (rs1+rs2)
lbs     rd, offset(rs)          # Load Byte Sign-extended
lbsx    rd, rs1, rs2            # Load Byte Sign-extended Indexed

lhz     rd, offset(rs)          # Load Halfword Zero-extended
lhzx    rd, rs1, rs2            # Load Halfword Zero-extended Indexed
lhs     rd, offset(rs)          # Load Halfword Sign-extended
lhsx    rd, rs1, rs2            # Load Halfword Sign-extended Indexed

lwz     rd, offset(rs)          # Load Word Zero-extended
lwzx    rd, rs1, rs2            # Load Word Zero-extended Indexed
lws     rd, offset(rs)          # Load Word Sign-extended (for 64-bit)
lwsx    rd, rs1, rs2            # Load Word Sign-extended Indexed

ld      rd, offset(rs)          # Load Doubleword
ldx     rd, rs1, rs2            # Load Doubleword Indexed

lq      rd, offset(rs)          # Load Quadword (128-bit)
lqx     rd, rs1, rs2            # Load Quadword Indexed

# Load with update (post-increment addressing)
lbzu    rd, offset(rs)          # Load Byte Zero-ext, rs += offset
lhzu    rd, offset(rs)          # Load Halfword Zero-ext, rs += offset
lwzu    rd, offset(rs)          # Load Word Zero-ext, rs += offset
ldu     rd, offset(rs)          # Load Doubleword, rs += offset

# Load with pre-increment
lbzp    rd, offset(rs)          # rs += offset, then load byte
lhzp    rd, offset(rs)          # rs += offset, then load halfword
lwzp    rd, offset(rs)          # rs += offset, then load word
ldp     rd, offset(rs)          # rs += offset, then load doubleword

# Store instructions (register → memory)
# Format: st<size> rs, offset(rd)

stb     rs, offset(rd)          # Store Byte
stbx    rs, rd1, rd2            # Store Byte Indexed
sth     rs, offset(rd)          # Store Halfword
sthx    rs, rd1, rd2            # Store Halfword Indexed
stw     rs, offset(rd)          # Store Word
stwx    rs, rd1, rd2            # Store Word Indexed
std     rs, offset(rd)          # Store Doubleword
stdx    rs, rd1, rd2            # Store Doubleword Indexed
stq     rs, offset(rd)          # Store Quadword
stqx    rs, rd1, rd2            # Store Quadword Indexed

# Store with update
stbu    rs, offset(rd)          # Store Byte, rd += offset
sthu    rs, offset(rd)          # Store Halfword, rd += offset
stwu    rs, offset(rd)          # Store Word, rd += offset
stdu    rs, offset(rd)          # Store Doubleword, rd += offset

# Store with pre-increment
stbp    rs, offset(rd)          # rd += offset, then store byte
sthp    rs, offset(rd)          # rd += offset, then store halfword
stwp    rs, offset(rd)          # rd += offset, then store word
stdp    rs, offset(rd)          # rd += offset, then store doubleword

# Load/store with byte reversal (endian swap)
lhbrx   rd, rs1, rs2            # Load Halfword Byte-Reversed Indexed
lwbrx   rd, rs1, rs2            # Load Word Byte-Reversed Indexed
ldbrx   rd, rs1, rs2            # Load Doubleword Byte-Reversed Indexed
sthbrx  rs, rd1, rd2            # Store Halfword Byte-Reversed Indexed
stwbrx  rs, rd1, rd2            # Store Word Byte-Reversed Indexed
stdbrx  rs, rd1, rd2            # Store Doubleword Byte-Reversed Indexed

# Multiple load/store (ARM-style)
ldm     {r0-r7}, (sp)           # Load Multiple from stack
stm     {r0-r7}, (sp)           # Store Multiple to stack
ldmia   {r0-r7}, (rs)!          # Load Multiple Increment After, update rs
stmia   {r0-r7}, (rs)!          # Store Multiple Increment After, update rs
ldmdb   {r0-r7}, (rs)!          # Load Multiple Decrement Before
stmdb   {r0-r7}, (rs)!          # Store Multiple Decrement Before (push)
```

### Integer Arithmetic

```assembly
# Add instructions
add     rd, rs1, rs2            # rd = rs1 + rs2
addi    rd, rs, imm             # rd = rs + imm (sign-extended)
addis   rd, rs, imm             # rd = rs + (imm << 16)
addc    rd, rs1, rs2            # Add with Carry
addic   rd, rs, imm             # Add Immediate with Carry
adde    rd, rs1, rs2            # Add Extended (with carry)
addme   rd, rs                  # Add to Minus One Extended
addze   rd, rs                  # Add to Zero Extended

# Subtract instructions
sub     rd, rs1, rs2            # rd = rs1 - rs2
subi    rd, rs, imm             # rd = rs - imm
subic   rd, rs, imm             # Subtract Immediate with Carry
subc    rd, rs1, rs2            # Subtract with Carry
sube    rd, rs1, rs2            # Subtract Extended
subme   rd, rs                  # Subtract from Minus One Extended
subze   rd, rs                  # Subtract from Zero Extended

# Multiply instructions
mull.w  rd, rs1, rs2            # Multiply Low Word (signed)
mulh.w  rd, rs1, rs2            # Multiply High Word (signed)
mulhu.w rd, rs1, rs2            # Multiply High Word (unsigned)
mulli   rd, rs, imm             # Multiply Low Immediate
mull.d  rd, rs1, rs2            # Multiply Low Doubleword
mulh.d  rd, rs1, rs2            # Multiply High Doubleword

# Divide instructions
div.w   rd, rs1, rs2            # Divide Word (signed)
divu.w  rd, rs1, rs2            # Divide Word (unsigned)
div.d   rd, rs1, rs2            # Divide Doubleword (signed)
divu.d  rd, rs1, rs2            # Divide Doubleword (unsigned)
rem.w   rd, rs1, rs2            # Remainder Word (signed, modulo)
remu.w  rd, rs1, rs2            # Remainder Word (unsigned)
rem.d   rd, rs1, rs2            # Remainder Doubleword
remu.d  rd, rs1, rs2            # Remainder Doubleword (unsigned)

# Negate
neg     rd, rs                  # rd = -rs
neg.o   rd, rs                  # Negate with overflow check

# Absolute value
abs     rd, rs                  # rd = |rs|
```

### Logical and Bit Operations

```assembly
# Logical operations
and     rd, rs1, rs2            # rd = rs1 & rs2
andi    rd, rs, imm             # rd = rs & imm
andis   rd, rs, imm             # rd = rs & (imm << 16)
andc    rd, rs1, rs2            # rd = rs1 & ~rs2 (and complement)

or      rd, rs1, rs2            # rd = rs1 | rs2
ori     rd, rs, imm             # rd = rs | imm
oris    rd, rs, imm             # rd = rs | (imm << 16)
orc     rd, rs1, rs2            # rd = rs1 | ~rs2

xor     rd, rs1, rs2            # rd = rs1 ^ rs2
xori    rd, rs, imm             # rd = rs ^ imm
xoris   rd, rs, imm             # rd = rs ^ (imm << 16)

nor     rd, rs1, rs2            # rd = ~(rs1 | rs2)
nand    rd, rs1, rs2            # rd = ~(rs1 & rs2)
not     rd, rs                  # rd = ~rs (pseudo: nor rd, rs, r0)

# Shift operations
sll     rd, rs, rt              # Shift Left Logical (by register)
slli    rd, rs, shamt           # Shift Left Logical Immediate
srl     rd, rs, rt              # Shift Right Logical
srli    rd, rs, shamt           # Shift Right Logical Immediate
sra     rd, rs, rt              # Shift Right Arithmetic
srai    rd, rs, shamt           # Shift Right Arithmetic Immediate

# Rotate operations
rotl    rd, rs, rt              # Rotate Left
rotli   rd, rs, shamt           # Rotate Left Immediate
rotr    rd, rs, rt              # Rotate Right
rotri   rd, rs, shamt           # Rotate Right Immediate

# Rotate with mask (PowerPC-style)
rlwinm  rd, rs, sh, mb, me      # Rotate Left Word Immediate then AND with Mask
rlwnm   rd, rs, rb, mb, me      # Rotate Left Word then AND with Mask
rlwimi  rd, rs, sh, mb, me      # Rotate Left Word Immediate then Mask Insert

# Bit field operations
bfextu  rd, rs, pos, len        # Bit Field Extract Unsigned
bfexts  rd, rs, pos, len        # Bit Field Extract Signed
bfins   rd, rs, pos, len        # Bit Field Insert
bfclr   rd, pos, len            # Bit Field Clear

# Count operations
clz     rd, rs                  # Count Leading Zeros
ctz     rd, rs                  # Count Trailing Zeros
popcnt  rd, rs                  # Population Count (number of 1 bits)
parity  rd, rs                  # Parity (XOR of all bits)
```

### Comparison and Condition Codes

```assembly
# Compare instructions (set condition register)
cmp     rs1, rs2                # Compare (signed)
cmpi    rs, imm                 # Compare Immediate (signed)
cmpu    rs1, rs2                # Compare Unsigned
cmpui   rs, imm                 # Compare Unsigned Immediate

# Compare and set register
cmplt   rd, rs1, rs2            # rd = (rs1 < rs2) ? 1 : 0 (signed)
cmpltu  rd, rs1, rs2            # rd = (rs1 < rs2) ? 1 : 0 (unsigned)
cmple   rd, rs1, rs2            # rd = (rs1 <= rs2) ? 1 : 0
cmpleu  rd, rs1, rs2            # rd = (rs1 <= rs2) ? 1 : 0 (unsigned)
cmpeq   rd, rs1, rs2            # rd = (rs1 == rs2) ? 1 : 0
cmpne   rd, rs1, rs2            # rd = (rs1 != rs2) ? 1 : 0

# Select (conditional move)
sel     rd, rs1, rs2, cc        # rd = cc ? rs1 : rs2
seleq   rd, rs1, rs2            # rd = (zero flag) ? rs1 : rs2
selne   rd, rs1, rs2            # rd = (!zero flag) ? rs1 : rs2
```

### Branch and Jump Instructions

```assembly
# Unconditional branches
b       target                  # Branch
bl      target                  # Branch and Link (call)
br      rs                      # Branch Register
blr     rs                      # Branch and Link Register
ret                             # Return (pseudo: br ra)

# Conditional branches (based on condition flags)
beq     target                  # Branch if Equal
bne     target                  # Branch if Not Equal
blt     target                  # Branch if Less Than
bge     target                  # Branch if Greater or Equal
ble     target                  # Branch if Less or Equal
bgt     target                  # Branch if Greater Than

# Unsigned conditional branches
bltu    target                  # Branch if Less Than Unsigned
bgeu    target                  # Branch if Greater or Equal Unsigned
bleu    target                  # Branch if Less or Equal Unsigned
bgtu    target                  # Branch if Greater Than Unsigned

# Compare and branch (fused)
beqz    rs, target              # Branch if Equal to Zero
bnez    rs, target              # Branch if Not Equal to Zero
bltz    rs, target              # Branch if Less Than Zero
bgez    rs, target              # Branch if Greater or Equal to Zero
blez    rs, target              # Branch if Less or Equal to Zero
bgtz    rs, target              # Branch if Greater Than Zero

# PowerPC-style condition register branches
bcr     cr_bit, target          # Branch if Condition Register bit set
bcrl    cr_bit, target          # Branch if CR bit set and Link

# Test and branch
tbeq    rs1, rs2, target        # Test and Branch if Equal
tbne    rs1, rs2, target        # Test and Branch if Not Equal
tblt    rs1, rs2, target        # Test and Branch if Less Than
tbge    rs1, rs2, target        # Test and Branch if Greater or Equal

# Jump instructions
j       target                  # Jump (26-bit offset)
jal     target                  # Jump and Link
jr      rs                      # Jump Register
jalr    rd, rs                  # Jump and Link Register (rd = PC+4, PC = rs)
```

### Data Movement

```assembly
# Move instructions
mov     rd, rs                  # Move register (pseudo: or rd, rs, r0)
movi    rd, imm                 # Move immediate (pseudo: addi rd, r0, imm)
movz    rd, rs, rt              # Move if Zero (rd = (rt == 0) ? rs : rd)
movn    rd, rs, rt              # Move if Not Zero (rd = (rt != 0) ? rs : rd)

# Load immediate
li      rd, imm32               # Load Immediate (32-bit)
lis     rd, imm16               # Load Immediate Shifted (imm << 16)
lui     rd, imm20               # Load Upper Immediate (imm << 12)

# Load address
la      rd, symbol              # Load Address (pseudo)
lla     rd, symbol              # Load Local Address

# Swap
swap    rd, rs                  # Swap contents of rd and rs

# Conditional move
cmov.eq rd, rs1, rs2            # rd = (eq) ? rs1 : rs2
cmov.ne rd, rs1, rs2            # rd = (ne) ? rs1 : rs2
cmov.lt rd, rs1, rs2            # rd = (lt) ? rs1 : rs2
cmov.ge rd, rs1, rs2            # rd = (ge) ? rs1 : rs2
```

## Floating-Point Instructions

### Floating-Point Arithmetic

```assembly
# Single precision (32-bit)
fadd.s      fd, fs1, fs2        # FP Add Single
fsub.s      fd, fs1, fs2        # FP Subtract Single
fmul.s      fd, fs1, fs2        # FP Multiply Single
fdiv.s      fd, fs1, fs2        # FP Divide Single
fsqrt.s     fd, fs              # FP Square Root Single
fmadd.s     fd, fs1, fs2, fs3   # FP Multiply-Add: fd = (fs1*fs2)+fs3
fmsub.s     fd, fs1, fs2, fs3   # FP Multiply-Sub: fd = (fs1*fs2)-fs3
fnmadd.s    fd, fs1, fs2, fs3   # FP Negative Multiply-Add
fnmsub.s    fd, fs1, fs2, fs3   # FP Negative Multiply-Sub
fabs.s      fd, fs              # FP Absolute Value Single
fneg.s      fd, fs              # FP Negate Single

# Double precision (64-bit)
fadd.d      fd, fs1, fs2        # FP Add Double
fsub.d      fd, fs1, fs2        # FP Subtract Double
fmul.d      fd, fs1, fs2        # FP Multiply Double
fdiv.d      fd, fs1, fs2        # FP Divide Double
fsqrt.d     fd, fs              # FP Square Root Double
fmadd.d     fd, fs1, fs2, fs3   # FP Multiply-Add Double
fmsub.d     fd, fs1, fs2, fs3   # FP Multiply-Sub Double
fnmadd.d    fd, fs1, fs2, fs3   # FP Negative Multiply-Add Double
fnmsub.d    fd, fs1, fs2, fs3   # FP Negative Multiply-Sub Double
fabs.d      fd, fs              # FP Absolute Value Double
fneg.d      fd, fs              # FP Negate Double

# Quad precision (128-bit)
fadd.q      fd, fs1, fs2        # FP Add Quad
fsub.q      fd, fs1, fs2        # FP Subtract Quad
fmul.q      fd, fs1, fs2        # FP Multiply Quad
fdiv.q      fd, fs1, fs2        # FP Divide Quad
fsqrt.q     fd, fs              # FP Square Root Quad

# Half precision (16-bit)
fadd.h      fd, fs1, fs2        # FP Add Half
fsub.h      fd, fs1, fs2        # FP Subtract Half
fmul.h      fd, fs1, fs2        # FP Multiply Half
fdiv.h      fd, fs1, fs2        # FP Divide Half

# bfloat16
fadd.bf     fd, fs1, fs2        # FP Add BFloat16
fsub.bf     fd, fs1, fs2        # FP Subtract BFloat16
fmul.bf     fd, fs1, fs2        # FP Multiply BFloat16

# Decimal floating-point
dadd.32     fd, fs1, fs2        # Decimal Add (Decimal32)
dsub.32     fd, fs1, fs2        # Decimal Subtract
dmul.32     fd, fs1, fs2        # Decimal Multiply
ddiv.32     fd, fs1, fs2        # Decimal Divide
dadd.64     fd, fs1, fs2        # Decimal Add (Decimal64)
dadd.128    fd, fs1, fs2        # Decimal Add (Decimal128)
```

### Floating-Point Comparison

```assembly
fcmp.s      fs1, fs2            # FP Compare Single (set flags)
fcmpi.s     fs, imm             # FP Compare Immediate Single
fcmp.d      fs1, fs2            # FP Compare Double
fcmp.q      fs1, fs2            # FP Compare Quad

# FP compare and set
fcmpeq.s    fd, fs1, fs2        # fd = (fs1 == fs2)
fcmplt.s    fd, fs1, fs2        # fd = (fs1 < fs2)
fcmple.s    fd, fs1, fs2        # fd = (fs1 <= fs2)
fcmpun.s    fd, fs1, fs2        # fd = (fs1 unordered fs2)

# FP conditional branches
fbeq.s      target              # Branch if FP Equal
fbne.s      target              # Branch if FP Not Equal
fblt.s      target              # Branch if FP Less Than
fbge.s      target              # Branch if FP Greater or Equal
```

### Floating-Point Conversions

```assembly
# FP to integer
fcvt.w.s    rd, fs              # Convert Single to Word (signed)
fcvt.wu.s   rd, fs              # Convert Single to Word (unsigned)
fcvt.l.s    rd, fs              # Convert Single to Long (64-bit signed)
fcvt.lu.s   rd, fs              # Convert Single to Long (unsigned)

fcvt.w.d    rd, fs              # Convert Double to Word
fcvt.l.d    rd, fs              # Convert Double to Long

# Integer to FP
fcvt.s.w    fd, rs              # Convert Word to Single (signed)
fcvt.s.wu   fd, rs              # Convert Word (unsigned) to Single
fcvt.s.l    fd, rs              # Convert Long to Single
fcvt.s.lu   fd, rs              # Convert Long (unsigned) to Single

fcvt.d.w    fd, rs              # Convert Word to Double
fcvt.d.l    fd, rs              # Convert Long to Double

# FP to FP conversions
fcvt.s.d    fd, fs              # Convert Double to Single
fcvt.d.s    fd, fs              # Convert Single to Double
fcvt.s.h    fd, fs              # Convert Half to Single
fcvt.h.s    fd, fs              # Convert Single to Half
fcvt.s.bf   fd, fs              # Convert BFloat16 to Single
fcvt.bf.s   fd, fs              # Convert Single to BFloat16
```

### Floating-Point Load/Store

```assembly
flw     fd, offset(rs)          # FP Load Word (Single)
fld     fd, offset(rs)          # FP Load Doubleword (Double)
flq     fd, offset(rs)          # FP Load Quadword (Quad)
flh     fd, offset(rs)          # FP Load Halfword (Half/BFloat16)

fsw     fs, offset(rd)          # FP Store Word
fsd     fs, offset(rd)          # FP Store Doubleword
fsq     fs, offset(rd)          # FP Store Quadword
fsh     fs, offset(rd)          # FP Store Halfword

# Indexed
flwx    fd, rs1, rs2            # FP Load Word Indexed
fldx    fd, rs1, rs2            # FP Load Doubleword Indexed
fswx    fs, rd1, rd2            # FP Store Word Indexed
fsdx    fs, rd1, rd2            # FP Store Doubleword Indexed
```

## Vector Instructions

### Vector Arithmetic

```assembly
# Vector add (width-agnostic, configured via VSETVL)
vadd        vd, vs1, vs2        # Vector Add
vaddi       vd, vs, imm         # Vector Add Immediate
vsub        vd, vs1, vs2        # Vector Subtract
vmul        vd, vs1, vs2        # Vector Multiply
vdiv        vd, vs1, vs2        # Vector Divide
vrem        vd, vs1, vs2        # Vector Remainder

# Vector add with element size suffix
vadd.8      vd, vs1, vs2        # Vector Add 8-bit elements
vadd.16     vd, vs1, vs2        # Vector Add 16-bit elements
vadd.32     vd, vs1, vs2        # Vector Add 32-bit elements
vadd.64     vd, vs1, vs2        # Vector Add 64-bit elements

# Vector FP operations
vfadd.s     vd, vs1, vs2        # Vector FP Add Single
vfadd.d     vd, vs1, vs2        # Vector FP Add Double
vfmul.s     vd, vs1, vs2        # Vector FP Multiply Single
vfdiv.s     vd, vs1, vs2        # Vector FP Divide Single
vfmadd.s    vd, vs1, vs2, vs3   # Vector FP Multiply-Add

# Vector absolute/negate
vabs        vd, vs              # Vector Absolute Value
vneg        vd, vs              # Vector Negate
vfabs.s     vd, vs              # Vector FP Absolute Value
vfneg.s     vd, vs              # Vector FP Negate
```

### Vector Logical

```assembly
vand        vd, vs1, vs2        # Vector AND
vor         vd, vs1, vs2        # Vector OR
vxor        vd, vs1, vs2        # Vector XOR
vnot        vd, vs              # Vector NOT

vsll        vd, vs1, vs2        # Vector Shift Left Logical
vslli       vd, vs, imm         # Vector Shift Left Logical Immediate
vsrl        vd, vs1, vs2        # Vector Shift Right Logical
vsrli       vd, vs, imm         # Vector Shift Right Logical Immediate
vsra        vd, vs1, vs2        # Vector Shift Right Arithmetic
vsrai       vd, vs, imm         # Vector Shift Right Arithmetic Immediate
```

### Vector Load/Store

```assembly
vld         vd, offset(rs)      # Vector Load
vst         vs, offset(rd)      # Vector Store
vldx        vd, rs1, rs2        # Vector Load Indexed
vstx        vs, rd1, rd2        # Vector Store Indexed

# Strided load/store
vlds        vd, rs, stride      # Vector Load Strided
vsts        vs, rd, stride      # Vector Store Strided

# Segmented load/store
vldseg.2    vd, rs              # Load 2 segments (deinterleave)
vldseg.4    vd, rs              # Load 4 segments
vstseg.2    vs, rd              # Store 2 segments (interleave)

# Whole register load/store
vld.r       vd, (rs)            # Load whole vector register
vst.r       vs, (rd)            # Store whole vector register
```

### Vector Configuration

```assembly
vsetvl      rd, rs              # Set vector length (VL = min(rs, VLMAX))
vsetvli     rd, imm             # Set vector length immediate
vsettype    vtype               # Set vector type (element width, LMUL)
vgetlength  rd                  # Get current vector length
```

### Vector Reduction

```assembly
vredsum     vd, vs              # Vector Reduce Sum
vredmax     vd, vs              # Vector Reduce Maximum
vredmin     vd, vs              # Vector Reduce Minimum
vredand     vd, vs              # Vector Reduce AND
vredor      vd, vs              # Vector Reduce OR
vredxor     vd, vs              # Vector Reduce XOR

vfredsum.s  vd, vs              # Vector FP Reduce Sum
vfredmax.s  vd, vs              # Vector FP Reduce Maximum
vfredmin.s  vd, vs              # Vector FP Reduce Minimum
```

### Vector Permute

```assembly
vperm       vd, vs1, vs2, vm    # Vector Permute (mask in vm)
vmerge      vd, vs1, vs2, vm    # Vector Merge (select by mask)
vrgather    vd, vs, vi          # Vector Gather (indexed)
vcompress   vd, vs, vm          # Vector Compress (pack active elements)

vslideup    vd, vs, offset      # Vector Slide Up
vslidedown  vd, vs, offset      # Vector Slide Down
```

## Matrix Instructions

### Matrix Configuration

```assembly
msetdim     mr, rows, cols, elem_size, layout
                                # Set matrix dimensions
mgetdim     rd_rows, rd_cols, mr
                                # Get matrix dimensions
```

### Matrix Load/Store

```assembly
mld         mr, addr            # Matrix Load
mst         mr, addr            # Matrix Store
mldtile     mr, tile_r, tile_c, addr
                                # Matrix Load Tile
msttile     mr, tile_r, tile_c, addr
                                # Matrix Store Tile
```

### Matrix Arithmetic

```assembly
madd        md, ma, mb          # Matrix Add: md = ma + mb
msub        md, ma, mb          # Matrix Subtract: md = ma - mb
mmul        md, ma, mb          # Matrix Multiply: md = ma × mb
mmul.t      md, ma, mb          # Matrix Multiply Transpose: md = ma × mb^T
mtrans      md, ms              # Matrix Transpose: md = ms^T

mscale      md, ms, scalar      # Matrix Scale: md = scalar * ms
mhadamard   md, ma, mb          # Hadamard Product: md = ma ⊙ mb (element-wise)

# Matrix-vector operations
mmvmul      vd, m, vs           # Matrix-Vector Multiply: vd = m × vs
mvmmul      vd, vs, m           # Vector-Matrix Multiply: vd = vs × m
```

### Matrix Reduction

```assembly
msum        rd, ms              # Matrix Sum: rd = sum(all elements of ms)
mmax        rd, ms              # Matrix Maximum
mmin        rd, ms              # Matrix Minimum
mnorm.1     rd, ms              # Matrix 1-norm
mnorm.2     rd, ms              # Matrix 2-norm (Frobenius)
mnorm.inf   rd, ms              # Matrix infinity-norm
```

## System Instructions

### Control and Status Registers

```assembly
# Move from/to system register
mfsr        rd, sr              # Move From System Register
mtsr        sr, rs              # Move To System Register

# Specific system registers
mfpc        rd                  # Move From Program Counter
mtpc        rs                  # Move To Program Counter (jump)
mfsp        rd                  # Move From Stack Pointer
mtsp        rs                  # Move To Stack Pointer
mfstatus    rd                  # Move From Status Register
mtstatus    rs                  # Move To Status Register

# Condition register operations (PowerPC-style)
mfcr        rd                  # Move From Condition Register
mtcr        rs                  # Move To Condition Register
mcrxr       crf                 # Move to CR from XER
```

### Trap and Exception

```assembly
trap                            # Trap unconditional
trapeq                          # Trap if Equal
trapne                          # Trap if Not Equal
traplt                          # Trap if Less Than
trapge                          # Trap if Greater or Equal

syscall     imm                 # System Call
break       imm                 # Breakpoint
```

### Synchronization

```assembly
sync                            # Synchronize
isync                           # Instruction Synchronize
dsync                           # Data Synchronize
lwsync                          # Lightweight Sync (load/store ordering)

fence                           # Memory Fence
fence.i                         # Instruction Fence
fence.tso                       # Total Store Ordering Fence

eieio                           # Enforce In-order Execution of I/O (PowerPC)
```

### Atomics

```assembly
# Load-linked / Store-conditional
ll          rd, (rs)            # Load-Linked
sc          rd, rs, (rt)        # Store-Conditional (rd = success)

# Atomic memory operations
amoadd.w    rd, rs, (rt)        # Atomic Add Word
amoswap.w   rd, rs, (rt)        # Atomic Swap Word
amoand.w    rd, rs, (rt)        # Atomic AND Word
amoor.w     rd, rs, (rt)        # Atomic OR Word
amoxor.w    rd, rs, (rt)        # Atomic XOR Word
amomin.w    rd, rs, (rt)        # Atomic Minimum Word
amomax.w    rd, rs, (rt)        # Atomic Maximum Word

# Compare-and-swap
cas.w       rd, rs_old, rs_new, (rt)
                                # Compare and Swap Word
cas.d       rd, rs_old, rs_new, (rt)
                                # Compare and Swap Doubleword
```

### Cache Management

```assembly
dcbf        (rs)                # Data Cache Block Flush
dcbi        (rs)                # Data Cache Block Invalidate
dcbst       (rs)                # Data Cache Block Store
dcbt        (rs)                # Data Cache Block Touch (prefetch)
dcbtst      (rs)                # Data Cache Block Touch for Store
dcbz        (rs)                # Data Cache Block set to Zero

icbi        (rs)                # Instruction Cache Block Invalidate
```

## CHERI Capability Instructions

### Capability Inspection

```assembly
cgetbase    rd, cs              # Get Capability Base
cgetlen     rd, cs              # Get Capability Length
cgetperm    rd, cs              # Get Capability Permissions
cgettype    rd, cs              # Get Capability Type (otype)
cgettag     rd, cs              # Get Capability Tag
cgetaddr    rd, cs              # Get Capability Address
cgetoffset  rd, cs              # Get Capability Offset
cget top    rd, cs              # Get Capability Top (base + length)
```

### Capability Modification

```assembly
csetaddr    cd, cs, rt          # Set Capability Address
cincoffset  cd, cs, rt          # Increment Capability Offset
csetbounds  cd, cs, rt          # Set Capability Bounds
candperm    cd, cs, rt          # AND Capability Permissions
cseal       cd, cs, ct          # Seal Capability
cunseal     cd, cs, ct          # Unseal Capability
cfromptr    cd, cs, rt          # Create Capability from Pointer
ctoptr      rd, cs, ct          # Convert Capability to Pointer
```

### Capability Memory Access

```assembly
clb         rd, offset(cs)      # Capability Load Byte
clh         rd, offset(cs)      # Capability Load Halfword
clw         rd, offset(cs)      # Capability Load Word
cld         rd, offset(cs)      # Capability Load Doubleword
clc         cd, offset(cs)      # Capability Load Capability

csb         rs, offset(ct)      # Capability Store Byte
csh         rs, offset(ct)      # Capability Store Halfword
csw         rs, offset(ct)      # Capability Store Word
csd         rs, offset(ct)      # Capability Store Doubleword
csc         cs, offset(ct)      # Capability Store Capability
```

### Capability Control Flow

```assembly
cjalr       cd, cs              # Capability Jump and Link Register
cjr         cs                  # Capability Jump Register
ccall       cs, ct              # Capability Call (cross-domain)
cret                            # Capability Return
```

## Secure Enclave Instructions

```assembly
ecreate     rd, code_ptr, code_sz, data_ptr, data_sz
                                # Enclave Create
eenter      enclave_id, entry, params
                                # Enclave Enter
eexit       result              # Enclave Exit
eresume     enclave_id          # Enclave Resume
edestroy    enclave_id          # Enclave Destroy
egetinfo    rd_base, rd_sz, enclave_id
                                # Enclave Get Info
eattest     rd, enclave_id, user_data, len
                                # Enclave Attest (generate report)
eseal       rd, data_ptr, len, enclave_id
                                # Enclave Seal Data
eunseal     rd, sealed_ptr, len, enclave_id
                                # Enclave Unseal Data

# Secure world transitions
smc         imm                 # Secure Monitor Call
smcret                          # Secure Monitor Call Return
getworld    rd                  # Get Current World (0=Normal, 1=Secure)
setmemreg   region, base, size, flags
                                # Set Memory Region Security
```

## Neural Network Instructions

### Forward Pass

```assembly
nn.dense.fwd    md, mi, mw, vb  # Dense Layer Forward
nn.relu         md, ms          # ReLU Activation
nn.gelu         md, ms          # GELU Activation
nn.sigmoid      md, ms          # Sigmoid Activation
nn.tanh         md, ms          # Tanh Activation
nn.softmax      md, ms, axis    # Softmax
nn.bn.fwd       md, mi, vg, vb, vm, vv, epsilon
                                # Batch Normalization Forward
nn.dropout.fwd  md, mi, mm, rate
                                # Dropout Forward
nn.conv2d.fwd   md, mi, mk, vb, stride, padding
                                # 2D Convolution Forward
nn.pool.max     md, mi, kernel, stride, pad
                                # Max Pooling
nn.pool.avg     md, mi, kernel, stride, pad
                                # Average Pooling
```

### Backward Pass

```assembly
nn.dense.bwd    mgi, mgw, vgb, mgo, mi, mw
                                # Dense Layer Backward
nn.relu.grad    mgi, mgo, mi    # ReLU Gradient
nn.sigmoid.grad mgi, mgo, mo    # Sigmoid Gradient
nn.tanh.grad    mgi, mgo, mo    # Tanh Gradient
nn.bn.bwd       mgi, vgg, vgb, mgo, mi, vg, vm, vv, eps
                                # Batch Normalization Backward
nn.conv2d.bwd.in  mgi, mgo, mk, stride, pad
                                # Conv2D Backward (input gradient)
nn.conv2d.bwd.ker mgk, mgo, mi, stride, pad
                                # Conv2D Backward (kernel gradient)
```

### Loss Functions

```assembly
nn.loss.crossent    vl, ml, mt  # Cross-Entropy Loss
nn.loss.crossent.g  mg, ml, mt  # Cross-Entropy Gradient
nn.loss.mse         vl, mp, mt  # Mean Squared Error
nn.loss.mse.g       mg, mp, mt  # MSE Gradient
```

### Optimizers

```assembly
nn.opt.sgd      mp, mg, lr      # SGD Update
nn.opt.sgd.mom  mp, mg, mv, lr, mom
                                # SGD with Momentum
nn.opt.adam     mp, mg, mm, mv, lr, b1, b2, eps, t
                                # Adam Update
nn.opt.rmsprop  mp, mg, mc, lr, decay, eps
                                # RMSprop Update
```

## DSP and Multimedia Instructions

### DSP Accumulators

```assembly
setaccw     acc, width          # Set Accumulator Width (24-128 bits)
clracc      acc                 # Clear Accumulator
ldacc       acc, rs             # Load Accumulator from GPR
stacc       rd, acc             # Store Accumulator to GPR

mac         acc, rs1, rs2       # Multiply-Accumulate (signed)
macu        acc, rs1, rs2       # Multiply-Accumulate (unsigned)
msu         acc, rs1, rs2       # Multiply-Subtract
macf        acc, rs1, rs2       # Fractional MAC (Q15×Q15)
macf.32     acc, rs1, rs2       # Fractional MAC (Q31×Q31)

shlacc      acc, shamt          # Shift Left Accumulator
shracc      acc, shamt          # Shift Right Accumulator
shracc.r    acc, shamt          # Shift Right Accumulator with Rounding

extrh       rd, acc             # Extract High word from Accumulator
extrl       rd, acc             # Extract Low word from Accumulator
```

### SIMD Packed Operations

```assembly
# Packed byte operations
padd.b      rd, rs1, rs2        # Packed Add Bytes (4×8-bit in 32-bit)
psub.b      rd, rs1, rs2        # Packed Subtract Bytes
pmul.b      rd, rs1, rs2        # Packed Multiply Bytes

# Packed halfword operations
padd.h      rd, rs1, rs2        # Packed Add Halfwords (2×16-bit)
psub.h      rd, rs1, rs2        # Packed Subtract Halfwords
pmul.h      rd, rs1, rs2        # Packed Multiply Halfwords

# Packed saturation
paddsat.b   rd, rs1, rs2        # Packed Add Saturate Bytes
psubsat.b   rd, rs1, rs2        # Packed Subtract Saturate Bytes
```

## Quantum Computing Instructions

```assembly
# Single-qubit gates
qh          qubit               # Hadamard Gate
qx          qubit               # Pauli-X Gate
qy          qubit               # Pauli-Y Gate
qz          qubit               # Pauli-Z Gate
qs          qubit               # S Gate
qt          qubit               # T Gate
qrx         qubit, angle        # Rotation X
qry         qubit, angle        # Rotation Y
qrz         qubit, angle        # Rotation Z

# Two-qubit gates
qcnot       control, target     # Controlled-NOT
qcz         control, target     # Controlled-Z
qswap       q1, q2              # Swap
qtoffoli    c1, c2, target      # Toffoli (CCNOT)

# Measurement
qmeasure    rd, qubit           # Measure Qubit (collapse)
qprob       rd, qubit           # Get Probability (no collapse)
qmeasure.all                    # Measure All Qubits

# State management
qinit       num_qubits          # Initialize Quantum State
qreset      qubit               # Reset Qubit to |0⟩
```

## Instruction Encoding Summary

### Encoding Formats

```
R-type (Register): Used for 3-operand register operations
    31    26 25   21 20   16 15   11 10    6  5     0
    +-------+-------+-------+-------+-------+--------+
    | opcode|  rd   |  rs1  |  rs2  | func  |  ext   |
    +-------+-------+-------+-------+-------+--------+

I-type (Immediate): Used for 2-operand with immediate
    31    26 25   21 20   16 15                    0
    +-------+-------+-------+-----------------------+
    | opcode|  rd   |  rs   |      immediate        |
    +-------+-------+-------+-----------------------+

S-type (Store): Used for store operations
    31    26 25   21 20   16 15   11 10            0
    +-------+-------+-------+-------+---------------+
    | opcode|  rs   |  base |  rd   |    offset     |
    +-------+-------+-------+-------+---------------+

B-type (Branch): Used for conditional branches
    31    26 25   21 20   16 15                    0
    +-------+-------+-------+-----------------------+
    | opcode|  rs1  |  rs2  |      offset           |
    +-------+-------+-------+-----------------------+

J-type (Jump): Used for unconditional jumps
    31    26 25                                    0
    +-------+---------------------------------------+
    | opcode|            offset (26-bit)            |
    +-------+---------------------------------------+
```

## Pseudo-Instructions

Many common operations are implemented as pseudo-instructions (assembler macros):

```assembly
# Pseudo          # Actual
nop               # or r0, r0, r0
mov  rd, rs       # or rd, rs, r0
not  rd, rs       # nor rd, rs, r0
neg  rd, rs       # sub rd, r0, rs
li   rd, imm32    # lui + ori sequence
la   rd, symbol   # lui + addi sequence
ret               # br ra
call target       # jal ra, target
tail target       # j target (optimize tail call)
push rs           # stdu rs, -8(sp)
pop  rd           # ldu rd, 8(sp)
```

## Summary

This redesigned ISA provides:
- **Consistent naming**: Similar operations use similar patterns
- **Clear suffixes**: Data width indicated (.b/.h/.w/.d/.q)
- **Type prefixes**: Instruction class clearly indicated (f/v/m/c/nn)
- **Orthogonality**: Addressing modes work uniformly
- **Modern features**: Matches PowerPC/ARM64/OpenRISC conventions
- **Extensibility**: Easy to add new instructions following patterns

The mnemonic system is:
- Self-documenting
- Easy to learn
- Consistent across instruction classes
- Compatible with modern assemblers and compilers

---

**Document Status**: ISA Redesign Complete
**Compatibility**: Semantic compatibility with original DLX, new mnemonics
**Toolchain**: Requires assembler/compiler updates
**Benefits**: Clearer code, easier learning curve, better maintenance
