#!/bin/bash
#
# Build OpenSBI Bootloader for Darwin-0.3/RISC-V
#
# This script builds OpenSBI firmware images for various platforms
#

set -e

# Configuration
OPENSBI_VERSION="v1.3"
BUILD_DIR="${BUILD_DIR:-$(pwd)/build/opensbi}"
OUTPUT_DIR="${OUTPUT_DIR:-$(pwd)/build/boot}"
CROSS_COMPILE="${CROSS_COMPILE:-riscv64-unknown-elf-}"
PLATFORM="${PLATFORM:-generic}"
FW_JUMP_ADDR="${FW_JUMP_ADDR:-0x80200000}"

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
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

check_toolchain() {
    log_info "Checking for RISC-V toolchain..."

    if ! command -v ${CROSS_COMPILE}gcc &> /dev/null; then
        log_error "RISC-V toolchain not found!"
        log_info "Please install toolchain or run:"
        log_info "  ./scripts/toolchain/build-toolchain.sh"
        exit 1
    fi

    local version=$(${CROSS_COMPILE}gcc --version | head -n1)
    log_info "Found toolchain: $version"
}

download_opensbi() {
    log_info "Downloading OpenSBI..."

    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"

    if [ -d "opensbi" ]; then
        log_warn "OpenSBI source already exists, updating..."
        cd opensbi
        git fetch
        git checkout "$OPENSBI_VERSION"
    else
        git clone https://github.com/riscv/opensbi.git
        cd opensbi
        git checkout "$OPENSBI_VERSION"
    fi
}

build_opensbi_generic() {
    log_info "Building OpenSBI for generic platform..."
    log_info "  Platform: $PLATFORM"
    log_info "  Jump address: $FW_JUMP_ADDR"
    log_info "  Cross compiler: $CROSS_COMPILE"

    cd "$BUILD_DIR/opensbi"

    # Clean previous build
    make distclean || true

    # Build firmware
    make \
        CROSS_COMPILE=$CROSS_COMPILE \
        PLATFORM=$PLATFORM \
        FW_JUMP_ADDR=$FW_JUMP_ADDR \
        -j$(nproc) 2>&1 | tee build.log

    log_info "Build complete!"
}

build_opensbi_qemu() {
    log_info "Building OpenSBI for QEMU virt machine..."

    PLATFORM="generic" \
    FW_JUMP_ADDR="0x80200000" \
    build_opensbi_generic

    # Copy to output
    mkdir -p "$OUTPUT_DIR/qemu"
    cp build/platform/generic/firmware/fw_jump.bin "$OUTPUT_DIR/qemu/"
    cp build/platform/generic/firmware/fw_jump.elf "$OUTPUT_DIR/qemu/"

    log_info "QEMU firmware: $OUTPUT_DIR/qemu/fw_jump.bin"
}

build_opensbi_sifive() {
    log_info "Building OpenSBI for SiFive platforms..."

    # SiFive HiFive Unleashed / Unmatched
    PLATFORM="generic" \
    FW_JUMP_ADDR="0x80200000" \
    build_opensbi_generic

    # Copy to output
    mkdir -p "$OUTPUT_DIR/sifive"
    cp build/platform/generic/firmware/fw_jump.bin "$OUTPUT_DIR/sifive/"
    cp build/platform/generic/firmware/fw_jump.elf "$OUTPUT_DIR/sifive/"

    log_info "SiFive firmware: $OUTPUT_DIR/sifive/fw_jump.bin"
}

build_payload_firmware() {
    log_info "Building firmware with embedded kernel payload..."

    if [ ! -f "$DARWIN_KERNEL" ]; then
        log_warn "Darwin kernel not found at: $DARWIN_KERNEL"
        log_warn "Skipping payload firmware build"
        return
    fi

    cd "$BUILD_DIR/opensbi"

    make \
        CROSS_COMPILE=$CROSS_COMPILE \
        PLATFORM=$PLATFORM \
        FW_PAYLOAD_PATH="$DARWIN_KERNEL" \
        -j$(nproc)

    mkdir -p "$OUTPUT_DIR/payload"
    cp build/platform/generic/firmware/fw_payload.bin "$OUTPUT_DIR/payload/"
    cp build/platform/generic/firmware/fw_payload.elf "$OUTPUT_DIR/payload/"

    log_info "Payload firmware: $OUTPUT_DIR/payload/fw_payload.bin"
}

create_boot_images() {
    log_info "Creating bootable images..."

    mkdir -p "$OUTPUT_DIR/images"

    # Create combined image if kernel is available
    if [ -f "$DARWIN_KERNEL" ]; then
        log_info "Creating combined boot image (OpenSBI + Darwin)..."

        # fw_jump.bin at offset 0
        # Darwin kernel at offset 2MB (0x200000)
        dd if=/dev/zero of="$OUTPUT_DIR/images/darwin-boot.img" bs=1M count=16
        dd if="$OUTPUT_DIR/qemu/fw_jump.bin" of="$OUTPUT_DIR/images/darwin-boot.img" conv=notrunc
        dd if="$DARWIN_KERNEL" of="$OUTPUT_DIR/images/darwin-boot.img" seek=2 bs=1M conv=notrunc

        log_info "Combined image: $OUTPUT_DIR/images/darwin-boot.img"
    fi
}

generate_documentation() {
    log_info "Generating firmware documentation..."

    cat > "$OUTPUT_DIR/README.md" << 'EOF'
# OpenSBI Firmware for Darwin-0.3/RISC-V

## Firmware Images

### fw_jump.bin
Jump firmware - loads and jumps to kernel at specified address.

**Usage with QEMU:**
```bash
qemu-system-riscv64 \
    -machine virt \
    -cpu rv64 \
    -m 2G \
    -bios fw_jump.bin \
    -kernel darwin-kernel.bin \
    -serial stdio \
    -nographic
```

### fw_payload.bin
Payload firmware - kernel embedded in firmware image.

**Usage with QEMU:**
```bash
qemu-system-riscv64 \
    -machine virt \
    -cpu rv64 \
    -m 2G \
    -bios none \
    -kernel fw_payload.bin \
    -serial stdio \
    -nographic
```

## Memory Layout

```
0x80000000  - OpenSBI firmware (M-mode)
0x80200000  - Darwin kernel (S-mode)
```

## Platform Configuration

- **Generic**: Works with QEMU virt and most SiFive platforms
- **Jump Address**: 0x80200000 (2MB offset from firmware)
- **Build**: OpenSBI v1.3

## Files

- `fw_jump.bin` - Binary firmware (for bootloader)
- `fw_jump.elf` - ELF firmware (for debugging)
- `fw_payload.bin` - Firmware with embedded kernel
- `darwin-boot.img` - Combined bootable image

## Building Custom Firmware

To rebuild with different settings:

```bash
export PLATFORM=generic
export FW_JUMP_ADDR=0x80200000
export DARWIN_KERNEL=/path/to/mach_kernel.bin
./scripts/bootloader/build-opensbi.sh
```

## References

- OpenSBI: https://github.com/riscv/opensbi
- RISC-V SBI Spec: https://github.com/riscv/riscv-sbi-doc
EOF

    log_info "Documentation: $OUTPUT_DIR/README.md"
}

print_summary() {
    echo ""
    echo "=========================================="
    echo "  OpenSBI Build Complete!"
    echo "=========================================="
    echo ""
    echo "Firmware location: $OUTPUT_DIR"
    echo ""
    ls -lh "$OUTPUT_DIR"/*/fw_*.bin 2>/dev/null || true
    echo ""
    echo "Platforms built:"
    echo "  ✓ Generic (QEMU, SiFive)"
    echo ""
    if [ -f "$OUTPUT_DIR/images/darwin-boot.img" ]; then
        echo "Boot image:"
        echo "  ✓ darwin-boot.img (combined firmware + kernel)"
        echo ""
    fi
    echo "To test with QEMU:"
    echo "  qemu-system-riscv64 -machine virt -cpu rv64 -m 2G \\"
    echo "    -bios $OUTPUT_DIR/qemu/fw_jump.bin \\"
    echo "    -kernel <darwin-kernel.bin> \\"
    echo "    -serial stdio -nographic"
    echo ""
}

main() {
    echo "=========================================="
    echo "  OpenSBI Bootloader Builder"
    echo "  for Darwin-0.3/RISC-V"
    echo "=========================================="
    echo ""

    # Parse arguments
    while [[ $# -gt 0 ]]; do
        case $1 in
            --platform)
                PLATFORM="$2"
                shift 2
                ;;
            --kernel)
                export DARWIN_KERNEL="$2"
                shift 2
                ;;
            --jump-addr)
                FW_JUMP_ADDR="$2"
                shift 2
                ;;
            *)
                log_error "Unknown option: $1"
                exit 1
                ;;
        esac
    done

    check_toolchain
    download_opensbi
    build_opensbi_qemu
    build_opensbi_sifive

    # Build payload if kernel is provided
    if [ -n "$DARWIN_KERNEL" ]; then
        build_payload_firmware
        create_boot_images
    fi

    generate_documentation
    print_summary

    log_info "Build complete!"
}

main "$@"
