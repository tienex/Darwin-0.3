#!/bin/bash
#
# Analyze Darwin kernel binary
#
# Provides detailed information about the kernel image
#

set -e

# Configuration
KERNEL_ELF="${KERNEL_ELF:-build/kernel/mach_kernel.kernel}"
KERNEL_BIN="${KERNEL_BIN:-build/kernel/mach_kernel.bin}"
CROSS_COMPILE="${CROSS_COMPILE:-riscv64-unknown-elf-}"

# Colors
GREEN='\033[0;32m'
CYAN='\033[0;36m'
YELLOW='\033[1;33m'
NC='\033[0m'

log_section() {
    echo -e "\n${CYAN}═══ $1 ═══${NC}\n"
}

if [ ! -f "$KERNEL_ELF" ]; then
    echo "Kernel not found: $KERNEL_ELF"
    exit 1
fi

echo "=========================================="
echo "  Darwin Kernel Analysis"
echo "=========================================="

# File information
log_section "File Information"
ls -lh "$KERNEL_ELF"
[ -f "$KERNEL_BIN" ] && ls -lh "$KERNEL_BIN"

# Section sizes
log_section "Section Sizes"
${CROSS_COMPILE}size "$KERNEL_ELF"

# Section headers
log_section "Section Headers"
${CROSS_COMPILE}objdump -h "$KERNEL_ELF" | head -20

# Entry point
log_section "Entry Point"
${CROSS_COMPILE}readelf -h "$KERNEL_ELF" | grep "Entry point"

# Symbol statistics
log_section "Symbol Statistics"
echo "Total symbols: $(${CROSS_COMPILE}nm "$KERNEL_ELF" | wc -l)"
echo "Global symbols: $(${CROSS_COMPILE}nm -g "$KERNEL_ELF" | wc -l)"
echo ""

# Important symbols
echo "Key symbols:"
${CROSS_COMPILE}nm "$KERNEL_ELF" | grep -E '(_start|_riscv_init|trap_handler|pmap_bootstrap|main)$' || echo "  (none found)"

# Top 20 largest symbols
log_section "Largest Symbols (Top 20)"
${CROSS_COMPILE}nm -S --size-sort "$KERNEL_ELF" | tail -20

# Dependencies
log_section "Dependencies"
${CROSS_COMPILE}readelf -d "$KERNEL_ELF" 2>/dev/null || echo "No dynamic dependencies (static kernel)"

echo ""
