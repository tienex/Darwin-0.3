# Building Darwin-0.3 for RISC-V

Complete guide for the **toolchain**, **bootloader**, and **kernel** build process.

## Overview

```
┌─────────────────┐
│  RISC-V GCC     │ ← Cross-compilation toolchain
│  Toolchain      │
└────────┬────────┘
         │
         ├─────────────────────────────────────┐
         │                                     │
         ▼                                     ▼
┌─────────────────┐                   ┌─────────────────┐
│   OpenSBI       │                   │  Darwin Kernel  │
│  (M-mode FW)    │                   │   (S-mode OS)   │
└────────┬────────┘                   └────────┬────────┘
         │                                     │
         └─────────────────┬───────────────────┘
                           │
                           ▼
                   ┌─────────────────┐
                   │  Boot Image     │
                   │  (fw + kernel)  │
                   └─────────────────┘
```

## Part 1: RISC-V Toolchain

### Option A: Use Pre-built Toolchain (Recommended)

**For Ubuntu/Debian:**
```bash
sudo apt-get install gcc-riscv64-unknown-elf
sudo apt-get install binutils-riscv64-unknown-elf
sudo apt-get install gdb-multiarch
```

**For macOS (Homebrew):**
```bash
brew tap riscv/riscv
brew install riscv-tools
```

**Verify installation:**
```bash
riscv64-unknown-elf-gcc --version
# Should show: riscv64-unknown-elf-gcc (GCC) 10.x.x or newer
```

### Option B: Build Toolchain from Source

**Prerequisites:**
```bash
# Ubuntu/Debian
sudo apt-get install autoconf automake autotools-dev curl python3 \
    libmpc-dev libmpfr-dev libgmp-dev gawk build-essential bison \
    flex texinfo gperf libtool patchutils bc zlib1g-dev libexpat-dev

# macOS
brew install gawk gnu-sed gmp mpfr libmpc isl zlib expat
```

**Build process (takes 30-60 minutes):**
```bash
# Clone the toolchain repository
git clone https://github.com/riscv/riscv-gnu-toolchain
cd riscv-gnu-toolchain

# Checkout stable version
git checkout 2023.10.18

# Configure for RV64
# --with-arch=rv64gc: Base ISA + standard extensions (IMAFD + C)
# --with-abi=lp64d: 64-bit with hardware floating-point
./configure \
    --prefix=/opt/riscv \
    --with-arch=rv64gc \
    --with-abi=lp64d \
    --enable-multilib

# Build (use all CPU cores)
make -j$(nproc)

# Add to PATH
echo 'export PATH=/opt/riscv/bin:$PATH' >> ~/.bashrc
source ~/.bashrc
```

**For RV32 support (optional):**
```bash
# Build RV32 variant in addition to RV64
./configure \
    --prefix=/opt/riscv32 \
    --with-arch=rv32gc \
    --with-abi=ilp32d

make -j$(nproc)
```

**Test the toolchain:**
```bash
# Create test program
cat > test.c << 'EOF'
#include <stdio.h>
int main() {
    printf("Hello RISC-V!\n");
    return 0;
}
EOF

# Compile
riscv64-unknown-elf-gcc -march=rv64g -mabi=lp64d -o test test.c

# Examine binary
riscv64-unknown-elf-objdump -d test | less
```

### Toolchain Components

After installation, you should have:

```bash
riscv64-unknown-elf-gcc      # C compiler
riscv64-unknown-elf-g++      # C++ compiler (not needed for kernel)
riscv64-unknown-elf-as       # Assembler
riscv64-unknown-elf-ld       # Linker
riscv64-unknown-elf-objcopy  # Object file converter
riscv64-unknown-elf-objdump  # Disassembler
riscv64-unknown-elf-ar       # Archive utility
riscv64-unknown-elf-ranlib   # Archive indexer
riscv64-unknown-elf-strip    # Symbol stripper
riscv64-unknown-elf-gdb      # Debugger
```

## Part 2: OpenSBI Bootloader

OpenSBI (Open Source Supervisor Binary Interface) is the M-mode firmware that loads your kernel.

### Download and Build OpenSBI

```bash
# Clone OpenSBI
git clone https://github.com/riscv/opensbi.git
cd opensbi

# Checkout stable version
git checkout v1.3

# Build for generic platform (works with QEMU and many boards)
make CROSS_COMPILE=riscv64-unknown-elf- PLATFORM=generic

# Output files are in:
# build/platform/generic/firmware/fw_jump.elf
# build/platform/generic/firmware/fw_jump.bin
```

### Build for Specific Platforms

**For SiFive HiFive Unleashed:**
```bash
make CROSS_COMPILE=riscv64-unknown-elf- \
     PLATFORM=generic \
     FW_JUMP_ADDR=0x80200000
```

**For QEMU virt machine:**
```bash
make CROSS_COMPILE=riscv64-unknown-elf- \
     PLATFORM=generic \
     FW_JUMP_ADDR=0x80200000
```

### OpenSBI Firmware Types

OpenSBI provides three firmware types:

1. **fw_jump.bin** - Jump to fixed address (recommended for Darwin)
   - Loads and jumps to kernel at specified address
   - Good for testing and development

2. **fw_payload.bin** - Contains embedded payload
   - Kernel is embedded in firmware image
   - Good for production deployment

3. **fw_dynamic.bin** - Dynamic firmware
   - More flexible but more complex

**For Darwin, we use fw_jump:**
```bash
# OpenSBI will jump to 0x80200000 where kernel is loaded
# Kernel expects:
#   a0 = hartid (hardware thread ID)
#   a1 = device tree pointer
```

### Creating Combined Boot Image

**Method 1: Separate files**
```bash
# Load OpenSBI at 0x80000000
# Load Darwin kernel at 0x80200000
```

**Method 2: Combined image**
```bash
# Create fw_payload with embedded kernel
make CROSS_COMPILE=riscv64-unknown-elf- \
     PLATFORM=generic \
     FW_PAYLOAD_PATH=../Darwin-0.3/kernel-7/conf/RELEASE_RISCV/mach_kernel.bin
```

## Part 3: Building Darwin Kernel

### Prerequisites

Ensure you have:
- ✅ RISC-V toolchain installed
- ✅ Darwin-0.3 source code
- ✅ Git repository up to date

### Quick Build

```bash
cd Darwin-0.3/kernel-7/conf

# Build for RISC-V 64-bit (Release)
make ARCH=RISCV TYPE=RELEASE mach_kernel.kernel

# Output: RELEASE_RISCV/mach_kernel.kernel
```

### Build Options

**Different build types:**
```bash
# Release build (optimized)
make ARCH=RISCV TYPE=RELEASE mach_kernel.kernel

# Debug build (with symbols, no optimization)
make ARCH=RISCV TYPE=DEBUG mach_kernel.kernel

# Profile build (with profiling support)
make ARCH=RISCV TYPE=PROFILE mach_kernel.kernel
```

**For RV32:**
```bash
# Note: RV32 support is provided but less tested
# Would need to modify Makefile.riscv to use rv32 toolchain
make ARCH=RISCV32 TYPE=RELEASE mach_kernel.kernel
```

### Detailed Build Steps

**1. Configure the build:**
```bash
cd kernel-7/conf

# Review configuration
cat MASTER.riscv

# Optionally create custom config
cp MASTER.riscv CUSTOM.riscv
# Edit CUSTOM.riscv to add/remove options
```

**2. Clean previous builds:**
```bash
make ARCH=RISCV TYPE=RELEASE clean
```

**3. Build the kernel:**
```bash
make ARCH=RISCV TYPE=RELEASE mach_kernel.kernel VERBOSE=1
```

**4. Verify the output:**
```bash
ls -lh RELEASE_RISCV/mach_kernel.kernel

# Check kernel size (should be < 10MB)
size RELEASE_RISCV/mach_kernel.kernel

# Examine sections
riscv64-unknown-elf-objdump -h RELEASE_RISCV/mach_kernel.kernel
```

### Build Artifacts

After successful build:
```
kernel-7/conf/RELEASE_RISCV/
├── mach_kernel.kernel      # ELF executable
├── mach_kernel.kernel.sys  # Symbol table
├── *.o                     # Object files
└── build.log              # Build log
```

### Converting to Binary Format

For bootloader compatibility:
```bash
cd RELEASE_RISCV

# Convert ELF to raw binary
riscv64-unknown-elf-objcopy \
    -O binary \
    mach_kernel.kernel \
    mach_kernel.bin

# Verify size
ls -lh mach_kernel.bin
```

### Build Troubleshooting

**Error: "riscv64-unknown-elf-gcc: command not found"**
```bash
# Fix PATH
export PATH=/opt/riscv/bin:$PATH
# Or install toolchain (see Part 1)
```

**Error: "No rule to make target..."**
```bash
# Ensure you're in kernel-7/conf
pwd  # Should show .../Darwin-0.3/kernel-7/conf

# Clean and rebuild
make ARCH=RISCV TYPE=RELEASE clean
make ARCH=RISCV TYPE=RELEASE mach_kernel.kernel
```

**Error: Missing header files**
```bash
# Ensure all source files are present
git status
git submodule update --init --recursive
```

### Customizing Kernel Configuration

Edit `kernel-7/conf/MASTER.riscv`:

```makefile
# Enable/disable features
options GDB              # Kernel debugger
options DEBUG            # Debug code
options SHOW_SPACE       # Print structure sizes
options EVENTMETER       # Event metering

# Add device drivers
pseudo-device vol        # Volume support
pseudo-device ppp 2      # PPP support
```

### Build Scripts

**Create helper script `build-riscv.sh`:**
```bash
#!/bin/bash
set -e

cd kernel-7/conf

echo "Building Darwin/RISC-V kernel..."

# Clean
make ARCH=RISCV TYPE=RELEASE clean

# Build
make ARCH=RISCV TYPE=RELEASE mach_kernel.kernel -j$(nproc)

# Convert to binary
cd RELEASE_RISCV
riscv64-unknown-elf-objcopy -O binary \
    mach_kernel.kernel \
    mach_kernel.bin

echo "Build complete!"
echo "Output: $(pwd)/mach_kernel.bin"
ls -lh mach_kernel.bin
```

## Part 4: Complete Build Flow

### Full Build from Scratch

```bash
#!/bin/bash
# complete-build.sh - Build everything from scratch

set -e

BUILD_DIR="$(pwd)/build"
mkdir -p "$BUILD_DIR"

# Step 1: Build toolchain (only needed once)
if [ ! -d "/opt/riscv" ]; then
    echo "Building RISC-V toolchain..."
    git clone https://github.com/riscv/riscv-gnu-toolchain
    cd riscv-gnu-toolchain
    ./configure --prefix=/opt/riscv --with-arch=rv64gc --with-abi=lp64d
    make -j$(nproc)
    cd ..
fi

export PATH=/opt/riscv/bin:$PATH

# Step 2: Build OpenSBI
echo "Building OpenSBI..."
cd "$BUILD_DIR"
if [ ! -d "opensbi" ]; then
    git clone https://github.com/riscv/opensbi.git
fi
cd opensbi
make CROSS_COMPILE=riscv64-unknown-elf- \
     PLATFORM=generic \
     FW_JUMP_ADDR=0x80200000
cp build/platform/generic/firmware/fw_jump.bin "$BUILD_DIR/"
cd ..

# Step 3: Build Darwin kernel
echo "Building Darwin kernel..."
cd Darwin-0.3/kernel-7/conf
make ARCH=RISCV TYPE=RELEASE clean
make ARCH=RISCV TYPE=RELEASE mach_kernel.kernel
cd RELEASE_RISCV
riscv64-unknown-elf-objcopy -O binary \
    mach_kernel.kernel \
    "$BUILD_DIR/mach_kernel.bin"

echo ""
echo "Build complete!"
echo "OpenSBI firmware: $BUILD_DIR/fw_jump.bin"
echo "Darwin kernel:    $BUILD_DIR/mach_kernel.bin"
```

## Part 5: Testing

### Test in QEMU

```bash
# Install QEMU with RISC-V support
sudo apt-get install qemu-system-misc  # Ubuntu/Debian
brew install qemu                      # macOS

# Run Darwin kernel
qemu-system-riscv64 \
    -machine virt \
    -cpu rv64 \
    -m 2G \
    -bios fw_jump.bin \
    -kernel mach_kernel.bin \
    -serial stdio \
    -nographic \
    -append "console=uart8250,mmio,0x10000000"
```

### Test on Real Hardware

**For SiFive HiFive Unleashed:**
```bash
# Copy files to SD card
sudo dd if=fw_jump.bin of=/dev/sdX1 bs=1M
sudo dd if=mach_kernel.bin of=/dev/sdX1 seek=2 bs=1M

# Connect serial console
screen /dev/ttyUSB0 115200

# Power on board
```

## Part 6: Deployment

### Creating Bootable Image

```bash
# Create SD card image
dd if=/dev/zero of=darwin-riscv.img bs=1M count=512

# Partition
fdisk darwin-riscv.img
# Create partition at offset 2048

# Format
mkfs.ext4 -E offset=1048576 darwin-riscv.img

# Mount and copy files
mkdir -p /mnt/darwin
mount -o loop,offset=1048576 darwin-riscv.img /mnt/darwin
cp fw_jump.bin /mnt/darwin/
cp mach_kernel.bin /mnt/darwin/
umount /mnt/darwin
```

### Automated Build Makefile

Create `Makefile.riscv-complete`:
```makefile
.PHONY: all toolchain firmware kernel image clean

all: toolchain firmware kernel image

toolchain:
	@echo "Building RISC-V toolchain..."
	# ... toolchain build commands ...

firmware:
	@echo "Building OpenSBI..."
	# ... OpenSBI build commands ...

kernel:
	@echo "Building Darwin kernel..."
	cd kernel-7/conf && make ARCH=RISCV TYPE=RELEASE mach_kernel.kernel

image: firmware kernel
	@echo "Creating boot image..."
	# ... image creation commands ...

clean:
	rm -rf build/
```

## Summary

**Minimal steps to get started:**

```bash
# 1. Install toolchain
sudo apt-get install gcc-riscv64-unknown-elf

# 2. Build kernel
cd Darwin-0.3/kernel-7/conf
make ARCH=RISCV TYPE=RELEASE mach_kernel.kernel

# 3. Test in QEMU
qemu-system-riscv64 -machine virt -cpu rv64 -m 2G \
    -bios default -kernel RELEASE_RISCV/mach_kernel.kernel \
    -serial stdio -nographic
```

**Production deployment:**

1. Build complete toolchain
2. Build OpenSBI for your platform
3. Build Darwin kernel with platform config
4. Create combined boot image
5. Deploy to target hardware

---

**Build time estimates:**
- Toolchain: 30-60 minutes (first time only)
- OpenSBI: 2-5 minutes
- Darwin kernel: 5-10 minutes
- Total (first build): ~45-75 minutes
- Incremental rebuild: ~5-10 minutes
