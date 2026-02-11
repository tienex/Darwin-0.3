# DLXSIM - DLX RISC Processor Simulator
## Table of Contents - Version 1.0

**C-based Software Simulator for DLX Architecture**

---

## Chapters

- [Chapter 01: Overview](Chapter01_Overview.md)
  - Introduction to DLXSIM
  - Feature Summary
  - Capabilities

- [Chapter 02: Architecture](Chapter02_Architecture.md)
  - DLX Processor Specification
  - Register Set (GPR, FPR)
  - Memory Map
  - Instruction Set Summary

- [Chapter 03: Building](Chapter03_Building.md)
  - Prerequisites
  - Compilation
  - Installation
  - Build Targets

- [Chapter 04: Usage](Chapter04_Usage.md)
  - Basic Execution
  - Command-Line Options
  - Example Sessions
  - Exit Codes

- [Chapter 05: Programming](Chapter05_Programming.md)
  - Assembly Programs
  - System Calls
  - Function Calling Convention
  - Memory-Mapped I/O

- [Chapter 06: Debugging](Chapter06_Debugging.md)
  - Instruction Tracing
  - Register Inspection
  - Memory Inspection
  - Breakpoints

- [Chapter 07: Architecture Details](Chapter07_Architecture_Details.md)
  - Instruction Formats (R-Type, I-Type, J-Type)
  - Exception Vectors
  - Status Register
  - Cause Register
  - Endianness

- [Chapter 08: Implementation Details](Chapter08_Implementation_Details.md)
  - Source Files
  - Key Data Structures
  - Execution Loop
  - Memory Subsystem
  - Instruction Decoder

- [Chapter 09: Performance, Testing, and Integration](Chapter09_Performance_Testing_Integration.md)
  - Performance Characteristics
  - Testing Procedures
  - Darwin Integration

- [Chapter 10: Future and Support](Chapter10_Future_and_Support.md)
  - Future Enhancements
  - Known Limitations
  - Troubleshooting
  - References
  - License and Authors
  - Support

---

## Quick Reference

### Source Files
- `src/dlx.h` - Architecture definitions (365 lines)
- `src/dlxsim.c` - Main simulator (487 lines)
- `src/memory.c` - Memory subsystem (187 lines)
- `src/device.c` - I/O devices (68 lines)
- `Makefile` - Build system (65 lines)

**Total**: ~1,172 lines of C code

### Key Features
- Complete DLX instruction set
- 16MB simulated memory
- Memory-mapped I/O (console, timer, keyboard)
- Exception handling
- Basic MMU simulation
- Instruction tracing
- Cycle-accurate timing

### Performance
- 10-50 million DLX instructions/second (modern CPU)
- Suitable for OS kernel development
- Cycle-accurate timing

---

**Version**: 1.0
**Last Updated**: October 2024
**Status**: Complete and Tested
**Platform**: DLX32 (32-bit architecture)
