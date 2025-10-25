# RISC-V 32/64-bit Support for Darwin-0.3

This document describes the RISC-V architecture support added to the Darwin-0.3 kernel.

## Overview

RISC-V support has been added to Darwin-0.3, supporting both RV32 (32-bit) and RV64 (64-bit) architectures. This implementation follows the existing patterns established by the i386 and PowerPC ports.

## CPU Type Definitions

Added to `kernel-7/mach/machine.h`:
- **CPU_TYPE_RISCV** (19): Main CPU type for RISC-V processors
- **CPU_SUBTYPE_RISCV_ALL** through **CPU_SUBTYPE_RISCV64_G**: Subtypes for various RISC-V configurations

The subtypes support:
- RV32/RV64 base ISAs
- Standard extensions (M, A, F, D, C)
- G extension (general-purpose: IMAFD)

## Directory Structure

### Headers
- `kernel-7/mach/riscv/` - Machine-dependent Mach headers
  - boolean.h, vm_types.h, vm_param.h, kern_return.h
  - exception.h, thread_status.h, simple_lock.h
  - ndr.h, syscall_sw.h, machine_types.defs

- `architecture-1/riscv/` - Architecture-specific headers
  - alignment.h, byte_order.h, limits.h, ansi.h
  - asm_help.h, reg_help.h

### Implementation
- `kernel-7/machdep/riscv/` - Machine-dependent kernel code
  - Core files for initialization, traps, memory management

- `kernel-7/bsd/dev/riscv/` - Device drivers

### Configuration
- `kernel-7/conf/MASTER.riscv` - Master configuration
- `kernel-7/conf/Makefile.riscv` - Build rules
- `kernel-7/conf/files.riscv` - File list

## Memory Layout

### RV32
- User space: 0x00000000 - 0xC0000000
- Kernel space: 0xC0000000 - 0xFFFFFFFF
- Page size: 4KB

### RV64
- User space: 0x0000000000000000 - 0x0000004000000000
- Kernel space: 0xFFFFFFE000000000 - 0xFFFFFFFF80000000
- Page size: 4KB

## Key Features

### Thread State
- Full 32-register general-purpose register file (x0-x31)
- Floating-point registers (f0-f31)
- Program counter and control/status registers
- Exception state tracking

### Atomic Operations
- Simple lock implementation using RISC-V atomic instructions
- Support for amoswap.w.aq/rl (acquire/release semantics)
- Fallback for systems without 'A' extension

### Exception Handling
Exception codes for:
- Page faults (instruction, load, store)
- Illegal instructions
- Misaligned accesses
- System calls (ecall from U/S/M modes)
- Breakpoints

### System Calls
- System call number in a7 register
- Arguments in a0-a6
- Return value in a0
- ecall instruction for trap to kernel

## Build System Integration

Updated `kernel-7/conf/Makefile`:
- HEADER_ARCHS includes RISCV
- BUILD_ARCHS includes RISCV
- DEFAULT_ARCH detection for RISC-V

## Building

To build for RISC-V:
```bash
cd kernel-7/conf
make ARCH=RISCV TYPE=RELEASE mach_kernel.kernel
```

## Compiler Support

Requires RISC-V GCC toolchain:
- riscv64-unknown-elf-gcc
- riscv64-unknown-elf-ld
- riscv64-unknown-elf-as

Default configuration: `-march=rv64g -mabi=lp64d`

## Implementation Status

### Complete
- CPU type and subtype definitions
- Machine-dependent header files
- Architecture-specific headers
- Build system integration
- Configuration files
- Basic startup code

### To Be Implemented
- Full trap/exception handling
- Complete memory management (pmap)
- Device drivers
- Interrupt handling
- Context switching
- System call implementation

## References

- RISC-V ISA Specification: https://riscv.org/specifications/
- RISC-V Privileged Architecture: https://riscv.org/specifications/privileged-isa/
- Darwin/XNU Architecture Documentation

## Authors

Implemented following the architectural patterns established by the original Darwin i386 and PowerPC ports.
