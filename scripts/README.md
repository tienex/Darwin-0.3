# Darwin-0.3 RISC-V Build Scripts

Complete build automation for Darwin-0.3 on RISC-V architecture.

## Quick Start

Build everything (toolchain, bootloader, and kernel):

```bash
./scripts/build-all.sh
```

Test in QEMU:

```bash
./build/run-qemu.sh
```

## Build Scripts

### Complete Build System

**`build-all.sh`** - Master build script
- Builds toolchain, bootloader, and kernel
- Creates bootable images
- Generates documentation
- Sets up test environment

```bash
# Build everything
./scripts/build-all.sh

# Skip toolchain if already installed
./scripts/build-all.sh --skip-toolchain

# Build debug kernel
./scripts/build-all.sh --type DEBUG
```

### Toolchain

**`toolchain/build-toolchain.sh`** - Build RISC-V GCC toolchain
- Downloads and builds riscv-gnu-toolchain
- Installs to `/opt/riscv` by default
- Supports both RV32 and RV64
- Takes 30-60 minutes

```bash
# Build toolchain (one-time setup)
./scripts/toolchain/build-toolchain.sh

# Custom install location
INSTALL_PREFIX=/custom/path ./scripts/toolchain/build-toolchain.sh
```

### Bootloader

**`bootloader/build-opensbi.sh`** - Build OpenSBI firmware
- Downloads and builds OpenSBI
- Creates firmware for QEMU and SiFive platforms
- Generates fw_jump.bin and fw_payload.bin

```bash
# Build bootloader
./scripts/bootloader/build-opensbi.sh

# Build with embedded kernel
./scripts/bootloader/build-opensbi.sh --kernel path/to/kernel.bin
```

### Kernel

**`kernel/build-kernel.sh`** - Build Darwin kernel
- Compiles Darwin-0.3 kernel for RISC-V
- Converts to binary format
- Generates symbols and disassembly
- Analyzes kernel image

```bash
# Build release kernel
./scripts/kernel/build-kernel.sh

# Build debug kernel
./scripts/kernel/build-kernel.sh --type DEBUG

# Clean build with disassembly
./scripts/kernel/build-kernel.sh --clean --disasm
```

## Utility Scripts

### Testing

**`utils/test-qemu.sh`** - Run kernel in QEMU
- Simple QEMU test harness
- Automatic file detection
- Configurable memory and CPUs

```bash
# Run with defaults (2GB RAM, 1 CPU)
./scripts/utils/test-qemu.sh

# Custom configuration
MEMORY=4G CPUS=2 ./scripts/utils/test-qemu.sh
```

### Debugging

**`utils/test-qemu-debug.sh`** - Start QEMU with GDB server
- Waits for GDB connection on port 1234
- Stops at boot for debugging

```bash
# Terminal 1: Start QEMU
./scripts/utils/test-qemu-debug.sh

# Terminal 2: Connect GDB
./scripts/utils/debug-gdb.sh
```

**`utils/debug-gdb.sh`** - GDB debugging session
- Connects to QEMU GDB server
- Loads symbols automatically
- Pre-configured breakpoints
- Helper commands

```bash
# Start GDB session
./scripts/utils/debug-gdb.sh

# Show help
./scripts/utils/debug-gdb.sh --help
```

### Analysis

**`utils/analyze-kernel.sh`** - Analyze kernel binary
- Shows file sizes and sections
- Lists symbols and dependencies
- Displays memory layout

```bash
./scripts/utils/analyze-kernel.sh
```

## Build Options

### Environment Variables

Common variables that affect builds:

```bash
# Toolchain
export INSTALL_PREFIX=/opt/riscv     # Toolchain install location
export CROSS_COMPILE=riscv64-unknown-elf-

# Build configuration
export BUILD_TYPE=RELEASE            # RELEASE, DEBUG, or PROFILE
export JOBS=8                        # Parallel build jobs

# Output
export BUILD_ROOT=/path/to/build     # Build output directory
export OUTPUT_DIR=/path/to/output    # Kernel output directory
```

### Build Types

- **RELEASE**: Optimized build (-O2), no debug symbols
- **DEBUG**: Debug build (-g), with debug symbols, no optimization
- **PROFILE**: Profiling build with instrumentation

## Directory Structure

```
scripts/
├── build-all.sh              # Master build script
├── toolchain/
│   └── build-toolchain.sh    # Build RISC-V GCC
├── bootloader/
│   └── build-opensbi.sh      # Build OpenSBI firmware
├── kernel/
│   └── build-kernel.sh       # Build Darwin kernel
└── utils/
    ├── test-qemu.sh          # QEMU test runner
    ├── test-qemu-debug.sh    # QEMU debug server
    ├── debug-gdb.sh          # GDB debugger
    └── analyze-kernel.sh     # Kernel analyzer
```

## Typical Workflows

### First-Time Build

```bash
# 1. Build complete system (includes toolchain)
./scripts/build-all.sh

# 2. Test in QEMU
cd build
./run-qemu.sh
```

### Development Workflow

```bash
# 1. Make changes to kernel source
vim kernel-7/machdep/riscv/trap.c

# 2. Rebuild kernel only
./scripts/kernel/build-kernel.sh --clean

# 3. Test
./build/run-qemu.sh
```

### Debugging Workflow

```bash
# Terminal 1: Start QEMU in debug mode
./scripts/utils/test-qemu-debug.sh

# Terminal 2: Connect GDB
./scripts/utils/debug-gdb.sh
(gdb) break trap_handler
(gdb) continue
```

### Release Build

```bash
# Build optimized release version
./scripts/build-all.sh --type RELEASE --skip-toolchain

# Analyze the output
./scripts/utils/analyze-kernel.sh

# Create deployment package
cd build
tar czf darwin-riscv-$(date +%Y%m%d).tar.gz \
    boot/ kernel/ images/ *.sh README.md
```

## Requirements

### Software

- **Build Tools**: gcc, make, autoconf, automake, git
- **Libraries**: libmpc-dev, libmpfr-dev, libgmp-dev
- **Python**: python3 (for build scripts)
- **QEMU**: qemu-system-riscv64 (for testing)

### Hardware

- **Disk Space**: 10GB (15GB for toolchain build)
- **Memory**: 8GB RAM (16GB recommended for toolchain)
- **CPU**: Multi-core recommended for parallel builds

## Troubleshooting

### "riscv64-unknown-elf-gcc: command not found"

```bash
# Option 1: Build toolchain
./scripts/toolchain/build-toolchain.sh

# Option 2: Add to PATH
export PATH=/opt/riscv/bin:$PATH
```

### "Permission denied" errors

```bash
# Make scripts executable
chmod +x scripts/**/*.sh
```

### Build failures

```bash
# Clean and rebuild
./scripts/kernel/build-kernel.sh --clean

# Check build log
cat build/kernel/build.log
```

### QEMU issues

```bash
# Check QEMU version (need 6.0+)
qemu-system-riscv64 --version

# Install/update QEMU
sudo apt-get update
sudo apt-get install qemu-system-misc
```

## Advanced Usage

### Custom Platforms

To build for specific hardware:

1. Edit `kernel-7/conf/MASTER.riscv`
2. Create platform-specific code in `kernel-7/machdep/riscv/platforms/`
3. Rebuild kernel

### Cross-Platform Builds

Build on one machine, deploy to another:

```bash
# On build machine
./scripts/build-all.sh
tar czf darwin-riscv.tar.gz build/

# On target machine
tar xzf darwin-riscv.tar.gz
cd build
# Deploy to hardware
```

### Continuous Integration

Example CI configuration:

```yaml
# .github/workflows/build.yml
- name: Build Darwin/RISC-V
  run: |
    ./scripts/build-all.sh --skip-toolchain
    ./scripts/utils/analyze-kernel.sh
```

## Support

For build issues:
1. Check script output and logs
2. Review RISCV_BUILD.md for detailed documentation
3. Verify prerequisites are installed
4. Ensure disk space is available

For kernel issues:
1. Check RISCV_SUPPORT.md for implementation status
2. Use GDB for debugging
3. Review kernel build log

## References

- **Build Guide**: RISCV_BUILD.md
- **Implementation**: RISCV_SUPPORT.md
- **Porting Guide**: RISCV_PORTING_GUIDE.md
- **RISC-V Spec**: https://riscv.org/specifications/
- **OpenSBI**: https://github.com/riscv/opensbi
- **GNU Toolchain**: https://github.com/riscv/riscv-gnu-toolchain
