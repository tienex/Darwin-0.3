# AlphaStation 500 and 600 Hardware Support

## Overview

This document describes the hardware support for DEC AlphaStation 500 and 600 workstations in Darwin-0.3.

## Supported Systems

### AlphaStation 500 (Maverick)

**Specifications:**
- **CPU**: 21164 (EV5) or 21164A (EV56)
- **Clock Speed**: 266-500 MHz
- **Chipset**: 21172 CIA (Cache/memory, I/O bridge, Asic)
- **Memory**: Up to 1GB RAM, ECC supported
- **System Bus**: 128-bit, 83 MHz
- **Expansion**:
  - 3x PCI slots (32-bit, 33 MHz)
  - 2x ISA slots (optional)
- **Graphics**: Integrated S3 ViRGE or Matrox MGA
- **Storage**: Dual IDE channels, optional SCSI
- **Network**: Ethernet (10/100 Mbps via PCI)
- **I/O**:
  - 2x Serial ports (COM1, COM2)
  - Parallel port
  - PS/2 keyboard and mouse
  - Floppy drive controller

**Model Variants:**
- AlphaStation 500/266 (266 MHz EV5)
- AlphaStation 500/333 (333 MHz EV5)
- AlphaStation 500/400 (400 MHz EV56)
- AlphaStation 500/500 (500 MHz EV56)

### AlphaStation 600 (Alcor)

**Specifications:**
- **CPU**: 21164 (EV5)
- **Clock Speed**: 266-333 MHz
- **Chipset**: 21171 CIA
- **Memory**: Up to 2GB RAM, ECC supported
- **System Bus**: 128-bit, 83 MHz
- **Expansion**:
  - 5x PCI slots (32-bit, 33 MHz)
  - 3x ISA slots
- **Graphics**: PCI graphics card
- **Storage**: Dual IDE channels, optional SCSI
- **Network**: Ethernet (10/100 Mbps via PCI)
- **I/O**:
  - 2x Serial ports
  - Parallel port
  - PS/2 keyboard and mouse
  - Floppy drive controller

**Model Variants:**
- AlphaStation 600/266 (266 MHz EV5)
- AlphaStation 600/300 (300 MHz EV5)
- AlphaStation 600/333 (333 MHz EV5)

## Hardware Architecture

### System Block Diagram

```
┌─────────────────────────────────────────────────────────┐
│  CPU (21164/21164A)                                     │
│  - 8KB I-cache, 8KB D-cache                             │
│  - 96KB L2 cache (on-chip or external)                  │
└──────────────────┬──────────────────────────────────────┘
                   │ 128-bit, 83 MHz
┌──────────────────┴──────────────────────────────────────┐
│  CIA Chipset (21171/21172)                              │
│  ┌──────────────┬─────────────┬────────────────────┐   │
│  │ Memory       │ PCI Bridge  │ Interrupt          │   │
│  │ Controller   │             │ Controller         │   │
│  └──────────────┴─────────────┴────────────────────┘   │
└──┬──────────────┬─────────────┬────────────────────────┘
   │              │             │
   │ DRAM         │ PCI Bus     │ ISA Bridge
   │              │             │
┌──┴────┐    ┌────┴─────┐  ┌───┴────────┐
│       │    │          │  │            │
│  RAM  │    │ PCI      │  │ ISA        │
│       │    │ Devices  │  │ Devices    │
│       │    │          │  │            │
└───────┘    └──────────┘  └────────────┘
             - Graphics   - RTC/NVRAM
             - SCSI       - Keyboard/Mouse
             - Network    - Serial/Parallel
             - Sound      - Floppy
```

### CIA Chipset Features

The CIA (21171/21172/21174) provides:

1. **Memory Controller**
   - Support for 64MB-2GB DRAM
   - ECC error detection and correction
   - Interleaved memory access
   - Optimized for 128-bit bus width

2. **PCI Host Bridge**
   - PCI 2.1 compliant
   - 32-bit, 33 MHz
   - Multiple PCI devices supported
   - Posted write buffers
   - Read-ahead cache

3. **DMA Scatter-Gather**
   - Hardware TLB for DMA translations
   - Support for non-contiguous memory
   - Two DMA windows:
     - Window 0: Direct-mapped (0-8MB)
     - Window 1: Scatter-gather (8MB-1GB)

4. **Interrupt Controller**
   - Routes PCI interrupts to CPU
   - 4 PCI interrupt lines (INTA-INTD)
   - Level-triggered interrupts
   - Interrupt sharing support

## Memory Map

### Physical Address Space

| Address Range              | Size  | Description                |
|----------------------------|-------|----------------------------|
| 0x0000000000-0x003FFFFFFF  | 1GB   | Main Memory (DRAM)         |
| 0x8000000000-0x87FFFFFFFF  | 32GB  | PCI Memory Space           |
| 0x8580000000-0x85FFFFFFFF  | 2GB   | PCI I/O Space              |
| 0x8700000000-0x877FFFFFFF  | 2GB   | PCI Configuration Space    |
| 0x8740000000-0x87BFFFFFFF  | 2GB   | CIA CSR (Control/Status)   |

### Virtual Address Space

| Address Range              | Size  | Description                |
|----------------------------|-------|----------------------------|
| 0x0000000000-0x00003FFFFF  | 4TB   | User Space                 |
| 0xFFFFFC0000000000-...     | 128TB | Kernel KSEG (cached)       |
| 0xFFFFFE0000000000-...     | 128TB | Kernel K1SEG (uncached)    |

## I/O Subsystem

### PCI Bus

**Configuration Space Access:**
```
Address = 0x8700000000 | (bus << 16) | (device << 11) | (function << 8) | register
```

**Memory Space:**
- Base: 0x8000000000
- Size: 32GB
- Cacheable, prefetchable regions supported

**I/O Space:**
- Base: 0x8580000000
- Size: 2GB
- Non-cacheable, non-prefetchable

### ISA Bus

The ISA bus is bridged from PCI through the chipset.

**Standard ISA I/O Ports:**
- 0x000-0x01F: DMA controller
- 0x020-0x03F: Interrupt controller (master PIC)
- 0x040-0x05F: Timer
- 0x060-0x06F: Keyboard controller
- 0x070-0x07F: RTC/NVRAM
- 0x0A0-0x0BF: Interrupt controller (slave PIC)
- 0x170-0x177: Secondary IDE
- 0x1F0-0x1F7: Primary IDE
- 0x2F8-0x2FF: COM2 (serial port 2)
- 0x378-0x37F: Parallel port
- 0x3B0-0x3DF: VGA
- 0x3F0-0x3F7: Floppy controller
- 0x3F8-0x3FF: COM1 (serial port 1)

## Interrupt System

### PCI Interrupts

PCI devices use level-triggered, shareable interrupts.

**Interrupt Routing:**
- PCI INTA → CPU IRQ 16
- PCI INTB → CPU IRQ 17
- PCI INTC → CPU IRQ 18
- PCI INTD → CPU IRQ 19

### ISA Interrupts

ISA devices use edge-triggered interrupts via 8259 PIC.

**IRQ Assignment:**
| IRQ  | Device              |
|------|---------------------|
| 0    | System Timer        |
| 1    | Keyboard            |
| 2    | Cascade (slave PIC) |
| 3    | COM2                |
| 4    | COM1                |
| 5    | Available           |
| 6    | Floppy              |
| 7    | Parallel            |
| 8    | RTC                 |
| 9    | Available           |
| 10   | Available           |
| 11   | Available           |
| 12   | PS/2 Mouse          |
| 13   | FPU (not used)      |
| 14   | Primary IDE         |
| 15   | Secondary IDE       |

## Real-Time Clock

**Hardware:** MC146818A-compatible RTC with 114 bytes NVRAM

**Features:**
- Date and time keeping
- Battery-backed
- Alarm function
- Periodic interrupt (adjustable rate)
- Century register (register 0x32)
- NVRAM for system configuration

**Access:**
- Address port: 0x70
- Data port: 0x71

## Boot Process

1. **Console Firmware (SRM)**
   - Initializes hardware
   - Creates HWRPB (Hardware Restart Parameter Block)
   - Sets up initial page tables
   - Loads kernel from disk

2. **Kernel Bootstrap**
   - `start.s`: Entry point from firmware
   - Receive HWRPB in register a0
   - Initialize CPU and PALcode interface
   - Detect platform type

3. **Platform Initialization**
   - Detect AlphaStation 500 or 600
   - Initialize CIA chipset
   - Set up memory controller
   - Configure PCI bridge

4. **Device Initialization**
   - Enumerate PCI devices
   - Initialize ISA bus
   - Set up RTC
   - Configure interrupt controller

5. **Kernel Main**
   - Continue with generic kernel initialization
   - Mount root filesystem
   - Start init process

## Known Hardware Issues

### AlphaStation 500
- Some early units had cache coherency issues (fixed in later revisions)
- PCI bus may be limited to 2-3 active devices under heavy load
- S3 graphics can be unstable with some monitors

### AlphaStation 600
- More slots than AS500 but same PCI bus bandwidth
- Heavy PCI usage can impact system performance
- ISA slots may conflict with some PCI devices

## Performance Characteristics

### Memory Bandwidth
- Peak: 664 MB/s (128-bit @ 83 MHz, EV5)
- Typical: 400-500 MB/s with contention

### PCI Bandwidth
- Peak: 132 MB/s (32-bit @ 33 MHz)
- Typical: 80-100 MB/s with multiple devices

### CPU Performance
- EV5 @ 266 MHz: ~530 SPECint95, ~360 SPECfp95
- EV5 @ 333 MHz: ~660 SPECint95, ~450 SPECfp95
- EV56 @ 500 MHz: ~1000 SPECint95, ~680 SPECfp95

## Darwin Support Status

### ✅ Implemented
- Platform detection (AS500/AS600)
- CIA chipset initialization
- PCI bus enumeration
- PCI configuration space access
- ISA bus support
- 8259 PIC interrupt controller
- RTC/NVRAM access
- I/O port operations
- DMA scatter-gather setup

### ⚠️ Partial
- Device drivers (stubs present)
- Interrupt handling (framework ready)
- Power management

### ❌ Not Yet Implemented
- Graphics drivers
- Network drivers
- SCSI drivers
- Sound drivers
- Power management (ACPI)
- Suspend/resume

## References

- *AlphaStation 500 Product Brief*, Digital Equipment Corporation
- *AlphaStation 600 Technical Summary*, Digital Equipment Corporation
- *CIA Chip Specification*, Digital Semiconductor
- *Alpha Firmware Reference Manual*, Compaq Computer Corporation

## Development Notes

To enable AlphaStation support in Darwin kernel:

1. Configure kernel for Alpha architecture
2. Platform detection is automatic via HWRPB
3. Default platform is AlphaStation 500
4. Boot with SRM console firmware

Example boot sequence:
```
P00>>> boot dka0 -flags n
```

Where:
- `dka0` is the boot device
- `-flags n` passes single-user mode flag to kernel

---

**Last Updated:** October 2025
**Document Version:** 1.0
