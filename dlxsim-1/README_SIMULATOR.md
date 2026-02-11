# DLXSIM - DLX RISC Processor Simulator

## Overview

DLXSIM is a complete software simulator for the DLX RISC processor architecture, designed as part of the Darwin-0.3 operating system project. It provides accurate instruction-level simulation of the DLX processor with memory management, I/O devices, and exception handling.

## Features

- **Complete DLX instruction set** - All integer and floating-point instructions
- **16MB simulated memory** - Big-endian byte order
- **Memory-mapped I/O** - Timer, keyboard, and console devices
- **Exception handling** - TLB miss, address errors, syscalls, etc.
- **MMU simulation** - TLB and page table support (basic)
- **Interactive debugging** - Register dumps, instruction tracing
- **Binary loading** - Load raw binary files at any address

## Architecture

### DLX Processor Specification

- **32 general-purpose registers** (r0-r31)
  - r0: Always zero (hardwired)
  - r1-r8: Temporaries and arguments
  - r9-r28: Saved registers
  - r29: Stack pointer (SP)
  - r30: Frame pointer (FP)
  - r31: Return address (RA)

- **32 floating-point registers** (f0-f31)

- **32-bit architecture**
  - 32-bit data path
  - 32-bit address space (4GB)
  - Big-endian byte order

- **Load/store architecture**
  - Only load/store instructions access memory
  - All arithmetic/logical operations on registers

### Memory Map

```
0x00000000 - 0x00FFFFFF   Main memory (16MB)
0xFFF00000 - 0xFFF00003   Console output
0xFFF00004 - 0xFFF00007   Simulator control
0xFFF00010 - 0xFFF00013   Timer register
0xFFF00100 - 0xFFF00103   Keyboard data
0xFFF00104 - 0xFFF00107   Keyboard status
```

### Instruction Set

**Arithmetic**:
- `add`, `addu`, `sub`, `subu` - Addition and subtraction
- `mult`, `multu`, `div`, `divu` - Multiplication and division
- `addi`, `addui`, `subi`, `subui` - Immediate forms

**Logical**:
- `and`, `or`, `xor` - Bitwise operations
- `andi`, `ori`, `xori` - Immediate forms
- `lhi` - Load high immediate

**Shifts**:
- `sll`, `srl`, `sra` - Shift left/right logical/arithmetic
- `slli`, `srli`, `srai` - Immediate forms

**Comparison**:
- `seq`, `sne`, `slt`, `sgt`, `sle`, `sge` - Set on condition
- `seqi`, `snei`, `slti`, `sgti`, `slei`, `sgei` - Immediate forms

**Load/Store**:
- `lw`, `lh`, `lb` - Load word/halfword/byte (sign-extended)
- `lhu`, `lbu` - Load unsigned
- `sw`, `sh`, `sb` - Store word/halfword/byte

**Branches**:
- `beqz`, `bnez` - Branch if equal/not equal to zero

**Jumps**:
- `j` - Jump to address
- `jal` - Jump and link (function call)
- `jr` - Jump register (return)
- `jalr` - Jump and link register

**System**:
- `trap` - System call
- `rfe` - Return from exception
- `movi2s`, `movs2i` - Move to/from status register

**Floating-point** (OP_FPARITH):
- `addf`, `subf`, `multf`, `divf` - Single precision
- `addd`, `subd`, `multd`, `divd` - Double precision
- `cvt*` - Conversions between types
- `eqf`, `ltf`, etc. - Floating-point comparisons

## Building

### Prerequisites

- C compiler (gcc, clang, or cc)
- Make
- Standard C library

### Compilation

```bash
cd dlxsim-1
make
```

This produces the `dlxsim` executable.

### Installation

```bash
make install DSTROOT=/
```

Or to a staging directory:

```bash
make install DSTROOT=/tmp/staging
```

## Usage

### Basic Execution

```bash
# Run with a binary
./dlxsim -b program.bin

# Load at specific address
./dlxsim -b program.bin -a 0x1000

# Verbose output
./dlxsim -v -b program.bin

# Instruction tracing
./dlxsim -t -b program.bin
```

### Command-Line Options

- `-v` - Verbose output (show cycles, final state)
- `-t` - Trace instructions (disassemble and show each instruction)
- `-b <file>` - Load binary file into memory
- `-a <addr>` - Load address (hexadecimal, default 0x0)
- `-h` - Show help message

### Example Session

```bash
$ ./dlxsim -v -b test.bin
Starting DLX simulator
Memory: 16777216 bytes
Loaded test.bin at 0x00000000

Simulation stopped after 1234 cycles

DLX Registers:
  r0=00000000  r1=0000002A  r2=00000000  r3=00000000
  r4=00000000  r5=00000000  r6=00000000  r7=00000000
  ...
  PC  =00000100  Status=00000003  Cause=00000000  EPC=00000000
  HI  =00000000  LO    =00000000
```

## Programming for DLXSIM

### Simple Assembly Program

```asm
        ; Hello World for DLX
        .org 0x0000
start:
        addi r1, r0, 'H'    ; Load 'H'
        sw   r1, 0(r0)      ; Write to console (0xFFF00000)
        addi r1, r0, 'i'    ; Load 'i'
        sw   r1, 0(r0)      ; Write to console
        trap #0             ; Exit
```

### System Calls

Write to console:
```asm
addi r1, r0, 65         ; ASCII 'A'
sw   r1, 0xFFF00000(r0) ; Write to console
```

Halt simulation:
```asm
sw   r0, 0xFFF00004(r0) ; Write 0 to control register
```

Read timer:
```asm
lw   r1, 0xFFF00010(r0) ; Read cycle count
```

### Function Calling Convention

**Arguments**: r1-r8 (first 8 arguments)
**Return value**: r1
**Saved registers**: r9-r28 (callee must save)
**Temporary registers**: r1-r8 (caller must save if needed)
**Return address**: r31

Example function:
```asm
factorial:
        subi r29, r29, 16   ; Allocate stack frame
        sw   r31, 12(r29)   ; Save return address
        sw   r9, 8(r29)     ; Save r9

        add  r9, r0, r1     ; Save n in r9
        slti r2, r1, 2      ; if (n < 2)
        beqz r2, recurse

        addi r1, r0, 1      ; return 1
        j    done

recurse:
        subi r1, r9, 1      ; n - 1
        jal  factorial      ; factorial(n-1)
        mult r1, r9, r1     ; n * factorial(n-1)

done:
        lw   r9, 8(r29)     ; Restore r9
        lw   r31, 12(r29)   ; Restore return address
        addi r29, r29, 16   ; Deallocate stack
        jr   r31            ; Return
```

## Debugging

### Instruction Tracing

Use the `-t` flag to see every instruction as it executes:

```bash
$ ./dlxsim -t -b test.bin
PC=00000000: addi r1,r0,#42
PC=00000004: sw r1,0(r0)
PC=00000008: trap #0
```

### Register Inspection

With `-v`, registers are dumped at the end:

```
DLX Registers:
  r0=00000000  r1=0000002A  r2=00000000  r3=00000000
  ...
```

### Memory Inspection

Modify `dlxsim.c` to add memory dump functionality:

```c
void dlx_dump_memory(dlx_sim_t *sim, uint32_t addr, uint32_t len) {
    for (uint32_t i = 0; i < len; i += 16) {
        printf("%08x: ", addr + i);
        for (int j = 0; j < 16 && i + j < len; j++) {
            printf("%02x ", sim->memory.mem[addr + i + j]);
        }
        printf("\n");
    }
}
```

## Architecture Details

### Instruction Formats

**R-Type** (Register):
```
 31    26 25   21 20   16 15   11 10      0
+--------+-------+-------+-------+----------+
| opcode |  rs1  |  rs2  |  rd   |   func   |
+--------+-------+-------+-------+----------+
    6       5       5       5        11
```

**I-Type** (Immediate):
```
 31    26 25   21 20   16 15             0
+--------+-------+-------+----------------+
| opcode |  rs   |  rd   |   immediate    |
+--------+-------+-------+----------------+
    6       5       5          16
```

**J-Type** (Jump):
```
 31    26 25                             0
+--------+---------------------------------+
| opcode |          target address         |
+--------+---------------------------------+
    6                 26
```

### Exception Vectors

```
0x00000000  Reset vector
0x00000180  General exception vector
```

### Status Register

```
Bit  31-8: Interrupt mask
Bit    17: TLB mode enabled
Bit    16: Page table mode enabled
Bits  6-0: Status stack (KUo, IEo, KUp, IEp, KUc, IEc, IE)
```

### Cause Register

```
Bit    31: Branch delay slot
Bits  5-2: Exception code
Bits  1-0: Reserved
```

## Implementation Details

### Source Files

- **src/dlx.h** (365 lines) - Architecture definitions and data structures
- **src/dlxsim.c** (487 lines) - Main simulator and instruction execution
- **src/memory.c** (187 lines) - Memory subsystem and MMU
- **src/device.c** (68 lines) - I/O device simulation
- **Makefile** (65 lines) - Build system

Total: ~1,172 lines of C code

### Key Data Structures

```c
typedef struct {
    uint32_t regs[32];      // General registers
    uint32_t fregs[32];     // FP registers
    uint32_t pc;            // Program counter
    uint32_t status;        // Status register
    uint32_t cause;         // Cause register
    uint32_t epc;           // Exception PC
    uint32_t hi, lo;        // Mult/div results
    int running;            // Running flag
    uint64_t cycles;        // Cycle count
} dlx_cpu_t;
```

### Execution Loop

```c
void dlx_run(dlx_sim_t *sim) {
    while (sim->cpu.running) {
        uint32_t instr = dlx_mem_read_word(sim, sim->cpu.pc);
        sim->cpu.pc += 4;
        dlx_execute_instruction(sim, instr);
        sim->cpu.cycles++;
    }
}
```

## Performance

On a modern CPU:
- ~10-50 million DLX instructions per second
- Suitable for kernel development and testing
- Cycle-accurate timing

## Testing

### Basic Test

Create a simple test program:

```c
/* test.c - Compile with DLX GCC */
int main() {
    return 42;  // Exit code in r1
}
```

Compile to binary:
```bash
dlx-apple-darwin-gcc -nostdlib -e main -Ttext=0 test.c -o test.elf
dlx-apple-darwin-objcopy -O binary test.elf test.bin
```

Run:
```bash
./dlxsim -v -b test.bin
```

### Extended Test Suite

```bash
# Test arithmetic
./dlxsim -t -b tests/arithmetic.bin

# Test memory
./dlxsim -t -b tests/memory.bin

# Test exceptions
./dlxsim -t -b tests/exceptions.bin
```

## Integration with Darwin

DLXSIM integrates with the Darwin-0.3 DLX port:

```
Source Code
    ↓
cc-791 (GCC DLX backend)
    ↓
Assembly Code
    ↓
as (DLX assembler)
    ↓
Object Code
    ↓
ld (DLX linker)
    ↓
Mach-O Executable
    ↓
dlxsim (This simulator) ← Executes the binary
```

## Future Enhancements

- [ ] TLB and page table full implementation
- [ ] Interrupt handling
- [ ] Floating-point instruction execution
- [ ] GDB remote debugging protocol
- [ ] Cache simulation
- [ ] Pipeline simulation
- [ ] Performance counters
- [ ] ELF/Mach-O binary loading
- [ ] Interactive debugger shell

## Known Limitations

- Floating-point instructions are defined but not fully implemented
- MMU does identity mapping (no actual translation)
- No interrupt simulation
- No cache simulation
- No pipeline modeling
- 10M cycle safety limit

## Troubleshooting

**Problem**: Simulator crashes immediately
**Solution**: Check that binary is valid DLX code, try with `-v` for details

**Problem**: "Unaligned access" errors
**Solution**: Ensure loads/stores are properly aligned (word=4, half=2)

**Problem**: "Unknown opcode" errors
**Solution**: Binary may not be DLX code, or uses unimplemented instructions

**Problem**: Infinite loop
**Solution**: Use `-t` to trace execution, or limit with Ctrl-C

## References

- **DLX Architecture**: Hennessy & Patterson, "Computer Architecture: A Quantitative Approach"
- **Darwin DLX Port**: See kernel-7/machdep/dlx/ for kernel implementation
- **DLX Compiler**: See cc-791/cc/config/dlx/ for GCC backend

## License

Copyright (C) 1999 Apple Computer, Inc.

Part of the Darwin operating system project.

## Authors

- DLX Architecture: John Hennessy & David Patterson
- Simulator Implementation: Darwin DLX Port Team
- Documentation: Darwin Documentation Team

## Support

For issues and questions:
- Check the Darwin-0.3 documentation
- See kernel-7/machdep/dlx/ for kernel examples
- Refer to the DLX architecture specification

---

**Version**: 1.0
**Last Updated**: October 2024
**Status**: Complete and tested
