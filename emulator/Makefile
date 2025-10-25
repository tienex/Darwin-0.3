##
# Makefile for MMIX Emulators
#
# Builds both GIMMIX and MMIXware simulators for Darwin-MMIX development
##

.PHONY: all gimmix mmixware clean test help

# Default target
all: gimmix mmixware

help:
	@echo "MMIX Emulators Build System"
	@echo "============================"
	@echo ""
	@echo "Targets:"
	@echo "  all          - Build all emulators (default)"
	@echo "  gimmix       - Build GIMMIX interactive debugger"
	@echo "  mmixware     - Build MMIXware simulators (mmix-sim, mmix-pipe, mmixal)"
	@echo "  test         - Run GIMMIX test suite"
	@echo "  clean        - Clean all build artifacts"
	@echo "  help         - Show this help message"
	@echo ""
	@echo "Requirements:"
	@echo "  - GCC or Clang"
	@echo "  - GNU Make"
	@echo "  - CWEB (texlive-binaries for MMIXware)"
	@echo "  - Ruby (optional, for GIMMIX tests)"
	@echo ""
	@echo "Examples:"
	@echo "  make all                    # Build everything"
	@echo "  make gimmix                 # Build GIMMIX only"
	@echo "  make mmixware               # Build MMIXware only"
	@echo "  make test                   # Run tests"
	@echo ""

##
# GIMMIX - Interactive Debugger
#
# Features:
#   - GDB stub support
#   - Interactive debugging
#   - Automated testing
#   - Code coverage
##
gimmix:
	@echo "Building GIMMIX interactive debugger..."
	@echo "========================================"
	@if [ ! -d "gimmix" ]; then \
		echo "ERROR: gimmix/ directory not found"; \
		echo "Run: git clone https://github.com/Nils-TUD/GIMMIX.git gimmix"; \
		exit 1; \
	fi
	@echo "Building MMIXware dependency..."
	cd gimmix/org && $(MAKE)
	@echo ""
	@echo "Building GIMMIX..."
	cd gimmix && $(MAKE)
	@echo ""
	@echo "GIMMIX built successfully!"
	@echo "Run: cd gimmix && ./build/gimmix program.mmo"
	@echo ""

##
# MMIXware - Official Simulators
#
# Includes:
#   - mmix-sim: Simple behavioral simulator
#   - mmix-pipe: Pipeline simulator
#   - mmixal: MMIX assembler
#   - mmmix: Meta-simulator
##
mmixware:
	@echo "Building MMIXware official simulators..."
	@echo "========================================="
	@if [ ! -d "mmixware" ]; then \
		echo "ERROR: mmixware/ directory not found"; \
		echo "Run: git clone https://github.com/ascherer/mmix.git mmixware"; \
		exit 1; \
	fi
	@echo "Checking for CWEB..."
	@if ! command -v ctangle >/dev/null 2>&1; then \
		echo "ERROR: CWEB not found"; \
		echo "Install CWEB:"; \
		echo "  Debian/Ubuntu: apt-get install texlive-binaries"; \
		echo "  Arch Linux:    pacman -S texlive-core"; \
		echo "  macOS:         brew install cweb"; \
		exit 1; \
	fi
	@echo "Building with CWEB..."
	cd mmixware && $(MAKE)
	@echo ""
	@echo "MMIXware built successfully!"
	@echo "Simulators available:"
	@echo "  mmixware/mmix-sim    - Simple simulator"
	@echo "  mmixware/mmix-pipe   - Pipeline simulator"
	@echo "  mmixware/mmixal      - Assembler"
	@echo "  mmixware/mmmix       - Meta-simulator"
	@echo ""
	@echo "Run: cd mmixware && ./mmix-sim program.mmo"
	@echo ""

##
# Testing
##
test: gimmix
	@echo "Running GIMMIX test suite..."
	@echo "============================"
	cd gimmix && ./runtests.sh
	@echo ""
	@echo "All tests passed!"
	@echo ""

##
# Clean
##
clean:
	@echo "Cleaning build artifacts..."
	@if [ -d "gimmix" ]; then \
		cd gimmix/org && $(MAKE) clean; \
		cd gimmix && $(MAKE) clean; \
	fi
	@if [ -d "mmixware" ]; then \
		cd mmixware && $(MAKE) clean; \
	fi
	@echo "Clean complete."

##
# Development targets
##

# Build GIMMIX in debug mode
gimmix-debug:
	cd gimmix && $(MAKE) CFLAGS="-g -O0 -DDEBUG"

# Build documentation
docs:
	@if [ -d "gimmix" ]; then \
		cd gimmix && $(MAKE) doc; \
	fi
	@echo "Documentation built in gimmix/doc/"

# Run code coverage analysis
coverage: gimmix
	@if [ -d "gimmix" ]; then \
		cd gimmix && ./runcov.sh; \
	fi

# Run memory leak check
memcheck: gimmix
	@if [ -d "gimmix" ]; then \
		cd gimmix && ./runmemchk.sh; \
	fi

# Install emulators to system (requires root)
install: all
	@echo "Installing emulators..."
	@echo "Not implemented yet - emulators run from source directory"

##
# Darwin-specific targets
##

# Test Darwin bootloader
test-darwin-boot:
	@echo "Testing Darwin bootloader..."
	@if [ ! -f "../boot-2/mmix/bootloader/boot.mmo" ]; then \
		echo "ERROR: Darwin bootloader not built"; \
		echo "Build with: cd ../boot-2/mmix/bootloader && make"; \
		exit 1; \
	fi
	cd gimmix && ./build/gimmix ../boot-2/mmix/bootloader/boot.mmo

# Test Darwin kernel
test-darwin-kernel:
	@echo "Testing Darwin kernel..."
	@if [ ! -f "../kernel-7/mach_kernel" ]; then \
		echo "ERROR: Darwin kernel not built"; \
		echo "Build with: cd ../kernel-7 && make ARCH=mmix"; \
		exit 1; \
	fi
	cd gimmix && ./build/gimmix --kernel ../kernel-7/mach_kernel

# Debug Darwin boot sequence
debug-darwin-boot:
	@echo "Debugging Darwin boot sequence..."
	@if [ ! -f "../boot-2/mmix/bootloader/boot.mmo" ]; then \
		echo "ERROR: Darwin bootloader not built"; \
		exit 1; \
	fi
	cd gimmix && $(MAKE) debug PROG=../boot-2/mmix/bootloader/boot.mmo

##
# Information targets
##

# Show emulator versions
version:
	@echo "MMIX Emulator Versions:"
	@echo "======================="
	@if [ -d "gimmix/.git" ]; then \
		echo "GIMMIX:"; \
		cd gimmix && git log -1 --format="  Commit: %h%n  Date:   %ai%n  Author: %an"; \
	fi
	@echo ""
	@if [ -d "mmixware/.git" ]; then \
		echo "MMIXware:"; \
		cd mmixware && git log -1 --format="  Commit: %h%n  Date:   %ai%n  Author: %an"; \
	fi

# Check prerequisites
check-prereqs:
	@echo "Checking prerequisites..."
	@echo "========================="
	@echo -n "GCC:      "
	@if command -v gcc >/dev/null 2>&1; then \
		gcc --version | head -1; \
	else \
		echo "NOT FOUND"; \
	fi
	@echo -n "Make:     "
	@if command -v make >/dev/null 2>&1; then \
		make --version | head -1; \
	else \
		echo "NOT FOUND"; \
	fi
	@echo -n "CWEB:     "
	@if command -v ctangle >/dev/null 2>&1; then \
		echo "Found (ctangle)"; \
	else \
		echo "NOT FOUND (required for MMIXware)"; \
	fi
	@echo -n "Ruby:     "
	@if command -v ruby >/dev/null 2>&1; then \
		ruby --version | head -1; \
	else \
		echo "NOT FOUND (optional, for GIMMIX tests)"; \
	fi
	@echo -n "GDB:      "
	@if command -v gdb >/dev/null 2>&1; then \
		gdb --version | head -1; \
	else \
		echo "NOT FOUND (optional, for debugging)"; \
	fi
	@echo ""

# Show build status
status:
	@echo "MMIX Emulators Status:"
	@echo "======================"
	@echo ""
	@echo -n "GIMMIX:    "
	@if [ -f "gimmix/build/gimmix" ]; then \
		echo "BUILT"; \
	elif [ -d "gimmix" ]; then \
		echo "NOT BUILT (run: make gimmix)"; \
	else \
		echo "NOT CLONED"; \
	fi
	@echo -n "MMIXware:  "
	@if [ -f "mmixware/mmix-sim" ]; then \
		echo "BUILT"; \
	elif [ -d "mmixware" ]; then \
		echo "NOT BUILT (run: make mmixware)"; \
	else \
		echo "NOT CLONED"; \
	fi
	@echo ""
