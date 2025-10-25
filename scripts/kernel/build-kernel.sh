#!/bin/bash
#
# Build Darwin-0.3 Kernel for RISC-V
#
# This script builds the Darwin kernel with all necessary steps
#

set -e

# Configuration
DARWIN_ROOT="${DARWIN_ROOT:-$(cd "$(dirname "$0")/../.." && pwd)}"
BUILD_TYPE="${BUILD_TYPE:-RELEASE}"
ARCH="${ARCH:-RISCV}"
JOBS="${JOBS:-$(nproc)}"
OUTPUT_DIR="${OUTPUT_DIR:-$(pwd)/build/kernel}"
CROSS_COMPILE="${CROSS_COMPILE:-riscv64-unknown-elf-}"

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
BLUE='\033[0;34m'
NC='\033[0m'

log_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

log_build() {
    echo -e "${BLUE}[BUILD]${NC} $1"
}

check_toolchain() {
    log_info "Checking for RISC-V toolchain..."

    if ! command -v ${CROSS_COMPILE}gcc &> /dev/null; then
        log_error "RISC-V toolchain not found!"
        log_info "Set CROSS_COMPILE or install toolchain:"
        log_info "  ./scripts/toolchain/build-toolchain.sh"
        exit 1
    fi

    local version=$(${CROSS_COMPILE}gcc --version | head -n1)
    log_info "✓ Toolchain: $version"
}

check_sources() {
    log_info "Checking Darwin source tree..."

    if [ ! -d "$DARWIN_ROOT/kernel-7" ]; then
        log_error "Darwin source not found at: $DARWIN_ROOT"
        exit 1
    fi

    if [ ! -f "$DARWIN_ROOT/kernel-7/conf/MASTER.riscv" ]; then
        log_error "RISC-V configuration not found!"
        log_info "Ensure RISC-V support is integrated"
        exit 1
    fi

    log_info "✓ Darwin sources found"
}

clean_build() {
    if [ "$CLEAN" = "1" ]; then
        log_info "Cleaning previous build..."
        cd "$DARWIN_ROOT/kernel-7/conf"
        make ARCH=$ARCH TYPE=$BUILD_TYPE clean || true
    fi
}

configure_build() {
    log_info "Configuring build..."
    log_info "  Architecture: $ARCH"
    log_info "  Build type: $BUILD_TYPE"
    log_info "  Jobs: $JOBS"
    log_info "  Output: $OUTPUT_DIR"

    cd "$DARWIN_ROOT/kernel-7/conf"

    # Show configuration
    log_info "Build configuration from MASTER.$ARCH:"
    head -20 "MASTER.$ARCH" | grep -E '^(machine|cpu|options|makeoptions)' || true
}

build_kernel() {
    log_build "Building Darwin kernel..."

    cd "$DARWIN_ROOT/kernel-7/conf"

    # Build with progress
    make ARCH=$ARCH TYPE=$BUILD_TYPE mach_kernel.kernel -j$JOBS 2>&1 | tee build.log

    # Check if build succeeded
    if [ ! -f "${BUILD_TYPE}_${ARCH}/mach_kernel.kernel" ]; then
        log_error "Build failed! Check build.log for details"
        exit 1
    fi

    log_info "✓ Kernel built successfully"
}

convert_to_binary() {
    log_info "Converting kernel to binary format..."

    cd "$DARWIN_ROOT/kernel-7/conf/${BUILD_TYPE}_${ARCH}"

    # Convert ELF to binary
    ${CROSS_COMPILE}objcopy \
        -O binary \
        mach_kernel.kernel \
        mach_kernel.bin

    log_info "✓ Binary created: mach_kernel.bin"
}

generate_symbols() {
    log_info "Generating symbol file..."

    cd "$DARWIN_ROOT/kernel-7/conf/${BUILD_TYPE}_${ARCH}"

    # Generate symbol map
    ${CROSS_COMPILE}nm -n mach_kernel.kernel > mach_kernel.sym

    # Generate disassembly (optional)
    if [ "$DISASM" = "1" ]; then
        log_info "Generating disassembly..."
        ${CROSS_COMPILE}objdump -d mach_kernel.kernel > mach_kernel.S
    fi

    log_info "✓ Symbols: mach_kernel.sym"
}

analyze_kernel() {
    log_info "Analyzing kernel image..."

    cd "$DARWIN_ROOT/kernel-7/conf/${BUILD_TYPE}_${ARCH}"

    local elf_size=$(stat -f%z mach_kernel.kernel 2>/dev/null || stat -c%s mach_kernel.kernel)
    local bin_size=$(stat -f%z mach_kernel.bin 2>/dev/null || stat -c%s mach_kernel.bin)

    echo ""
    echo "Kernel Statistics:"
    echo "=================="
    ${CROSS_COMPILE}size mach_kernel.kernel

    echo ""
    echo "File Sizes:"
    echo "==========="
    printf "  ELF: %'d bytes (%.2f MB)\n" $elf_size $(echo "scale=2; $elf_size/1048576" | bc)
    printf "  BIN: %'d bytes (%.2f MB)\n" $bin_size $(echo "scale=2; $bin_size/1048576" | bc)

    echo ""
    echo "Section Information:"
    echo "==================="
    ${CROSS_COMPILE}objdump -h mach_kernel.kernel | grep -A1 "Idx Name"

    echo ""
}

copy_to_output() {
    log_info "Copying artifacts to output directory..."

    mkdir -p "$OUTPUT_DIR"

    cd "$DARWIN_ROOT/kernel-7/conf/${BUILD_TYPE}_${ARCH}"

    cp mach_kernel.kernel "$OUTPUT_DIR/"
    cp mach_kernel.bin "$OUTPUT_DIR/"
    cp mach_kernel.sym "$OUTPUT_DIR/"
    [ -f mach_kernel.S ] && cp mach_kernel.S "$OUTPUT_DIR/"
    cp ../build.log "$OUTPUT_DIR/"

    log_info "✓ Artifacts copied to: $OUTPUT_DIR"
}

create_boot_config() {
    log_info "Creating boot configuration..."

    cat > "$OUTPUT_DIR/boot.conf" << EOF
# Darwin-0.3 RISC-V Boot Configuration
# Generated: $(date)

# Kernel Information
KERNEL_ELF=$OUTPUT_DIR/mach_kernel.kernel
KERNEL_BIN=$OUTPUT_DIR/mach_kernel.bin
KERNEL_LOAD_ADDR=0x80200000

# Build Information
BUILD_DATE=$(date +%Y-%m-%d)
BUILD_TYPE=$BUILD_TYPE
BUILD_ARCH=$ARCH
BUILD_HOST=$(hostname)

# QEMU Test Command
QEMU_CMD=qemu-system-riscv64 -machine virt -cpu rv64 -m 2G \\
  -bios fw_jump.bin \\
  -kernel $OUTPUT_DIR/mach_kernel.bin \\
  -serial stdio -nographic \\
  -append "console=uart8250,mmio,0x10000000"

# Serial Console
CONSOLE=uart8250,mmio,0x10000000,115200n8
EOF

    log_info "✓ Boot config: $OUTPUT_DIR/boot.conf"
}

run_tests() {
    if [ "$RUN_TESTS" != "1" ]; then
        return
    fi

    log_info "Running kernel tests..."

    # Basic sanity checks
    cd "$DARWIN_ROOT/kernel-7/conf/${BUILD_TYPE}_${ARCH}"

    # Check for required symbols
    log_info "Checking for required symbols..."
    local required_symbols=("_start" "_riscv_init" "trap_handler" "pmap_bootstrap")

    for sym in "${required_symbols[@]}"; do
        if ${CROSS_COMPILE}nm mach_kernel.kernel | grep -q " $sym$"; then
            log_info "  ✓ Found: $sym"
        else
            log_warn "  ✗ Missing: $sym"
        fi
    done
}

generate_readme() {
    log_info "Generating README..."

    cat > "$OUTPUT_DIR/README.md" << 'EOF'
# Darwin-0.3 RISC-V Kernel

This directory contains the built Darwin kernel for RISC-V.

## Files

- `mach_kernel.kernel` - ELF executable (for debugging)
- `mach_kernel.bin` - Raw binary (for bootloader)
- `mach_kernel.sym` - Symbol table
- `mach_kernel.S` - Disassembly (if generated)
- `build.log` - Build log
- `boot.conf` - Boot configuration

## Testing with QEMU

```bash
# Install QEMU
sudo apt-get install qemu-system-misc

# Run kernel
qemu-system-riscv64 \
    -machine virt \
    -cpu rv64 \
    -m 2G \
    -bios /path/to/fw_jump.bin \
    -kernel mach_kernel.bin \
    -serial stdio \
    -nographic
```

## Debugging with GDB

```bash
# Terminal 1: Start QEMU with GDB server
qemu-system-riscv64 \
    -machine virt -cpu rv64 -m 2G \
    -kernel mach_kernel.bin \
    -serial stdio -nographic \
    -s -S

# Terminal 2: Connect GDB
riscv64-unknown-elf-gdb mach_kernel.kernel
(gdb) target remote :1234
(gdb) break _start
(gdb) continue
```

## Kernel Configuration

This kernel was built with the following configuration:
EOF

    # Append build info
    cat "$OUTPUT_DIR/boot.conf" >> "$OUTPUT_DIR/README.md"

    log_info "✓ README: $OUTPUT_DIR/README.md"
}

print_summary() {
    echo ""
    echo "=========================================="
    echo "  Darwin Kernel Build Complete!"
    echo "=========================================="
    echo ""
    echo "Build artifacts:"
    ls -lh "$OUTPUT_DIR"/mach_kernel.* 2>/dev/null || true
    echo ""
    echo "Quick test:"
    echo "  qemu-system-riscv64 -machine virt -cpu rv64 -m 2G \\"
    echo "    -bios fw_jump.bin \\"
    echo "    -kernel $OUTPUT_DIR/mach_kernel.bin \\"
    echo "    -serial stdio -nographic"
    echo ""
    echo "Output directory: $OUTPUT_DIR"
    echo ""
}

main() {
    echo "=========================================="
    echo "  Darwin-0.3 Kernel Builder"
    echo "  RISC-V Architecture"
    echo "=========================================="
    echo ""

    # Parse arguments
    while [[ $# -gt 0 ]]; do
        case $1 in
            --clean)
                CLEAN=1
                shift
                ;;
            --type)
                BUILD_TYPE="$2"
                shift 2
                ;;
            --disasm)
                DISASM=1
                shift
                ;;
            --test)
                RUN_TESTS=1
                shift
                ;;
            --output)
                OUTPUT_DIR="$2"
                shift 2
                ;;
            *)
                log_error "Unknown option: $1"
                echo "Usage: $0 [--clean] [--type TYPE] [--disasm] [--test] [--output DIR]"
                exit 1
                ;;
        esac
    done

    check_toolchain
    check_sources
    clean_build
    configure_build
    build_kernel
    convert_to_binary
    generate_symbols
    analyze_kernel
    copy_to_output
    create_boot_config
    run_tests
    generate_readme
    print_summary

    log_info "Build complete!"
}

main "$@"
