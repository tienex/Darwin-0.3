#!/bin/bash
#
# Build RISC-V GNU Toolchain for Darwin-0.3
#
# This script builds a complete RISC-V cross-compilation toolchain
# supporting both RV32 and RV64 with all standard extensions.
#

set -e

# Configuration
TOOLCHAIN_VERSION="2023.10.18"
INSTALL_PREFIX="${INSTALL_PREFIX:-/opt/riscv}"
BUILD_DIR="${BUILD_DIR:-$(pwd)/build/toolchain}"
JOBS="${JOBS:-$(nproc)}"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

log_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

check_prerequisites() {
    log_info "Checking prerequisites..."

    local missing_deps=()

    # Check for required tools
    for cmd in git make gcc g++ autoconf automake; do
        if ! command -v $cmd &> /dev/null; then
            missing_deps+=($cmd)
        fi
    done

    if [ ${#missing_deps[@]} -ne 0 ]; then
        log_error "Missing required dependencies: ${missing_deps[*]}"
        log_info "On Ubuntu/Debian, install with:"
        echo "  sudo apt-get install autoconf automake autotools-dev curl python3 \\"
        echo "      libmpc-dev libmpfr-dev libgmp-dev gawk build-essential bison \\"
        echo "      flex texinfo gperf libtool patchutils bc zlib1g-dev libexpat-dev"
        exit 1
    fi

    log_info "All prerequisites satisfied"
}

download_toolchain() {
    log_info "Downloading RISC-V GNU toolchain..."

    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"

    if [ -d "riscv-gnu-toolchain" ]; then
        log_warn "Toolchain source already exists, using existing directory"
        cd riscv-gnu-toolchain
        git pull || true
    else
        git clone --recursive https://github.com/riscv/riscv-gnu-toolchain
        cd riscv-gnu-toolchain
        git checkout "$TOOLCHAIN_VERSION"
        git submodule update --init --recursive
    fi
}

build_rv64_toolchain() {
    log_info "Building RV64 toolchain..."
    log_info "  Architecture: rv64gc (RV64IMAFD + Compressed)"
    log_info "  ABI: lp64d (64-bit with hardware FP)"
    log_info "  Install prefix: $INSTALL_PREFIX"
    log_info "  Jobs: $JOBS"

    cd "$BUILD_DIR/riscv-gnu-toolchain"

    # Clean previous build
    make clean || true

    # Configure
    ./configure \
        --prefix="$INSTALL_PREFIX" \
        --with-arch=rv64gc \
        --with-abi=lp64d \
        --enable-multilib

    # Build
    log_info "Building (this may take 30-60 minutes)..."
    make -j"$JOBS" 2>&1 | tee build-rv64.log

    log_info "RV64 toolchain built successfully"
}

build_rv32_toolchain() {
    log_info "Building RV32 toolchain (optional)..."
    log_info "  Architecture: rv32gc (RV32IMAFD + Compressed)"
    log_info "  ABI: ilp32d (32-bit with hardware FP)"

    read -p "Do you want to build RV32 toolchain? (y/N) " -n 1 -r
    echo

    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        log_info "Skipping RV32 toolchain"
        return
    fi

    cd "$BUILD_DIR"

    # Clone again for RV32 (or use different build directory)
    if [ ! -d "riscv-gnu-toolchain-rv32" ]; then
        cp -r riscv-gnu-toolchain riscv-gnu-toolchain-rv32
    fi

    cd riscv-gnu-toolchain-rv32

    # Configure for RV32
    ./configure \
        --prefix="${INSTALL_PREFIX}-rv32" \
        --with-arch=rv32gc \
        --with-abi=ilp32d

    # Build
    make -j"$JOBS" 2>&1 | tee build-rv32.log

    log_info "RV32 toolchain built successfully"
}

install_toolchain() {
    log_info "Installing toolchain to $INSTALL_PREFIX..."

    cd "$BUILD_DIR/riscv-gnu-toolchain"

    # Install
    sudo make install

    log_info "Toolchain installed successfully"
}

configure_environment() {
    log_info "Configuring environment..."

    # Add to PATH in shell configuration
    local shell_rc="$HOME/.bashrc"
    if [ -f "$HOME/.zshrc" ]; then
        shell_rc="$HOME/.zshrc"
    fi

    if ! grep -q "$INSTALL_PREFIX/bin" "$shell_rc"; then
        echo "" >> "$shell_rc"
        echo "# RISC-V Toolchain" >> "$shell_rc"
        echo "export PATH=\"$INSTALL_PREFIX/bin:\$PATH\"" >> "$shell_rc"
        log_info "Added toolchain to PATH in $shell_rc"
    else
        log_info "Toolchain already in PATH"
    fi

    # Export for current session
    export PATH="$INSTALL_PREFIX/bin:$PATH"
}

verify_installation() {
    log_info "Verifying installation..."

    export PATH="$INSTALL_PREFIX/bin:$PATH"

    if command -v riscv64-unknown-elf-gcc &> /dev/null; then
        local version=$(riscv64-unknown-elf-gcc --version | head -n1)
        log_info "✓ GCC: $version"
    else
        log_error "GCC not found in PATH"
        return 1
    fi

    if command -v riscv64-unknown-elf-ld &> /dev/null; then
        local version=$(riscv64-unknown-elf-ld --version | head -n1)
        log_info "✓ LD: $version"
    else
        log_error "LD not found in PATH"
        return 1
    fi

    # Test compilation
    log_info "Testing compilation..."
    cat > /tmp/test-riscv.c << 'EOF'
#include <stdio.h>
int main() {
    return 0;
}
EOF

    if riscv64-unknown-elf-gcc -march=rv64g -mabi=lp64d -o /tmp/test-riscv /tmp/test-riscv.c 2>&1; then
        log_info "✓ Test compilation successful"
        rm -f /tmp/test-riscv /tmp/test-riscv.c
    else
        log_error "Test compilation failed"
        return 1
    fi

    log_info "All verification tests passed!"
}

print_summary() {
    echo ""
    echo "=========================================="
    echo "  RISC-V Toolchain Build Complete!"
    echo "=========================================="
    echo ""
    echo "Installation directory: $INSTALL_PREFIX"
    echo ""
    echo "Available tools:"
    echo "  riscv64-unknown-elf-gcc      - C compiler"
    echo "  riscv64-unknown-elf-g++      - C++ compiler"
    echo "  riscv64-unknown-elf-as       - Assembler"
    echo "  riscv64-unknown-elf-ld       - Linker"
    echo "  riscv64-unknown-elf-objcopy  - Object file converter"
    echo "  riscv64-unknown-elf-objdump  - Disassembler"
    echo "  riscv64-unknown-elf-gdb      - Debugger"
    echo ""
    echo "To use the toolchain, add this to your PATH:"
    echo "  export PATH=\"$INSTALL_PREFIX/bin:\$PATH\""
    echo ""
    echo "This has been added to your shell configuration."
    echo "Run 'source ~/.bashrc' or start a new shell session."
    echo ""
}

main() {
    echo "=========================================="
    echo "  RISC-V GNU Toolchain Builder"
    echo "  for Darwin-0.3"
    echo "=========================================="
    echo ""

    check_prerequisites
    download_toolchain
    build_rv64_toolchain
    build_rv32_toolchain
    install_toolchain
    configure_environment
    verify_installation
    print_summary

    log_info "Build process complete!"
}

# Run main function
main "$@"
