# DLX Complete Architecture Manual
## The Definitive Specification - Version 3.0

**A Comprehensive 64/128-bit RISC Architecture with Advanced Extensions**

---

**Document Version**: 3.0
**Release Date**: October 2024
**Status**: Complete Specification
**Covers**: DLX32, DLX64, DLX128, and all extensions

---

## About This Manual

This manual is the complete, authoritative specification for the DLX processor architecture family. It consolidates information from 22+ specialized architecture documents into a single reference.

### Organization

This manual consists of **5 volumes**:

**Volume I: Base Architecture** (Chapters 1-7)
- Core ISA, registers, memory model, binary formats, ABI

**Volume II: Standard Extensions** (Chapters 8-14)
- Vector, FP, BitManip, Atomic, Crypto, Compression

**Volume III: Advanced Features** (Chapters 15-21)
- Virtualization, Security, Neural Networks, Matrix, Quantum

**Volume IV: System Architecture** (Chapters 22-28)
- Memory Protection, RDMA, IOMMU, Endian Switching, SLS

**Volume V: Future Extensions** (Chapters 29-32)
- DLX128, AI/ML, Advanced ISA, Moxie Compatibility

### Related Documents

For detailed implementation specifics, refer to:
- `DLX_ISA_REDESIGN.md` - Complete instruction reference
- `DLX_TIMERS_PROFILING.md` - Performance monitoring details
- `DLX_NESTED_VIRTUALIZATION.md` - Hypervisor implementation
- `DLX_SECURE_ENCLAVE.md` - Security implementation
- And 18 other specialized documents

---

# VOLUME I: BASE ARCHITECTURE

# Chapter 1: Introduction

## 1.1 Overview

The DLX architecture is a modern RISC processor family supporting:

- **32-bit** (DLX32) - Embedded and legacy systems
- **64-bit** (DLX64) - General-purpose computing (primary focus)
- **128-bit** (DLX128) - Future exascale and AI/ML workloads

### Key Design Principles

1. **Simplicity**: Load/store architecture with orthogonal instructions
2. **Extensibility**: Modular extensions that compose cleanly
3. **Performance**: Designed for deep pipelines and out-of-order execution
4. **Security**: Hardware-enforced protection (CHERI, enclaves)
5. **Compatibility**: Binary compatibility across implementations

## 1.2 Feature Summary

### Base ISA (All Implementations)
- 32 general-purpose registers (64-bit in DLX64)
- Load/store architecture
- Integer arithmetic and logical operations
- Branch and jump instructions
- System calls and exceptions

### Standard Extensions (Common)
- **F**: Single-precision floating-point
- **D**: Double-precision floating-point
- **Q**: Quad-precision floating-point
- **V**: Vector operations (RV-V compatible)
- **B**: Bit manipulation
- **A**: Atomic instructions
- **C**: Compressed instructions (16-bit)

### Advanced Extensions (Optional)
- **H**: Hypervisor support with nested virtualization
- **K**: Cryptography (AES, SHA, RSA, ECC)
- **M**: Matrix operations for AI/ML
- **N**: Neural network acceleration
- **X**: CHERI capabilities
- **E**: Secure enclaves with memory encryption
- **T**: High-precision timers and PMU
- **P**: RDMA and high-performance networking
- **I**: IOMMU and SR-IOV
- **R**: Register banking (4 modes)
- **S**: Single-level storage
- **Q**: Quantum computing interface

### DLX-Specific Features
- **4-Ring Protection**: Fine-grained privilege (Ring 0-3)
- **Register Banking**: Hardware context switching
- **Endian Switching**: Per-level byte order control
- **Multiple Page Sizes**: 4KB to 16GB pages
- **Moxie Compatibility**: Run Moxie binaries natively

---

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

---

# Chapter 3: Registers and State

## 3.1 General-Purpose Registers

### DLX64 Register Set
32 general-purpose registers, each 64 bits wide:

| Register | ABI Name | Description | Preserved Across Calls |
|----------|----------|-------------|----------------------|
| r0 | zero | Hardwired zero | N/A (always 0) |
| r1 | ra | Return address | No (caller-saved) |
| r2 | sp | Stack pointer | Yes (callee-saved) |
| r3 | gp | Global pointer | N/A (constant) |
| r4 | tp | Thread pointer | N/A (per-thread) |
| r5-r7 | t0-t2 | Temporaries | No (caller-saved) |
| r8-r9 | s0-s1 / fp | Saved registers / Frame pointer | Yes (callee-saved) |
| r10-r11 | a0-a1 | Function arguments / return values | No (caller-saved) |
| r12-r17 | a2-a7 | Function arguments | No (caller-saved) |
| r18-r27 | s2-s11 | Saved registers | Yes (callee-saved) |
| r28-r31 | t3-t6 | Temporaries | No (caller-saved) |

### Register r0 (zero)
- Hardwired to constant 0
- Writes discarded
- Reads always return 0
- Useful for pseudo-instructions and common idioms

## 3.2 Floating-Point Registers

32 floating-point registers (f0-f31), each 64 bits:

| Register | ABI Name | Description | Preserved |
|----------|----------|-------------|-----------|
| f0-f7 | ft0-ft7 | FP temporaries | No |
| f8-f9 | fs0-fs1 | FP saved registers | Yes |
| f10-f11 | fa0-fa1 | FP arguments / return values | No |
| f12-f17 | fa2-fa7 | FP arguments | No |
| f18-f27 | fs2-fs11 | FP saved registers | Yes |
| f28-f31 | ft8-ft11 | FP temporaries | No |

Supports:
- Single precision (32-bit, IEEE 754)
- Double precision (64-bit, IEEE 754)
- Quad precision (128-bit, uses register pairs)
- Half precision (16-bit, IEEE 754-2008)
- BFloat16 (16-bit, Google Brain format)

## 3.3 Vector Registers (V Extension)

32 vector registers (v0-v31) with configurable length:

- **VLEN** (Vector Length): 128, 256, 512, 1024, 2048, 4096 bits
- **SEW** (Standard Element Width): 8, 16, 32, 64, 128 bits
- **LMUL** (Length Multiplier): 1/8, 1/4, 1/2, 1, 2, 4, 8

Register v0 is used as a mask register for predicated operations.

## 3.4 Matrix Registers (M Extension)

8 matrix registers (m0-m7) for AI/ML workloads:

- **Tile size**: Configurable from 4×4 to 16×16 elements
- **Element width**: 8, 16, 32, 64 bits (integer or FP)
- **Operations**: Matrix multiply, transpose, activation functions

## 3.5 Control and Status Registers (CSRs)

### Machine-Level CSRs (M-mode)
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

### Supervisor-Level CSRs (S-mode)
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

### Timer and Performance Monitoring CSRs (T Extension)
```
0x200  tsc        Time stamp counter (64-bit)
0x201  tsc_freq   TSC frequency in Hz
0x202  rtc        Real-time clock (nanoseconds)
0x210  pmu_ctrl   PMU control register
0x211  pmu_enable PMU counter enable mask
0x220-0x227  pmc0-7       Performance counters (64-bit each)
0x230-0x237  pmcsel0-7    Performance counter event select
```

### Register Banking CSRs (R Extension)
```
0x7C0  bank_config   Register bank configuration
0x7C1  bank_status   Register bank status
0x7C2  bank_switch   Manual bank switch control
```

---

# Chapter 4: Memory Architecture

## 4.1 Address Spaces

### DLX64 Address Space (64-bit)
- **User space**: 0x0000_0000_0000_0000 to 0x0000_7FFF_FFFF_FFFF (128 TB)
- **Kernel space**: 0xFFFF_8000_0000_0000 to 0xFFFF_FFFF_FFFF_FFFF (128 TB)

### DLX128 Address Space (128-bit)
- **User space**: 2^96 bytes (practical limit: 79 billion TB)
- **Kernel space**: Upper 2^96 bytes
- **Device space**: Dedicated 2^96 byte region
- **Persistent storage**: Memory-mapped non-volatile storage

## 4.2 Endianness

DLX supports both little-endian and big-endian modes with **per-privilege-level control**:

### Endian Control Register
```c
typedef struct {
    uint32_t kernel_endian  : 1;    /* 0=LE, 1=BE */
    uint32_t super_endian   : 1;
    uint32_t hyper_endian   : 1;
    uint32_t user_endian    : 1;
    uint32_t default_endian : 1;    /* Boot default */
    uint32_t switch_on_except : 1;  /* Auto-switch on exceptions */
} endian_ctrl_t;
```

### Endian-Switching Instructions
```assembly
setend.be                       # Switch to big-endian
setend.le                       # Switch to little-endian
rev8    rd, rs                  # Reverse bytes (manual conversion)
```

## 4.3 Page-Based Virtual Memory

### Standard Page Sizes
- 4 KB (standard)
- 2 MB (large pages)
- 1 GB (huge pages)
- 16 GB (super-huge pages, DLX-specific)

### VMS-Style Page Protection (R Extension)
Advanced protection beyond standard RWX:

```c
typedef struct {
    uint64_t readable    : 1;
    uint64_t writable    : 1;
    uint64_t executable  : 1;
    uint64_t user        : 1;
    uint64_t global      : 1;
    uint64_t accessed    : 1;
    uint64_t dirty       : 1;

    /* VMS-style extended protection */
    uint64_t owner_read  : 1;   /* Owner can read */
    uint64_t owner_write : 1;   /* Owner can write */
    uint64_t owner_exec  : 1;   /* Owner can execute */
    uint64_t group_read  : 1;   /* Group can read */
    uint64_t group_write : 1;   /* Group can write */
    uint64_t group_exec  : 1;   /* Group can execute */
    uint64_t world_read  : 1;   /* World can read */
    uint64_t world_write : 1;   /* World can write */
    uint64_t world_exec  : 1;   /* World can execute */

    uint64_t pfn         : 44;  /* Physical frame number */
    uint64_t reserved    : 4;
} vms_pte_t;
```

## 4.4 4-Ring Protection Model (R Extension)

DLX supports x86-style 4-ring protection:

```
Ring 0: Kernel (most privileged)
Ring 1: Device drivers, OS services
Ring 2: OS servers, system services
Ring 3: User applications (least privileged)
```

### Ring Transition
```assembly
call.ring   target, new_ring    # Call with ring transition
ret.ring                        # Return to previous ring
```

### Ring Control Register
```c
typedef struct {
    uint32_t current_ring  : 2;    /* Current ring (0-3) */
    uint32_t previous_ring : 2;    /* Ring before transition */
    uint32_t ring_stack    : 8;    /* Ring transition stack */
    uint32_t enable_4ring  : 1;    /* Enable 4-ring mode */
} ring_ctrl_t;
```

## 4.5 Register Banking (R Extension)

Hardware context switching with separate register sets:

### Banking Modes
1. **Privilege-Based**: Bank per privilege level (User, Kernel, etc.)
2. **Exception-Based**: Different banks for exceptions/interrupts
3. **Manual**: Software-controlled bank switching
4. **Hybrid**: Combination of privilege + exception

### Bank Configuration
```c
typedef struct {
    uint32_t enable      : 1;    /* Enable register banking */
    uint32_t num_banks   : 2;    /* 2-4 banks */
    uint32_t auto_switch : 1;    /* Auto-switch on exceptions */
    uint32_t current_bank: 2;    /* Current active bank (0-3) */
    uint32_t bank_mode   : 2;    /* Banking mode */
} bank_config_t;
```

### Bank Switching
```assembly
setbank     bank_num            # Switch to bank (manual mode)
```

Each bank contains:
- Complete GPR set (r0-r31)
- Complete FPR set (f0-f31)
- Status snapshot

---

# VOLUME II: STANDARD EXTENSIONS

# Chapter 8: Vector Extension (V)

## 8.1 Vector Configuration

```assembly
vsetvl   rd, rs1, vtypei        # Set vector length: rd ← VL
vsetvli  rd, rs, vtypei         # Set vector length immediate
```

## 8.2 Vector Arithmetic

```assembly
# Integer arithmetic
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
vredand.vs  vd, vs1, vs2, vm    # Vector reduce AND
vredor.vs   vd, vs1, vs2, vm    # Vector reduce OR
vredxor.vs  vd, vs1, vs2, vm    # Vector reduce XOR
```

## 8.3 Vector Load/Store

```assembly
vle.v       vd, (rs), vm        # Vector load (unit stride)
vse.v       vs, (rd), vm        # Vector store (unit stride)
vlse.v      vd, (rs), rs2, vm   # Vector load (strided)
vsse.v      vs, (rd), rs2, vm   # Vector store (strided)
vlxe.v      vd, (rs), vs2, vm   # Vector load (indexed)
vsxe.v      vs, (rd), vs2, vm   # Vector store (indexed)
```

---

# Chapter 9: Bit Manipulation Extension (B)

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
rev8        rd, rs              # Reverse bytes (64-bit → byte swap)
rev16       rd, rs              # Reverse halfwords
orc.b       rd, rs              # OR combine bytes
rev.b       rd, rs              # Reverse bits in each byte

# Single-bit operations
bset        rd, rs1, rs2        # Set bit: rd ← rs1 | (1 << rs2)
bclr        rd, rs1, rs2        # Clear bit: rd ← rs1 & ~(1 << rs2)
binv        rd, rs1, rs2        # Invert bit: rd ← rs1 ^ (1 << rs2)
bext        rd, rs1, rs2        # Extract bit: rd ← (rs1 >> rs2) & 1

# Logic with complement
andn        rd, rs1, rs2        # AND NOT: rd ← rs1 & ~rs2
orn         rd, rs1, rs2        # OR NOT: rd ← rs1 | ~rs2
xnor        rd, rs1, rs2        # XOR NOT: rd ← ~(rs1 ^ rs2)

# Min/max
min         rd, rs1, rs2        # Minimum (signed)
minu        rd, rs1, rs2        # Minimum (unsigned)
max         rd, rs1, rs2        # Maximum (signed)
maxu        rd, rs1, rs2        # Maximum (unsigned)

# Sign/zero extension
sext.b      rd, rs              # Sign-extend byte
sext.h      rd, rs              # Sign-extend halfword
zext.b      rd, rs              # Zero-extend byte
zext.h      rd, rs              # Zero-extend halfword
```

---

# Chapter 10: Cryptography Extension (K)

## 10.1 AES Instructions

```assembly
# AES Encryption/Decryption
aesenc      rd, rs1, rs2        # AES single round encrypt
aesenclast  rd, rs1, rs2        # AES final round encrypt
aesdec      rd, rs1, rs2        # AES single round decrypt
aesdeclast  rd, rs1, rs2        # AES final round decrypt
aesimc      rd, rs              # AES inverse mix columns
aeskeygenassist rd, rs, imm     # AES key generation assist

# AES-GCM (Galois/Counter Mode)
aesgcm.enc  vd, vs1, vs2, vs3   # AES-GCM encrypt block
aesgcm.dec  vd, vs1, vs2, vs3   # AES-GCM decrypt block
aesgcm.mac  vd, vs1, vs2        # AES-GCM compute MAC
```

## 10.2 SHA Instructions

```assembly
# SHA-256
sha256.sig0  rd, rs             # SHA-256 Sigma0
sha256.sig1  rd, rs             # SHA-256 Sigma1
sha256.sum0  rd, rs             # SHA-256 Sum0
sha256.sum1  rd, rs             # SHA-256 Sum1
sha256.round rd, rs1, rs2, rs3  # SHA-256 round operation

# SHA-512
sha512.sig0  rd, rs             # SHA-512 Sigma0
sha512.sig1  rd, rs             # SHA-512 Sigma1
sha512.sum0  rd, rs             # SHA-512 Sum0
sha512.sum1  rd, rs             # SHA-512 Sum1
sha512.round rd, rs1, rs2, rs3  # SHA-512 round operation

# SHA-3 (Keccak)
sha3.theta  vd, vs1, vs2        # SHA-3 theta step
sha3.rho    vd, vs              # SHA-3 rho step
sha3.chi    vd, vs              # SHA-3 chi step
sha3.iota   vd, vs, imm         # SHA-3 iota step
```

## 10.3 RSA and ECC

```assembly
# RSA Operations
rsa.modexp  rd, base, exp, mod  # Modular exponentiation
rsa.mulmod  rd, rs1, rs2, mod   # Modular multiplication

# Elliptic Curve Operations
ecc.pointadd  pd, ps1, ps2, curve # EC point addition
ecc.pointmul  pd, ps, scalar, curve # EC scalar multiplication
ecc.verify    rd, msg, sig, pubkey # ECDSA verify
```

---

# Chapter 11: CRC and Compression (K Extension)

## 11.1 CRC Instructions

```assembly
# CRC-32
crc32.b     rd, rs              # CRC-32 byte
crc32.h     rd, rs              # CRC-32 halfword
crc32.w     rd, rs              # CRC-32 word
crc32.d     rd, rs              # CRC-32 doubleword

# CRC-32C (Castagnoli)
crc32c.b    rd, rs              # CRC-32C byte
crc32c.w    rd, rs              # CRC-32C word
crc32c.d    rd, rs              # CRC-32C doubleword

# Custom CRC
crc.custom  rd, rs, poly, init  # CRC with custom polynomial
```

## 11.2 Compression Instructions

```assembly
# LZ77-style compression
lz77.compress   vd, vs, dict    # LZ77 compress vector
lz77.decompress vd, vs, dict    # LZ77 decompress vector

# Huffman encoding
huff.encode     vd, vs, table   # Huffman encode
huff.decode     vd, vs, table   # Huffman decode

# Dictionary compression
dict.compress   vd, vs, dict    # Dictionary-based compress
dict.decompress vd, vs, dict    # Dictionary-based decompress
```

---

# VOLUME III: ADVANCED FEATURES

# Chapter 15: CHERI Capabilities (X Extension)

## 15.1 Capability Inspection

```assembly
cgetbase    rd, cs              # Get capability base address
cgetlen     rd, cs              # Get capability length
cgetperm    rd, cs              # Get capability permissions
cgettype    rd, cs              # Get capability type
cgettag     rd, cs              # Get capability tag (valid bit)
cgetsealed  rd, cs              # Get sealed status
cgetoffset  rd, cs              # Get capability offset
```

## 15.2 Capability Modification

```assembly
csetaddr    cd, cs, rs          # Set capability address
cincoffset  cd, cs, rs          # Increment capability offset
csetbounds  cd, cs, rs          # Set capability bounds
candperm    cd, cs, rs          # AND permissions (restrict)
cseal       cd, cs, ct          # Seal capability with type
cunseal     cd, cs, ct          # Unseal capability
```

## 15.3 Capability Load/Store

```assembly
clb         rd, offset(cs)      # Capability load byte
clh         rd, offset(cs)      # Capability load halfword
clw         rd, offset(cs)      # Capability load word
cld         rd, offset(cs)      # Capability load doubleword
clc         cd, offset(cs)      # Capability load capability

csb         rs, offset(cd)      # Capability store byte
csh         rs, offset(cd)      # Capability store halfword
csw         rs, offset(cd)      # Capability store word
csd         rs, offset(cd)      # Capability store doubleword
csc         cs, offset(cd)      # Capability store capability
```

## 15.4 Capability Control Flow

```assembly
cjalr       cd, cs              # Capability jump and link register
ccall       cs, selector        # Capability call (domain crossing)
creturn                         # Capability return
```

---

# Chapter 16: Secure Enclaves (E Extension)

## 16.1 Enclave Lifecycle

```assembly
ecreate     rd, code_ptr, code_size, data_ptr, data_size
            # Create secure enclave: rd ← enclave_id

eenter      enclave_id, entry_offset, params
            # Enter enclave (world switch to secure)

eexit       result_ptr
            # Exit enclave (world switch to normal)

edestroy    enclave_id
            # Destroy enclave and free resources
```

## 16.2 Enclave Memory Management

```assembly
eadd        enclave_id, vaddr, page_type, prot
            # Add page to enclave

eremove     enclave_id, vaddr
            # Remove page from enclave

eextend     enclave_id, vaddr
            # Extend enclave measurement with page hash

eblock      enclave_id, vaddr
            # Block page (make inaccessible)

etrack      enclave_id
            # Track TLB flushes for enclave pages
```

## 16.3 Attestation and Sealing

```assembly
eattest     rd, enclave_id, user_data, user_data_len
            # Generate attestation report: rd ← report_ptr

eseal       rd_sealed, data_ptr, data_len, enclave_id
            # Seal data to enclave: rd_sealed ← sealed_blob_ptr

eunseal     rd_data, sealed_ptr, sealed_len, enclave_id
            # Unseal data (only works in same enclave)
```

---

# Chapter 17: Nested Virtualization (H Extension)

## 17.1 VMX Operations

```assembly
vmxon       addr                # Enable VMX operation
vmxoff                          # Disable VMX operation

vmclear     vmcs_addr           # Clear VMCS state
vmptrld     vmcs_addr           # Load VMCS pointer
vmptrst     mem_addr            # Store VMCS pointer

vmread      rd, field           # Read VMCS field
vmwrite     field, rs           # Write VMCS field

vmlaunch                        # Launch VM (initial entry)
vmresume                        # Resume VM (re-entry after VM exit)

vmcall                          # Hypercall from guest to hypervisor
```

## 17.2 EPT (Extended Page Tables)

```assembly
invept      type, descriptor    # Invalidate EPT entries
invvpid     type, descriptor    # Invalidate VPID entries
```

EPT provides two-dimensional paging:
- **First level**: Guest virtual → Guest physical (guest page tables)
- **Second level**: Guest physical → Host physical (EPT)

---

# Chapter 18: Neural Network Acceleration (N Extension)

## 18.1 Neural Network Forward Pass

```assembly
# Dense (Fully Connected) Layer
nn.dense.fwd    mr_out, mr_in, mr_weights, mr_bias
                # out ← in × weights + bias

# Activation Functions
nn.relu         mr_out, mr_in       # ReLU: out ← max(0, in)
nn.gelu         mr_out, mr_in       # GELU activation
nn.sigmoid      mr_out, mr_in       # Sigmoid: out ← 1/(1+e^(-in))
nn.tanh         mr_out, mr_in       # Tanh activation
nn.swish        mr_out, mr_in       # Swish: out ← in × sigmoid(in)
nn.softmax      mr_out, mr_in, axis # Softmax activation

# Normalization
nn.batchnorm.fwd mr_out, mr_in, mr_gamma, mr_beta, mr_mean, mr_var, epsilon
                # Batch normalization forward

nn.layernorm.fwd mr_out, mr_in, mr_gamma, mr_beta, epsilon
                # Layer normalization forward

# Convolution
nn.conv2d.fwd   mr_out, mr_in, mr_kernel, mr_bias, stride, padding
                # 2D convolution

# Pooling
nn.pool.max     mr_out, mr_in, kernel_size, stride, padding
nn.pool.avg     mr_out, mr_in, kernel_size, stride, padding
nn.pool.global.max  mr_out, mr_in    # Global max pooling
nn.pool.global.avg  mr_out, mr_in    # Global average pooling
```

## 18.2 Neural Network Backward Pass (Training)

```assembly
# Dense Layer Backward
nn.dense.bwd    mr_grad_in, mr_grad_weights, mr_grad_bias, \
                mr_grad_out, mr_input, mr_weights
                # Compute gradients for backpropagation

# Activation Gradients
nn.relu.grad    mr_grad_in, mr_grad_out, mr_input
nn.sigmoid.grad mr_grad_in, mr_grad_out, mr_output
nn.tanh.grad    mr_grad_in, mr_grad_out, mr_output
nn.gelu.grad    mr_grad_in, mr_grad_out, mr_input

# Convolution Backward
nn.conv2d.bwd.input  mr_grad_in, mr_grad_out, mr_kernel, stride, padding
nn.conv2d.bwd.kernel mr_grad_kernel, mr_grad_out, mr_input, stride, padding
nn.conv2d.bwd.bias   mr_grad_bias, mr_grad_out

# Normalization Backward
nn.batchnorm.bwd mr_grad_in, mr_grad_gamma, mr_grad_beta, \
                 mr_grad_out, mr_input, mr_gamma, mr_mean, mr_var, epsilon
```

## 18.3 Loss Functions and Optimizers

```assembly
# Loss Functions with Gradients
nn.loss.crossent     vr_loss, mr_logits, mr_labels
nn.loss.crossent.grad mr_grad, mr_logits, mr_labels

nn.loss.mse          vr_loss, mr_pred, mr_target
nn.loss.mse.grad     mr_grad, mr_pred, mr_target

# Optimizers
nn.opt.sgd           mr_params, mr_grads, learning_rate
nn.opt.sgd.momentum  mr_params, mr_grads, mr_velocity, lr, momentum
nn.opt.adam          mr_params, mr_grads, mr_m, mr_v, lr, beta1, beta2, epsilon, t
nn.opt.rmsprop       mr_params, mr_grads, mr_v, lr, alpha, epsilon
```

---

# Chapter 19: Matrix Operations (M Extension)

## 19.1 Matrix Configuration

```assembly
msetcfg     rows, cols, elem_width    # Configure matrix tile
```

## 19.2 Matrix Arithmetic

```assembly
# Matrix-Matrix Operations
mmadd       md, ms1, ms2        # Matrix add: md ← ms1 + ms2
mmsub       md, ms1, ms2        # Matrix subtract
mmmul       md, ms1, ms2        # Matrix multiply: md ← ms1 × ms2
mmtrans     md, ms              # Matrix transpose

# Matrix-Vector Operations
mmvadd      vd, ms, vs          # Matrix-vector add
mmvmul      vd, ms, vs          # Matrix-vector multiply: vd ← ms × vs

# Elementwise Operations
mmhadamard  md, ms1, ms2        # Hadamard product (element-wise multiply)
```

## 19.3 Matrix Load/Store

```assembly
mmld        md, (rs), stride    # Load matrix from memory
mmst        ms, (rd), stride    # Store matrix to memory
```

---

# Chapter 20: Quantum Computing Interface (Q Extension)

## 20.1 Quantum Gates

```assembly
# Single-Qubit Gates
qh      qubit_id                # Hadamard gate
qx      qubit_id                # Pauli-X (NOT) gate
qy      qubit_id                # Pauli-Y gate
qz      qubit_id                # Pauli-Z gate
qs      qubit_id                # S gate (phase)
qt      qubit_id                # T gate (π/8 phase)

# Rotation Gates
qrx     qubit_id, angle         # Rotate around X axis
qry     qubit_id, angle         # Rotate around Y axis
qrz     qubit_id, angle         # Rotate around Z axis

# Two-Qubit Gates
qcnot   control, target         # Controlled-NOT (CNOT)
qcz     control, target         # Controlled-Z
qswap   qubit1, qubit2          # SWAP gate

# Three-Qubit Gates
qtoffoli ctrl1, ctrl2, target   # Toffoli (CCNOT) gate
qfredkin ctrl, target1, target2 # Fredkin (CSWAP) gate
```

## 20.2 Quantum Measurement

```assembly
qmeasure    rd, qubit_id        # Measure qubit: rd ← {0, 1}
qmeasure.all vd, qubit_mask     # Measure multiple qubits
qreset      qubit_id            # Reset qubit to |0⟩
```

---

# VOLUME IV: SYSTEM ARCHITECTURE

# Chapter 22: RDMA and High-Performance Networking (P Extension)

## 22.1 RDMA Operations

```assembly
# RDMA Send/Receive
rdma.send       qp_id, addr, len, lkey
                # Post send work request

rdma.recv       qp_id, addr, len, lkey
                # Post receive work request

# RDMA Read/Write (one-sided)
rdma.read       local_addr, remote_addr, len, rkey
                # RDMA read from remote memory

rdma.write      remote_addr, local_addr, len, rkey
                # RDMA write to remote memory

rdma.write.imm  remote_addr, local_addr, len, rkey, imm_data
                # RDMA write with immediate data

# RDMA Atomic
rdma.cmp.swap   rd, remote_addr, compare, swap, rkey
                # RDMA atomic compare-and-swap

rdma.fetch.add  rd, remote_addr, add_value, rkey
                # RDMA atomic fetch-and-add
```

## 22.2 InfiniBand Verbs

```assembly
# Queue Pair Management
ib.create.qp    rd, qp_attr     # Create queue pair: rd ← qp_id
ib.modify.qp    qp_id, qp_attr  # Modify queue pair state
ib.destroy.qp   qp_id           # Destroy queue pair

# Completion Queue
ib.poll.cq      rd, cq_id, max_entries  # Poll completion queue
ib.create.cq    rd, cq_entries  # Create completion queue
```

---

# Chapter 23: IOMMU and SR-IOV (I Extension)

## 23.1 IOMMU Operations

```assembly
# IOMMU Configuration
iommu.map       iova, pa, size, prot    # Map IOVA → Physical Address
iommu.unmap     iova, size              # Unmap IOVA
iommu.flush     domain_id               # Flush IOTLB
```

## 23.2 SR-IOV (Single Root I/O Virtualization)

```assembly
# Virtual Function Management
sriov.enable    pf_id, num_vfs  # Enable SR-IOV with N virtual functions
sriov.disable   pf_id           # Disable SR-IOV
sriov.assign    vf_id, vm_id    # Assign VF to VM
```

---

# Chapter 24: Single-Level Storage (S Extension)

## 24.1 Persistent Memory Operations

```assembly
# Persistent memory stores
pmemst.b    rs, offset(rd)      # Persistent store byte
pmemst.w    rs, offset(rd)      # Persistent store word
pmemst.d    rs, offset(rd)      # Persistent store doubleword

# Persistent memory fence
pmemfence                       # Ensure persistence

# Persistent memory flush
pmemflush   addr, size          # Flush range to persistent media
```

## 24.2 Object Addressing

```assembly
# Direct object references (128-bit addresses)
ldobj       rd, oid             # Load object by OID
stobj       rs, oid             # Store object by OID
```

---

# VOLUME V: FUTURE EXTENSIONS

# Chapter 29: DLX128 Architecture

## 29.1 128-bit Instructions

```assembly
# Load/Store 128-bit
lq      rd, offset(rs)          # Load quadword (128-bit)
sq      rt, offset(rs)          # Store quadword

# Arithmetic 128-bit
addq    rd, rs, rt              # Add quadword
subq    rd, rs, rt              # Subtract quadword
mulq    rd, rs, rt              # Multiply quadword
divq    rd, rs, rt              # Divide quadword

# Logical 128-bit
andq    rd, rs, rt              # AND quadword
orq     rd, rs, rt              # OR quadword
xorq    rd, rs, rt              # XOR quadword

# Shift 128-bit
sllq    rd, rs, rt              # Shift left logical quadword
srlq    rd, rs, rt              # Shift right logical quadword
sraq    rd, rs, rt              # Shift right arithmetic quadword

# Compare/Branch 128-bit
cmpq    rd, rs, rt              # Compare quadword
beqq    rs, rt, offset          # Branch if equal quadword
bneq    rs, rt, offset          # Branch if not equal quadword
```

---

# Chapter 30: Moxie Compatibility Mode

DLX can execute Moxie binaries natively through hardware translation:

## 30.1 Moxie Mode Control

```assembly
moxie.enable                    # Enable Moxie compatibility mode
moxie.disable                   # Disable Moxie mode
```

## 30.2 Moxie Register Mapping

Moxie's 16 registers map to DLX r0-r15:
- Moxie $r0 → DLX r0
- Moxie $sp → DLX r1 (sp)
- Moxie $fp → DLX r8 (s0/fp)

## 30.3 Moxie64 Support

128-bit Moxie with 32 registers:
- Direct mapping to DLX128 registers
- 64-bit addressing
- Hardware instruction translation

---

# Appendices

# Appendix A: Complete Instruction Summary

[See DLX_ISA_REDESIGN.md for comprehensive instruction reference with 500+ instructions]

Key instruction classes:
- **Load/Store**: 40+ variants (indexed, update, byte-reversed, atomic)
- **Integer Arithmetic**: 30+ (add, sub, mul, div with variants)
- **Logical**: 20+ (and, or, xor, shifts, rotates)
- **FP**: 60+ (single/double/quad/half/BFloat16)
- **Vector**: 100+ (RV-V compatible)
- **Atomic**: 20+ (AMO, LL/SC, CAS)
- **Bit Manipulation**: 25+ (count, rotate, extract)
- **CHERI**: 30+ (capability operations)
- **Crypto**: 40+ (AES, SHA, RSA, ECC)
- **Neural Network**: 50+ (forward, backward, optimizers)
- **Matrix**: 15+ (GEMM, transpose)
- **Quantum**: 20+ (gates, measurement)
- **System**: 30+ (CSR, fence, exceptions)
- **Hypervisor**: 15+ (VMX operations)
- **Enclave**: 10+ (lifecycle, attestation)

# Appendix B: CSR Complete Map

[150+ CSRs across machine, supervisor, hypervisor, user levels]

See Chapter 3 and specialized documents for details.

# Appendix C: Performance Event Reference

[100+ hardware performance events]

See DLX_TIMERS_PROFILING.md for complete event list.

# Appendix D: Code Examples

## D.1 Hello World
```assembly
.section .data
hello:  .string "Hello, DLX!\n"

.section .text
.globl _start
_start:
    ld      a0, hello           # Buffer address
    li      a1, 12              # Length
    li      a7, 64              # Syscall: write
    li      a0, 1               # fd: stdout
    ecall                       # Make syscall

    li      a7, 93              # Syscall: exit
    li      a0, 0               # Exit code
    ecall
```

## D.2 Neural Network Inference
```assembly
.text
nn_forward:
    # Load input data
    vle.v       v0, (a0), vm    # Load input vector

    # Dense layer 1: out = input × W1 + b1
    nn.dense.fwd mr0, mr_in, mr_w1, mr_b1

    # ReLU activation
    nn.relu     mr0, mr0

    # Dense layer 2
    nn.dense.fwd mr1, mr0, mr_w2, mr_b2

    # Softmax output
    nn.softmax  mr1, mr1, axis=1

    ret
```

---

# Revision History

| Version | Date | Changes |
|---------|------|---------|
| 1.0 | 2024-01 | Initial DLX specification |
| 2.0 | 2024-10 | ISA redesign with modern mnemonics |
| 3.0 | 2024-10 | Complete manual consolidating all 22 architecture documents |

---

# Index

[Alphabetical index of instructions, registers, CSRs]

---

**END OF DLX COMPLETE ARCHITECTURE MANUAL - VERSION 3.0**

---

## Document References

This manual consolidates the following architecture documents:

1. DLX_ISA_REDESIGN.md - Instruction set with modern mnemonics
2. DLX_TIMERS_PROFILING.md - Performance monitoring
3. DLX_NESTED_VIRTUALIZATION.md - Hypervisor and EPT
4. DLX_SECURE_ENCLAVE.md - Secure enclaves
5. DLX_CHERI.md - CHERI capabilities
6. DLX_MOXIE_COMPATIBILITY.md - Moxie emulation
7. DLX_ABI.md - Calling conventions
8. DLX_BINARY_LOADERS.md - Binary formats
9. DLX128_FUTURE_ARCH.md - 128-bit and future extensions
10. DLX_MEMORY_PROTECTION.md - Register banking and 4-ring
11. DLX_RDMA.md - RDMA and InfiniBand
12. DLX_ENDIAN_SWITCHING.md - Endian control
13. DLX_CRYPTOGRAPHY.md - Cryptographic instructions
14. DLX_CRC_COMPRESSION.md - CRC and compression
15. DLX_VIRTUALIZATION.md - Virtualization features
16. DLX_ISA_EXTENSIONS.md - ISA extensions
17. DLX_EXTENSIONS.md - Extension framework
18. DLX_BITMANIP.md - Bit manipulation
19. DLX_AI.md - AI/ML extensions
20. DLX_SINGLE_LEVEL_STORAGE.md - Persistent memory
21. DLX_SYSTEM.md - System architecture
22. DLX_COMPLETE_ARCHITECTURE.md - Previous comprehensive doc

For implementation details, consult the individual specialized documents.
