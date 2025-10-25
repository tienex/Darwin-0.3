# Darwin IA64 Toolchain Setup Guide

## Overview

This guide explains how to set up the cross-compilation toolchain for building Darwin IA64 components.

**Toolchain Components:**
- GNU Binutils 2.41 (assembler, linker, objcopy, objdump)
- GCC 13.2.0 (C/C++ cross-compiler)
- Target: ia64-unknown-linux-gnu

## Quick Start

### Automated Installation

```bash
# Run the automated setup script
./build-toolchain.sh

# Follow prompts and wait for build to complete (~30-60 minutes)
```

### Manual Installation

If you prefer manual installation or need to customize:

```bash
# Set installation prefix (default: /opt/ia64-darwin)
export TOOLCHAIN_PREFIX=/opt/ia64-darwin

# Run build script
./build-toolchain.sh
```

## Prerequisites

Before running the toolchain setup, ensure you have:

**Required packages (Debian/Ubuntu):**
```bash
sudo apt-get install -y \
    build-essential \
    wget \
    tar \
    make \
    gcc \
    g++ \
    bison \
    flex \
    texinfo \
    libgmp-dev \
    libmpfr-dev \
    libmpc-dev
```

**Required packages (RHEL/CentOS/Fedora):**
```bash
sudo yum install -y \
    gcc \
    gcc-c++ \
    make \
    wget \
    tar \
    bison \
    flex \
    texinfo \
    gmp-devel \
    mpfr-devel \
    libmpc-devel
```

**Disk space requirements:**
- Build: ~5 GB
- Installed: ~1 GB

## Using the Toolchain

### Environment Setup

After installation completes, source the environment script:

```bash
source /opt/ia64-darwin/setup-env.sh
```

This sets:
- `PATH`: Adds toolchain binaries
- `LD_LIBRARY_PATH`: Adds toolchain libraries
- `ARCH`: Set to `ia64`
- `CROSS_COMPILE`: Set to `ia64-unknown-linux-gnu-`
- `RC_ARCHS`: Set to `ia64 ia64be ia64_32`

### Verify Installation

```bash
# Check compiler version
ia64-unknown-linux-gnu-gcc --version

# Check binutils
ia64-unknown-linux-gnu-ld --version
ia64-unknown-linux-gnu-as --version
ia64-unknown-linux-gnu-objdump --version
```

Expected output:
```
ia64-unknown-linux-gnu-gcc (GCC) 13.2.0
```

## Building Darwin Components

### Building Userland Libraries

```bash
# Source environment
source /opt/ia64-darwin/setup-env.sh

# Build Libc
cd Libc-1
make ARCH=ia64

# Build for all variants
make ARCH=ia64 RC_ARCHS="ia64 ia64be ia64_32"
```

### Building the Bootloader

```bash
# Source environment
source /opt/ia64-darwin/setup-env.sh

# Build EFI bootloader
cd boot-2/ia64
make

# Result: BOOTIA64.EFI
```

### Building the Kernel

```bash
# Source environment
source /opt/ia64-darwin/setup-env.sh

# Configure kernel
cd kernel-7/conf
./doconf -c MASTER.ia64 RELEASE

# Build kernel
cd ../build/RELEASE
make

# Result: mach_kernel
```

## Toolchain Architecture

### Directory Structure

```
/opt/ia64-darwin/
├── bin/                          # Toolchain binaries
│   ├── ia64-unknown-linux-gnu-gcc
│   ├── ia64-unknown-linux-gnu-ld
│   ├── ia64-unknown-linux-gnu-as
│   ├── ia64-unknown-linux-gnu-ar
│   ├── ia64-unknown-linux-gnu-objcopy
│   └── ia64-unknown-linux-gnu-objdump
├── bin-wrappers/                 # Darwin-specific wrappers
│   ├── ia64-darwin-gcc           # GCC with Darwin flags
│   └── ia64-darwin-ld            # LD with Darwin flags
├── lib/                          # Runtime libraries
│   └── gcc/ia64-unknown-linux-gnu/13.2.0/
├── libexec/                      # GCC internal tools
├── ia64-unknown-linux-gnu/       # Target-specific files
│   ├── bin/
│   ├── include/
│   └── lib/
└── setup-env.sh                  # Environment setup script
```

### Wrapper Scripts

Darwin-specific compiler wrappers in `/opt/ia64-darwin/bin-wrappers/`:

**ia64-darwin-gcc:**
- Adds `-O2 -fno-strict-aliasing -fno-common -pipe -fno-builtin`
- Use for Darwin kernel/userland builds

**ia64-darwin-ld:**
- Adds `-static`
- Use for Darwin static linking

### Cross-Compilation Targets

The toolchain supports three IA64 variants:

**ia64 (LP64, little-endian):**
```bash
ia64-unknown-linux-gnu-gcc -D__ia64__ program.c -o program
```

**ia64be (LP64, big-endian):**
```bash
ia64-unknown-linux-gnu-gcc -D__ia64__ -D__BIG_ENDIAN__ -mbig-endian program.c -o program
```

**ia64_32 (ILP32, little-endian):**
```bash
ia64-unknown-linux-gnu-gcc -D__ia64__ -D__ia64_32__ -milp32 program.c -o program
```

## Compiler Flags Reference

### Architecture-Specific Flags

**Required for Darwin builds:**
```
-D__ia64__              # Define IA64 architecture
-fno-strict-aliasing    # Disable strict aliasing (kernel safety)
-fno-common             # No common blocks (Darwin requirement)
-pipe                   # Use pipes instead of temp files
-fno-builtin            # No compiler builtins (kernel)
```

**Optimization flags:**
```
-O2                     # Standard optimization
-O3                     # Aggressive optimization (userland)
-Os                     # Optimize for size
```

**IA64-Specific flags:**
```
-mfixed-range=f32-f127  # Reserve FP registers (kernel)
-mno-sdata              # Disable small data section
-mbig-endian            # Big-endian mode (ia64be)
-milp32                 # ILP32 mode (ia64_32)
```

### Linker Flags

**EFI bootloader:**
```
-nostdlib               # No standard libraries
-znocombreloc           # Don't combine reloc sections
-shared                 # Shared object (EFI requirement)
-Bsymbolic              # Bind symbols locally
-T efi/efi_ia64.lds     # Custom linker script
```

**Kernel:**
```
-e _start               # Entry point
-Ttext A000000000000000 # Load address (Region 5)
-static                 # Static linking
```

## Common Build Issues

### Issue: "ia64-unknown-linux-gnu-gcc: command not found"

**Cause:** Environment not sourced or toolchain not in PATH

**Solution:**
```bash
source /opt/ia64-darwin/setup-env.sh
# OR
export PATH="/opt/ia64-darwin/bin:$PATH"
```

### Issue: "undefined reference to __muldi3"

**Cause:** Missing libgcc link

**Solution:** Add `-lgcc` to linker flags:
```bash
ia64-unknown-linux-gnu-gcc program.c -o program -lgcc
```

### Issue: "relocation truncated to fit"

**Cause:** Code/data exceeds addressable range

**Solution:** Use appropriate memory model:
```bash
ia64-unknown-linux-gnu-gcc -mcmodel=large program.c -o program
```

### Issue: EFI bootloader won't load

**Cause:** Incorrect PE/COFF format

**Solution:** Verify objcopy command:
```bash
ia64-unknown-linux-gnu-objcopy \
    -j .text -j .sdata -j .data -j .dynamic \
    -j .dynsym -j .rel -j .rela -j .reloc \
    --target=efi-app-ia64 bootia64.so BOOTIA64.EFI
```

### Issue: Kernel link fails with "cannot find -lcc"

**Cause:** Darwin expects libcc, not libgcc

**Solution:** Create symlink:
```bash
cd /opt/ia64-darwin/lib/gcc/ia64-unknown-linux-gnu/13.2.0/
ln -s libgcc.a libcc.a
```

## Advanced Configuration

### Custom Toolchain Prefix

```bash
export TOOLCHAIN_PREFIX=/usr/local/ia64-darwin
./build-toolchain.sh
```

### Building with Different GCC Version

Edit `build-toolchain.sh`:
```bash
GCC_VERSION="12.3.0"  # Change version
```

### Parallel Build (faster)

The script auto-detects CPU count, but you can override:
```bash
export NPROC=16
./build-toolchain.sh
```

### Adding Additional Languages

Edit `build-toolchain.sh` in `build_gcc()`:
```bash
--enable-languages=c,c++,fortran  # Add Fortran
```

## Testing the Toolchain

### Basic Compile Test

```bash
cat > test.c << 'EOF'
#include <stdint.h>

int main(void) {
    uint64_t val = 0x123456789ABCDEF0ULL;
    return 0;
}
EOF

ia64-unknown-linux-gnu-gcc -c test.c -o test.o
ia64-unknown-linux-gnu-objdump -d test.o
```

### Register Stack Engine Test

```bash
cat > rse_test.c << 'EOF'
#include <stdint.h>

void recursive_function(int depth) {
    if (depth > 0) {
        recursive_function(depth - 1);
    }
}

int main(void) {
    recursive_function(100);  // Tests RSE
    return 0;
}
EOF

ia64-unknown-linux-gnu-gcc rse_test.c -o rse_test
```

### Assembly Output Test

```bash
cat > asm_test.c << 'EOF'
int add(int a, int b) {
    return a + b;
}
EOF

ia64-unknown-linux-gnu-gcc -S -O2 asm_test.c
cat asm_test.s  # View generated IA64 assembly
```

Expected output should contain IA64 instructions like:
- `alloc` (register stack allocation)
- `add` (integer add)
- `br.ret.sptk.many` (return with speculation)

## Toolchain Maintenance

### Updating the Toolchain

To update to a newer GCC or binutils version:

1. Edit `build-toolchain.sh`
2. Change version numbers
3. Re-run build script (it will overwrite existing installation)

### Uninstalling

```bash
# Remove toolchain
sudo rm -rf /opt/ia64-darwin

# Remove from shell profile
# Edit ~/.bashrc or ~/.zshrc and remove:
# source /opt/ia64-darwin/setup-env.sh
```

## Integration with Build Systems

### Make Integration

Add to Makefile:
```makefile
# Source toolchain
SHELL := /bin/bash
export PATH := /opt/ia64-darwin/bin:$(PATH)
export CROSS_COMPILE := ia64-unknown-linux-gnu-

CC := $(CROSS_COMPILE)gcc
LD := $(CROSS_COMPILE)ld
AS := $(CROSS_COMPILE)as
AR := $(CROSS_COMPILE)ar
```

### CMake Integration

```cmake
# toolchain-ia64.cmake
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR ia64)

set(CMAKE_C_COMPILER /opt/ia64-darwin/bin/ia64-unknown-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER /opt/ia64-darwin/bin/ia64-unknown-linux-gnu-g++)

set(CMAKE_FIND_ROOT_PATH /opt/ia64-darwin/ia64-unknown-linux-gnu)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
```

Usage:
```bash
cmake -DCMAKE_TOOLCHAIN_FILE=toolchain-ia64.cmake ..
```

## Performance Tips

### Optimization Levels

**Userland:**
- `-O2`: Good balance (recommended)
- `-O3`: Maximum performance
- `-Os`: Minimize size

**Kernel:**
- `-O2`: Required (higher levels may break kernel)
- Add `-fno-strict-aliasing -fno-omit-frame-pointer`

### Link-Time Optimization (LTO)

```bash
# Compile with LTO
ia64-unknown-linux-gnu-gcc -O3 -flto -c program.c

# Link with LTO
ia64-unknown-linux-gnu-gcc -O3 -flto program.o -o program
```

### Profile-Guided Optimization (PGO)

```bash
# Step 1: Build with instrumentation
ia64-unknown-linux-gnu-gcc -O2 -fprofile-generate program.c -o program

# Step 2: Run program to collect profile data
./program  # (on target hardware or simulator)

# Step 3: Rebuild with profile data
ia64-unknown-linux-gnu-gcc -O2 -fprofile-use program.c -o program
```

## Reference

### Official Documentation

- **GNU Binutils:** https://sourceware.org/binutils/docs/
- **GCC Manual:** https://gcc.gnu.org/onlinedocs/
- **IA64 Architecture:** Intel Itanium Architecture Software Developer's Manual

### Darwin IA64 Documentation

- `README.md` - Project overview
- `IA64_IMPLEMENTATION_GUIDE.md` - Architecture details
- `IA64_GAP_ANALYSIS.md` - Implementation status
- `ITANIUM2_BOOT_GUIDE.md` - Itanium 2 boot process
- `TOOLCHAIN_SETUP.md` - This document

### Getting Help

**Build issues:**
1. Check prerequisites are installed
2. Verify disk space (5GB+ free)
3. Check build logs in `sources/`

**Runtime issues:**
1. Verify environment is sourced
2. Check `ia64-unknown-linux-gnu-gcc --version`
3. Test with simple program

**Darwin-specific issues:**
- Consult `IA64_GAP_ANALYSIS.md` for known limitations
- Check kernel-7/conf/files.ia64 for missing implementations

---

**Last Updated:** 2025-10-25
**Toolchain Version:** Binutils 2.41 + GCC 13.2.0
**Target:** ia64-unknown-linux-gnu
