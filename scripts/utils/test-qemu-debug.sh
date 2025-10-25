#!/bin/bash
#
# Start QEMU with GDB server for debugging
#

set -e

# Configuration
FW_JUMP="${FW_JUMP:-build/boot/qemu/fw_jump.bin}"
KERNEL="${KERNEL:-build/kernel/mach_kernel.bin}"
MEMORY="${MEMORY:-2G}"
GDB_PORT="${GDB_PORT:-1234}"

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m'

log_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

echo "=========================================="
echo "  QEMU Debug Server"
echo "=========================================="
echo ""

if [ ! -f "$FW_JUMP" ] || [ ! -f "$KERNEL" ]; then
    log_warn "Missing firmware or kernel files"
    log_info "Run: ./scripts/build-all.sh"
    exit 1
fi

log_info "Firmware: $FW_JUMP"
log_info "Kernel: $KERNEL"
log_info "GDB port: $GDB_PORT"
echo ""
log_info "QEMU is waiting for GDB connection..."
log_info "In another terminal, run:"
echo ""
echo -e "${CYAN}  ./scripts/utils/debug-gdb.sh${NC}"
echo ""

qemu-system-riscv64 \
    -machine virt \
    -cpu rv64 \
    -m $MEMORY \
    -bios "$FW_JUMP" \
    -kernel "$KERNEL" \
    -serial stdio \
    -nographic \
    -gdb tcp::$GDB_PORT \
    -S \
    "$@"
