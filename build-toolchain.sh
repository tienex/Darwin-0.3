#!/bin/bash
#
# Darwin IA64 Toolchain Setup Script
#
# This script builds and configures the complete toolchain for
# building Darwin IA64 (Itanium) system.
#

set -e

# Configuration
TOOLCHAIN_PREFIX="${TOOLCHAIN_PREFIX:-/opt/ia64-darwin}"
TARGET_ARCH="ia64-unknown-linux-gnu"
BINUTILS_VERSION="2.41"
GCC_VERSION="13.2.0"
NPROC=$(nproc)

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

error() {
    echo -e "${RED}[ERROR]${NC} $1"
    exit 1
}

# Check prerequisites
check_prerequisites() {
    info "Checking prerequisites..."

    local missing=0
    for cmd in wget tar make gcc g++ bison flex; do
        if ! command -v $cmd &> /dev/null; then
            error "Required command '$cmd' not found. Please install it."
            missing=1
        fi
    done

    [ $missing -eq 1 ] && exit 1

    info "All prerequisites satisfied"
}

# Download sources
download_sources() {
    info "Downloading toolchain sources..."

    mkdir -p sources
    cd sources

    # Binutils
    if [ ! -f "binutils-${BINUTILS_VERSION}.tar.xz" ]; then
        info "Downloading binutils ${BINUTILS_VERSION}..."
        wget https://ftp.gnu.org/gnu/binutils/binutils-${BINUTILS_VERSION}.tar.xz
    fi

    # GCC
    if [ ! -f "gcc-${GCC_VERSION}.tar.xz" ]; then
        info "Downloading GCC ${GCC_VERSION}..."
        wget https://ftp.gnu.org/gnu/gcc/gcc-${GCC_VERSION}/gcc-${GCC_VERSION}.tar.xz
    fi

    cd ..
    info "Sources downloaded"
}

# Build binutils
build_binutils() {
    info "Building binutils for IA64..."

    cd sources
    tar xf binutils-${BINUTILS_VERSION}.tar.xz
    cd binutils-${BINUTILS_VERSION}

    mkdir -p build-ia64
    cd build-ia64

    ../configure \
        --target=${TARGET_ARCH} \
        --prefix=${TOOLCHAIN_PREFIX} \
        --enable-targets=ia64-linux \
        --disable-nls \
        --disable-werror

    make -j${NPROC}
    make install

    cd ../../..
    info "Binutils built and installed"
}

# Build GCC (cross-compiler)
build_gcc() {
    info "Building GCC for IA64..."

    cd sources
    tar xf gcc-${GCC_VERSION}.tar.xz
    cd gcc-${GCC_VERSION}

    # Download prerequisites
    ./contrib/download_prerequisites

    mkdir -p build-ia64
    cd build-ia64

    ../configure \
        --target=${TARGET_ARCH} \
        --prefix=${TOOLCHAIN_PREFIX} \
        --enable-languages=c,c++ \
        --disable-nls \
        --disable-libssp \
        --disable-libmudflap \
        --without-headers \
        --with-gnu-as \
        --with-gnu-ld

    make -j${NPROC} all-gcc
    make -j${NPROC} all-target-libgcc
    make install-gcc
    make install-target-libgcc

    cd ../../..
    info "GCC built and installed"
}

# Create environment setup script
create_env_script() {
    info "Creating environment setup script..."

    cat > ${TOOLCHAIN_PREFIX}/setup-env.sh << 'EOF'
#!/bin/bash
#
# Darwin IA64 Toolchain Environment Setup
#

export TOOLCHAIN_PREFIX="@@TOOLCHAIN_PREFIX@@"
export PATH="${TOOLCHAIN_PREFIX}/bin:${PATH}"
export LD_LIBRARY_PATH="${TOOLCHAIN_PREFIX}/lib:${LD_LIBRARY_PATH}"

# IA64 build variables
export ARCH=ia64
export CROSS_COMPILE=ia64-unknown-linux-gnu-

# Darwin build variables
export RC_ARCHS="ia64 ia64be ia64_32"
export DARWIN_IA64_ROOT="$(pwd)"

echo "Darwin IA64 Toolchain Environment Loaded"
echo "  Toolchain: ${TOOLCHAIN_PREFIX}"
echo "  Compiler: $(ia64-unknown-linux-gnu-gcc --version | head -1)"
echo "  Architectures: ${RC_ARCHS}"
EOF

    sed -i "s|@@TOOLCHAIN_PREFIX@@|${TOOLCHAIN_PREFIX}|g" ${TOOLCHAIN_PREFIX}/setup-env.sh
    chmod +x ${TOOLCHAIN_PREFIX}/setup-env.sh

    info "Environment script created: ${TOOLCHAIN_PREFIX}/setup-env.sh"
}

# Create wrapper scripts
create_wrappers() {
    info "Creating build wrapper scripts..."

    mkdir -p ${TOOLCHAIN_PREFIX}/bin-wrappers

    # IA64 GCC wrapper with Darwin flags
    cat > ${TOOLCHAIN_PREFIX}/bin-wrappers/ia64-darwin-gcc << 'EOF'
#!/bin/bash
exec ia64-unknown-linux-gnu-gcc \
    -O2 \
    -fno-strict-aliasing \
    -fno-common \
    -pipe \
    -fno-builtin \
    "$@"
EOF
    chmod +x ${TOOLCHAIN_PREFIX}/bin-wrappers/ia64-darwin-gcc

    # IA64 LD wrapper
    cat > ${TOOLCHAIN_PREFIX}/bin-wrappers/ia64-darwin-ld << 'EOF'
#!/bin/bash
exec ia64-unknown-linux-gnu-ld \
    -static \
    "$@"
EOF
    chmod +x ${TOOLCHAIN_PREFIX}/bin-wrappers/ia64-darwin-ld

    info "Wrapper scripts created"
}

# Test toolchain
test_toolchain() {
    info "Testing toolchain..."

    export PATH="${TOOLCHAIN_PREFIX}/bin:${PATH}"

    # Test compiler
    cat > /tmp/test.c << 'EOF'
#include <stdint.h>
int main(void) {
    uint64_t val = 0x123456789ABCDEF0ULL;
    return 0;
}
EOF

    if ia64-unknown-linux-gnu-gcc -c /tmp/test.c -o /tmp/test.o; then
        info "Compiler test: PASSED"
    else
        error "Compiler test: FAILED"
    fi

    if ia64-unknown-linux-gnu-objdump -d /tmp/test.o > /dev/null 2>&1; then
        info "Binutils test: PASSED"
    else
        error "Binutils test: FAILED"
    fi

    rm -f /tmp/test.c /tmp/test.o

    info "Toolchain tests passed"
}

# Main installation
main() {
    echo "========================================"
    echo "Darwin IA64 Toolchain Setup"
    echo "========================================"
    echo ""
    echo "Target: ${TARGET_ARCH}"
    echo "Prefix: ${TOOLCHAIN_PREFIX}"
    echo "Binutils: ${BINUTILS_VERSION}"
    echo "GCC: ${GCC_VERSION}"
    echo ""

    read -p "Continue with installation? [y/N] " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        exit 1
    fi

    check_prerequisites
    download_sources
    build_binutils
    build_gcc
    create_env_script
    create_wrappers
    test_toolchain

    echo ""
    echo "========================================"
    echo "Toolchain Installation Complete!"
    echo "========================================"
    echo ""
    echo "To use the toolchain, run:"
    echo "  source ${TOOLCHAIN_PREFIX}/setup-env.sh"
    echo ""
}

main "$@"
