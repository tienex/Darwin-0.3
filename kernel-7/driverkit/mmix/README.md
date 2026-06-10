# MMIX Device Drivers for Darwin

## Overview

This directory contains device drivers for the MMIX emulator. These drivers provide kernel-level access to emulated hardware devices through memory-mapped I/O.

## Drivers

### MMIXConsole
**File**: MMIXConsole.m/h
**Purpose**: Console input/output driver

Provides character-based I/O through emulator console device.

**Memory Map**:
- Base: 0xFFFFFFFF00000000
- Output: Base + 0x00 (write byte)
- Input: Base + 0x08 (read byte)

**Usage**:
```objc
MMIXConsole *console = [[MMIXConsole alloc] init...];
[console putc:'A'];
[console puts:"Hello, MMIX!\n"];
int c = [console getc];
```

### MMIXDisk
**File**: MMIXDisk.m/h
**Purpose**: Block device driver for virtual disk

Provides sector-based read/write access to emulated disk.

**Memory Map**:
- Base: 0xFFFFFFFF00001000
- Sector Number: Base + 0x00 (8 bytes)
- Data Buffer: Base + 0x08 (8192 bytes)
- Command: Base + 0x2008 (1=read, 2=write)
- Status: Base + 0x2010 (0=ready, 1=busy, 2=error)

**Sector Size**: 8192 bytes (8 KB)
**Default Capacity**: 1 GB (131072 sectors)

**Usage**:
```objc
MMIXDisk *disk = [[MMIXDisk alloc] init...];
unsigned char buffer[8192];
[disk readSector:0 buffer:buffer count:1];
[disk writeSector:100 buffer:data count:10];
```

### MMIXTimer
**File**: MMIXTimer.m/h
**Purpose**: System timer based on MMIX rI register

Provides high-resolution timing using the MMIX interval counter.

**Frequency**: 1 GHz (1 tick per nanosecond)
**Register**: rI (Special Register 12)

**Usage**:
```objc
MMIXTimer *timer = [[MMIXTimer alloc] init...];
unsigned long long now = [timer readCounter];
[timer setInterval:1000000];  // 1ms
[timer enableInterrupts];
```

## Device Integration

### Device Probing

Drivers implement the DriverKit probe/init pattern:

```objc
+ (BOOL)probe:deviceDescription
{
    // Create and initialize driver instance
    // Register with kernel
}

- initFromDeviceDescription:deviceDescription
{
    // Map device registers
    // Initialize hardware
    // Set device properties
}
```

### Memory-Mapped I/O

All devices use memory-mapped I/O in the high address range:
- 0xFFFFFFFF00000000 - 0xFFFFFFFFFFFFFFFF: Device space

Devices must be mapped as non-cacheable in the MMU.

## Adding New Drivers

To add a new MMIX emulator driver:

1. Create header file (.h) with device interface
2. Implement driver class (.m) inheriting from KernDevice
3. Define memory-mapped register addresses
4. Implement probe/init methods
5. Add driver to kernel configuration
6. Update this README

## Emulator Requirements

The MMIX emulator must provide:

1. **Memory-mapped devices** at documented addresses
2. **8KB sector size** for disk operations
3. **Byte-wide I/O** for console
4. **Status/command registers** for async operations
5. **rI register** for timer functionality

## Compilation

Drivers are compiled as part of the kernel build:

```bash
cd /path/to/Darwin-0.3/kernel-7
make ARCH=mmix
```

## Testing

Test drivers using emulator with debug mode:

```bash
mmix-emulator -debug -boot darwin_kernel.mmix
```

Enable driver debugging in kernel config:
```
DEBUG_MMIX_CONSOLE=1
DEBUG_MMIX_DISK=1
DEBUG_MMIX_TIMER=1
```

## References

- MMIX Architecture Specification
- Darwin DriverKit Documentation
- MMIX Emulator API Specification
