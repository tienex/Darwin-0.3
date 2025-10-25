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
