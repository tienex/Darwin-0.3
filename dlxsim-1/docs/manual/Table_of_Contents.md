# DLX Complete Architecture Manual
## Table of Contents - Version 3.0

**A Comprehensive 32/64/128-bit RISC Architecture with Advanced Extensions**

---

## Front Matter
- [Chapter 00: Front Matter](Chapter00_Front_Matter.md)

## VOLUME I: BASE ARCHITECTURE

### Core ISA and System
- [Chapter 01: Introduction](Chapter01_Introduction.md)
  - Overview, Design Principles, Feature Summary
  - DLX32, DLX64, DLX128 variants
  - Extension nomenclature (F, D, Q, V, B, A, C, H, K, M, N, X, E, T, P, I, R, S, Q)

- [Chapter 02: Instruction Set Architecture](Chapter02_Instruction_Set_Architecture.md)
  - Instruction naming convention
  - Load/Store, Integer Arithmetic, Logical Operations
  - Comparison, Branches, Jumps
  - Floating-Point, Atomic, System Instructions
  - Cache Control

- [Chapter 03: Registers and State](Chapter03_Registers_and_State.md)
  - General-Purpose Registers (32 × 64-bit)
  - Floating-Point Registers (32 × 64-bit)
  - Vector Registers (32 × VLEN-bit)
  - Matrix Registers (8 × tile)
  - Control and Status Registers (CSRs)

- [Chapter 04: Memory Architecture](Chapter04_Memory_Architecture.md)
  - Address Spaces (DLX64, DLX128)
  - Endianness control
  - Page-Based Virtual Memory
  - VMS-Style Page Protection
  - 4-Ring Protection Model
  - Register Banking

### Binary Formats and ABI
- [Chapter 05: Binary Formats](Chapter05_Binary_Formats.md)
  - ELF (EM_DLX = 0x9999)
  - PE/COFF (IMAGE_FILE_MACHINE_DLX)
  - Mach-O (CPU_TYPE_DLX)
  - DLX Relocation Types
  - Memory Layouts

- [Chapter 06: ABI and Calling Conventions](Chapter06_ABI_and_Calling_Conventions.md)
  - Data Types (int8-int64, float32-float128, BFloat16)
  - Register Usage (calling convention)
  - Stack Frame Layout
  - Function Prologue/Epilogue
  - Position-Independent Code (PIC)
  - Thread-Local Storage (TLS)
  - Register Banking Mode

- [Chapter 07: System Programming](Chapter07_System_Programming.md)
  - Exception Handling
  - System Calls
  - Dynamic Linking (PLT/GOT)
  - Interrupt Handling
  - Context Switching
  - Memory Management

## VOLUME II: STANDARD EXTENSIONS

### Vector and SIMD
- [Chapter 08: Vector Extension (V)](Chapter08_Vector_Extension.md)
  - Vector Configuration (VLEN, SEW, LMUL)
  - Vector Arithmetic (add, sub, mul, div)
  - Widening Operations
  - Reduction Operations
  - Vector Load/Store

### Bit Manipulation and Floating-Point
- [Chapter 09: Bit Manipulation Extension (B)](Chapter09_Bit_Manipulation_Extension.md)
  - Count Operations (clz, ctz, pcnt)
  - Rotate Operations (rol, ror)
  - Byte Operations (rev8, rev16, orc.b)
  - Single-bit Operations (bset, bclr, binv, bext)
  - Logic with Complement (andn, orn, xnor)
  - Min/Max, Sign/Zero Extension

### Cryptography and Compression
- [Chapter 10: Cryptography Extension (K)](Chapter10_Cryptography_Extension.md)
  - AES Instructions (aesenc, aesdec, aesgcm)
  - SHA Instructions (SHA-256, SHA-512, SHA-3)
  - RSA and ECC (modexp, pointadd, pointmul)

- [Chapter 11: CRC and Compression](Chapter11_CRC_and_Compression.md)
  - CRC Instructions (CRC-32, CRC-32C)
  - LZ77 Compression
  - Huffman Encoding
  - Dictionary Compression

- [Chapter 12: Floating-Point Extension (F/D/Q)](Chapter12_Floating_Point_Extension.md)
  - Single Precision (F extension)
  - Double Precision (D extension)
  - Quad Precision (Q extension)
  - Half Precision and BFloat16
  - FMA, Min/Max, Conversions
  - Denormal and NaN Handling

- [Chapter 13: Atomic Operations Extension (A)](Chapter13_Atomic_Operations_Extension.md)
  - Load-Reserved / Store-Conditional (LR/SC)
  - Atomic Memory Operations (AMO)
  - Compare-and-Swap (CAS)
  - Memory Barriers (fence)
  - Synchronization Primitives
  - Lock-Free Data Structures

- [Chapter 14: Compressed Instructions Extension (C)](Chapter14_Compressed_Instructions.md)
  - 16-bit Instruction Encodings
  - Compressed Integer, FP, Load/Store
  - Register Encoding (3-bit specifiers)
  - Code Size Reduction (25-30%)

## VOLUME III: ADVANCED FEATURES

### Security and Protection
- [Chapter 15: CHERI Capabilities (X Extension)](Chapter15_CHERI_Capabilities.md)
  - Capability Inspection (cgetbase, cgetlen, cgetperm)
  - Capability Modification (csetaddr, csetbounds, cseal)
  - Capability Load/Store
  - Capability Control Flow

- [Chapter 16: Secure Enclaves (E Extension)](Chapter16_Secure_Enclaves.md)
  - Enclave Lifecycle (ecreate, eenter, eexit)
  - Enclave Memory Management
  - Attestation and Sealing

- [Chapter 17: Nested Virtualization (H Extension)](Chapter17_Nested_Virtualization.md)
  - VMX Operations (vmxon, vmlaunch, vmresume)
  - Extended Page Tables (EPT)
  - Nested Virtualization (L0/L1/L2)

- [Chapter 21: Advanced Security Features](Chapter21_Advanced_Security_Features.md)
  - Memory Tagging Extension (MTE)
  - Control-Flow Integrity (CFI)
  - Pointer Authentication (PAC)
  - Memory Encryption Engine (MEE)
  - Secure Boot, TEE, Side-Channel Mitigations

### AI/ML and Specialized Computing
- [Chapter 18: Neural Network Acceleration (N Extension)](Chapter18_Neural_Network_Acceleration.md)
  - Neural Network Forward Pass
  - Neural Network Backward Pass (Training)
  - Loss Functions and Optimizers
  - Activation Functions, Normalization

- [Chapter 19: Matrix Operations (M Extension)](Chapter19_Matrix_Operations.md)
  - Matrix Configuration
  - Matrix Arithmetic (add, mul, transpose)
  - Matrix Load/Store

- [Chapter 20: Quantum Computing Interface (Q Extension)](Chapter20_Quantum_Computing_Interface.md)
  - Quantum Gates (single, two, three-qubit)
  - Quantum Measurement
  - Qubit Reset

## VOLUME IV: SYSTEM ARCHITECTURE

### High-Performance I/O
- [Chapter 22: RDMA and High-Performance Networking (P Extension)](Chapter22_RDMA_and_High_Performance_Networking.md)
  - RDMA Operations (send, recv, read, write)
  - InfiniBand Verbs
  - Queue Pair Management

- [Chapter 23: IOMMU and SR-IOV (I Extension)](Chapter23_IOMMU_and_SRIOV.md)
  - IOMMU Operations (map, unmap, flush)
  - SR-IOV (Virtual Function Management)

- [Chapter 24: Single-Level Storage (S Extension)](Chapter24_Single_Level_Storage.md)
  - Persistent Memory Operations
  - Object Addressing
  - Persistent Fence and Flush

### Memory Protection and Management
- [Chapter 25: Memory Protection and Register Banking (R Extension)](Chapter25_Memory_Protection_and_Register_Banking.md)
  - Protection Ring Model (2-ring, 4-ring)
  - VMS-Style Page Protection
  - Register Banking (4 modes)
  - Domain Protection, Capability Integration
  - Multiple Page Sizes, Memory Protection Keys

- [Chapter 26: Endian Switching](Chapter26_Endian_Switching.md)
  - Endian Control Register
  - Per-Privilege-Level Endianness
  - Byte-Reversed Load/Store
  - Network Byte Order

### Performance and System Features
- [Chapter 27: Performance Monitoring and Timers (T Extension)](Chapter27_Performance_Monitoring_and_Timers.md)
  - Time Stamp Counter (TSC)
  - Real-Time Clock (RTC)
  - Performance Monitoring Counters (8 × PMC)
  - 100+ Performance Events
  - Statistical Profiling

- [Chapter 28: Advanced System Features](Chapter28_Advanced_System_Features.md)
  - Power Management (DVFS, C-states, P-states)
  - Debug and Trace
  - Error Detection and Correction (ECC)
  - Machine Check Architecture (MCA)
  - Thermal Management, Cache Management
  - Reliability, Availability, Serviceability (RAS)

## VOLUME V: FUTURE EXTENSIONS

### Extended Architectures
- [Chapter 29: DLX128 Architecture](Chapter29_DLX128_Architecture.md)
  - 128-bit Instructions (lq, sq, addq, mulq)
  - 128-bit Address Space

- [Chapter 30: Moxie Compatibility Mode](Chapter30_Moxie_Compatibility_Mode.md)
  - Moxie Mode Control
  - Moxie Register Mapping
  - Moxie64 Support

- [Chapter 31: Advanced AI/ML Extensions](Chapter31_Advanced_AI_ML_Extensions.md)
  - Tensor Processing
  - Transformer Acceleration (Multi-Head Attention)
  - Sparse Operations
  - Mixed-Precision Training
  - Quantization Support
  - Recurrent Neural Networks (LSTM, GRU)
  - Graph Neural Networks
  - Model Parallelism

- [Chapter 32: Future ISA Directions](Chapter32_Future_ISA_Directions.md)
  - DLX256 Architecture
  - Photonic Computing
  - Neuromorphic Computing
  - DNA Computing
  - Advanced Quantum Extensions
  - Reversible Computing
  - Approximate and Probabilistic Computing
  - Homomorphic Encryption
  - Post-Silicon Technologies

## Appendices
- [Appendices](Appendices.md)
  - Appendix A: Complete Instruction Summary (500+ instructions)
  - Appendix B: CSR Complete Map (150+ CSRs)
  - Appendix C: Performance Event Reference (100+ events)
  - Appendix D: Code Examples
  - Revision History
  - Document References

---

## Extension Summary

| Extension | Chapter | Description |
|-----------|---------|-------------|
| **F** | 12 | Single-precision floating-point |
| **D** | 12 | Double-precision floating-point |
| **Q** | 12 | Quad-precision floating-point |
| **V** | 8 | Vector operations (RV-V compatible) |
| **B** | 9 | Bit manipulation |
| **A** | 13 | Atomic instructions |
| **C** | 14 | Compressed 16-bit instructions |
| **H** | 17 | Hypervisor support with nested virtualization |
| **K** | 10 | Cryptography (AES, SHA, RSA, ECC) |
| **M** | 19 | Matrix operations for AI/ML |
| **N** | 18 | Neural network acceleration |
| **X** | 15 | CHERI capabilities |
| **E** | 16 | Secure enclaves with memory encryption |
| **T** | 27 | High-precision timers and PMU |
| **P** | 22 | RDMA and high-performance networking |
| **I** | 23 | IOMMU and SR-IOV |
| **R** | 25 | Register banking (4 modes) |
| **S** | 24 | Single-level storage |
| **Q** (quantum) | 20 | Quantum computing interface |

**Additional Features:**
- CRC/Compression (Chapter 11)
- Endian Switching (Chapter 26)
- Advanced Security (Chapter 21)
- Advanced System (Chapter 28)
- Advanced AI/ML (Chapter 31)
- Future ISA (Chapter 32)

---

**Total Chapters**: 33 (including Front Matter and Appendices)
**Total Extensions Documented**: 19 major extensions + 6 additional feature sets
**Total Instructions**: 500+
**Total CSRs**: 150+
**Total Performance Events**: 100+

---

**Version**: 3.0
**Last Updated**: October 2024
**Status**: Complete - All Extensions Documented
