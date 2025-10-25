# Chapter 2: Architecture

## 2.1 DLX Processor Specification

### General-Purpose Registers

**32 general-purpose registers** (r0-r31)
- r0: Always zero (hardwired)
- r1-r8: Temporaries and arguments
- r9-r28: Saved registers
- r29: Stack pointer (SP)
- r30: Frame pointer (FP)
- r31: Return address (RA)

### Floating-Point Registers

**32 floating-point registers** (f0-f31)

### Architecture Details

- **32-bit architecture**
  - 32-bit data path
  - 32-bit address space (4GB)
  - Big-endian byte order

- **Load/store architecture**
  - Only load/store instructions access memory
  - All arithmetic/logical operations on registers

## 2.2 Memory Map

```
0x00000000 - 0x00FFFFFF   Main memory (16MB)
0xFFF00000 - 0xFFF00003   Console output
0xFFF00004 - 0xFFF00007   Simulator control
0xFFF00010 - 0xFFF00013   Timer register
0xFFF00100 - 0xFFF00103   Keyboard data
0xFFF00104 - 0xFFF00107   Keyboard status
```

## 2.3 Instruction Set

### Arithmetic
- `add`, `addu`, `sub`, `subu` - Addition and subtraction
- `mult`, `multu`, `div`, `divu` - Multiplication and division
- `addi`, `addui`, `subi`, `subui` - Immediate forms

### Logical
- `and`, `or`, `xor` - Bitwise operations
- `andi`, `ori`, `xori` - Immediate forms
- `lhi` - Load high immediate

### Shifts
- `sll`, `srl`, `sra` - Shift left/right logical/arithmetic
- `slli`, `srli`, `srai` - Immediate forms

### Comparison
- `seq`, `sne`, `slt`, `sgt`, `sle`, `sge` - Set on condition
- `seqi`, `snei`, `slti`, `sgti`, `slei`, `sgei` - Immediate forms

### Load/Store
- `lw`, `lh`, `lb` - Load word/halfword/byte (sign-extended)
- `lhu`, `lbu` - Load unsigned
- `sw`, `sh`, `sb` - Store word/halfword/byte

### Branches
- `beqz`, `bnez` - Branch if equal/not equal to zero

### Jumps
- `j` - Jump to address
- `jal` - Jump and link (function call)
- `jr` - Jump register (return)
- `jalr` - Jump and link register

### System
- `trap` - System call
- `rfe` - Return from exception
- `movi2s`, `movs2i` - Move to/from status register

### Floating-point (OP_FPARITH)
- `addf`, `subf`, `multf`, `divf` - Single precision
- `addd`, `subd`, `multd`, `divd` - Double precision
- `cvt*` - Conversions between types
- `eqf`, `ltf`, etc. - Floating-point comparisons
