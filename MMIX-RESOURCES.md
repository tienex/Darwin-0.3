# MMIX Emulator and Linux Kernel Resources

## Summary

This document provides comprehensive information about MMIX emulators, simulators, and the Linux kernel port for MMIX architecture. These resources are valuable for understanding how to actually run code on MMIX and learning from existing OS implementations.

---

## 1. Official MMIX Simulators (MMIXware)

### Overview
MMIXware is the official software package by Donald Knuth for MMIX development.

### Components

**MMIX-SIM** (Simple Simulator)
- Behavioral simulator without pipeline or caches
- Best for beginners and program testing
- Executes MMIX programs instruction by instruction
- Included in official MMIXware package

**MMIX-PIPE** (Pipeline Simulator)
- Advanced meta-simulator with configurable pipeline
- Simulates realistic hardware with caches and pipeline
- Can test different hardware configurations
- Part of "mmmix" super meta-simulator

**MMIXAL** (Assembler)
- Assembles MMIX symbolic files to object format
- Produces .mmo (MMIX object) files
- Standard assembler for MMIX development

### Download Locations

**Official Source**:
- Stanford University: https://www-cs-faculty.stanford.edu/~knuth/mmix-news.html
- Direct download: `mmix.tar.gz` from Knuth's website
- Latest version: February 13, 2023

**GitHub Mirror (Enhanced)**:
- Repository: https://github.com/ascherer/mmix
- Features: C23 compiler compatibility, shared library support
- Includes: mmix-sim.w, mmix-pipe.w, mmixal.w (CWEB sources)
- Build requires: CWEB system (literate programming)

**MMIX Home Page**:
- URL: https://mmix.cs.hm.edu/ (official MMIX resource hub)
- Provides:
  - Sources and documentation
  - Win32 executables
  - Linux 32-bit binaries
  - Mac OS X executables
  - MMIXVD Visual Debugger

**Arch Linux AUR**:
- Package: `mmixware` in Arch User Repository
- Easy installation on Arch-based distributions

---

## 2. Third-Party MMIX Simulators

### GIMMIX (GitHub)

**Repository**: https://github.com/Nils-TUD/GIMMIX

**Features**:
- Interactive debugging with GDB stub support
- Modified GDB 6.4.50 for remote debugging
- Automated testing framework with unit tests
- Code coverage analysis (runcov.sh script)
- ROM generation for binary format conversion
- Support for both user and kernel programs

**Build Requirements**:
- Linux system (tested on Ubuntu 10.04)
- `ctangle` utility (from texlive-binaries)
- Ruby interpreter (for tests and coverage)
- C compiler (gcc)

**Build Instructions**:
```bash
# Build MMIXware first
cd org && make

# Return to root and build GIMMIX
cd .. && make

# Optional: generate documentation
make doc
```

**Running Programs**:
```bash
# Interactive debugging
make debug PROG=user/alignment

# Debug kernel programs
make debugx PROG=kernel/exceptions1

# With custom parameters
PARAMS="arg1 arg2" make debug PROG=yourprogram
```

**Project Stats**:
- Language: Assembly (77.3%), C (10.8%), CWeb (10.7%)
- License: GPL-2.0
- Activity: 30 commits, 8 stars, 2 forks

### Other Simulators

**jlamothe/mmix**:
- Repository: https://github.com/jlamothe/mmix
- Another MMIX emulator implementation
- GitHub: https://github.com/jlamothe/mmix

**MMIX WASM**:
- Web-based MMIX simulator
- Blog post: https://blog.y2kbugger.com/mmix-wasm.html
- Runs in browser via WebAssembly

---

## 3. MMIX GNU Toolchain

### Overview
The GNU toolchain provides professional development tools for MMIX, enabling C/C++ programming rather than just assembly.

### Components

**mmix-gcc** (GNU C Compiler for MMIX)
- Ported by Hans-Peter Nilson
- Supports C and C++
- Includes newlib C library with printf, standard I/O, etc.
- Part of official GCC distribution

**mmix-binutils** (Binary Utilities)
- mmix-as: GNU assembler for MMIX
- mmix-ld: Linker
- mmix-objcopy, mmix-objdump: Object file utilities
- Supports both ELF and native .mmo format

**Verified Working Versions**:
- gcc-4.3.5
- newlib-1.19.0
- binutils-2.21

### Resources

**Documentation**:
- GCC MMIX Options: https://gcc.gnu.org/onlinedocs/gcc/MMIX-Options.html
- Binutils MMIX Options: https://sourceware.org/binutils/docs/as/MMIX_002dOpts.html
- Hello World Example: https://mmix.cs.hm.edu/examples/hellognu.html
- Installation Guide: http://bitrange.com/mmix/install.html

**Download**:
- Linux Binaries: https://mmix.cs.hm.edu/bin/index.html
- Source: Available through standard GCC and binutils sources

### Object File Formats

**ELF Format**:
- Standard GNU toolchain format
- Used by mmix-gcc and mmix-binutils
- Compatible with GNU debugger

**MMO Format**:
- Native MMIX object format
- Produced by mmixal assembler
- Also supported by GNU toolchain

---

## 4. MMIX Linux Kernel Port

### VMMMIX - The Linux-Capable MMIX VM

**Project**: VMMMIX by Eiji Yoshiya
**Purpose**: Virtual machine that can run Linux on MMIX

**Features**:
- Console I/O support
- Hard disk device (HDD)
- Ethernet I/O for networking
- Runs modified Linux kernel 2.6.18
- ROOT NFS support

**Website**: http://www007.upp.so-net.ne.jp/eiji-y/vmmmix/vmmmix.html

### Linux Kernel Patches

**Patch Name**: `linux-2.6.18-mmix-20091231.patch.gz`
**Date**: December 31, 2009
**Author**: Eiji Yoshiya (eiji-y@pb3.so-net.ne.jp)

**Download Locations**:
1. VMMMIX project page (most complete patch):
   - http://www007.upp.so-net.ne.jp/eiji-y/vmmmix/vmmmix.html

2. MMIX Home Page example:
   - http://mmix.cs.hm.edu/examples/Linux/linux-2.6.18.patch

3. GitHub repository:
   - https://github.com/eiji-y/linux-mmix
   - Fork of Linux kernel with MMIX port
   - 311,521 commits
   - Created July 1, 2012

### Kernel Details

**Target Version**: Linux 2.6.18
**Architecture**: arch/mmix/
**Cross-compilation**: CROSS_COMPILE := mmix-

**Includes**:
- MMIX-specific kernel configuration
- Memory management for MMIX
- I/O subsystem adaptations
- Device drivers for VMMMIX devices

**Known Issues**:
- "There is still a bug in the system"
- Occasionally hangs and requires debugging
- Experimental/research quality, not production-ready

### GitHub Repository Status

**eiji-y/linux-mmix**:
- Status: Inactive (last updated 2012)
- Stars: 1
- Forks: 0
- Tags: 286 (various kernel versions)
- Based on Linux 3.x branch

**Build Procedure**:
Standard Linux kernel build process:
```bash
# Configure kernel
make menuconfig  # or xconfig, config

# Build kernel
make

# Install modules
make modules_install

# Configure bootloader (LILO)
```

---

## 5. Key Resources for Darwin-MMIX Development

### Most Relevant Resources

**For Understanding MMIX Hardware**:
1. **GIMMIX simulator** - Best for development/debugging
   - Interactive debugging
   - Can modify to add Darwin-specific devices

2. **MMIXware simulators** - Official reference
   - MMIX-SIM for simple testing
   - MMIX-PIPE for realistic hardware simulation

**For Understanding OS Implementation**:
1. **Linux-MMIX patch** - Complete OS port example
   - See how memory management is implemented
   - Study exception handling approach
   - Learn device driver architecture
   - File: `linux-2.6.18-mmix-20091231.patch.gz`

2. **VMMMIX design** - Device specification example
   - Console I/O implementation
   - Disk device interface
   - Network device interface

**For Toolchain**:
1. **mmix-gcc** - Required for building Darwin kernel
   - Need version compatible with Darwin's C dialect
   - gcc-4.3.5 is known working

2. **mmix-binutils** - Assembler and linker
   - Need for building assembly files (start.s, etc.)
   - binutils-2.21 is known working

### Recommended Study Order

1. **Get GIMMIX working**:
   - Clone and build GIMMIX
   - Run sample programs
   - Understand debugging interface

2. **Study Linux-MMIX patch**:
   - Download patch: `linux-2.6.18-mmix-20091231.patch.gz`
   - Review arch/mmix/ directory structure
   - Compare with Darwin's machdep/mmix/
   - Study memory management (pmap equivalent)
   - Study exception handling (trap equivalent)
   - Study process management (pcb equivalent)

3. **Adapt GIMMIX for Darwin**:
   - Add Darwin-specific devices
   - Implement Darwin boot protocol
   - Add Darwin debugging support

4. **Build mmix-gcc toolchain**:
   - Download gcc-4.3.5, binutils-2.21, newlib-1.19.0
   - Build cross-compiler for MMIX
   - Test with simple Darwin kernel build

---

## 6. Comparison: Linux-MMIX vs Darwin-MMIX

### Similarities
- Both need arch/mmix or machdep/mmix directory
- Both need pmap/memory management
- Both need trap/exception handling
- Both need process control blocks
- Both need device drivers

### Differences
- **Kernel Structure**: Linux is monolithic, Darwin is Mach microkernel
- **System Calls**: Linux uses different syscall numbers/ABI
- **Device Model**: Linux uses different driver framework than DriverKit
- **Boot Process**: Linux uses different boot protocol
- **Memory Model**: Darwin uses Mach VM, Linux uses Linux VM

### What Darwin Can Learn from Linux-MMIX
1. **Hardware abstraction**: How to abstract MMIX specifics
2. **Exception handling**: Proven approach for MMIX traps
3. **TLB management**: Working TLB code for MMIX
4. **Context switching**: Register save/restore strategies
5. **Device interface**: How to interface with emulator devices

---

## 7. Practical Next Steps for Darwin-MMIX

### Immediate Actions

1. **Clone GIMMIX**:
   ```bash
   git clone https://github.com/Nils-TUD/GIMMIX.git
   cd GIMMIX
   cd org && make
   cd .. && make
   ```

2. **Get Linux Patch**:
   ```bash
   # Download from GitHub
   git clone https://github.com/eiji-y/linux-mmix.git

   # Or download patch directly
   wget http://www007.upp.so-net.ne.jp/eiji-y/vmmmix/linux-2.6.18-mmix-20091231.patch.gz
   ```

3. **Study Key Files**:
   - linux-mmix/arch/mmix/kernel/start.S (compare with Darwin start.s)
   - linux-mmix/arch/mmix/mm/pmap.c (compare with Darwin pmap.c)
   - linux-mmix/arch/mmix/kernel/traps.c (compare with Darwin trap.c)
   - linux-mmix/arch/mmix/kernel/process.c (compare with Darwin pcb.c)

4. **Build MMIX Toolchain**:
   - Follow guide at http://bitrange.com/mmix/install.html
   - Or download prebuilt from https://mmix.cs.hm.edu/bin/index.html

### Medium-term Goals

1. **Enhance GIMMIX** for Darwin:
   - Add Darwin boot protocol
   - Add Darwin-specific devices
   - Implement Darwin debugging interface

2. **Complete Darwin Implementation**:
   - Finish TLB management in pmap.c
   - Implement system call handlers
   - Add device drivers
   - Test boot sequence

3. **Test and Debug**:
   - Boot Darwin kernel in enhanced GIMMIX
   - Debug with GDB stub
   - Fix issues and iterate

---

## 8. Contact and Community

### Project Maintainers

**Donald Knuth** (MMIX Creator):
- Stanford University
- https://www-cs-faculty.stanford.edu/~knuth/

**Eiji Yoshiya** (Linux-MMIX Author):
- Email: eiji-y@pb3.so-net.ne.jp
- Website: http://www007.upp.so-net.ne.jp/eiji-y/

**Hans-Peter Nilson** (GCC MMIX Port):
- Contributed MMIX backend to GCC

### Community Resources

**GitHub**:
- Search for "MMIX" topics: https://github.com/topics/mmix
- Multiple implementations and tools available

**MMIX Home Page**:
- https://mmix.cs.hm.edu/
- Official community hub with examples and documentation

**The Art of Computer Programming**:
- Volumes 1-4 use MMIX for all example programs
- Best resource for understanding MMIX design philosophy

---

## Conclusion

The MMIX ecosystem provides excellent resources for implementing Darwin on MMIX:

1. **Simulators**: GIMMIX and MMIXware provide working emulators
2. **Toolchain**: mmix-gcc and mmix-binutils enable C development
3. **Reference**: Linux-MMIX patch shows complete OS implementation
4. **Community**: Active development and documentation

The Darwin-MMIX implementation can leverage all these resources, particularly by studying the Linux port and adapting GIMMIX for Darwin-specific needs.

**Key Downloads**:
- GIMMIX: https://github.com/Nils-TUD/GIMMIX
- MMIXware: https://www-cs-faculty.stanford.edu/~knuth/mmix-news.html
- Linux-MMIX: https://github.com/eiji-y/linux-mmix
- Toolchain: https://mmix.cs.hm.edu/bin/index.html
