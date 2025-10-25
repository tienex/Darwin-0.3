#!/bin/bash
#
# Test Darwin-0.3 in QEMU
#
# This script provides easy testing of Darwin kernel in QEMU
#

set -e

# Configuration
FW_JUMP="${FW_JUMP:-build/boot/qemu/fw_jump.bin}"
KERNEL="${KERNEL:-build/kernel/mach_kernel.bin}"
MEMORY="${MEMORY:-2G}"
CPUS="${CPUS:-1}"

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

log_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

check_qemu() {
    if ! command -v qemu-system-riscv64 &> /dev/null; then
        log_warn "QEMU not found!"
        log_info "Install with:"
        echo "  Ubuntu/Debian: sudo apt-get install qemu-system-misc"
        echo "  macOS: brew install qemu"
        exit 1
    fi

    local version=$(qemu-system-riscv64 --version | head -n1)
    log_info "QEMU: $version"
}

check_files() {
    if [ ! -f "$FW_JUMP" ]; then
        log_warn "Firmware not found: $FW_JUMP"
        log_info "Run: ./scripts/bootloader/build-opensbi.sh"
        exit 1
    fi

    if [ ! -f "$KERNEL" ]; then
        log_warn "Kernel not found: $KERNEL"
        log_info "Run: ./scripts/kernel/build-kernel.sh"
        exit 1
    fi

    log_info "Firmware: $FW_JUMP"
    log_info "Kernel: $KERNEL"
}

run_qemu() {
    log_info "Starting QEMU..."
    log_info "Memory: $MEMORY"
    log_info "CPUs: $CPUS"
    echo ""
    echo "Press Ctrl-A then X to exit QEMU"
    echo ""

    qemu-system-riscv64 \
        -machine virt \
        -cpu rv64 \
        -smp $CPUS \
        -m $MEMORY \
        -bios "$FW_JUMP" \
        -kernel "$KERNEL" \
        -serial stdio \
        -nographic \
        "$@"
}

main() {
    echo "=========================================="
    echo "  Darwin-0.3 QEMU Test"
    echo "=========================================="
    echo ""

    check_qemu
    check_files
    run_qemu "$@"
}

main "$@"
