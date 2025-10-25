# Chapter 6: ABI and Calling Conventions

## 6.1 Data Types

### Integer Types
```c
typedef signed char        int8_t;     /* 8-bit signed */
typedef unsigned char      uint8_t;    /* 8-bit unsigned */
typedef signed short       int16_t;    /* 16-bit signed */
typedef unsigned short     uint16_t;   /* 16-bit unsigned */
typedef signed int         int32_t;    /* 32-bit signed */
typedef unsigned int       uint32_t;   /* 32-bit unsigned */
typedef signed long long   int64_t;    /* 64-bit signed */
typedef unsigned long long uint64_t;   /* 64-bit unsigned */
```

### Floating-Point Types
```c
typedef float              float32_t;  /* 32-bit IEEE 754 */
typedef double             float64_t;  /* 64-bit IEEE 754 */
typedef __float128         float128_t; /* 128-bit quad */
typedef __bfloat16         bfloat16_t; /* 16-bit BFloat16 */
```

### Alignment Requirements
```c
#define ALIGN_INT8      1
#define ALIGN_INT16     2
#define ALIGN_INT32     4
#define ALIGN_INT64     8
#define ALIGN_INT128    16
#define ALIGN_FLOAT32   4
#define ALIGN_FLOAT64   8
#define ALIGN_FLOAT128  16
#define ALIGN_VECTOR128 16
#define ALIGN_VECTOR256 32

/* Stack alignment */
#define STACK_ALIGN     16  /* 16-byte aligned stack */
```

## 6.2 Register Usage

### General Purpose Registers (DLX64)

| Register | ABI Name | Purpose | Preserved Across Calls |
|----------|----------|---------|----------------------|
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

### Floating-Point Registers

| Register | ABI Name | Purpose | Preserved |
|----------|----------|---------|-----------|
| f0-f7 | ft0-ft7 | FP temporaries | No |
| f8-f9 | fs0-fs1 | FP saved registers | Yes |
| f10-f11 | fa0-fa1 | FP arguments / return values | No |
| f12-f17 | fa2-fa7 | FP arguments | No |
| f18-f27 | fs2-fs11 | FP saved registers | Yes |
| f28-f31 | ft8-ft11 | FP temporaries | No |

### Vector Registers

| Register | ABI Name | Purpose | Preserved |
|----------|----------|---------|-----------|
| v0-v1 | vv0-vv1 | Vector return values | No |
| v2-v9 | va0-va7 | Vector arguments | No |
| v10-v15 | vt0-vt5 | Vector temporaries | No |
| v16-v31 | vs0-vs15 | Vector saved registers | Yes |

## 6.3 Calling Convention

### Argument Passing

**Integer and Pointer Arguments:**
- First 8 arguments: a0-a7 (r10-r17)
- Additional arguments: passed on stack

**Floating-Point Arguments:**
- First 8 FP arguments: fa0-fa7 (f10-f17)
- Additional FP arguments: passed on stack

**Vector Arguments:**
- First 8 vector arguments: va0-va7 (v2-v9)
- Additional vector arguments: passed on stack

**Return Values:**
- Integer/pointer: a0-a1 (r10-r11)
- Floating-point: fa0-fa1 (f10-f11)
- Vector: vv0-vv1 (v0-v1)

### Stack Frame Layout

```
High Address
+------------------+
| Previous frame   |
+------------------+
| Return address   |  sp + frame_size - 8 (saved by caller)
+------------------+
| Saved fp         |  sp + frame_size - 16 (if used)
+------------------+
| Saved registers  |  Callee-saved regs (s0-s11, fs0-fs11)
+------------------+
| Local variables  |  Function local storage
+------------------+
| Outgoing args    |  Args beyond a0-a7
+------------------+  <- sp (stack pointer)
Low Address
```

### Function Prologue (Standard)
```assembly
function:
    addi    sp, sp, -frame_size     # Allocate stack frame
    sd      ra, frame_size-8(sp)    # Save return address
    sd      s0, frame_size-16(sp)   # Save frame pointer (if used)
    addi    s0, sp, frame_size      # Set up frame pointer
    # Save other callee-saved registers as needed
```

### Function Epilogue (Standard)
```assembly
    # Restore callee-saved registers
    ld      s0, frame_size-16(sp)   # Restore frame pointer
    ld      ra, frame_size-8(sp)    # Restore return address
    addi    sp, sp, frame_size      # Deallocate stack frame
    jr      ra                      # Return
```

## 6.4 Position-Independent Code (PIC)

### Global Offset Table (GOT)
```assembly
# Access global variable via GOT
    auipc   gp, %pcrel_hi(_GLOBAL_OFFSET_TABLE_)
    addi    gp, gp, %pcrel_lo(_GLOBAL_OFFSET_TABLE_)
    ld      t0, symbol@GOT(gp)      # Load symbol address from GOT
    ld      a0, 0(t0)               # Load actual value
```

### Procedure Linkage Table (PLT)
```assembly
# Call external function via PLT
    call    function@PLT            # Call through PLT stub
```

## 6.5 Thread-Local Storage (TLS)

### TLS Models

**1. Local Exec (LE) - Fastest, executable only**
```assembly
    lui     t0, %tprel_hi(tls_var)
    add     t0, t0, tp
    ld      a0, %tprel_lo(tls_var)(t0)
```

**2. Initial Exec (IE) - Shared library, known offset**
```assembly
    ld      t0, tls_var@GOTTPOFF(gp)
    add     t0, t0, tp
    ld      a0, 0(t0)
```

**3. Global Dynamic (GD) - Full dynamic TLS**
```assembly
    call    __tls_get_addr
    # a0 contains TLS variable address
    ld      a0, 0(a0)
```

## 6.6 Register Banking Mode

In register banking mode, each privilege level has its own register set:

```c
struct dlx_bank_config {
    uint32_t enable_banking;        /* 1 = banking enabled */
    uint32_t bank_user;             /* Bank for ring 3 (user) */
    uint32_t bank_supervisor;       /* Bank for ring 2 */
    uint32_t bank_executive;        /* Bank for ring 1 */
    uint32_t bank_kernel;           /* Bank for ring 0 */
    uint32_t shared_registers;      /* Bitmask of shared regs */
};
```

Typically shared across banks:
- r0 (zero) - always hardwired to 0
- r2 (sp) - stack pointer
- r3 (gp) - global pointer
- r4 (tp) - thread pointer
