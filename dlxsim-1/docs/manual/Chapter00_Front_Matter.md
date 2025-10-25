# DLX Complete Architecture Manual
## The Definitive Specification - Version 3.0

**A Comprehensive 64/128-bit RISC Architecture with Advanced Extensions**

---

**Document Version**: 3.0
**Release Date**: October 2024
**Status**: Complete Specification
**Covers**: DLX32, DLX64, DLX128, and all extensions

---

## About This Manual

This manual is the complete, authoritative specification for the DLX processor architecture family. It consolidates information from 22+ specialized architecture documents into a single reference.

### Organization

This manual consists of **5 volumes**:

**Volume I: Base Architecture** (Chapters 1-7)
- Core ISA, registers, memory model, binary formats, ABI
- Note: Chapters 5-6 describe system features, not ISA instructions

**Volume II: Standard Extensions** (Chapters 8-14)
- Vector, FP, BitManip, Atomic, Crypto, Compression

**Volume III: Advanced Features** (Chapters 15-21)
- Virtualization, Security, Neural Networks, Matrix, Quantum

**Volume IV: System Architecture** (Chapters 22-26)
- Single-Level Storage, Memory Protection, Endian Switching, Performance Monitoring
- Note: RDMA and IOMMU/SR-IOV moved to simulator documentation (not ISA)

**Volume V: Future Extensions** (Chapters 27-30)
- DLX128, Moxie Compatibility, Advanced AI/ML, Future ISA Directions

### Related Documents

For detailed implementation specifics, refer to:
- `DLX_ISA_REDESIGN.md` - Complete instruction reference
- `DLX_TIMERS_PROFILING.md` - Performance monitoring details
- `DLX_NESTED_VIRTUALIZATION.md` - Hypervisor implementation
- `DLX_SECURE_ENCLAVE.md` - Security implementation
- And 18 other specialized documents
