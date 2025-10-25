# MMIX Bootloader for Darwin

## Overview

This directory contains the bootloader implementation for MMIX emulator support in Darwin-0.3. The bootloader initializes the MMIX architecture, sets up the virtual memory system, and loads the Darwin kernel.

## Architecture

The MMIX bootloader is designed for emulator environments and follows a simplified boot sequence compared to physical hardware:

1. **Stage 1 - Emulator Entry**: Emulator loads bootloader into memory at 0x0000000000001000
2. **Stage 2 - Hardware Init**: Initialize MMIX special registers and MMU
3. **Stage 3 - Kernel Load**: Load Darwin kernel from virtual disk
4. **Stage 4 - Kernel Entry**: Transfer control to kernel

## Memory Map

```
0x0000000000000000 - 0x0000000000001000: Trap vectors
0x0000000000001000 - 0x0000000000010000: Bootloader code
0x0000000000010000 - 0x0000000000100000: Bootloader stack/heap
0x0000000000100000 - 0x0000000010000000: Kernel load area
0x8000000000000000 - 0xFFFFFFFFFFFFFFFF: Kernel virtual space
```

## Files

- **boot.s**: Assembly entry point and low-level initialization
- **main.c**: C bootloader main logic
- **console.c**: Early console output via emulator
- **disk.c**: Virtual disk I/O for kernel loading
- **mmu.c**: MMU initialization and page table setup
- **boot.h**: Common bootloader definitions
- **Makefile**: Build configuration

## Building

```bash
cd /path/to/Darwin-0.3/boot-2/mmix
make
```

Output: `mmix_bootloader` - Ready to load into emulator

## Emulator Integration

The bootloader expects the following emulator features:

1. **Console Device** at virtual address 0xFFFFFFFF00000000
   - Write byte to 0xFFFFFFFF00000000 for output
   - Read byte from 0xFFFFFFFF00000008 for input

2. **Disk Device** at virtual address 0xFFFFFFFF00001000
   - Sector number at offset 0x00 (8 bytes)
   - Sector buffer at offset 0x08 (8 KB)
   - Command register at offset 0x2008 (1=read, 2=write)
   - Status register at offset 0x2010 (0=ready, 1=busy, 2=error)

3. **Memory**: At least 256 MB RAM starting at 0x00000000

## Boot Parameters

The bootloader passes the following parameters to the kernel via boot_args structure:

- Physical memory size
- Bootloader version
- Console device address
- Disk device address
- Initial page tables
- Device tree (if available)

## References

- MMIX Architecture Specification (Donald Knuth)
- Darwin Boot Process Documentation
- MMIX Emulator Specification
