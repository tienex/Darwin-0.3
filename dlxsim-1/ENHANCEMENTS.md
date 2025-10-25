# DLXSIM Enhanced Features

## Summary of Enhancements

This document describes the major architectural enhancements added to DLXSIM to support modern processor features and multiple executable formats.

## 1. Endianness Control

### Status Register Bits
- **STATUS_SYS_BE** (bit 18): System mode big-endian control
  - 1 = Big-endian in kernel/system mode
  - 0 = Little-endian in kernel/system mode

- **STATUS_USR_BE** (bit 19): User mode big-endian control  
  - 1 = Big-endian in user mode
  - 0 = Little-endian in user mode

### Usage
```c
/* Set system mode to big-endian, user mode to little-endian */
status |= STATUS_SYS_BE;     /* System = BE */
status &= ~STATUS_USR_BE;    /* User = LE */
movi2s r0, status
```

### Memory Access
All memory operations (lw, lh, lb, sw, sh, sb) now respect the current endianness based on the processor mode (system/user) and the corresponding status register bit.

## 2. Executable Format Support

### Supported Formats
1. **RAW** - Raw binary (original)
2. **Mach-O** - Darwin/macOS executable format
3. **PE/COFF** - Windows executable format

### Auto-Detection
The simulator automatically detects the executable format based on magic numbers:
- Mach-O: `0xFEEDFACE` (32-bit) or `0xFEEDFACF` (64-bit)
- PE: `MZ` signature
- RAW: Default if no magic number matches

### Command-Line Usage
```bash
# Automatic format detection
./dlxsim -b program.macho
./dlxsim -b program.exe
./dlxsim -b program.bin
```

## 3. DLX64 Architecture

### Overview
DLX64 extends the DLX architecture to 64-bit:
- 64-bit general-purpose registers (r0-r31)
- 64-bit floating-point registers (f0-f31)  
- 64-bit address space
- 64-bit PC and addressing modes
- Backward compatible with DLX32

### New Instructions
- `addd`, `subd`, `multd`, `divd` - 64-bit integer arithmetic
- `ldw` - Load double-word (64-bit)
- `sdw` - Store double-word (64-bit)
- Extended immediate modes

### Status
DLX64 support is PLANNED for future release.

## 4. Compressed Instructions (DLX-C)

### Overview
16-bit compressed instruction format for code density:
- Most common instructions encoded in 16 bits
- Intermixed with 32-bit instructions
- ~30% code size reduction

### Encoding
```
15  13  12  10  9   7  6   4  3   0
+------+------+-----+-----+-----+
|  op  |  rs  | rd  | imm | func |
+------+------+-----+-----+-----+
```

### Common Compressed Instructions
- `c.addi` - Add immediate
- `c.lw` / `c.sw` - Load/store word
- `c.jr` - Jump register
- `c.beqz` - Branch if equal zero
- `c.li` - Load immediate

### Status  
Compressed instruction support is PLANNED for future release.

## 5. Enhanced FPU

### 64-bit FP Registers
Floating-point registers updated to true 64-bit:
- Native double-precision operations
- IEEE 754 compliance
- Rounding modes
- Exception handling (overflow, underflow, invalid, divide-by-zero)

### New FP Instructions
- Full IEEE 754 operation set
- Fused multiply-add (FMA)
- Square root, reciprocal
- Conversions between all types

### Status
Enhanced FPU is PARTIALLY IMPLEMENTED (architecture defined, execution pending).

## Implementation Status

| Feature | Status | Notes |
|---------|--------|-------|
| Endianness Control | ✅ COMPLETE | System and user mode independent control |
| Mach-O Loading | ✅ COMPLETE | Basic loader for Mach-O executables |
| PE/COFF Loading | ✅ COMPLETE | Basic loader for PE/COFF executables |
| Format Auto-Detection | ✅ COMPLETE | Automatic format recognition |
| DLX64 Architecture | 🔄 PLANNED | 64-bit extension |
| Compressed Instructions | 🔄 PLANNED | 16-bit encoding |
| Enhanced FPU | 🔄 PARTIAL | Architecture ready, execution TBD |

## Usage Examples

### Endianness Switching
```asm
; Start in big-endian system mode
lhi  r1, 0xFFF0         ; r1 = 0xFFF00000
ori  r1, r1, 0x1234     ; r1 = 0xFFF01234

; Switch to little-endian user mode
movs2i r2, r0           ; Get current status
andi r2, r2, 0xFFF7     ; Clear STATUS_USR_BE
movi2s r0, r2           ; Update status

; Now in user mode, loads/stores are little-endian
lw   r3, 0(r1)          ; Load with LE byte order
```

### Loading Different Formats
```bash
# Load Mach-O executable
./dlxsim -v -b kernel.macho

# Load PE executable  
./dlxsim -v -b program.exe

# Load raw binary at specific address
./dlxsim -v -b firmware.bin -a 0xFFFF0000
```

## Future Work

1. **Complete DLX64**: Full 64-bit instruction execution
2. **Compressed Instructions**: 16-bit encoding/decoding
3. **Enhanced FPU**: IEEE 754 execution engine
4. **MMU Enhancements**: Full page table and TLB implementation
5. **Cache Simulation**: L1/L2 cache modeling
6. **Pipeline Model**: Accurate cycle timing
7. **GDB Support**: Remote debugging protocol

## Documentation Updates

See updated README.md for:
- Complete instruction reference
- Endianness programming guide
- Executable format specifications
- DLX64 architecture details
- Compressed instruction encoding

## Compatibility

- **Backward Compatible**: All existing DLX32 code runs unchanged
- **Default Mode**: Big-endian system mode (original DLX behavior)
- **Format Support**: RAW binaries work as before
- **API Stable**: No breaking changes to simulator interface

---

**Version**: 2.0  
**Status**: Enhanced with endianness and multi-format support  
**Date**: October 2024
