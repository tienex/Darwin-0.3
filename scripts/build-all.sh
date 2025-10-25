#!/bin/bash
#
# Complete Build Script for Darwin-0.3/RISC-V
#
# This script builds the entire system:
# 1. RISC-V toolchain (if needed)
# 2. OpenSBI bootloader
# 3. Darwin kernel
# 4. Bootable images
#

set -e

# Configuration
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
DARWIN_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_ROOT="${BUILD_ROOT:-$(pwd)/build}"
SKIP_TOOLCHAIN="${SKIP_TOOLCHAIN:-0}"
SKIP_BOOTLOADER="${SKIP_BOOTLOADER:-0}"
BUILD_TYPE="${BUILD_TYPE:-RELEASE}"

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
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

log_step() {
    echo -e "${CYAN}[STEP]${NC} $1"
}

print_header() {
    echo ""
    echo "╔════════════════════════════════════════╗"
    echo "║   Darwin-0.3 Complete Build System    ║"
    echo "║        RISC-V Architecture             ║"
    echo "╚════════════════════════════════════════╝"
    echo ""
}

check_prerequisites() {
    log_step "Checking prerequisites..."

    local missing=()

    # Required tools
    for tool in git make gcc; do
        if ! command -v $tool &> /dev/null; then
            missing+=($tool)
        fi
    done

    if [ ${#missing[@]} -ne 0 ]; then
        log_error "Missing required tools: ${missing[*]}"
        log_info "Install with:"
        echo "  Ubuntu/Debian: sudo apt-get install ${missing[*]}"
        echo "  macOS: brew install ${missing[*]}"
        exit 1
    fi

    log_info "✓ Prerequisites OK"
}

setup_environment() {
    log_step "Setting up build environment..."

    # Create build directories
    mkdir -p "$BUILD_ROOT"/{toolchain,opensbi,kernel,boot,images}

    # Set environment variables
    export DARWIN_ROOT
    export BUILD_DIR="$BUILD_ROOT"
    export OUTPUT_DIR="$BUILD_ROOT"

    log_info "✓ Build root: $BUILD_ROOT"
}

build_toolchain() {
    if [ "$SKIP_TOOLCHAIN" = "1" ]; then
        log_info "Skipping toolchain build (SKIP_TOOLCHAIN=1)"
        return
    fi

    # Check if toolchain already exists
    if command -v riscv64-unknown-elf-gcc &> /dev/null; then
        local version=$(riscv64-unknown-elf-gcc --version | head -n1)
        log_info "Toolchain already installed: $version"

        read -p "Rebuild toolchain? (y/N) " -n 1 -r
        echo
        if [[ ! $REPLY =~ ^[Yy]$ ]]; then
            return
        fi
    fi

    log_step "Building RISC-V toolchain..."
    log_warn "This will take 30-60 minutes..."

    cd "$DARWIN_ROOT"
    bash scripts/toolchain/build-toolchain.sh

    log_info "✓ Toolchain build complete"
}

build_bootloader() {
    if [ "$SKIP_BOOTLOADER" = "1" ]; then
        log_info "Skipping bootloader build (SKIP_BOOTLOADER=1)"
        return
    fi

    log_step "Building OpenSBI bootloader..."

    cd "$DARWIN_ROOT"
    bash scripts/bootloader/build-opensbi.sh \
        --platform generic

    log_info "✓ Bootloader build complete"
}

build_kernel() {
    log_step "Building Darwin kernel..."

    cd "$DARWIN_ROOT"
    bash scripts/kernel/build-kernel.sh \
        --type "$BUILD_TYPE" \
        --clean

    log_info "✓ Kernel build complete"
}

create_boot_image() {
    log_step "Creating bootable image..."

    local FW_FILE="$BUILD_ROOT/boot/qemu/fw_jump.bin"
    local KERNEL_FILE="$BUILD_ROOT/kernel/mach_kernel.bin"
    local OUTPUT_IMAGE="$BUILD_ROOT/images/darwin-riscv-boot.img"

    if [ ! -f "$FW_FILE" ]; then
        log_error "Firmware not found: $FW_FILE"
        return 1
    fi

    if [ ! -f "$KERNEL_FILE" ]; then
        log_error "Kernel not found: $KERNEL_FILE"
        return 1
    fi

    # Create 16MB image
    dd if=/dev/zero of="$OUTPUT_IMAGE" bs=1M count=16 2>/dev/null

    # Write firmware at offset 0
    dd if="$FW_FILE" of="$OUTPUT_IMAGE" conv=notrunc 2>/dev/null

    # Write kernel at 2MB offset (0x200000)
    dd if="$KERNEL_FILE" of="$OUTPUT_IMAGE" seek=2 bs=1M conv=notrunc 2>/dev/null

    log_info "✓ Boot image: $OUTPUT_IMAGE"
}

create_qemu_scripts() {
    log_step "Creating QEMU test scripts..."

    # Simple run script
    cat > "$BUILD_ROOT/run-qemu.sh" << 'EOF'
#!/bin/bash
# Run Darwin-0.3 in QEMU

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

qemu-system-riscv64 \
    -machine virt \
    -cpu rv64 \
    -m 2G \
    -bios "$SCRIPT_DIR/boot/qemu/fw_jump.bin" \
    -kernel "$SCRIPT_DIR/kernel/mach_kernel.bin" \
    -serial stdio \
    -nographic \
    "$@"
EOF

    # Debug script with GDB
    cat > "$BUILD_ROOT/debug-qemu.sh" << 'EOF'
#!/bin/bash
# Debug Darwin-0.3 in QEMU with GDB

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

echo "Starting QEMU with GDB server on port 1234..."
echo "Connect with: riscv64-unknown-elf-gdb $SCRIPT_DIR/kernel/mach_kernel.kernel"
echo ""

qemu-system-riscv64 \
    -machine virt \
    -cpu rv64 \
    -m 2G \
    -bios "$SCRIPT_DIR/boot/qemu/fw_jump.bin" \
    -kernel "$SCRIPT_DIR/kernel/mach_kernel.bin" \
    -serial stdio \
    -nographic \
    -s -S \
    "$@"
EOF

    chmod +x "$BUILD_ROOT/run-qemu.sh"
    chmod +x "$BUILD_ROOT/debug-qemu.sh"

    log_info "✓ QEMU scripts created"
}

generate_documentation() {
    log_step "Generating build documentation..."

    cat > "$BUILD_ROOT/README.md" << EOF
# Darwin-0.3 for RISC-V - Build Output

Generated: $(date)
Build type: $BUILD_TYPE
Build host: $(hostname)

## Directory Structure

\`\`\`
build/
├── toolchain/          # RISC-V GCC toolchain
├── opensbi/            # OpenSBI firmware source
├── boot/               # Bootloader binaries
│   ├── qemu/          # QEMU firmware
│   └── sifive/        # SiFive firmware
├── kernel/             # Darwin kernel
│   ├── mach_kernel.kernel   # ELF executable
│   ├── mach_kernel.bin      # Raw binary
│   └── mach_kernel.sym      # Symbols
├── images/             # Bootable images
│   └── darwin-riscv-boot.img
├── run-qemu.sh         # Test in QEMU
└── debug-qemu.sh       # Debug with GDB
\`\`\`

## Quick Start

### Test in QEMU

\`\`\`bash
./run-qemu.sh
\`\`\`

### Debug with GDB

Terminal 1:
\`\`\`bash
./debug-qemu.sh
\`\`\`

Terminal 2:
\`\`\`bash
riscv64-unknown-elf-gdb kernel/mach_kernel.kernel
(gdb) target remote :1234
(gdb) break _start
(gdb) continue
\`\`\`

## Rebuilding

\`\`\`bash
# Rebuild everything
cd $(dirname $BUILD_ROOT)
./scripts/build-all.sh

# Rebuild only kernel
./scripts/kernel/build-kernel.sh --clean

# Rebuild only bootloader
./scripts/bootloader/build-opensbi.sh
\`\`\`

## Components

### Toolchain
- **Version**: RISC-V GCC 10.x+
- **Target**: riscv64-unknown-elf
- **ISA**: rv64gc (RV64IMAFD + Compressed)
- **ABI**: lp64d (64-bit with hardware FP)

### Bootloader
- **Firmware**: OpenSBI v1.3
- **Type**: fw_jump.bin
- **Jump Address**: 0x80200000

### Kernel
- **Version**: Darwin-0.3
- **Architecture**: RISC-V 64-bit
- **Load Address**: 0x80200000
- **Build**: $BUILD_TYPE

## System Requirements

### Building
- Linux or macOS
- 8GB RAM (16GB recommended for toolchain build)
- 10GB free disk space
- Internet connection (for downloads)

### Testing
- QEMU 6.0+ with riscv64 support
- 2GB RAM for VM

## Troubleshooting

### QEMU hangs at boot
- Check that firmware and kernel are at correct addresses
- Verify serial console configuration

### No console output
- Ensure QEMU has \`-serial stdio\` option
- Check kernel console configuration

### Build errors
- Ensure toolchain is in PATH
- Check \`build.log\` files for details
- Verify all prerequisites are installed

## References

- Darwin Source: https://github.com/apple-oss-distributions/
- RISC-V ISA: https://riscv.org/specifications/
- OpenSBI: https://github.com/riscv/opensbi
- QEMU: https://www.qemu.org/docs/master/system/target-riscv.html

EOF

    log_info "✓ Documentation: $BUILD_ROOT/README.md"
}

generate_manifest() {
    log_step "Generating build manifest..."

    cat > "$BUILD_ROOT/MANIFEST.txt" << EOF
Darwin-0.3 RISC-V Build Manifest
=================================

Build Information:
  Date: $(date)
  Host: $(hostname)
  User: $(whoami)
  Type: $BUILD_TYPE

Component Versions:
  Darwin: 0.3
  OpenSBI: v1.3
  RISC-V GCC: $(riscv64-unknown-elf-gcc --version 2>/dev/null | head -n1 || echo "Not found")

Files Generated:
EOF

    # List all generated files with sizes
    find "$BUILD_ROOT" -type f -name "*.bin" -o -name "*.elf" -o -name "*.kernel" | while read file; do
        local size=$(stat -f%z "$file" 2>/dev/null || stat -c%s "$file")
        printf "  %-60s %10d bytes\n" "$(basename $file)" "$size" >> "$BUILD_ROOT/MANIFEST.txt"
    done

    log_info "✓ Manifest: $BUILD_ROOT/MANIFEST.txt"
}

run_post_build_checks() {
    log_step "Running post-build checks..."

    local errors=0

    # Check for required files
    local required_files=(
        "$BUILD_ROOT/boot/qemu/fw_jump.bin"
        "$BUILD_ROOT/kernel/mach_kernel.bin"
        "$BUILD_ROOT/kernel/mach_kernel.kernel"
    )

    for file in "${required_files[@]}"; do
        if [ -f "$file" ]; then
            log_info "✓ $(basename $file)"
        else
            log_error "✗ Missing: $(basename $file)"
            errors=$((errors + 1))
        fi
    done

    if [ $errors -gt 0 ]; then
        log_error "Build verification failed with $errors errors"
        return 1
    fi

    log_info "✓ All checks passed"
}

print_summary() {
    echo ""
    echo "╔════════════════════════════════════════╗"
    echo "║      Build Complete Successfully!      ║"
    echo "╚════════════════════════════════════════╝"
    echo ""
    echo "Build output: $BUILD_ROOT"
    echo ""
    echo "Quick Test:"
    echo "  cd $BUILD_ROOT"
    echo "  ./run-qemu.sh"
    echo ""
    echo "Components built:"
    if [ "$SKIP_TOOLCHAIN" != "1" ]; then
        echo "  ✓ RISC-V Toolchain"
    fi
    if [ "$SKIP_BOOTLOADER" != "1" ]; then
        echo "  ✓ OpenSBI Bootloader"
    fi
    echo "  ✓ Darwin Kernel"
    echo "  ✓ Boot Images"
    echo ""
    echo "Next steps:"
    echo "  1. Test in QEMU: $BUILD_ROOT/run-qemu.sh"
    echo "  2. Debug: $BUILD_ROOT/debug-qemu.sh"
    echo "  3. Deploy to hardware (see README.md)"
    echo ""
}

main() {
    print_header

    # Parse arguments
    while [[ $# -gt 0 ]]; do
        case $1 in
            --skip-toolchain)
                SKIP_TOOLCHAIN=1
                shift
                ;;
            --skip-bootloader)
                SKIP_BOOTLOADER=1
                shift
                ;;
            --type)
                BUILD_TYPE="$2"
                shift 2
                ;;
            --help)
                echo "Usage: $0 [OPTIONS]"
                echo ""
                echo "Options:"
                echo "  --skip-toolchain      Skip toolchain build"
                echo "  --skip-bootloader     Skip bootloader build"
                echo "  --type TYPE           Build type (RELEASE, DEBUG, PROFILE)"
                echo "  --help                Show this help"
                exit 0
                ;;
            *)
                log_error "Unknown option: $1"
                exit 1
                ;;
        esac
    done

    local start_time=$(date +%s)

    check_prerequisites
    setup_environment
    build_toolchain
    build_bootloader
    build_kernel
    create_boot_image
    create_qemu_scripts
    generate_documentation
    generate_manifest
    run_post_build_checks

    local end_time=$(date +%s)
    local duration=$((end_time - start_time))

    print_summary

    log_info "Total build time: $((duration / 60)) minutes $((duration % 60)) seconds"
}

# Trap errors
trap 'log_error "Build failed at line $LINENO"' ERR

main "$@"
