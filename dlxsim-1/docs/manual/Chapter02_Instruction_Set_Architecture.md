# Chapter 2: Instruction Set Architecture

## 2.1 Instruction Naming Convention

DLX uses modern, consistent mnemonics:

```
<prefix><operation><.width><modifier>

Prefixes:
  l/s   = Load/Store
  f     = Floating-point
  v     = Vector
  m     = Matrix
  c     = Capability (CHERI)
  e     = Enclave
  nn    = Neural network
  q     = Quantum
  crc   = CRC/checksum
  comp  = Compression

Width Suffixes:
  .b/.h/.w/.d/.q = Byte/Half/Word/Double/Quad (8/16/32/64/128-bit)
  .s/.d/.q/.h/.bf = Single/Double/Quad/Half/BFloat16 (FP)

Modifiers:
  z/s   = Zero/Sign extend
  u     = Update (post-increment)
  x     = Indexed
  a     = Atomic
  br    = Byte-reversed
```

## 2.2 Load and Store Instructions

### Basic Loads (New Mnemonics)
```assembly
# Zero-extended loads
lbz     rd, offset(rs)          # Load byte zero-extended
lhz     rd, offset(rs)          # Load halfword zero-extended
lwz     rd, offset(rs)          # Load word zero-extended
ld      rd, offset(rs)          # Load doubleword
lq      rd, offset(rs)          # Load quadword (128-bit)

# Sign-extended loads
lbs     rd, offset(rs)          # Load byte sign-extended
lhs     rd, offset(rs)          # Load halfword sign-extended
lws     rd, offset(rs)          # Load word sign-extended

# Indexed loads (base + index)
lbzx    rd, rs1, rs2            # rd ← mem[rs1 + rs2] (byte, zero-ext)
lhzx    rd, rs1, rs2            # Halfword indexed
lwzx    rd, rs1, rs2            # Word indexed
ldx     rd, rs1, rs2            # Doubleword indexed

# Loads with update (post-increment)
lbzu    rd, offset(rs)          # Load byte, then rs ← rs + offset
lhzu    rd, offset(rs)          # Load halfword with update
lwzu    rd, offset(rs)          # Load word with update
ldu     rd, offset(rs)          # Load doubleword with update

# Byte-reversed loads (endian conversion)
lwbrx   rd, rs1, rs2            # Load word byte-reversed indexed
ldbrx   rd, rs1, rs2            # Load doubleword byte-reversed
lhbrx   rd, rs1, rs2            # Load halfword byte-reversed

# Atomic loads
ll.w    rd, offset(rs)          # Load-linked word
ll.d    rd, offset(rs)          # Load-linked doubleword
```

### Basic Stores
```assembly
# Standard stores
stb     rs, offset(rd)          # Store byte
sth     rs, offset(rd)          # Store halfword
stw     rs, offset(rd)          # Store word
std     rs, offset(rd)          # Store doubleword
stq     rs, offset(rd)          # Store quadword (128-bit)

# Indexed stores
stbx    rs, rd1, rd2            # mem[rd1 + rd2] ← rs (byte)
sthx    rs, rd1, rd2            # Halfword indexed
stwx    rs, rd1, rd2            # Word indexed
stdx    rs, rd1, rd2            # Doubleword indexed

# Stores with update
stbu    rs, offset(rd)          # Store byte, then rd ← rd + offset
sthu    rs, offset(rd)          # Store halfword with update
stwu    rs, offset(rd)          # Store word with update
stdu    rs, offset(rd)          # Store doubleword with update

# Byte-reversed stores
stwbrx  rs, rd1, rd2            # Store word byte-reversed indexed
stdbrx  rs, rd1, rd2            # Store doubleword byte-reversed
sthbrx  rs, rd1, rd2            # Store halfword byte-reversed

# Atomic stores
sc.w    rs, offset(rd)          # Store-conditional word
sc.d    rs, offset(rd)          # Store-conditional doubleword
```

## 2.3 Integer Arithmetic

### Addition and Subtraction
```assembly
add     rd, rs1, rs2            # Add: rd ← rs1 + rs2
addi    rd, rs, imm             # Add immediate
addc    rd, rs1, rs2            # Add with carry
addco   rd, rs1, rs2            # Add with carry, set overflow
adde    rd, rs1, rs2            # Add extended (with CA)

sub     rd, rs1, rs2            # Subtract: rd ← rs1 - rs2
subi    rd, rs, imm             # Subtract immediate
subc    rd, rs1, rs2            # Subtract with carry
subco   rd, rs1, rs2            # Subtract with carry, set overflow
sube    rd, rs1, rs2            # Subtract extended

neg     rd, rs                  # Negate: rd ← 0 - rs
nego    rd, rs                  # Negate with overflow check
```

### Multiplication and Division
```assembly
# Multiplication
mull.w  rd, rs1, rs2            # Multiply low word: rd ← (rs1 × rs2)[31:0]
mulh.w  rd, rs1, rs2            # Multiply high word (signed)
mulhu.w rd, rs1, rs2            # Multiply high word (unsigned)
mulhsu.w rd, rs1, rs2           # Multiply high (signed × unsigned)
mull.d  rd, rs1, rs2            # Multiply low doubleword
mulh.d  rd, rs1, rs2            # Multiply high doubleword
mulli   rd, rs, imm             # Multiply immediate

# Division
div.w   rd, rs1, rs2            # Divide word (signed): rd ← rs1 ÷ rs2
divu.w  rd, rs1, rs2            # Divide word (unsigned)
div.d   rd, rs1, rs2            # Divide doubleword (signed)
divu.d  rd, rs1, rs2            # Divide doubleword (unsigned)

# Remainder
rem.w   rd, rs1, rs2            # Remainder word (signed): rd ← rs1 mod rs2
remu.w  rd, rs1, rs2            # Remainder word (unsigned)
rem.d   rd, rs1, rs2            # Remainder doubleword (signed)
remu.d  rd, rs1, rs2            # Remainder doubleword (unsigned)
```

## 2.4 Logical Operations

```assembly
# Bitwise operations
and     rd, rs1, rs2            # Bitwise AND
andi    rd, rs, imm             # AND immediate
or      rd, rs1, rs2            # Bitwise OR
ori     rd, rs, imm             # OR immediate
xor     rd, rs1, rs2            # Bitwise XOR
xori    rd, rs, imm             # XOR immediate
nor     rd, rs1, rs2            # Bitwise NOR
nand    rd, rs1, rs2            # Bitwise NAND

# Shifts
sll.w   rd, rs1, rs2            # Shift left logical word
slli.w  rd, rs, shamt           # Shift left logical immediate
srl.w   rd, rs1, rs2            # Shift right logical word
srli.w  rd, rs, shamt           # Shift right logical immediate
sra.w   rd, rs1, rs2            # Shift right arithmetic word
srai.w  rd, rs, shamt           # Shift right arithmetic immediate

# Rotates
rol.w   rd, rs1, rs2            # Rotate left word
ror.w   rd, rs1, rs2            # Rotate right word
roli.w  rd, rs, shamt           # Rotate left immediate
rori.w  rd, rs, shamt           # Rotate right immediate
```

## 2.5 Comparison and Branches

### Set on Comparison
```assembly
slt     rd, rs1, rs2            # Set if less than (signed)
sltu    rd, rs1, rs2            # Set if less than (unsigned)
slti    rd, rs, imm             # Set if less than immediate
sltiu   rd, rs, imm             # Set if less than immediate (unsigned)
seq     rd, rs1, rs2            # Set if equal
sne     rd, rs1, rs2            # Set if not equal
```

### Conditional Branches
```assembly
# Compare with zero
beqz    rs, offset              # Branch if equal to zero
bnez    rs, offset              # Branch if not equal to zero
bltz    rs, offset              # Branch if less than zero (signed)
bgez    rs, offset              # Branch if greater or equal to zero
bgtz    rs, offset              # Branch if greater than zero
blez    rs, offset              # Branch if less or equal to zero

# Compare two registers
beq     rs1, rs2, offset        # Branch if equal
bne     rs1, rs2, offset        # Branch if not equal
blt     rs1, rs2, offset        # Branch if less than (signed)
bge     rs1, rs2, offset        # Branch if greater or equal (signed)
bltu    rs1, rs2, offset        # Branch if less than (unsigned)
bgeu    rs1, rs2, offset        # Branch if greater or equal (unsigned)
```

### Jumps
```assembly
j       offset                  # Jump
jal     rd, offset              # Jump and link (call): rd ← PC+4, PC ← PC+offset
jr      rs                      # Jump register: PC ← rs
jalr    rd, rs, offset          # Jump and link register: rd ← PC+4, PC ← rs+offset
```

## 2.6 Floating-Point Instructions

### Single Precision (.s)
```assembly
fadd.s  fd, fs1, fs2            # FP add single
fsub.s  fd, fs1, fs2            # FP subtract single
fmul.s  fd, fs1, fs2            # FP multiply single
fdiv.s  fd, fs1, fs2            # FP divide single
fsqrt.s fd, fs                  # FP square root single

fmadd.s  fd, fs1, fs2, fs3      # Fused multiply-add: fd ← fs1×fs2 + fs3
fmsub.s  fd, fs1, fs2, fs3      # Fused multiply-sub: fd ← fs1×fs2 - fs3
fnmadd.s fd, fs1, fs2, fs3      # Neg fused multiply-add
fnmsub.s fd, fs1, fs2, fs3      # Neg fused multiply-sub

# Comparisons
feq.s   rd, fs1, fs2            # FP equal: rd ← (fs1 == fs2)
flt.s   rd, fs1, fs2            # FP less than: rd ← (fs1 < fs2)
fle.s   rd, fs1, fs2            # FP less or equal: rd ← (fs1 ≤ fs2)

# Conversions
fcvt.s.w  fd, rs                # Convert word to single
fcvt.w.s  rd, fs                # Convert single to word
fcvt.s.wu fd, rs                # Convert unsigned word to single
fcvt.wu.s rd, fs                # Convert single to unsigned word
```

### Double Precision (.d)
```assembly
fadd.d  fd, fs1, fs2            # FP add double
fsub.d  fd, fs1, fs2            # FP subtract double
fmul.d  fd, fs1, fs2            # FP multiply double
fdiv.d  fd, fs1, fs2            # FP divide double
fsqrt.d fd, fs                  # FP square root double

fmadd.d  fd, fs1, fs2, fs3      # Fused multiply-add double
fmsub.d  fd, fs1, fs2, fs3      # Fused multiply-sub double

feq.d   rd, fs1, fs2            # FP equal double
flt.d   rd, fs1, fs2            # FP less than double
fle.d   rd, fs1, fs2            # FP less or equal double

fcvt.d.w  fd, rs                # Convert word to double
fcvt.w.d  rd, fs                # Convert double to word
fcvt.d.s  fd, fs                # Convert single to double
fcvt.s.d  fd, fs                # Convert double to single
```

### Quad Precision (.q) and Half Precision (.h)
```assembly
fadd.q  fd, fs1, fs2            # FP add quad (128-bit)
fsub.q  fd, fs1, fs2            # FP subtract quad
fmul.q  fd, fs1, fs2            # FP multiply quad
fdiv.q  fd, fs1, fs2            # FP divide quad

fadd.h  fd, fs1, fs2            # FP add half (16-bit)
fsub.h  fd, fs1, fs2            # FP subtract half

fadd.bf fd, fs1, fs2            # FP add BFloat16
fmul.bf fd, fs1, fs2            # FP multiply BFloat16
```

## 2.7 Atomic Instructions

```assembly
# Atomic Memory Operations
amoadd.w    rd, rs2, (rs1)      # Atomic add word
amoxor.w    rd, rs2, (rs1)      # Atomic XOR word
amoor.w     rd, rs2, (rs1)      # Atomic OR word
amoand.w    rd, rs2, (rs1)      # Atomic AND word
amomin.w    rd, rs2, (rs1)      # Atomic minimum word (signed)
amomax.w    rd, rs2, (rs1)      # Atomic maximum word (signed)
amominu.w   rd, rs2, (rs1)      # Atomic minimum word (unsigned)
amomaxu.w   rd, rs2, (rs1)      # Atomic maximum word (unsigned)
amoswap.w   rd, rs2, (rs1)      # Atomic swap word

# Doubleword variants
amoadd.d    rd, rs2, (rs1)      # Atomic add doubleword
amoxor.d    rd, rs2, (rs1)      # Atomic XOR doubleword
# ... (all operations have .d variants)

# Load-Linked / Store-Conditional
ll.w        rd, (rs)            # Load-linked word
sc.w        rd, rs2, (rs1)      # Store-conditional word (rd=0 if success)
ll.d        rd, (rs)            # Load-linked doubleword
sc.d        rd, rs2, (rs1)      # Store-conditional doubleword

# Compare-and-Swap
cas.w       rd, rs2, rs3, (rs1) # Compare-and-swap word
cas.d       rd, rs2, rs3, (rs1) # Compare-and-swap doubleword
```

## 2.8 System Instructions

```assembly
# CSR (Control and Status Register) Access
csrrw   rd, csr, rs             # Read/write CSR: rd ← CSR[csr], CSR[csr] ← rs
csrrs   rd, csr, rs             # Read and set bits: rd ← CSR, CSR ← CSR | rs
csrrc   rd, csr, rs             # Read and clear bits: rd ← CSR, CSR ← CSR & ~rs
csrrwi  rd, csr, imm            # Read/write CSR immediate
csrrsi  rd, csr, imm            # Read and set bits immediate
csrrci  rd, csr, imm            # Read and clear bits immediate

# Memory Ordering
fence                           # Memory fence (all loads/stores)
fence.r                         # Read fence
fence.w                         # Write fence
fence.rw                        # Read-write fence
fence.i                         # Instruction fence (synchronize I-cache)
sync                            # Full synchronization barrier

# System Calls and Exceptions
ecall                           # Environment call (syscall, trapped to OS)
ebreak                          # Environment break (breakpoint, trapped to debugger)
eret                            # Return from exception (user→kernel transition)
sret                            # Return from supervisor mode
mret                            # Return from machine mode

# Hints and Waits
wfi                             # Wait for interrupt
nop                             # No operation
hint    imm                     # Hint to implementation
pause                           # Pause (spin-wait hint)
```

## 2.9 Cache Control

```assembly
# Data Cache Operations
dcbf        offset(rs)          # Data cache block flush
dcbi        offset(rs)          # Data cache block invalidate
dcbst       offset(rs)          # Data cache block store
dcbz        offset(rs)          # Data cache block zero
dcba        offset(rs)          # Data cache block allocate
dcbt        offset(rs)          # Data cache block touch (prefetch for read)
dcbtst      offset(rs)          # Data cache block touch for store

# Instruction Cache Operations
icbi        offset(rs)          # Instruction cache block invalidate
icbt        offset(rs)          # Instruction cache block touch

# Prefetch Hints
pref.r      offset(rs)          # Prefetch for read
pref.w      offset(rs)          # Prefetch for write
pref.i      offset(rs)          # Prefetch for instruction
```
