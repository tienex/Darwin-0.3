# MMIX Emulator Specification for Darwin

## Overview

This document specifies the requirements for an MMIX emulator to successfully run Darwin-0.3. The emulator must provide hardware simulation, memory-mapped devices, and proper interrupt handling.

## Processor Requirements

### Architecture
- **CPU**: MMIX 64-bit RISC
- **Endianness**: Big-endian
- **Word Size**: 64 bits (8 bytes)
- **Instruction Size**: 32 bits (4 bytes)
- **Registers**: 256 general-purpose ($0-$255) + 32 special (rA-rZZ)

### Special Registers

Critical special registers that must be implemented:

| Register | Number | Purpose |
|----------|--------|---------|
| rA | 21 | Arithmetic status |
| rB | 0 | Bootstrap register (trap base) |
| rG | 19 | Global threshold |
| rI | 12 | Interval counter (timer) |
| rJ | 4 | Return jump address |
| rK | 15 | Interrupt mask |
| rL | 20 | Local threshold |
| rQ | 16 | Interrupt request |
| rT | 13 | Trap address |
| rV | 18 | Virtual translation |
| rW | 24 | Where-interrupted |
| rX | 25 | Execution register |
| rY | 26 | Y operand |
| rZ | 27 | Z operand |

### Instruction Set

Must support all MMIX instructions as defined in Knuth's specification:
- Arithmetic: ADD, SUB, MUL, DIV, etc.
- Logic: AND, OR, XOR, etc.
- Memory: LDB, LDW, LDT, LDO, STB, STW, STT, STO
- Control: JMP, BZ, BN, BP, PUSHJ, POP, etc.
- Special: GET, PUT, TRAP, SYNC

## Memory Layout

### Physical Memory

```
0x0000000000000000 - 0x0000000000000FFF: Trap vectors (4 KB)
0x0000000000001000 - 0x000000000000FFFF: Bootloader (60 KB)
0x0000000000010000 - 0x000000000FFFFFFF: System RAM (256 MB - 64 KB)
0xFFFFFFFF00000000 - 0xFFFFFFFFFFFFFFFF: Device space (4 GB)
```

### Virtual Memory

Support for:
- **Page Size**: 8192 bytes (8 KB)
- **Page Table Depth**: 2-level or 3-level
- **TLB Size**: Minimum 256 entries
- **Address Space**: Full 64-bit (2^64 bytes)

### Memory Protection

- Read/Write/Execute permissions per page
- User/Supervisor mode
- No-execute (NX) bit support

## Device Memory Map

All devices are memory-mapped in the high address range:

### Console Device
**Base Address**: 0xFFFFFFFF00000000

| Offset | Size | Access | Description |
|--------|------|--------|-------------|
| 0x00 | 8 bytes | W | Output character (bits 0-7) |
| 0x08 | 8 bytes | R | Input character (0 if none, bits 0-7) |
| 0x10 | 8 bytes | R | Status (bit 0: input ready) |

**Behavior**:
- Writing to 0x00 outputs character to console
- Reading from 0x08 returns next input character or 0
- Bit 0 of status indicates if input is available

### Disk Device
**Base Address**: 0xFFFFFFFF00001000

| Offset | Size | Access | Description |
|--------|------|--------|-------------|
| 0x0000 | 8 bytes | R/W | Sector number |
| 0x0008 | 8192 bytes | R/W | Data buffer |
| 0x2008 | 8 bytes | W | Command (1=read, 2=write) |
| 0x2010 | 8 bytes | R | Status (0=ready, 1=busy, 2=error) |

**Sector Size**: 8192 bytes (8 KB)
**Minimum Capacity**: 1 GB (131072 sectors)

**Operation**:
1. Write sector number to offset 0x0000
2. For write: Copy data to buffer at 0x0008
3. Write command to 0x2008 (1=read, 2=write)
4. Poll status at 0x2010 until not busy
5. For read: Copy data from buffer at 0x0008

### Network Device (Optional)
**Base Address**: 0xFFFFFFFF00002000

| Offset | Size | Access | Description |
|--------|------|--------|-------------|
| 0x0000 | 8 bytes | R/W | Packet length |
| 0x0008 | 2048 bytes | R/W | Packet buffer |
| 0x0808 | 8 bytes | W | Command (1=send, 2=recv) |
| 0x0810 | 8 bytes | R | Status |
| 0x0818 | 6 bytes | R | MAC address |

### Timer Device
**Implementation**: Use rI special register
**Frequency**: 1 GHz (1 tick = 1 nanosecond)

**Interrupt**: When rI reaches 0, raise interrupt bit 12 in rQ

## Interrupt System

### Interrupt Vector

Interrupts are handled via the TRAP mechanism:

1. Save PC to rW
2. Save current instruction to rX
3. Save rK to rY
4. Set PC to value in rT + (interrupt_number * 4)
5. Clear interrupt enable in rK

### Interrupt Sources

| Bit in rQ | Source | Priority |
|-----------|--------|----------|
| 0 | Power failure | Highest |
| 1 | Memory parity error | |
| 4 | External device 0 | |
| 8 | External device 1 | |
| 12 | Timer (rI underflow) | |
| 16 | Console input | |
| 24 | Network | Lowest |

### Interrupt Mask (rK)

Each bit in rK corresponds to an interrupt source. If bit is 1, that interrupt is enabled.

## Boot Sequence

1. **Power-On Reset**:
   - PC = 0x0000000000001000 (bootloader entry)
   - All registers = 0
   - Interrupts disabled (rK = 0)
   - $0 = physical memory size
   - $1 = bootloader size
   - $255 = emulator signature (0x4D4D4958)

2. **Bootloader Execution**:
   - Initialize console
   - Initialize disk
   - Set up basic page tables
   - Load kernel from disk
   - Transfer control to kernel

3. **Kernel Entry**:
   - PC = kernel entry point (typically 0x8000000000100000)
   - $0 = pointer to boot_args structure

## Emulator Command Line

Suggested command-line interface:

```bash
mmix-emulator [options] <bootloader.bin>

Options:
  -m, --memory SIZE     Physical memory size (default: 256M)
  -d, --disk IMAGE      Disk image file
  -n, --network TYPE    Network type (user, tap, none)
  -c, --console TYPE    Console type (stdio, pty, tcp)
  -g, --gdb PORT        Enable GDB remote debugging on PORT
  -v, --verbose         Verbose output
  -t, --trace           Instruction trace
      --debug-devices   Debug device I/O
```

Example:
```bash
mmix-emulator -m 256M -d darwin.img -c stdio bootloader.bin
```

## Debugging Support

### GDB Remote Protocol

Support GDB remote serial protocol for debugging:
- Read/write registers
- Read/write memory
- Set breakpoints
- Single-step execution
- Continue execution

### Trace Output

When `-t` flag is enabled, output each instruction:
```
[0x0000000000001000] SETL $254,#0000  ; $254 = 0x0000000000000000
[0x0000000000001004] ORMH $254,#0000  ; $254 = 0x0000000000000000
[0x0000000000001008] ORML $254,#0010  ; $254 = 0x0000000000100000
```

### Device Debugging

When `--debug-devices` is enabled, log all device I/O:
```
[CONSOLE] Write: 'D' (0x44) to 0xFFFFFFFF00000000
[DISK] Write sector 100 command at 0xFFFFFFFF00001008
[DISK] Status: busy -> ready
```

## Performance Targets

- **Instruction throughput**: Minimum 10 MIPS
- **Disk I/O**: Minimum 10 MB/sec
- **Console I/O**: 9600 baud equivalent minimum
- **Timer accuracy**: ±1% frequency deviation

## Compatibility Testing

The emulator must successfully:

1. Boot the Darwin bootloader
2. Load and execute the Darwin kernel
3. Complete kernel initialization
4. Mount root filesystem from virtual disk
5. Run init process
6. Execute user-space programs

Test with:
```bash
# Boot Darwin
mmix-emulator -d darwin_root.img bootloader.bin

# Expected output:
# Darwin MMIX Bootloader v1.0
# Initializing disk...
# Initializing MMU...
# Loading kernel: /mach_kernel
# Kernel loaded successfully
# Transferring control to kernel...
# Darwin/MMIX booting...
# [kernel messages]
# login:
```

## Error Handling

### Undefined Instructions

- Raise TRAP 0 (illegal instruction)
- Set rX to undefined instruction
- Set rY to instruction address

### Memory Access Violations

- Raise TRAP 2 (memory access violation)
- Set rY to fault address
- Set rZ to access type (read=0, write=1, execute=2)

### Device Errors

- Disk: Return error status (2) in status register
- Console: Ignore writes, return 0 for reads
- Network: Drop packets

## Reference Implementation

A reference MMIX emulator implementation is available at:
```
https://github.com/mmix-emulator/darwin-mmix
```

Or adapt existing MMIX simulators like `mmix-sim` from MMIXware.

## See Also

- MMIX Architecture Specification (Donald Knuth)
- Darwin Boot Process Documentation
- MMIX-ARCHITECTURE.md (this repository)
- Bootloader README (boot-2/mmix/README.md)
- Driver README (kernel-7/driverkit/mmix/README.md)
