# VOLUME I: BASE ARCHITECTURE

# Chapter 1: Introduction

## 1.1 Overview

The DLX architecture is a modern RISC processor family supporting:

- **32-bit** (DLX32) - Embedded and legacy systems
- **64-bit** (DLX64) - General-purpose computing (primary focus)
- **128-bit** (DLX128) - Future exascale and AI/ML workloads

### Key Design Principles

1. **Simplicity**: Load/store architecture with orthogonal instructions
2. **Extensibility**: Modular extensions that compose cleanly
3. **Performance**: Designed for deep pipelines and out-of-order execution
4. **Security**: Hardware-enforced protection (CHERI, enclaves)
5. **Compatibility**: Binary compatibility across implementations

## 1.2 Feature Summary

### Base ISA (All Implementations)
- 32 general-purpose registers (64-bit in DLX64)
- Load/store architecture
- Integer arithmetic and logical operations
- Branch and jump instructions
- System calls and exceptions

### Standard Extensions (Common)
- **F**: Single-precision floating-point
- **D**: Double-precision floating-point
- **Q**: Quad-precision floating-point
- **V**: Vector operations (RV-V compatible)
- **B**: Bit manipulation
- **A**: Atomic instructions
- **C**: Compressed instructions (16-bit)

### Advanced Extensions (Optional)
- **H**: Hypervisor support with nested virtualization
- **K**: Cryptography (AES, SHA, RSA, ECC)
- **M**: Matrix operations for AI/ML
- **N**: Neural network acceleration
- **X**: CHERI capabilities
- **E**: Secure enclaves with memory encryption
- **T**: High-precision timers and PMU
- **P**: RDMA and high-performance networking
- **I**: IOMMU and SR-IOV
- **R**: Register banking (4 modes)
- **S**: Single-level storage
- **Q**: Quantum computing interface

### DLX-Specific Features
- **4-Ring Protection**: Fine-grained privilege (Ring 0-3)
- **Register Banking**: Hardware context switching
- **Endian Switching**: Per-level byte order control
- **Multiple Page Sizes**: 4KB to 16GB pages
- **Moxie Compatibility**: Run Moxie binaries natively
