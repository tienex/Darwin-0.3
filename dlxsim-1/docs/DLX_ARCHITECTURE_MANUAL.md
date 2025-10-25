# DLX Architecture Reference Manual
## Complete Specification - Version 2.0

### The DLX RISC Processor Architecture
**A Modern 64-bit RISC Architecture with Advanced Extensions**

---

**Document Version**: 2.0
**Release Date**: October 2024
**Status**: Final Specification

---

## Copyright and License

Copyright © 2024. All Rights Reserved.

This specification is provided for educational and research purposes.

---

## Document Organization

This manual is organized into the following chapters:

1. **Introduction** - Overview and design philosophy
2. **Base ISA** - Core instruction set with modern mnemonics
3. **Registers and State** - Programmer-visible state
4. **Instruction Formats** - Encoding and formats
5. **Memory Architecture** - Addressing and memory model
6. **Binary Formats** - ELF, PE/COFF, Mach-O support
7. **ABI and Calling Conventions** - Standard interfaces
8. **Extensions** - Optional architectural extensions
9. **Virtualization** - Hypervisor and nested virtualization
10. **Security** - CHERI capabilities and secure enclaves
11. **Performance** - Timers and performance monitoring
12. **System Programming** - Privileged instructions and CSRs

---

## Table of Contents

- [Chapter 1: Introduction](#chapter-1-introduction)
- [Chapter 2: Base ISA](#chapter-2-base-isa)
- [Chapter 3: Registers and State](#chapter-3-registers-and-state)
- [Chapter 4: Instruction Formats](#chapter-4-instruction-formats)
- [Chapter 5: Memory Architecture](#chapter-5-memory-architecture)
- [Chapter 6: Binary Formats](#chapter-6-binary-formats)
- [Chapter 7: ABI and Calling Conventions](#chapter-7-abi-and-calling-conventions)
- [Chapter 8: Extensions](#chapter-8-extensions)
- [Chapter 9: Virtualization](#chapter-9-virtualization)
- [Chapter 10: Security](#chapter-10-security)
- [Chapter 11: Performance Monitoring](#chapter-11-performance-monitoring)
- [Chapter 12: System Programming](#chapter-12-system-programming)
- [Appendix A: Instruction Quick Reference](#appendix-a-instruction-quick-reference)
- [Appendix B: CSR Register Map](#appendix-b-csr-register-map)
- [Appendix C: Pseudo-Instructions](#appendix-c-pseudo-instructions)
- [Appendix D: Code Examples](#appendix-d-code-examples)

---

# Chapter 1: Introduction

## 1.1 Overview

The DLX architecture is a modern 64-bit RISC processor architecture designed for:

- **High performance** through pipelined execution and SMT
- **Scalability** from embedded systems to servers
- **Extensibility** with modular architectural extensions
- **Security** through CHERI capabilities and secure enclaves
- **Virtualization** with nested hypervisor support

### Key Features

- **64-bit architecture** with 32/64-bit operation modes
- **32 general-purpose registers** (64-bit each)
- **Load/store architecture** with orthogonal instruction set
- **Modern mnemonics** inspired by PowerPC, ARM64, and RISC-V
- **Wishbone B4 bus** interface for SoC integration
- **SMT support** for 1-16 hardware threads per core
- **SMP support** for multi-core systems with cache coherence

### Design Philosophy

1. **Simplicity**: Regular, orthogonal instruction set
2. **Performance**: Designed for efficient pipelining
3. **Modularity**: Extensions are optional and composable
4. **Compatibility**: Binary compatibility across implementations
5. **Openness**: Clear specification for education and research

## 1.2 Architecture Profile

### Base Integer Architecture

- **Register width**: 64 bits (configurable to 32-bit)
- **Address space**: 64-bit virtual, 64-bit physical
- **Endianness**: Switchable little/big endian
- **Alignment**: Natural alignment required (configurable)
- **Page size**: 4 KB, 2 MB, 1 GB (configurable)

### Privilege Levels

DLX supports multiple privilege models:

- **2-ring model**: User (Ring 3), Kernel (Ring 0)
- **4-ring model**: User (Ring 3), Services (Rings 1-2), Kernel (Ring 0)
- **Register banking**: Banked user/kernel registers

### Memory Model

- **Load/store**: All memory access through load/store instructions
- **Weakly ordered**: Memory ordering through explicit fences
- **Cache coherent**: MESI protocol for multi-core systems
- **Virtual memory**: Page-based with TLB and page tables

---

# Chapter 2: Base ISA

## 2.1 Instruction Set Overview

The DLX instruction set uses modern, consistent mnemonics:

### Naming Convention

```
<prefix><operation><suffix><modifier>

Prefixes:
  l   = Load
  s   = Store
  f   = Floating-point
  v   = Vector
  m   = Matrix
  c   = Capability (CHERI)

Suffixes:
  .b  = Byte (8-bit)
  .h  = Halfword (16-bit)
  .w  = Word (32-bit)
  .d  = Doubleword (64-bit)
  .q  = Quadword (128-bit)

Modifiers:
  z   = Zero-extend
  s   = Sign-extend
  u   = Update (post-increment)
  x   = Indexed addressing
```

## 2.2 Load and Store Instructions

### Load Instructions

```assembly
# Load with zero-extension
lbz     rd, offset(rs)          # Load byte (zero-extend)
lhz     rd, offset(rs)          # Load halfword (zero-extend)
lwz     rd, offset(rs)          # Load word (zero-extend)
ld      rd, offset(rs)          # Load doubleword
lq      rd, offset(rs)          # Load quadword (128-bit)

# Load with sign-extension
lbs     rd, offset(rs)          # Load byte (sign-extend)
lhs     rd, offset(rs)          # Load halfword (sign-extend)
lws     rd, offset(rs)          # Load word (sign-extend)

# Indexed loads (base + index)
lbzx    rd, rs1, rs2            # Load byte indexed
lhzx    rd, rs1, rs2            # Load halfword indexed
lwzx    rd, rs1, rs2            # Load word indexed
ldx     rd, rs1, rs2            # Load doubleword indexed

# Load with update (post-increment)
lbzu    rd, offset(rs)          # Load byte, rs += offset
lhzu    rd, offset(rs)          # Load halfword, rs += offset
lwzu    rd, offset(rs)          # Load word, rs += offset
ldu     rd, offset(rs)          # Load doubleword, rs += offset

# Special loads
ll.w    rd, offset(rs)          # Load-linked word (atomic)
ll.d    rd, offset(rs)          # Load-linked doubleword
lwbrx   rd, rs1, rs2            # Load word byte-reversed
ldbrx   rd, rs1, rs2            # Load doubleword byte-reversed
```

### Store Instructions

```assembly
# Basic stores
stb     rs, offset(rd)          # Store byte
sth     rs, offset(rd)          # Store halfword
stw     rs, offset(rd)          # Store word
std     rs, offset(rd)          # Store doubleword
stq     rs, offset(rd)          # Store quadword (128-bit)

# Indexed stores
stbx    rs, rd1, rd2            # Store byte indexed
sthx    rs, rd1, rd2            # Store halfword indexed
stwx    rs, rd1, rd2            # Store word indexed
stdx    rs, rd1, rd2            # Store doubleword indexed

# Store with update
stbu    rs, offset(rd)          # Store byte, rd += offset
sthu    rs, offset(rd)          # Store halfword, rd += offset
stwu    rs, offset(rd)          # Store word, rd += offset
stdu    rs, offset(rd)          # Store doubleword, rd += offset

# Special stores
sc.w    rs, offset(rd)          # Store-conditional word (atomic)
sc.d    rs, offset(rd)          # Store-conditional doubleword
stwbrx  rs, rd1, rd2            # Store word byte-reversed
stdbrx  rs, rd1, rd2            # Store doubleword byte-reversed
```

## 2.3 Integer Arithmetic

### Basic Arithmetic

```assembly
# Addition
add     rd, rs1, rs2            # Add
addi    rd, rs, imm             # Add immediate
addc    rd, rs1, rs2            # Add with carry
addco   rd, rs1, rs2            # Add with carry, set overflow
adde    rd, rs1, rs2            # Add extended (with carry)

# Subtraction
sub     rd, rs1, rs2            # Subtract
subi    rd, rs, imm             # Subtract immediate
subc    rd, rs1, rs2            # Subtract with carry
subco   rd, rs1, rs2            # Subtract with carry, set overflow
sube    rd, rs1, rs2            # Subtract extended

# Negation
neg     rd, rs                  # Negate
nego    rd, rs                  # Negate with overflow
```

### Multiplication and Division

```assembly
# Multiplication
mull.w  rd, rs1, rs2            # Multiply low word
mulh.w  rd, rs1, rs2            # Multiply high word (signed)
mulhu.w rd, rs1, rs2            # Multiply high word (unsigned)
mulhsu.w rd, rs1, rs2           # Multiply high (signed × unsigned)

mull.d  rd, rs1, rs2            # Multiply low doubleword
mulh.d  rd, rs1, rs2            # Multiply high doubleword

mulli   rd, rs, imm             # Multiply immediate

# Division
div.w   rd, rs1, rs2            # Divide word (signed)
divu.w  rd, rs1, rs2            # Divide word (unsigned)
div.d   rd, rs1, rs2            # Divide doubleword (signed)
divu.d  rd, rs1, rs2            # Divide doubleword (unsigned)

rem.w   rd, rs1, rs2            # Remainder word (signed)
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

# Shifts
sll.w   rd, rs1, rs2            # Shift left logical word
slli.w  rd, rs, shamt           # Shift left logical immediate
srl.w   rd, rs1, rs2            # Shift right logical word
srli.w  rd, rs, shamt           # Shift right logical immediate
sra.w   rd, rs1, rs2            # Shift right arithmetic word
srai.w  rd, rs, shamt           # Shift right arithmetic immediate

sll.d   rd, rs1, rs2            # Shift left logical doubleword
srl.d   rd, rs1, rs2            # Shift right logical doubleword
sra.d   rd, rs1, rs2            # Shift right arithmetic doubleword
```

## 2.5 Comparison and Conditional

```assembly
# Set on comparison
slt     rd, rs1, rs2            # Set if less than (signed)
sltu    rd, rs1, rs2            # Set if less than (unsigned)
slti    rd, rs, imm             # Set if less than immediate
sltiu   rd, rs, imm             # Set if less than immediate (unsigned)

sgt     rd, rs1, rs2            # Set if greater than (signed)
sgtu    rd, rs1, rs2            # Set if greater than (unsigned)
sle     rd, rs1, rs2            # Set if less or equal (signed)
sleu    rd, rs1, rs2            # Set if less or equal (unsigned)

seq     rd, rs1, rs2            # Set if equal
sne     rd, rs1, rs2            # Set if not equal
```

## 2.6 Branch Instructions

```assembly
# Conditional branches (compare with zero)
beqz    rs, offset              # Branch if equal to zero
bnez    rs, offset              # Branch if not equal to zero
bltz    rs, offset              # Branch if less than zero
bgez    rs, offset              # Branch if greater or equal to zero
bgtz    rs, offset              # Branch if greater than zero
blez    rs, offset              # Branch if less or equal to zero

# Conditional branches (compare two registers)
beq     rs1, rs2, offset        # Branch if equal
bne     rs1, rs2, offset        # Branch if not equal
blt     rs1, rs2, offset        # Branch if less than (signed)
bge     rs1, rs2, offset        # Branch if greater or equal (signed)
bltu    rs1, rs2, offset        # Branch if less than (unsigned)
bgeu    rs1, rs2, offset        # Branch if greater or equal (unsigned)

# Unconditional jump
j       offset                  # Jump
jal     rd, offset              # Jump and link (call)
jr      rs                      # Jump register
jalr    rd, rs, offset          # Jump and link register
```

## 2.7 Floating-Point Instructions

```assembly
# Single precision (.s)
fadd.s  fd, fs1, fs2            # FP add single
fsub.s  fd, fs1, fs2            # FP subtract single
fmul.s  fd, fs1, fs2            # FP multiply single
fdiv.s  fd, fs1, fs2            # FP divide single
fsqrt.s fd, fs                  # FP square root single

fmadd.s  fd, fs1, fs2, fs3      # Fused multiply-add: fd = fs1*fs2 + fs3
fmsub.s  fd, fs1, fs2, fs3      # Fused multiply-sub: fd = fs1*fs2 - fs3
fnmadd.s fd, fs1, fs2, fs3      # Neg fused multiply-add
fnmsub.s fd, fs1, fs2, fs3      # Neg fused multiply-sub

# Double precision (.d)
fadd.d  fd, fs1, fs2            # FP add double
fsub.d  fd, fs1, fs2            # FP subtract double
fmul.d  fd, fs1, fs2            # FP multiply double
fdiv.d  fd, fs1, fs2            # FP divide double
fsqrt.d fd, fs                  # FP square root double

# Quad precision (.q)
fadd.q  fd, fs1, fs2            # FP add quad (128-bit)
fsub.q  fd, fs1, fs2            # FP subtract quad
fmul.q  fd, fs1, fs2            # FP multiply quad
fdiv.q  fd, fs1, fs2            # FP divide quad

# FP comparisons
feq.s   rd, fs1, fs2            # FP equal single
flt.s   rd, fs1, fs2            # FP less than single
fle.s   rd, fs1, fs2            # FP less or equal single
feq.d   rd, fs1, fs2            # FP equal double
flt.d   rd, fs1, fs2            # FP less than double
fle.d   rd, fs1, fs2            # FP less or equal double

# FP conversions
fcvt.s.w  fd, rs                # Convert word to single
fcvt.s.d  fd, fs                # Convert double to single
fcvt.d.s  fd, fs                # Convert single to double
fcvt.d.w  fd, rs                # Convert word to double
fcvt.w.s  rd, fs                # Convert single to word
fcvt.w.d  rd, fs                # Convert double to word

# FP load/store
fld.s   fd, offset(rs)          # FP load single
fld.d   fd, offset(rs)          # FP load double
fst.s   fs, offset(rd)          # FP store single
fst.d   fs, offset(rd)          # FP store double

# FP move
fmov.s  fd, fs                  # FP move single
fmov.d  fd, fs                  # FP move double
fmov    rd, fs                  # Move FP to integer
fmov    fd, rs                  # Move integer to FP
```

## 2.8 System Instructions

```assembly
# CSR (Control and Status Register) access
csrrw   rd, csr, rs             # Read/write CSR
csrrs   rd, csr, rs             # Read and set bits
csrrc   rd, csr, rs             # Read and clear bits
csrrwi  rd, csr, imm            # Read/write CSR immediate
csrrsi  rd, csr, imm            # Read and set bits immediate
csrrci  rd, csr, imm            # Read and clear bits immediate

# Synchronization
fence                           # Memory fence
fence.i                         # Instruction fence
sync                            # Full synchronization barrier

# System calls and exceptions
ecall                           # Environment call (syscall)
ebreak                          # Environment break (breakpoint)
eret                            # Return from exception
sret                            # Return from supervisor mode
mret                            # Return from machine mode

# Wait and hints
wfi                             # Wait for interrupt
nop                             # No operation
hint    imm                     # Hint to implementation
```

## 2.9 Atomic Instructions

```assembly
# Atomic memory operations (AMO)
amoadd.w    rd, rs2, (rs1)      # Atomic add word
amoxor.w    rd, rs2, (rs1)      # Atomic XOR word
amoor.w     rd, rs2, (rs1)      # Atomic OR word
amoand.w    rd, rs2, (rs1)      # Atomic AND word
amomin.w    rd, rs2, (rs1)      # Atomic minimum word
amomax.w    rd, rs2, (rs1)      # Atomic maximum word
amominu.w   rd, rs2, (rs1)      # Atomic minimum word (unsigned)
amomaxu.w   rd, rs2, (rs1)      # Atomic maximum word (unsigned)
amoswap.w   rd, rs2, (rs1)      # Atomic swap word

# Doubleword variants
amoadd.d    rd, rs2, (rs1)      # Atomic add doubleword
amoxor.d    rd, rs2, (rs1)      # Atomic XOR doubleword
# ... (similar for all operations)

# Load-linked / Store-conditional
ll.w        rd, (rs)            # Load-linked word
sc.w        rd, rs2, (rs1)      # Store-conditional word
ll.d        rd, (rs)            # Load-linked doubleword
sc.d        rd, rs2, (rs1)      # Store-conditional doubleword

# Compare and swap
cas.w       rd, rs2, rs3, (rs1) # Compare and swap word
cas.d       rd, rs2, rs3, (rs1) # Compare and swap doubleword
```

## 2.10 Cache Control

```assembly
# Data cache operations
dcbf        offset(rs)          # Data cache block flush
dcbi        offset(rs)          # Data cache block invalidate
dcbst       offset(rs)          # Data cache block store
dcbz        offset(rs)          # Data cache block zero
dcba        offset(rs)          # Data cache block allocate

# Instruction cache operations
icbi        offset(rs)          # Instruction cache block invalidate
icbt        offset(rs)          # Instruction cache block touch

# Prefetch
pref.r      offset(rs)          # Prefetch for read
pref.w      offset(rs)          # Prefetch for write
pref.i      offset(rs)          # Prefetch for instruction
```

---

# Chapter 3: Registers and State

## 3.1 General-Purpose Registers

DLX provides 32 general-purpose registers (r0-r31), each 64 bits wide in RV64 mode.

### Register Convention

| Register | ABI Name | Description | Saver |
|----------|----------|-------------|-------|
| r0 | zero | Hardwired zero | - |
| r1 | ra | Return address | Caller |
| r2 | sp | Stack pointer | Callee |
| r3 | gp | Global pointer | - |
| r4 | tp | Thread pointer | - |
| r5-r7 | t0-t2 | Temporaries | Caller |
| r8-r9 | s0-s1 | Saved registers | Callee |
| r10-r17 | a0-a7 | Function arguments / return values | Caller |
| r18-r27 | s2-s11 | Saved registers | Callee |
| r28-r31 | t3-t6 | Temporaries | Caller |

### Register r0 (zero)

Register r0 is hardwired to the constant 0. Writes to r0 are discarded, and reads always return 0.

## 3.2 Floating-Point Registers

32 floating-point registers (f0-f31), each 64 bits wide, supporting:
- Single precision (32-bit)
- Double precision (64-bit)
- Quad precision (128-bit, using register pairs)

### FP Register Convention

| Register | ABI Name | Description | Saver |
|----------|----------|-------------|-------|
| f0-f7 | ft0-ft7 | FP temporaries | Caller |
| f8-f9 | fs0-fs1 | FP saved registers | Callee |
| f10-f17 | fa0-fa7 | FP arguments / return values | Caller |
| f18-f27 | fs2-fs11 | FP saved registers | Callee |
| f28-f31 | ft8-ft11 | FP temporaries | Caller |

## 3.3 Vector Registers (V Extension)

32 vector registers (v0-v31), with configurable width (VLEN):
- Standard VLEN: 256 bits
- Extended VLEN: 512, 1024, 2048 bits

Vector registers support element widths of 8, 16, 32, 64, and 128 bits.

## 3.4 Matrix Registers (M Extension)

8 matrix registers (m0-m7), each capable of storing a tile:
- Configurable tile size: 4×4 to 16×16 elements
- Element widths: 8, 16, 32, 64 bits

## 3.5 Control and Status Registers (CSRs)

### Machine-Level CSRs

```
0x300  mstatus    Machine status register
0x301  misa       ISA and extensions
0x304  mie        Machine interrupt enable
0x305  mtvec      Machine trap vector base address
0x340  mscratch   Scratch register for machine trap handlers
0x341  mepc       Machine exception program counter
0x342  mcause     Machine trap cause
0x343  mtval      Machine bad address or instruction
0x344  mip        Machine interrupt pending
```

### Supervisor-Level CSRs

```
0x100  sstatus    Supervisor status register
0x104  sie        Supervisor interrupt enable
0x105  stvec      Supervisor trap vector base address
0x140  sscratch   Scratch register for supervisor trap handlers
0x141  sepc       Supervisor exception program counter
0x142  scause     Supervisor trap cause
0x143  stval      Supervisor bad address or instruction
0x144  sip        Supervisor interrupt pending
0x180  satp       Supervisor address translation and protection
```

### Hypervisor CSRs (H Extension)

```
0x600  hstatus    Hypervisor status register
0x602  hedeleg    Hypervisor exception delegation register
0x603  hideleg    Hypervisor interrupt delegation register
0x680  hgatp      Hypervisor guest address translation and protection
```

### Performance Monitoring CSRs

```
0x200  tsc        Time stamp counter
0x201  tsc_freq   TSC frequency in Hz
0x210  pmu_ctrl   PMU control register
0x220  pmc0       Performance counter 0
0x221  pmc1       Performance counter 1
...
0x227  pmc7       Performance counter 7
0x230  pmcsel0    Performance counter 0 event select
...
0x237  pmcsel7    Performance counter 7 event select
```

## 3.6 Program Counter (PC)

The Program Counter is a 64-bit register that holds the address of the current instruction.

---

# Chapter 4: Instruction Formats

## 4.1 Instruction Encoding

DLX uses fixed 32-bit instruction encoding with several formats:

### R-Type (Register)

```
 31    26 25   21 20   16 15   11 10    6  5      0
+--------+-------+-------+-------+-------+--------+
| opcode |  rs1  |  rs2  |  rd   | func  | func2  |
+--------+-------+-------+-------+-------+--------+
    6       5       5       5       5       6
```

Used for: Arithmetic, logical, shifts with register operands

### I-Type (Immediate)

```
 31    26 25   21 20   16 15                     0
+--------+-------+-------+-----------------------+
| opcode |  rs   |  rd   |      immediate        |
+--------+-------+-------+-----------------------+
    6       5       5            16
```

Used for: Immediate arithmetic, loads, branches

### J-Type (Jump)

```
 31    26 25                                     0
+--------+-----------------------------------------+
| opcode |           target address               |
+--------+-----------------------------------------+
    6                     26
```

Used for: Jumps and calls

### F-Type (Floating-Point)

```
 31    26 25   21 20   16 15   11 10    6  5    0
+--------+-------+-------+-------+-------+------+
| opcode |  fs1  |  fs2  |  fd   | func  | fmt  |
+--------+-------+-------+-------+-------+------+
    6       5       5       5       5      6
```

Used for: Floating-point operations

### V-Type (Vector)

```
 31    26 25   21 20   16 15   11 10    6  5    0
+--------+-------+-------+-------+-------+------+
| opcode |  vs1  |  vs2  |  vd   | func  | vm   |
+--------+-------+-------+-------+-------+------+
    6       5       5       5       5      6
```

Used for: Vector operations

## 4.2 Opcode Map

| Opcode | Format | Instruction Class |
|--------|--------|-------------------|
| 000000 | R | ALU (add, sub, and, or, xor, shifts) |
| 000001 | I | ALU immediate |
| 000010 | J | Jump (j, jal) |
| 000011 | I | Jump register (jr, jalr) |
| 000100 | I | Branch equal (beq, beqz) |
| 000101 | I | Branch not equal (bne, bnez) |
| 000110 | I | Branch less than (blt, bltz) |
| 000111 | I | Branch greater or equal (bge, bgez) |
| 001000 | I | Load byte (lb, lbu) |
| 001001 | I | Load halfword (lh, lhu) |
| 001010 | I | Load word (lw, lwu) |
| 001011 | I | Load doubleword (ld) |
| 001100 | I | Store byte (sb) |
| 001101 | I | Store halfword (sh) |
| 001110 | I | Store word (sw) |
| 001111 | I | Store doubleword (sd) |
| 010000 | F | FP add/sub |
| 010001 | F | FP mul/div |
| 010010 | F | FP compare |
| 010011 | F | FP convert |
| 010100 | V | Vector arithmetic |
| 010101 | V | Vector load/store |
| 011000 | I | Atomic operations |
| 011001 | I | Cache control |
| 011010 | I | CSR access |
| 011011 | I | System (ecall, ebreak, eret) |
| 100000 | - | CHERI capability operations |
| 100001 | - | Hypervisor (VMX) operations |
| 100010 | - | Secure enclave operations |
| 111111 | - | Extended/custom opcodes |

---

# Chapter 5: Memory Architecture

## 5.1 Address Space

DLX provides a 64-bit virtual address space:
- User space: 0x0000_0000_0000_0000 to 0x0000_7FFF_FFFF_FFFF (128 TB)
- Kernel space: 0xFFFF_8000_0000_0000 to 0xFFFF_FFFF_FFFF_FFFF (128 TB)

## 5.2 Endianness

DLX supports both little-endian and big-endian modes:
- Mode controlled by status register bit
- Switchable per exception level
- Default: Little-endian

## 5.3 Alignment

Natural alignment is required for multi-byte accesses:
- Halfword (2 bytes): Must be 2-byte aligned
- Word (4 bytes): Must be 4-byte aligned
- Doubleword (8 bytes): Must be 8-byte aligned
- Quadword (16 bytes): Must be 16-byte aligned

Unaligned accesses raise an alignment exception unless unaligned access is enabled in the status register.

## 5.4 Page-Based Virtual Memory

### Page Sizes

- 4 KB pages (standard)
- 2 MB pages (large pages)
- 1 GB pages (huge pages)

### Page Table Formats

**4-Level Page Table (RV64)**:
```
 63    48 47    39 38    30 29    21 20    12 11      0
+--------+--------+--------+--------+--------+---------+
| VPN[3] | VPN[2] | VPN[1] | VPN[0] | Offset | PTE     |
+--------+--------+--------+--------+--------+---------+
   9        9        9        9        12
```

### Page Table Entry (PTE)

```
 63    54 53    28 27    19 18    10 9     8  7  6  5  4  3  2  1  0
+--------+--------+--------+--------+-------+--+--+--+--+--+--+--+--+
|Reserved|  PPN   |Reserved|  PPN   | Flags |D |A |G |U |X |W |R |V |
+--------+--------+--------+--------+-------+--+--+--+--+--+--+--+--+

Flags:
  V = Valid
  R = Read permission
  W = Write permission
  X = Execute permission
  U = User accessible
  G = Global mapping
  A = Accessed
  D = Dirty
```

## 5.5 TLB (Translation Lookaside Buffer)

- ITLB: 64 entries (instruction)
- DTLB: 64 entries (data)
- Fully associative or set-associative
- Hardware or software page table walk

### TLB Management

```assembly
tlbi.all                        # Invalidate all TLB entries
tlbi.asid   asid                # Invalidate by ASID
tlbi.va     vaddr               # Invalidate by virtual address
tlbi.va.asid vaddr, asid        # Invalidate by VA and ASID
```

## 5.6 Memory Ordering

DLX uses a weak memory model with explicit ordering:

```assembly
fence       # Full memory fence
fence.r     # Load fence
fence.w     # Store fence
fence.rw    # Load/store fence
fence.i     # Instruction fence
```

### Memory Order Types

1. **Relaxed**: No ordering guarantees
2. **Acquire**: No earlier loads/stores can move after
3. **Release**: No later loads/stores can move before
4. **AcqRel**: Both acquire and release
5. **SeqCst**: Sequential consistency

---

# Chapter 6: Binary Formats

## 6.1 ELF (Executable and Linkable Format)

### ELF Header

```c
#define EM_DLX          0x9999      /* DLX machine type */
#define ELFCLASS64      2           /* 64-bit objects */
#define ELFDATA2LSB     1           /* Little-endian */

/* ELF flags for DLX ABI variants */
#define EF_DLX_ABI_2RING    0x00000001  /* 2-ring protection */
#define EF_DLX_ABI_4RING    0x00000002  /* 4-ring protection */
#define EF_DLX_ABI_BANK     0x00000004  /* Register banking */
```

### Relocation Types

```c
#define R_DLX_NONE          0       /* No relocation */
#define R_DLX_32            1       /* Direct 32-bit */
#define R_DLX_64            2       /* Direct 64-bit */
#define R_DLX_PC32          3       /* PC-relative 32-bit */
#define R_DLX_PC64          4       /* PC-relative 64-bit */
#define R_DLX_GOT32         5       /* 32-bit GOT entry */
#define R_DLX_PLT32         6       /* 32-bit PLT entry */
#define R_DLX_COPY          7       /* Copy symbol at runtime */
#define R_DLX_GLOB_DAT      8       /* Global data pointer */
#define R_DLX_JMP_SLOT      9       /* PLT jump slot */
#define R_DLX_RELATIVE      10      /* Adjust by program base */
#define R_DLX_TLS_DTPMOD64  11      /* TLS module ID */
#define R_DLX_TLS_DTPOFF64  12      /* TLS offset in module */
#define R_DLX_TLS_TPOFF64   13      /* TLS offset from TP */
```

## 6.2 PE/COFF (Portable Executable)

### PE Header

```c
#define IMAGE_MACHINE_DLX   0x9999  /* DLX machine type */

typedef struct {
    uint16_t  Machine;              /* IMAGE_MACHINE_DLX */
    uint16_t  NumberOfSections;
    uint32_t  TimeDateStamp;
    uint32_t  PointerToSymbolTable;
    uint32_t  NumberOfSymbols;
    uint16_t  SizeOfOptionalHeader;
    uint16_t  Characteristics;
} IMAGE_FILE_HEADER;
```

## 6.3 Mach-O (Mach Object File Format)

### Mach-O Header

```c
#define CPU_TYPE_DLX        0x9999  /* DLX CPU type */
#define CPU_SUBTYPE_DLX64   1       /* DLX 64-bit */

struct mach_header_64 {
    uint32_t    magic;              /* MH_MAGIC_64 */
    cpu_type_t  cputype;            /* CPU_TYPE_DLX */
    cpu_subtype_t cpusubtype;       /* CPU_SUBTYPE_DLX64 */
    uint32_t    filetype;
    uint32_t    ncmds;
    uint32_t    sizeofcmds;
    uint32_t    flags;
    uint32_t    reserved;
};
```

---

# Chapter 7: ABI and Calling Conventions

## 7.1 Register Usage Convention

### Integer Registers

| Register | Name | Usage | Preserved |
|----------|------|-------|-----------|
| r0 | zero | Hardwired zero | N/A |
| r1 | ra | Return address | No |
| r2 | sp | Stack pointer | Yes |
| r3 | gp | Global pointer | N/A |
| r4 | tp | Thread pointer | N/A |
| r5-r7 | t0-t2 | Temporaries | No |
| r8-r9 | s0-s1 | Saved | Yes |
| r10-r11 | a0-a1 | Arguments / return values | No |
| r12-r17 | a2-a7 | Arguments | No |
| r18-r27 | s2-s11 | Saved | Yes |
| r28-r31 | t3-t6 | Temporaries | No |

### Floating-Point Registers

| Register | Name | Usage | Preserved |
|----------|------|-------|-----------|
| f0-f7 | ft0-ft7 | FP temporaries | No |
| f8-f9 | fs0-fs1 | FP saved | Yes |
| f10-f11 | fa0-fa1 | FP arguments / return | No |
| f12-f17 | fa2-fa7 | FP arguments | No |
| f18-f27 | fs2-fs11 | FP saved | Yes |
| f28-f31 | ft8-ft11 | FP temporaries | No |

## 7.2 Function Calling Sequence

### Caller Responsibilities

1. Save caller-saved registers (if needed)
2. Place arguments in a0-a7 (integer) and fa0-fa7 (FP)
3. Additional arguments go on stack
4. Call function with `jal ra, function`
5. Retrieve return values from a0-a1 (integer) or fa0-fa1 (FP)
6. Restore caller-saved registers

### Callee Responsibilities

1. Save callee-saved registers (s0-s11, fs0-fs11)
2. Allocate stack frame
3. Execute function body
4. Place return values in a0-a1 or fa0-fa1
5. Deallocate stack frame
6. Restore callee-saved registers
7. Return with `jr ra` or `ret`

## 7.3 Stack Frame Layout

```
High addresses
    +------------------+
    | Arguments 8+     |  (if needed)
    +------------------+
    | Return address   |  saved ra
    +------------------+
    | Previous FP      |  saved s0/fp
    +------------------+  <- FP (s0)
    | Saved registers  |  s1-s11, fs0-fs11
    +------------------+
    | Local variables  |
    +------------------+
    | Spill area       |
    +------------------+
    | Outgoing args    |  args 8+ for called functions
    +------------------+  <- SP (r2)
Low addresses
```

### Prologue Example

```assembly
function:
    addi    sp, sp, -32         # Allocate stack frame
    std     ra, 24(sp)          # Save return address
    std     s0, 16(sp)          # Save frame pointer
    addi    s0, sp, 32          # Set new frame pointer
    std     s1, 8(sp)           # Save s1
    # Function body
```

### Epilogue Example

```assembly
    ld      s1, 8(sp)           # Restore s1
    ld      s0, 16(sp)          # Restore frame pointer
    ld      ra, 24(sp)          # Restore return address
    addi    sp, sp, 32          # Deallocate stack frame
    ret                         # Return (jr ra)
```

## 7.4 Variadic Functions

```c
typedef struct {
    uint64_t *arg_ptr;          /* Current argument pointer */
    uint64_t *stack_ptr;        /* Stack argument area */
    uint32_t  gpr_used;         /* GPRs used (0-8) */
    uint32_t  fpr_used;         /* FPRs used (0-8) */
} va_list[1];
```

## 7.5 Thread-Local Storage (TLS)

### TLS Models

**Local Exec (LE)**:
```assembly
mfsr    t0, $tp                 # Get thread pointer
ld      a0, tls_var@tpoff(t0)   # Load TLS variable
```

**Initial Exec (IE)**:
```assembly
ld      t0, tls_var@gottpoff(gp) # Get TLS offset from GOT
mfsr    t1, $tp                  # Get thread pointer
add     t0, t0, t1               # Calculate address
ld      a0, 0(t0)                # Load value
```

**General Dynamic (GD)**:
```assembly
ld      a0, tls_var@tlsgd(gp)   # Get TLS descriptor
jal     __tls_get_addr          # Get address
ld      a0, 0(a0)               # Load value
```

---

# Chapter 8: Extensions

## 8.1 Vector Extension (V)

### Vector Configuration

```assembly
vsetvl   rd, rs1, vtypei        # Set vector length
vsetvli  rd, rs, vtypei         # Set vector length immediate
```

### Vector Arithmetic

```assembly
vadd.vv     vd, vs1, vs2, vm    # Vector add (vector-vector)
vadd.vx     vd, vs1, rs, vm     # Vector add (vector-scalar)
vadd.vi     vd, vs1, imm, vm    # Vector add (vector-immediate)

vsub.vv     vd, vs1, vs2, vm    # Vector subtract
vmul.vv     vd, vs1, vs2, vm    # Vector multiply
vdiv.vv     vd, vs1, vs2, vm    # Vector divide

# Widening operations
vwadd.vv    vd, vs1, vs2, vm    # Widening add (2×SEW result)
vwmul.vv    vd, vs1, vs2, vm    # Widening multiply

# Reduction operations
vredsum.vs  vd, vs1, vs2, vm    # Vector reduce sum
vredmax.vs  vd, vs1, vs2, vm    # Vector reduce maximum
vredmin.vs  vd, vs1, vs2, vm    # Vector reduce minimum
```

### Vector Load/Store

```assembly
vle.v       vd, (rs), vm        # Vector load (unit stride)
vse.v       vs, (rd), vm        # Vector store (unit stride)
vlse.v      vd, (rs), rs2, vm   # Vector load (strided)
vsse.v      vs, (rd), rs2, vm   # Vector store (strided)
vlxe.v      vd, (rs), vs2, vm   # Vector load (indexed)
vsxe.v      vs, (rd), vs2, vm   # Vector store (indexed)
```

## 8.2 Bit Manipulation Extension (B)

```assembly
# Count operations
clz         rd, rs              # Count leading zeros
ctz         rd, rs              # Count trailing zeros
pcnt        rd, rs              # Population count (number of 1 bits)

# Rotate operations
rol         rd, rs1, rs2        # Rotate left
ror         rd, rs1, rs2        # Rotate right
rori        rd, rs, imm         # Rotate right immediate

# Byte operations
rev8        rd, rs              # Reverse bytes
orc.b       rd, rs              # OR combine bytes
rev.b       rd, rs              # Reverse bits in bytes

# Single-bit operations
bset        rd, rs1, rs2        # Set bit
bclr        rd, rs1, rs2        # Clear bit
binv        rd, rs1, rs2        # Invert bit
bext        rd, rs1, rs2        # Extract bit

# Logic with complement
andn        rd, rs1, rs2        # AND NOT
orn         rd, rs1, rs2        # OR NOT
xnor        rd, rs1, rs2        # XOR NOT

# Min/max
min         rd, rs1, rs2        # Minimum (signed)
minu        rd, rs1, rs2        # Minimum (unsigned)
max         rd, rs1, rs2        # Maximum (signed)
maxu        rd, rs1, rs2        # Maximum (unsigned)
```

## 8.3 CHERI Capability Extension (C)

### Capability Inspection

```assembly
cgetbase    rd, cs              # Get capability base address
cgetlen     rd, cs              # Get capability length
cgetperm    rd, cs              # Get capability permissions
cgettype    rd, cs              # Get capability type
cgettag     rd, cs              # Get capability tag (valid bit)
cgetsealed  rd, cs              # Get sealed status
```

### Capability Modification

```assembly
csetaddr    cd, cs, rs          # Set capability address
cincoffset  cd, cs, rs          # Increment capability offset
csetbounds  cd, cs, rs          # Set capability bounds
cseal       cd, cs, ct          # Seal capability with type
cunseal     cd, cs, ct          # Unseal capability
candperm    cd, cs, rs          # AND permissions
```

### Capability Load/Store

```assembly
clb         rd, offset(cs)      # Capability load byte
clh         rd, offset(cs)      # Capability load halfword
clw         rd, offset(cs)      # Capability load word
cld         rd, offset(cs)      # Capability load doubleword
csb         rs, offset(cd)      # Capability store byte
csh         rs, offset(cd)      # Capability store halfword
csw         rs, offset(cd)      # Capability store word
csd         rs, offset(cd)      # Capability store doubleword
```

## 8.4 Cryptography Extension

```assembly
# AES encryption/decryption
aesenc      rd, rs1, rs2        # AES single round encrypt
aesenclast  rd, rs1, rs2        # AES final round encrypt
aesdec      rd, rs1, rs2        # AES single round decrypt
aesdeclast  rd, rs1, rs2        # AES final round decrypt
aesimc      rd, rs              # AES inverse mix columns
aeskeygenassist rd, rs, imm     # AES key generation assist

# SHA-256
sha256sig0  rd, rs              # SHA-256 Sigma0
sha256sig1  rd, rs              # SHA-256 Sigma1
sha256sum0  rd, rs              # SHA-256 Sum0
sha256sum1  rd, rs              # SHA-256 Sum1

# SHA-512
sha512sig0  rd, rs              # SHA-512 Sigma0
sha512sig1  rd, rs              # SHA-512 Sigma1
sha512sum0  rd, rs              # SHA-512 Sum0
sha512sum1  rd, rs              # SHA-512 Sum1
```

---

# Chapter 9: Virtualization

## 9.1 Hypervisor Extension (H)

### VMX Operations

```assembly
vmxon       addr                # Enable VMX operation
vmxoff                          # Disable VMX operation
vmclear     vmcs_addr           # Clear VMCS
vmptrld     vmcs_addr           # Load VMCS pointer
vmptrst     mem_addr            # Store VMCS pointer
vmread      rd, field           # Read VMCS field
vmwrite     field, rs           # Write VMCS field
vmlaunch                        # Launch VM (initial entry)
vmresume                        # Resume VM (re-entry)
vmcall                          # Hypercall from guest
```

### EPT Operations

```assembly
invept      type, descriptor    # Invalidate EPT
invvpid     type, descriptor    # Invalidate VPID
```

## 9.2 Nested Virtualization

Supports up to 8 levels of nesting (L0 through L7).

### Shadow VMCS

For L1 hypervisor running L2 guest, L0 maintains:
- VMCS01: L0's view of L1
- VMCS12: L1's view of L2
- VMCS02: L0's shadow VMCS for L2

---

# Chapter 10: Security

## 10.1 Secure Enclave Extension

### Enclave Lifecycle

```assembly
ecreate     rd, code_ptr, code_size, data_ptr, data_size
eenter      enclave_id, entry_offset, params
eexit       result_ptr
edestroy    enclave_id
```

### Attestation and Sealing

```assembly
eattest     rd, enclave_id, user_data, user_data_len
eseal       rd_sealed, data_ptr, data_len, enclave_id
eunseal     rd_data, sealed_ptr, sealed_len, enclave_id
```

## 10.2 Memory Encryption

Secure enclaves use AES-256-GCM encryption with:
- Per-VM encryption keys
- Merkle tree integrity protection
- Anti-replay counters
- Remote attestation with RSA-4096/Ed25519

---

# Chapter 11: Performance Monitoring

## 11.1 Time Stamp Counter

```assembly
rdtsc       rd                  # Read TSC
rdtscp      rd, aux             # Read TSC with serialization
```

## 11.2 Performance Counters

```assembly
pmcread     rd, pmc_num         # Read PMC
pmcwrite    pmc_num, rs         # Write PMC
pmcreset    pmc_num             # Reset PMC
pmcfg       pmc_num, event, flags # Configure PMC
```

### Performance Events (Selected)

| Event | Code | Description |
|-------|------|-------------|
| CYCLES | 0x0000 | CPU cycles |
| INSTRUCTIONS | 0x0001 | Instructions retired |
| BRANCHES | 0x0002 | Branch instructions |
| BRANCH_MISS | 0x0003 | Branch mispredictions |
| L1D_MISS | 0x0203 | L1 data cache misses |
| L2_MISS | 0x0211 | L2 cache misses |
| TLB_MISS | 0x0303 | DTLB misses |

---

# Chapter 12: System Programming

## 12.1 Exception and Interrupt Handling

### Exception Types

| Code | Exception | Description |
|------|-----------|-------------|
| 0 | Instruction address misaligned | PC not 4-byte aligned |
| 1 | Instruction access fault | Fetch from invalid address |
| 2 | Illegal instruction | Invalid or privileged opcode |
| 3 | Breakpoint | EBREAK instruction |
| 4 | Load address misaligned | Unaligned load |
| 5 | Load access fault | Load from invalid address |
| 6 | Store address misaligned | Unaligned store |
| 7 | Store access fault | Store to invalid address |
| 8 | Environment call from U-mode | ECALL in user mode |
| 9 | Environment call from S-mode | ECALL in supervisor mode |
| 11 | Environment call from M-mode | ECALL in machine mode |
| 12 | Instruction page fault | Page not present (fetch) |
| 13 | Load page fault | Page not present (load) |
| 15 | Store page fault | Page not present (store) |

### Interrupt Types

| Code | Interrupt | Description |
|------|-----------|-------------|
| 0 | User software interrupt | |
| 1 | Supervisor software interrupt | |
| 3 | Machine software interrupt | |
| 4 | User timer interrupt | |
| 5 | Supervisor timer interrupt | |
| 7 | Machine timer interrupt | |
| 8 | User external interrupt | |
| 9 | Supervisor external interrupt | |
| 11 | Machine external interrupt | |

### Exception Handling Flow

1. **Exception occurs**
2. **Hardware updates CSRs**:
   - `mepc` ← PC of faulting instruction
   - `mcause` ← Exception code
   - `mtval` ← Faulting address or instruction
   - `mstatus.MPIE` ← `mstatus.MIE`
   - `mstatus.MIE` ← 0 (disable interrupts)
3. **Jump to trap handler**: PC ← `mtvec`
4. **Software handles exception**
5. **Return**: `mret` instruction

## 12.2 Privilege Modes

| Level | Encoding | Name | Abbreviation |
|-------|----------|------|--------------|
| 0 | 00 | User/Application | U |
| 1 | 01 | Supervisor | S |
| 2 | 10 | Hypervisor | H |
| 3 | 11 | Machine | M |

---

# Appendix A: Instruction Quick Reference

## A.1 Load/Store

| Mnemonic | Description |
|----------|-------------|
| lbz | Load byte zero-extended |
| lbs | Load byte sign-extended |
| lhz | Load halfword zero-extended |
| lhs | Load halfword sign-extended |
| lwz | Load word zero-extended |
| lws | Load word sign-extended |
| ld | Load doubleword |
| lq | Load quadword |
| stb | Store byte |
| sth | Store halfword |
| stw | Store word |
| std | Store doubleword |
| stq | Store quadword |

## A.2 Integer Arithmetic

| Mnemonic | Description |
|----------|-------------|
| add | Add |
| sub | Subtract |
| addi | Add immediate |
| mull.w | Multiply low word |
| mulh.w | Multiply high word |
| div.w | Divide word |
| rem.w | Remainder word |

## A.3 Logical

| Mnemonic | Description |
|----------|-------------|
| and | Bitwise AND |
| or | Bitwise OR |
| xor | Bitwise XOR |
| andi | AND immediate |
| ori | OR immediate |
| xori | XOR immediate |
| sll.w | Shift left logical word |
| srl.w | Shift right logical word |
| sra.w | Shift right arithmetic word |

## A.4 Branch and Jump

| Mnemonic | Description |
|----------|-------------|
| beq | Branch if equal |
| bne | Branch if not equal |
| blt | Branch if less than |
| bge | Branch if greater or equal |
| beqz | Branch if equal to zero |
| bnez | Branch if not equal to zero |
| j | Jump |
| jal | Jump and link |
| jr | Jump register |
| jalr | Jump and link register |

## A.5 Floating-Point

| Mnemonic | Description |
|----------|-------------|
| fadd.s | FP add single |
| fsub.s | FP subtract single |
| fmul.s | FP multiply single |
| fdiv.s | FP divide single |
| fsqrt.s | FP square root single |
| fmadd.s | FP fused multiply-add single |
| fadd.d | FP add double |
| fsub.d | FP subtract double |
| fmul.d | FP multiply double |
| fdiv.d | FP divide double |

## A.6 System

| Mnemonic | Description |
|----------|-------------|
| ecall | Environment call (syscall) |
| ebreak | Environment break (breakpoint) |
| eret | Return from exception |
| mret | Return from machine mode |
| sret | Return from supervisor mode |
| wfi | Wait for interrupt |
| fence | Memory fence |
| fence.i | Instruction fence |

---

# Appendix B: CSR Register Map

## B.1 Machine-Level CSRs

| Address | Name | Description |
|---------|------|-------------|
| 0x300 | mstatus | Machine status |
| 0x301 | misa | ISA and extensions |
| 0x304 | mie | Machine interrupt enable |
| 0x305 | mtvec | Machine trap vector base |
| 0x340 | mscratch | Machine scratch register |
| 0x341 | mepc | Machine exception PC |
| 0x342 | mcause | Machine trap cause |
| 0x343 | mtval | Machine bad address/instruction |
| 0x344 | mip | Machine interrupt pending |

## B.2 Supervisor-Level CSRs

| Address | Name | Description |
|---------|------|-------------|
| 0x100 | sstatus | Supervisor status |
| 0x104 | sie | Supervisor interrupt enable |
| 0x105 | stvec | Supervisor trap vector base |
| 0x140 | sscratch | Supervisor scratch register |
| 0x141 | sepc | Supervisor exception PC |
| 0x142 | scause | Supervisor trap cause |
| 0x143 | stval | Supervisor bad address/instruction |
| 0x144 | sip | Supervisor interrupt pending |
| 0x180 | satp | Supervisor address translation |

## B.3 Performance Monitoring CSRs

| Address | Name | Description |
|---------|------|-------------|
| 0x200 | tsc | Time stamp counter |
| 0x210 | pmu_ctrl | PMU control |
| 0x220-0x227 | pmc0-7 | Performance counters |
| 0x230-0x237 | pmcsel0-7 | Event select registers |

---

# Appendix C: Pseudo-Instructions

| Pseudo-instruction | Actual Instruction(s) | Description |
|--------------------|----------------------|-------------|
| nop | addi r0, r0, 0 | No operation |
| li rd, imm | addi rd, r0, imm | Load immediate |
| mv rd, rs | addi rd, rs, 0 | Move register |
| not rd, rs | xori rd, rs, -1 | Bitwise NOT |
| neg rd, rs | sub rd, r0, rs | Negate |
| seqz rd, rs | sltiu rd, rs, 1 | Set if equal to zero |
| snez rd, rs | sltu rd, r0, rs | Set if not equal to zero |
| sltz rd, rs | slt rd, rs, r0 | Set if less than zero |
| sgtz rd, rs | slt rd, r0, rs | Set if greater than zero |
| call offset | jal ra, offset | Function call |
| ret | jr ra | Return from function |
| j offset | jal r0, offset | Jump |

---

# Appendix D: Code Examples

## D.1 Hello World

```assembly
.section .data
hello:  .string "Hello, World!\n"

.section .text
.globl _start
_start:
    # Load address of string
    ld      a0, hello
    # System call: write
    li      a7, 64          # syscall number for write
    li      a0, 1           # fd = stdout
    ld      a1, hello       # buffer
    li      a2, 14          # length
    ecall                   # make system call

    # Exit
    li      a7, 93          # syscall number for exit
    li      a0, 0           # exit code
    ecall
```

## D.2 Factorial Function

```assembly
# int factorial(int n)
# Arguments: a0 = n
# Returns: a0 = n!

factorial:
    # Prologue
    addi    sp, sp, -16
    std     ra, 8(sp)
    std     s0, 0(sp)

    # Base case: if n < 2, return 1
    li      t0, 2
    blt     a0, t0, .base_case

    # Recursive case
    mv      s0, a0          # Save n
    addi    a0, a0, -1      # n-1
    jal     factorial       # factorial(n-1)
    mul     a0, s0, a0      # n * factorial(n-1)
    j       .epilogue

.base_case:
    li      a0, 1

.epilogue:
    # Restore and return
    ld      s0, 0(sp)
    ld      ra, 8(sp)
    addi    sp, sp, 16
    ret
```

## D.3 Memcpy Function

```assembly
# void *memcpy(void *dest, const void *src, size_t n)
# Arguments: a0 = dest, a1 = src, a2 = n
# Returns: a0 = dest

memcpy:
    mv      t0, a0          # Save dest for return
    beqz    a2, .done       # If n == 0, done

.loop:
    lbz     t1, 0(a1)       # Load byte from src
    stb     t1, 0(a0)       # Store byte to dest
    addi    a0, a0, 1       # dest++
    addi    a1, a1, 1       # src++
    addi    a2, a2, -1      # n--
    bnez    a2, .loop       # Continue if n > 0

.done:
    mv      a0, t0          # Return original dest
    ret
```

## D.4 Vector Dot Product

```assembly
# float dot_product(float *a, float *b, size_t n)
# Arguments: a0 = a, a1 = b, a2 = n
# Returns: fa0 = sum

dot_product:
    # Set up vector length
    vsetvli t0, a2, e32, m1    # Element width=32, LMUL=1

    fmv.s.x fa0, zero          # sum = 0.0

.loop:
    vle.v   v0, (a0)           # Load vector from a
    vle.v   v1, (a1)           # Load vector from b
    vfmul.vv v2, v0, v1        # v2 = v0 * v1
    vfredsum.vs v3, v2, v3     # Reduce sum into v3

    # Update pointers
    slli    t1, t0, 2          # t1 = t0 * 4 (sizeof(float))
    add     a0, a0, t1         # a += t0
    add     a1, a1, t1         # b += t0
    sub     a2, a2, t0         # n -= t0
    bnez    a2, .loop

    # Extract final sum
    vfmv.f.s fa0, v3           # Move to FP register
    ret
```

---

# Index

[Alphabetical index would go here in final print version]

---

# Revision History

| Version | Date | Changes |
|---------|------|---------|
| 1.0 | 2024-01 | Initial DLX specification |
| 2.0 | 2024-10 | ISA redesign with modern mnemonics, added extensions |

---

**End of DLX Architecture Reference Manual**
