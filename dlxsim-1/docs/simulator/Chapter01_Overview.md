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
