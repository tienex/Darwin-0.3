# Darwin IA64 - Itanium 2 Boot Guide

## Overview

This guide explains how to boot Darwin on Itanium 2 processors, including:
- Physical Itanium 2 hardware
- SKI (HP's IA-64 Simulator)
- QEMU with IA64 support

## Itanium 2 Processor Support

### Supported Itanium 2 Processors

Darwin IA64 supports all Itanium 2 family processors:

| Processor | Year | Process | Clock | Cores | L3 Cache | Notes |
|-----------|------|---------|-------|-------|----------|-------|
| **Madison** | 2003 | 130nm | 1.3-1.6 GHz | 1 | 3-6 MB | First Itanium 2 |
| **Madison 9M** | 2004 | 130nm | 1.6 GHz | 1 | 9 MB | Larger cache |
| **Deerfield** | 2004 | 130nm | 1.0-1.5 GHz | 1 | 1.5 MB | Low-power |
| **Montecito** | 2006 | 90nm | 1.4-1.6 GHz | 2 | 12 MB | **Dual-core** |
| **Montvale** | 2007 | 90nm | 1.6 GHz | 2 | 12 MB | Improved Montecito |
| **Tukwila** | 2010 | 65nm | 2.0 GHz | 4 | 24-32 MB | **Quad-core**, VT-i |
| **Poulson** | 2012 | 32nm | 2.5 GHz | 8 | 32 MB | **Eight-core** |

### Key Features Detected by Bootloader

The EFI bootloader automatically detects:
- **CPU family** (Itanium 1 vs Itanium 2)
- **Core count** (via SAL firmware)
- **VT-i support** (hardware virtualization on Tukwila+)
- **L3 cache size** (optimizes VHPT size)

## Boot Process

### EFI Boot Sequence

Unlike x86 (BIOS) or PowerPC (Open Firmware), Itanium uses **EFI (Extensible Firmware Interface)**:

```
1. Power-On → EFI Firmware (PAL/SAL)
2. EFI Boot Manager
3. Darwin EFI Bootloader (BOOTIA64.EFI)
4. Darwin Kernel (mach_kernel.ia64)
5. Userland
```

### EFI System Partition (ESP)

The EFI bootloader must be installed on the EFI System Partition:

```
/EFI/
  └── DARWIN/
      ├── BOOTIA64.EFI      # Darwin bootloader
      └── mach_kernel.ia64  # Darwin kernel
```

### PAL and SAL Firmware

Itanium firmware provides two abstraction layers:

**PAL (Processor Abstraction Layer):**
- Processor-specific operations
- Cache management
- TLB control
- CPU feature detection

**SAL (System Abstraction Layer):**
- Platform-specific operations
- CPU enumeration
- Error handling
- Platform initialization

The bootloader interfaces with both PAL and SAL to:
- Detect CPU features
- Count processors
- Get cache information
- Initialize system

## Virtual Memory Initialization

### Region-Based Addressing

IA64 uses an 8-region address space (64-bit with 3-bit region number):

```
Region 0-4: User space    (0x0000000000000000 - 0x9FFFFFFFFFFFFFFF)
Region 5:   Kernel virtual (0xA000000000000000 - 0xBFFFFFFFFFFFFFFF)
Region 6:   Kernel identity (0xC000000000000000 - 0xDFFFFFFFFFFFFFFF)
Region 7:   Kernel percpu   (0xE000000000000000 - 0xFFFFFFFFFFFFFFFF)
```

### VHPT (Virtual Hash Page Table)

The bootloader configures VHPT for hardware-walked TLB misses:

**VHPT Size Selection:**
- Small L3 cache (<6MB): 256 KB VHPT
- Large L3 cache (≥6MB): 2 MB VHPT

**Configuration:**
- Short format VHPT (Itanium 2 optimized)
- Hash function for fast lookups
- Configured in cr.pta register

### Translation Registers (TR)

The bootloader sets up guaranteed TLB entries:

```
iTR[0]: Kernel code  (16MB, execute+read, Region 5)
dTR[1]: Kernel data  (16MB, read+write, Region 5)
```

These entries are never evicted, ensuring critical kernel code/data are always mapped.

### Page Size

Darwin IA64 uses **16KB pages** (optimal for Itanium 2):
- Better TLB coverage
- Reduced TLB pressure
- Improved performance vs 4KB

## Running on Physical Itanium 2 Hardware

### Supported Systems

**HP Integrity Servers:**
- rx2600, rx2620, rx2660 (Madison/Montecito)
- rx3600, rx3620 (Montecito)
- rx4640 (Montecito)
- rx6600 (Montecito/Montvale)
- Superdome (multi-node systems)

**SGI Altix:**
- Altix 4700 (Montecito)
- Altix UV (Tukwila)

**Bull NovaScale:**
- NovaScale 5160 (Montecito)

### Installation Steps

1. **Prepare EFI System Partition:**
   ```
   # Create GPT partition table
   parted /dev/sda mklabel gpt

   # Create ESP (100MB)
   parted /dev/sda mkpart primary fat32 1MiB 101MiB
   parted /dev/sda set 1 boot on

   # Format ESP
   mkfs.vfat -F 32 /dev/sda1

   # Mount ESP
   mount /dev/sda1 /boot/efi
   ```

2. **Install Bootloader:**
   ```
   mkdir -p /boot/efi/EFI/DARWIN
   cp boot-2/ia64/efi/BOOTIA64.EFI /boot/efi/EFI/DARWIN/
   cp mach_kernel.ia64 /boot/efi/EFI/DARWIN/
   ```

3. **Configure EFI Boot Entry:**
   ```
   # From EFI shell:
   Shell> bcfg boot add 0 fs0:\EFI\DARWIN\BOOTIA64.EFI "Darwin IA64"

   # Or from Linux with efibootmgr:
   efibootmgr --create --disk /dev/sda --part 1 \
              --label "Darwin IA64" \
              --loader '\EFI\DARWIN\BOOTIA64.EFI'
   ```

4. **Boot:**
   - Reboot system
   - Select "Darwin IA64" from EFI Boot Manager
   - Bootloader loads kernel and boots Darwin

## Running on SKI (IA-64 Simulator)

SKI is HP's official IA-64 instruction set simulator.

### Installing SKI

```bash
# Debian/Ubuntu
apt-get install ski

# From source
wget http://ski.sourceforge.net/downloads/ski-1.3.2.tar.gz
tar xzf ski-1.3.2.tar.gz
cd ski-1.3.2
./configure --prefix=/usr/local
make && make install
```

### Booting Darwin on SKI

1. **Create disk image:**
   ```bash
   dd if=/dev/zero of=darwin_ia64.img bs=1M count=2048
   ```

2. **Install Darwin:**
   ```bash
   # Mount image
   losetup /dev/loop0 darwin_ia64.img

   # Partition and install (as above)
   ```

3. **Boot with SKI:**
   ```bash
   ski -i /boot/efi/EFI/DARWIN/BOOTIA64.EFI \
       -disk darwin_ia64.img \
       -console
   ```

### SKI-Specific Features

The bootloader detects SKI environment and enables:
- Console I/O via break instructions
- Debugging support
- Simplified device detection

### SKI Console Commands

```
# In SKI debugger:
ski> load /boot/efi/EFI/DARWIN/BOOTIA64.EFI
ski> run
ski> quit

# Set breakpoints:
ski> break 0xA000000000000000

# Examine memory:
ski> mem 0xA000000000000000 256

# Examine registers:
ski> reg
```

## Running on QEMU-IA64

QEMU has experimental IA64 support (qemu-system-ia64).

### Building QEMU with IA64

```bash
git clone https://git.qemu.org/git/qemu.git
cd qemu
./configure --target-list=ia64-softmmu --enable-debug
make
```

### Booting Darwin on QEMU

```bash
qemu-system-ia64 \
    -m 2048 \
    -drive file=darwin_ia64.img,format=raw \
    -bios /usr/share/edk2/ia64/OVMF.fd \
    -nographic \
    -serial stdio
```

**Note:** QEMU IA64 support is incomplete. SKI is recommended for emulation.

## Performance Tuning for Itanium 2

### Compiler Optimizations

```bash
# GCC for IA64
gcc -O3 -mtune=itanium2 -mbig-endian=no \
    -fomit-frame-pointer \
    -finline-functions \
    -fprefetch-loop-arrays \
    program.c -o program
```

### Memory Optimization

**Large Pages:**
Darwin supports multiple page sizes. For large datasets:
- Use 64KB or 256KB pages
- Reduces TLB pressure
- Improves memory performance

**NUMA Awareness:**
On multi-node systems (Superdome), optimize for NUMA:
- Allocate memory local to CPU
- Use node-affine scheduling

### Cache Optimization

**Itanium 2 Cache Hierarchy:**
- L1: 16KB instruction + 16KB data
- L2: 256KB unified
- L3: 3-32MB unified (processor dependent)

**Optimization tips:**
- Align critical structures to cache lines (128 bytes)
- Use prefetch instructions
- Minimize cache conflicts

## Troubleshooting

### Common Boot Issues

**"PAL/SAL initialization failed"**
- EFI firmware may be outdated
- Update to latest firmware for your system

**"Failed to initialize VHPT"**
- Insufficient memory
- Firmware memory allocation issue
- Try reducing VHPT size in bootloader

**"Kernel load failed"**
- Verify mach_kernel.ia64 is present
- Check file is not corrupted
- Ensure ESP is properly formatted

**"TLB miss in kernel"**
- TR entries not properly configured
- VHPT initialization issue
- Check bootloader logs

### Debugging

**Enable verbose boot:**
```
# In EFI shell, set boot argument:
Shell> edit startup.nsh
bootia64 -v
```

**SKI debugging:**
```
ski> run
# Boot fails...
ski> reg        # Check registers
ski> mem 0xA... # Check memory
ski> stack      # Check call stack
```

## References

### Documentation
- Intel Itanium Architecture Software Developer's Manual
- EFI Specification v1.10
- HP-UX for Itanium Documentation
- Linux/IA64 Kernel Documentation

### Hardware
- HP Integrity Server Product Line
- SGI Altix Documentation
- Bull NovaScale Technical Manuals

### Software
- SKI Simulator: http://ski.sourceforge.net/
- QEMU: https://www.qemu.org/
- GNU EFI: https://sourceforge.net/projects/gnu-efi/

## Performance Expectations

### Itanium 2 Performance Characteristics

**Typical Performance (relative to contemporary x86):**
- Integer: 0.7-0.9x (worse due to EPIC overhead)
- Floating-point: 1.2-1.5x (better due to 128 FP registers)
- Scientific computing: 1.5-2.0x (excellent for HPC)
- Memory bandwidth: 1.5-2.0x (superior to x86)

**Best Use Cases:**
- High-performance computing
- Database servers
- Virtualization (with VT-i)
- Mission-critical enterprise workloads

---

**Last Updated:** 2025
**Darwin Version:** 0.3
**Bootloader Version:** 1.0
