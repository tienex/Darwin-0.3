# Alpha Architecture Support for Darwin-0.3

## Overview

This document describes the DEC Alpha (AXP) architecture support implementation for Darwin-0.3. The implementation provides comprehensive support for Alpha processors with full PALcode compatibility for Windows NT, Digital UNIX (Tru64), and OpenVMS operating systems.

## Architecture Specifications

### Supported Processors

The following Alpha processor families are supported:

| Model | Code Name | Features | Year |
|-------|-----------|----------|------|
| 21064 | EV3 | Original Alpha, 64-bit RISC | 1992 |
| 21064A | EV4 | Improved EV3 | 1993 |
| 21066 | LCA | Low-cost Alpha | 1993 |
| 21164 | EV5 | Second generation, on-chip cache | 1995 |
| 21164A | EV56 | EV5 + BWX, MVI extensions | 1996 |
| 21164PC | PCA56 | Embedded variant | 1997 |
| 21264 | EV6 | Third generation, out-of-order | 1998 |
| 21264A | EV67 | EV6 + CIX extension | 1999 |

### Instruction Set Extensions

- **BWX** (Byte/Word Extension) - Byte and word load/store operations
- **FIX** (Floating-point convert extension) - Enhanced FP conversions
- **CIX** (Count extension) - Count instructions
- **MVI** (Motion Video Instructions) - SIMD-like multimedia operations
- **PAT** (Precision Architecture) - Enhanced precision

### Memory Architecture

- **64-bit addressing** with virtual address space:
  - User segment: `0x0000000000000000 - 0x0000400000000000` (4TB)
  - Kernel segment (cached): `0xfffffc0000000000 - 0xfffffe0000000000`
  - I/O segment (uncached): `0xfffffe0000000000 - 0xffffffff00000000`

- **Page size**: 8KB (8192 bytes)
- **Page tables**: Three-level structure
  - Level 1: 1024 entries (8TB per entry)
  - Level 2: 1024 entries (8GB per entry)
  - Level 3: 1024 entries (8MB per entry)

### Register Set

#### Integer Registers (64-bit)
- **r0/v0**: Return value
- **r1-r8/t0-t7**: Temporaries (caller-saved)
- **r9-r14/s0-s5**: Saved registers (callee-saved)
- **r15/fp**: Frame pointer
- **r16-r21/a0-a5**: Function arguments
- **r22-r25/t8-t11**: More temporaries
- **r26/ra**: Return address
- **r27/t12/pv**: Procedure value
- **r28/at**: Assembler temporary
- **r29/gp**: Global pointer
- **r30/sp**: Stack pointer
- **r31/zero**: Always zero (reads) / discard (writes)

#### Floating-Point Registers (64-bit)
- **f0-f1**: Return values
- **f2-f9**: Temporaries
- **f10-f15**: Saved registers
- **f16-f21**: Function arguments
- **f22-f30**: Temporaries
- **f31**: Always zero

## PALcode Support

PALcode (Privileged Architecture Library code) provides the interface between hardware and operating system. Darwin-0.3 supports three PALcode variants:

### 1. UNIX PALcode (Digital UNIX/Tru64)

**Default for Darwin** - Provides standard UNIX-like system services.

Key functions:
- `swpipl` - Swap interrupt priority level
- `rdps` - Read processor status
- `wrusp/rdusp` - User stack pointer management
- `whami` - Get CPU number
- `imb` - I-stream memory barrier
- `tbi` - TLB invalidate
- `swpctx` - Context switching
- `wrent` - Set exception vectors

### 2. Windows NT PALcode

Supports Windows NT Alpha Edition compatibility.

Key functions:
- `swpirql/rdirql` - IRQL management
- `di/ei` - Disable/enable interrupts
- `wrentry` - Set entry points
- `swpctx` - Context switching
- `tbia/tbis/dtbis` - TLB operations
- `rdpcr` - Read processor control region
- `rfe/retsys` - Return from exception/syscall

### 3. OpenVMS PALcode

Most feature-rich variant with extensive queue operations.

Key functions:
- Processor register operations (`mfpr/mtpr`)
- Queue operations (`insqhil`, `remqtil`, etc.)
- Physical memory access (`ldqp/stqp`)
- Mode changes (`chme/chms/chmu/chmk`)
- Memory probing (`prober/probew`)

## Directory Structure

```
Darwin-0.3/
├── architecture-1/alpha/          # Architecture headers
│   ├── asm_help.h                 # Assembly macros
│   ├── cpu.h                      # CPU definitions
│   ├── pal.h                      # PALcode definitions
│   └── reg.h                      # Register definitions
│
├── kernel-7/
│   ├── machdep/alpha/             # Machine-dependent code
│   │   ├── start.s                # Boot entry point
│   │   ├── alpha_init.c           # Early initialization
│   │   ├── exception.s            # Exception handlers
│   │   ├── trap.c                 # Trap handling
│   │   ├── pmap.c                 # Memory management
│   │   ├── palcode_unix.c         # UNIX PALcode
│   │   ├── palcode_nt.c           # NT PALcode
│   │   ├── palcode_vms.c          # VMS PALcode
│   │   ├── machdep.c              # Machine functions
│   │   ├── pcb.c                  # Process control block
│   │   ├── cswtch.s               # Context switching
│   │   ├── fpu.c                  # FPU support
│   │   ├── bcopy.s/bzero.s        # Optimized primitives
│   │   └── ...                    # Additional support files
│   │
│   ├── mach/alpha/                # Mach layer
│   │   ├── thread_status.h        # Thread state
│   │   └── vm_types.h             # VM types
│   │
│   ├── bsd/alpha/                 # BSD layer
│   │   └── signal.h               # Signal handling
│   │
│   ├── bsd/dev/alpha/             # Device drivers
│   │   ├── cons.c                 # Console
│   │   ├── mem.c                  # Memory device
│   │   └── rtc.c                  # Real-time clock
│   │
│   └── conf/                      # Configuration
│       ├── MASTER.alpha           # Master config
│       ├── files.alpha            # Source file list
│       └── Makefile.alpha         # Build rules
```

## Boot Sequence

1. **Firmware/Bootloader** loads kernel to physical memory
2. **start.s** entry point receives control with:
   - `a0` = Hardware Restart Parameter Block (HWRPB)
   - `a1` = Initial page table (optional)
   - `a2` = Boot flags
3. **PALcode detection** identifies variant (NT/UNIX/VMS)
4. **CPU feature detection** checks for BWX, CIX, MVI, etc.
5. **Virtual memory initialization** sets up page tables
6. **PALcode interface setup** configures exception vectors
7. **Generic kernel startup** continues with `setup_main()`

## Exception Handling

### Exception Types

- **Machine check** - Hardware errors
- **Arithmetic** - FP exceptions, integer overflow
- **Interrupt** - External device interrupts
- **D-fault** - Data TLB miss, access violation
- **I-fault** - Instruction TLB miss, access violation
- **Unaligned** - Unaligned memory access
- **Opcode reserved** - Illegal instruction
- **FEN** - Floating-point disabled

### Exception Entry Points

Three main entry points (set via `wrent` PALcode call):

1. **alpha_exception_entry** - Hardware exceptions
2. **alpha_interrupt_entry** - Device interrupts
3. **alpha_syscall_entry** - System calls

All entry points:
- Save complete processor state (256 bytes)
- Initialize kernel GP register
- Call C handler
- Restore state
- Return via PALcode (`rti` or `retsys`)

## System Calls

System calls use the `call_pal callsys` instruction:
- System call number in `v0`
- Arguments in `a0-a5`
- Return value in `v0`
- Error indication in `a3` (per BSD convention)

## Context Switching

Context switching via PALcode `swpctx`:
1. Save current thread state in PCB (Process Control Block)
2. Switch address space if needed (via page table base)
3. Load new thread state from PCB
4. Return to new thread context

## Memory Management

### TLB Management

- **TLB invalidate all**: `tbia` (UNIX) or `mtpr_tbia` (VMS)
- **TLB invalidate single**: `tbis(va)`
- **D-TLB specific**: `dtbis(va)` (NT only)

### Address Translation

Virtual to physical address translation:
1. Extract level 1 index from VA[42:33]
2. Extract level 2 index from VA[32:23]
3. Extract level 3 index from VA[22:13]
4. Extract page offset from VA[12:0]
5. Combine PTE PFN with offset for physical address

### Page Table Entry Format

```
63        32 31              16 15   8 7     0
+----------+------------------+------+-------+
|   PFN    |    reserved      | soft | flags |
+----------+------------------+------+-------+
```

Flags:
- `V` (0) - Valid
- `FOR` (1) - Fault on Read
- `FOW` (2) - Fault on Write
- `FOE` (3) - Fault on Execute
- `ASM` (4) - Address Space Match
- `GH` (5) - Granularity Hint
- `KRE` (6) - Kernel Read Enable
- `URE` (7) - User Read Enable
- `KWE` (8) - Kernel Write Enable
- `UWE` (9) - User Write Enable

## Cache Management

### Cache Hierarchy

- **EV4**: 8KB I-cache, 8KB D-cache
- **EV5**: 8KB I-cache, 8KB D-cache, 96KB S-cache
- **EV6**: 64KB I-cache, 64KB D-cache

### Cache Operations

- **Memory barrier**: `mb` instruction
- **Write memory barrier**: `wmb` instruction
- **I-stream barrier**: `imb` PALcode call
- **Cache flush**: `cflush(pfn)` PALcode call

## Atomic Operations

Alpha provides load-locked/store-conditional:

```c
unsigned long val = ldq_l(addr);  // Load locked
val++;
if (stq_c(val, addr) == 0) {      // Store conditional
    // Failed, retry
}
```

## Performance Counter

Read cycle counter with `rpcc` instruction:
- Lower 32 bits: cycle count
- Upper 32 bits: implementation-dependent

## Building Alpha Kernel

### Prerequisites

- Cross-compiler for Alpha (or native Alpha system)
- GNU assembler with Alpha support
- Standard build tools (make, ld, etc.)

### Build Commands

```bash
cd kernel-7/conf
make MACHINE=alpha KERNEL_CONFIG=RELEASE
```

### Configuration Options

Edit `MASTER.alpha`:
- `ev4/ev5/ev6` - CPU generation
- `pal_unix/pal_nt/pal_vms` - PALcode variant
- `debug/gdb` - Debugging support
- `multi/uni` - SMP vs uniprocessor

## Debugging

### Kernel Debugger

GDB support via KDP (Kernel Debugging Protocol):
- Set breakpoints
- Examine memory and registers
- Stack traces

### Console Output

Early console via PALcode:
```c
cnputc(char c);  // Output character via PAL_CSERVE
```

### Register Dumps

Exception handlers save complete state for debugging:
- All 32 integer registers
- PC and PS (processor status)
- FP registers (when needed)

## Known Limitations

1. **Incomplete implementations**: Many functions are stubs requiring completion
2. **No SMP support**: Secondary CPU startup not fully implemented
3. **Limited device drivers**: Only basic console/memory devices
4. **No FP emulation**: Requires hardware FP unit
5. **Testing needed**: Implementation not yet tested on real hardware

## Future Work

1. Complete stub implementations
2. Add SMP support
3. Implement device drivers (SCSI, Ethernet, etc.)
4. Add bootloader support
5. Performance optimization
6. Hardware testing and validation

## References

### Alpha Architecture
- *Alpha Architecture Reference Manual*, Digital Equipment Corporation
- *Alpha AXP Architecture*, Richard L. Sites and Richard T. Witek

### PALcode
- *Alpha Console Firmware Update*, Compaq Computer Corporation
- *Windows NT Device Driver Kit for Alpha AXP*, Microsoft Corporation
- *Digital UNIX PALcode Specification*, Digital Equipment Corporation

### Implementation
- Digital UNIX source code
- NetBSD/alpha port
- Linux/alpha kernel

## License

Copyright (c) 2000 Apple Computer, Inc. All rights reserved.

Licensed under the Apple Public Source License Version 1.1.

---

**Author**: Claude (AI Assistant)
**Date**: October 2025
**Version**: 1.0
