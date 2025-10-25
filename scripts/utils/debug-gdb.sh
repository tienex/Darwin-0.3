#!/bin/bash
#
# Debug Darwin-0.3 with GDB
#
# This script sets up a GDB debugging session
#

set -e

# Configuration
KERNEL_ELF="${KERNEL_ELF:-build/kernel/mach_kernel.kernel}"
GDB="${GDB:-riscv64-unknown-elf-gdb}"

# Colors
GREEN='\033[0;32m'
CYAN='\033[0;36m'
NC='\033[0m'

log_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

log_cmd() {
    echo -e "${CYAN}[CMD]${NC} $1"
}

check_gdb() {
    if ! command -v $GDB &> /dev/null; then
        echo "GDB not found: $GDB"
        echo "Install with toolchain or set GDB environment variable"
        exit 1
    fi

    local version=$($GDB --version | head -n1)
    log_info "GDB: $version"
}

check_kernel() {
    if [ ! -f "$KERNEL_ELF" ]; then
        echo "Kernel not found: $KERNEL_ELF"
        echo "Run: ./scripts/kernel/build-kernel.sh"
        exit 1
    fi

    log_info "Kernel: $KERNEL_ELF"
}

create_gdb_script() {
    cat > /tmp/darwin-gdb-init << 'EOF'
# Darwin-0.3 GDB Configuration

# Connect to QEMU
target remote :1234

# Set architecture
set architecture riscv:rv64

# Load symbols
symbol-file build/kernel/mach_kernel.kernel

# Useful breakpoints
break _start
break _riscv_init
break trap_handler

# Display setup
set disassemble-next-line on
set print pretty on

# Helper commands
define regs
    info registers
end
document regs
    Display all registers
end

define stack
    backtrace full
end
document stack
    Display full stack trace
end

# Print welcome message
echo \n
echo ==========================================\n
echo   Darwin-0.3 RISC-V Debugging Session\n
echo ==========================================\n
echo \n
echo Useful commands:\n
echo   regs    - Show all registers\n
echo   stack   - Show stack trace\n
echo   c       - Continue execution\n
echo   n       - Next instruction\n
echo   s       - Step into\n
echo \n
echo Ready to debug. Type 'continue' to start.\n
echo \n
EOF
}

print_instructions() {
    echo ""
    echo "=========================================="
    echo "  GDB Debugging Instructions"
    echo "=========================================="
    echo ""
    echo "1. In another terminal, start QEMU with:"
    echo ""
    log_cmd "   ./scripts/utils/test-qemu-debug.sh"
    echo ""
    echo "2. QEMU will wait for GDB to connect"
    echo "3. This GDB session will connect automatically"
    echo ""
    echo "Common GDB commands:"
    echo "  continue (c)  - Start/continue execution"
    echo "  break (b)     - Set breakpoint"
    echo "  next (n)      - Step over"
    echo "  step (s)      - Step into"
    echo "  info regs     - Show registers"
    echo "  backtrace (bt)- Show call stack"
    echo ""
}

main() {
    check_gdb
    check_kernel
    create_gdb_script

    if [ "$1" = "--help" ]; then
        print_instructions
        exit 0
    fi

    log_info "Starting GDB session..."
    log_info "GDB script: /tmp/darwin-gdb-init"

    $GDB -x /tmp/darwin-gdb-init
}

main "$@"
