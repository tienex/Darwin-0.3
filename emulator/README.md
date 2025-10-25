# MMIX Emulators for Darwin-0.3

This directory contains MMIX emulators and simulators for developing and testing the Darwin-MMIX kernel port.

## Directory Structure

```
emulator/
├── README.md                    # This file
├── Makefile                     # Build all emulators
├── MMIX-EMULATOR-SPEC.md       # Hardware specification for emulator
├── BUILDING-EMULATOR.md        # Guide for building emulators
├── mmix-emulator-config.example # Example emulator configuration
│
├── gimmix/                      # GIMMIX - Interactive Debugger
│   ├── README                   # GIMMIX documentation
│   ├── Makefile                 # Build GIMMIX
│   ├── src/                     # GIMMIX source code
│   ├── org/                     # MMIXware dependency
│   ├── include/                 # Headers
│   ├── lib/                     # Libraries
│   ├── tests/                   # Test suite
│   ├── tools/                   # Debugging tools
│   └── doc/                     # Documentation
│
└── mmixware/                    # Official MMIXware
    ├── README.md                # MMIXware documentation
    ├── Makefile                 # Build MMIXware tools
    ├── mmix-sim.w               # Simple simulator (CWEB)
    ├── mmix-pipe.w              # Pipeline simulator (CWEB)
    ├── mmixal.w                 # MMIX assembler (CWEB)
    ├── mmmix.w                  # Meta-simulator (CWEB)
    └── *.mms                    # Example MMIX programs
```

## Emulators Overview

### 1. GIMMIX (Recommended for Development)

**Repository**: https://github.com/Nils-TUD/GIMMIX
**License**: GPL-2.0

**Features**:
- Interactive debugger with GDB stub support
- Step-by-step execution
- Register and memory inspection
- Breakpoint support
- Code coverage analysis
- Automated testing framework
- Support for user and kernel programs

**Best For**:
- Darwin kernel debugging
- Interactive development
- Testing boot sequence
- Debugging exceptions and interrupts

**Build**:
```bash
make gimmix
```

**Run**:
```bash
cd gimmix
./build/gimmix program.mmo
```

### 2. MMIXware (Official Reference)

**Author**: Donald E. Knuth
**Repository**: https://github.com/ascherer/mmix
**License**: Public Domain / Knuth License

**Simulators**:

#### MMIX-SIM (Simple Simulator)
- Behavioral simulator without pipeline or caches
- Straightforward execution model
- Best for learning and simple testing
- Fast execution

#### MMIX-PIPE (Pipeline Simulator)
- Realistic pipeline simulation
- Configurable hardware (caches, branch prediction, etc.)
- Performance analysis
- Hardware exploration

#### MMMIX (Meta-Simulator)
- Ultra-configurable pipeline
- Advanced performance testing
- Complex hardware configurations

**Best For**:
- Reference implementation
- Performance analysis
- Understanding MMIX architecture
- Testing against official simulator

**Build**:
```bash
make mmixware
```

**Run**:
```bash
cd mmixware
./mmix-sim program.mmo        # Simple simulator
./mmix-pipe program.mmconfig  # Pipeline simulator
./mmixal program.mms          # Assembler
```

## Building All Emulators

### Prerequisites

**Required**:
- GCC or Clang C compiler
- GNU Make
- CWEB (for MMIXware)
  - Debian/Ubuntu: `apt-get install texlive-binaries`
  - Arch: `pacman -S texlive-core`

**Optional** (for GIMMIX enhancements):
- Ruby (for test generation and coverage)
- GDB 6.4+ (for remote debugging)
- Valgrind (for memory checking)

### Build Commands

Build everything:
```bash
make all
```

Build specific emulator:
```bash
make gimmix      # Build GIMMIX only
make mmixware    # Build MMIXware only
```

Clean build artifacts:
```bash
make clean
```

Run tests:
```bash
make test        # Run GIMMIX test suite
```

## Quick Start

### 1. Build Emulators

```bash
cd /path/to/Darwin-0.3/emulator
make all
```

### 2. Test with Example Program

```bash
# Assemble example
cd mmixware
./mmixal hello.mms

# Run in simple simulator
./mmix-sim hello.mmo

# Or run in GIMMIX
cd ../gimmix
./build/gimmix ../mmixware/hello.mmo
```

### 3. Run Darwin Bootloader

```bash
# Build Darwin bootloader (not yet implemented)
cd ../boot-2/mmix/bootloader
make

# Run in GIMMIX
cd ../../../emulator/gimmix
./build/gimmix ../../boot-2/mmix/bootloader/boot.mmo
```

## Debugging Darwin Kernel

### Using GIMMIX Debugger

1. **Start GIMMIX in debug mode**:
   ```bash
   cd gimmix
   make debug PROG=../../boot-2/mmix/bootloader/boot.mmo
   ```

2. **Available commands**:
   - `s` - Step one instruction
   - `c` - Continue execution
   - `b <addr>` - Set breakpoint
   - `p <reg>` - Print register value
   - `m <addr>` - Display memory
   - `q` - Quit debugger

3. **Inspect boot process**:
   - Set breakpoint at kernel entry: `b 0x8000000000100000`
   - Step through boot sequence
   - Examine register state
   - Verify page table setup

### Using GDB Remote Debugging

1. **Start GIMMIX GDB stub**:
   ```bash
   cd gimmix
   ./build/gimmix --gdb=1234 program.mmo
   ```

2. **Connect with GDB**:
   ```bash
   mmix-gdb program.mmo
   (gdb) target remote localhost:1234
   (gdb) break start
   (gdb) continue
   ```

## Configuration

### Emulator Configuration File

See `mmix-emulator-config.example` for sample configuration.

Example configuration for Darwin:
```ini
[memory]
size = 256M
base = 0x00000000

[devices]
console = 0xFFFFFFFF00000000
disk = 0xFFFFFFFF00001000
timer = 0xFFFFFFFF00002000

[boot]
entry = 0x1000
kernel_load = 0x100000
kernel_virt = 0x8000000000100000
```

## Testing Darwin Boot

### Test Boot Sequence

1. **Build bootloader**:
   ```bash
   cd boot-2/mmix/bootloader
   make
   ```

2. **Build kernel** (requires mmix-gcc):
   ```bash
   cd kernel-7
   make ARCH=mmix
   ```

3. **Run in emulator**:
   ```bash
   cd emulator/gimmix
   ./build/gimmix --boot ../../boot-2/mmix/bootloader/boot.mmo \
                  --kernel ../../kernel-7/mach_kernel
   ```

4. **Debug boot issues**:
   - Check register initialization
   - Verify page table setup
   - Trace exception handling
   - Monitor device I/O

## Emulator Customization

### Adding Darwin-Specific Devices

To add devices needed by Darwin (console, disk, network):

1. **Edit GIMMIX source**:
   ```bash
   cd gimmix/src
   vi devices.c  # Add device implementations
   ```

2. **Implement device interface**:
   ```c
   // Console device at 0xFFFFFFFF00000000
   uint64_t mmix_console_read(uint64_t addr) {
       // Return character or 0 if no input
   }

   void mmix_console_write(uint64_t addr, uint64_t value) {
       // Write character to console
       putchar(value & 0xFF);
   }
   ```

3. **Rebuild GIMMIX**:
   ```bash
   cd ../..
   make gimmix
   ```

### Modifying Boot Protocol

Edit `gimmix/src/boot.c` to match Darwin's boot protocol:

```c
// Set boot_args structure
regs[0] = boot_args_ptr;  // $0 = boot_args

// Jump to kernel entry
pc = 0x8000000000100000ULL;
```

## Performance Analysis

### Using MMIX-PIPE

1. **Create pipeline configuration**:
   ```bash
   cp mmixware/deluxe.mmconfig darwin.mmconfig
   # Edit darwin.mmconfig for Darwin-specific settings
   ```

2. **Run pipeline simulator**:
   ```bash
   cd mmixware
   ./mmix-pipe darwin.mmconfig
   ```

3. **Analyze results**:
   - Instruction counts
   - Cache hit rates
   - Pipeline stalls
   - Branch prediction accuracy

## Troubleshooting

### Build Issues

**CWEB not found**:
```bash
# Debian/Ubuntu
sudo apt-get install texlive-binaries

# Arch Linux
sudo pacman -S texlive-core

# macOS
brew install cweb
```

**GCC errors**:
- Ensure GCC or Clang is installed
- Check C compiler version: `gcc --version`
- Update if older than GCC 4.8

### Runtime Issues

**Segmentation fault**:
- Check program is valid MMIX object (.mmo)
- Verify memory addresses are valid
- Enable debugging: `./mmix-sim -v program.mmo`

**Hang or infinite loop**:
- Use debugger to find loop
- Check for missing TRAP 0,Halt,0
- Verify exception handler is set (rT register)

**Device not working**:
- Check memory-mapped addresses match specification
- Enable device tracing in emulator
- Verify device registers are properly mapped

## Additional Resources

**Documentation**:
- GIMMIX: See `gimmix/README`
- MMIXware: See `mmixware/README.md`
- MMIX Spec: `MMIX-EMULATOR-SPEC.md`
- Build Guide: `BUILDING-EMULATOR.md`

**Example Programs**:
- MMIXware examples: `mmixware/*.mms`
- GIMMIX tests: `gimmix/tests/`
- Darwin boot: `../boot-2/mmix/`

**Online Resources**:
- MMIX Home: https://mmix.cs.hm.edu/
- Knuth's Page: https://www-cs-faculty.stanford.edu/~knuth/mmix-news.html
- GIMMIX GitHub: https://github.com/Nils-TUD/GIMMIX
- MMIXware GitHub: https://github.com/ascherer/mmix

## Darwin-Specific Notes

### Boot Protocol

Darwin bootloader expects:
- Entry at 0x1000
- Load kernel to 0x100000
- Set up page tables (identity + kernel mapping)
- Pass boot_args in $0
- Jump to 0x8000000000100000

### Device Requirements

Darwin requires these devices:
- Console (0xFFFFFFFF00000000): Character I/O
- Disk (0xFFFFFFFF00001000): 8KB sector I/O
- Timer (0xFFFFFFFF00002000): Interval timer

### Exception Handling

Darwin expects:
- rT points to exception handler (lowmem_vectors.s)
- Exceptions save state and call trap()
- Interrupts use rI (timer) and external
- System calls use TRAP instruction

## Future Enhancements

### Planned Features

1. **Enhanced GIMMIX**:
   - Full Darwin device support
   - Darwin boot protocol
   - Network device emulation
   - Disk image support

2. **Darwin Integration**:
   - Automated kernel testing
   - Boot regression tests
   - Device driver testing framework
   - Performance profiling

3. **Tooling**:
   - Kernel debugger integration
   - Memory leak detection
   - Coverage analysis
   - Profiling support

### Contributing

To enhance emulators for Darwin:

1. Fork the emulator repository
2. Add Darwin-specific features
3. Test with Darwin kernel
4. Submit patches or pull requests

**GIMMIX patches**: Submit to https://github.com/Nils-TUD/GIMMIX
**MMIXware patches**: Submit to https://github.com/ascherer/mmix

---

**Last Updated**: 2025-10-25
**Darwin Version**: 0.3
**MMIX Architecture**: 64-bit RISC
