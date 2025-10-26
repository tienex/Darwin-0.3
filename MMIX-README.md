# MMIX 64-bit Support for Darwin-0.3

## Overview

This Darwin-0.3 distribution includes **complete 64-bit Mach-O support** for the MMIX architecture, Donald Knuth's 64-bit RISC processor from *The Art of Computer Programming*.

**Status**: ✅ Production Ready - All tools functional

## What is MMIX?

MMIX (pronounced "mix") is a 64-bit RISC architecture designed by Donald Knuth as the successor to MIX. It features:
- 64-bit registers and addressing
- 256 general-purpose registers
- Clean RISC instruction set
- Educational focus with production capabilities

## Quick Start

### 1. Write MMIX Assembly

Create `hello.s`:
```mmix
# MMIX Hello World
    .text
    .globl _main

_main:
    # Your MMIX code here
    SET $0,42        # Set register $0 to 42
    SET $255,0       # Return 0
    TRAP 0,Halt,0    # Exit
```

### 2. Assemble to 64-bit Object File

```bash
as -arch mmix -o hello.o hello.s
```

This creates a 64-bit Mach-O object file with:
- `mach_header_64` (32 bytes, magic = 0xfeedfacf)
- `LC_SEGMENT_64` load commands
- `section_64` structures
- `nlist_64` symbol table

### 3. Inspect the Object File

```bash
# View mach header
otool -h hello.o
# Output: magic 0xfeedfacf (MH_MAGIC_64)

# View load commands
otool -l hello.o
# Output: LC_SEGMENT_64 with 64-bit addresses

# List symbols
nm hello.o
# Output: 0000000000000000 T _main

# Show segment sizes
size hello.o
```

### 4. Link to Executable

```bash
ld -arch mmix -o hello hello.o
```

### 5. Inspect the Executable

```bash
file hello
# Output: hello: Mach-O 64-bit executable mmix

otool -h hello
otool -l hello
nm hello
```

## Supported Tools

All Darwin command-line tools support 64-bit MMIX binaries:

| Tool | Purpose | Example |
|------|---------|---------|
| `as` | Assembler | `as -arch mmix -o file.o file.s` |
| `ld` | Linker | `ld -arch mmix -o prog file.o` |
| `otool` | Object file display | `otool -l file.o` |
| `nm` | Symbol lister | `nm file.o` |
| `size` | Size analyzer | `size file.o` |
| `strip` | Symbol stripper | `strip file.o` |
| `strings` | String extractor | `strings file.o` |
| `ar` | Archiver | `ar -rc lib.a file.o` |
| `ranlib` | Archive indexer | `ranlib lib.a` |
| `lipo` | Universal binary tool | `lipo -create ...` |
| `file` | File type identifier | `file file.o` |

## 64-bit Mach-O Structures

### Mach Header (32 bytes)

```c
struct mach_header_64 {
    uint32_t magic;         // 0xfeedfacf (MH_MAGIC_64)
    cpu_type_t cputype;     // 24 (CPU_TYPE_MMIX)
    cpu_subtype_t cpusubtype;
    uint32_t filetype;
    uint32_t ncmds;
    uint32_t sizeofcmds;
    uint32_t flags;
    uint32_t reserved;      // NEW in 64-bit
};
```

### Segment Command (72 bytes)

```c
struct segment_command_64 {
    uint32_t cmd;           // 0x19 (LC_SEGMENT_64)
    uint32_t cmdsize;
    char segname[16];
    uint64_t vmaddr;        // 64-bit virtual address
    uint64_t vmsize;        // 64-bit size
    uint64_t fileoff;       // 64-bit file offset
    uint64_t filesize;      // 64-bit file size
    vm_prot_t maxprot;
    vm_prot_t initprot;
    uint32_t nsects;
    uint32_t flags;
};
```

### Section (80 bytes)

```c
struct section_64 {
    char sectname[16];
    char segname[16];
    uint64_t addr;          // 64-bit address
    uint64_t size;          // 64-bit size
    uint32_t offset;
    uint32_t align;
    uint32_t reloff;
    uint32_t nreloc;
    uint32_t flags;
    uint32_t reserved1;
    uint32_t reserved2;
    uint32_t reserved3;     // NEW in 64-bit
};
```

### Symbol (16 bytes)

```c
struct nlist_64 {
    union {
        uint32_t n_strx;
    } n_un;
    uint8_t n_type;
    uint8_t n_sect;
    uint16_t n_desc;
    uint64_t n_value;       // 64-bit symbol value
};
```

## Example Workflow

### Creating a Library

```bash
# 1. Write source files
cat > math.s << 'EOF'
    .text
    .globl _add
_add:
    ADDU $0,$0,$1
    POP 1,0
EOF

cat > util.s << 'EOF'
    .text
    .globl _multiply
_multiply:
    MULU $0,$0,$1
    POP 1,0
EOF

# 2. Assemble
as -arch mmix -o math.o math.s
as -arch mmix -o util.o util.s

# 3. Create archive
ar -rc libmymath.a math.o util.o
ranlib libmymath.a

# 4. List contents
ar -t libmymath.a
nm libmymath.a
```

### Creating a Universal Binary

```bash
# If you have multiple architectures
lipo -create program_i386 program_mmix -output program_universal

# Verify
lipo -info program_universal
file program_universal
```

### Stripping Symbols

```bash
# Copy before stripping
cp program program_debug

# Strip to reduce size
strip program

# Compare
size program_debug
size program
```

## Address Space

MMIX uses 64-bit addressing, allowing:
- Virtual address space: 2^64 bytes (16 exabytes)
- Segments can be placed anywhere in 64-bit space
- Symbols can have 64-bit values

Example with large addresses:
```mmix
    .text
    .org 0x123456789ABCD000   # 64-bit address
_start:
    LDOU $0,data,$1

    .data
    .org 0xFEDCBA9876543000   # Another 64-bit address
data:
    .quad 0x0102030405060708
```

## Verification

Run the included test script:

```bash
cd /path/to/Darwin-0.3
./test-mmix-64bit.sh
```

Expected output:
```
======================================
MMIX 64-bit Toolchain Verification
======================================

[Test 1] Creating MMIX assembly source...
✓ Assembly source created

[Test 2] Assembling to 64-bit object file...
✓ Assembly successful

[Test 3] Verifying 64-bit magic number...
✓ Correct magic number: MH_MAGIC_64 (0xfeedfacf)

[Test 4] Verifying file type identification...
✓ File identified as: Mach-O 64-bit object

...

SUCCESS: MMIX 64-bit toolchain is fully operational!
```

## Implementation Details

### What Was Implemented

1. **Assembler (as)**:
   - Outputs `mach_header_64` with MH_MAGIC_64
   - Generates `LC_SEGMENT_64` load commands
   - Creates `section_64` structures
   - Uses `nlist_64` for symbol table

2. **Linker (ld)**:
   - Reads 64-bit object files
   - Merges symbols from `nlist_64`
   - Writes 64-bit executables
   - Supports mixed 32/64-bit linking

3. **All Tools**:
   - Recognize MH_MAGIC_64 (0xfeedfacf)
   - Parse LC_SEGMENT_64 commands
   - Handle `section_64` structures
   - Process `nlist_64` symbol tables

### Key Files Modified

- `cctools-2/as/write_object.c` - 64-bit object generation
- `cctools-2/ld/pass1.c` - 64-bit input parsing
- `cctools-2/ld/pass2.c` - 64-bit output writing
- `cctools-2/ld/layout.c` - 64-bit header generation
- `cctools-2/ld/symbols.c` - Symbol format conversion
- `cctools-2/libstuff/ofile.c` - 64-bit file recognition (CRITICAL)
- `cctools-2/libstuff/bytesex.c` - 64-bit byte swapping
- `cctools-2/otool/ofile_print.c` - 64-bit display formatting

### Design Patterns

```c
// 1. Detect 64-bit files
if (magic == MH_MAGIC_64 || magic == SWAP_LONG(MH_MAGIC_64))
    is_64bit = TRUE;

// 2. Calculate header size
size_t hdr_size = is_64bit ?
    sizeof(struct mach_header_64) :
    sizeof(struct mach_header);

// 3. Locate load commands
char *load_cmds = is_64bit ?
    addr + sizeof(struct mach_header_64) :
    addr + sizeof(struct mach_header);

// 4. Swap byte order
if (swapped && is_64bit)
    swap_segment_command_64(seg, host_byte_sex);

// 5. Convert symbol formats
if (input_64bit && !output_64bit)
    nlist->n_value = (unsigned long)nlist_64->n_value;  // Truncate
else if (!input_64bit && output_64bit)
    nlist_64->n_value = (uint64_t)nlist->n_value;       // Extend
```

## Troubleshooting

### "Not a Mach-O file" Error

If you get this error, the tool doesn't recognize MH_MAGIC_64. This should not happen with the updated toolchain. Verify with:

```bash
hexdump -C file.o | head -1
# Should show: cf fa ed fe (0xfeedfacf in little-endian)
```

### Wrong Address Display

If addresses appear truncated or wrong, ensure you're using the updated otool:

```bash
otool -l file.o | grep vmaddr
# Should show: vmaddr 0x0000000000000000 (16 hex digits)
```

### Linking Failures

If linking fails, verify object file format:

```bash
otool -h file.o
# Check: magic should be 0xfeedfacf
# Check: cputype should be 24 (MMIX)
```

## Documentation

- `MMIX-FINAL-STATUS.md` - Complete implementation status
- `OFILE-64BIT-COMPLETION-SUMMARY.md` - Technical details of ofile.c fix
- `test-mmix-64bit.sh` - Verification test script
- `PHASE2-COMPLETION-SUMMARY.md` - Linker implementation details

## Architecture Constants

```c
// CPU Type (mach/machine.h)
#define CPU_TYPE_MMIX           24
#define CPU_ARCH_ABI64          0x01000000
#define CPU_TYPE_MMIX_64        (CPU_TYPE_MMIX | CPU_ARCH_ABI64)

// Magic Numbers (mach-o/loader.h)
#define MH_MAGIC                0xfeedface  // 32-bit
#define MH_MAGIC_64             0xfeedfacf  // 64-bit

// Load Commands
#define LC_SEGMENT              0x1         // 32-bit segment
#define LC_SEGMENT_64           0x19        // 64-bit segment
```

## Compatibility

- ✅ Backward compatible with 32-bit Mach-O
- ✅ All existing 32-bit tools still work
- ✅ Mixed 32/64-bit linking supported
- ✅ Cross-endian byte swapping functional
- ✅ Universal binaries with 64-bit slices

## Performance Notes

64-bit object files are larger than 32-bit:
- Header: 32 bytes vs 28 bytes (+14%)
- Segments: 72 bytes vs 56 bytes (+29%)
- Sections: 80 bytes vs 68 bytes (+18%)
- Symbols: 16 bytes vs 12 bytes (+33%)

But provide:
- Full 64-bit address space
- Natural 64-bit arithmetic
- Larger register set utilization

## Next Steps

1. **Write MMIX Programs**: Use the assembler for system software
2. **Build Libraries**: Create .a archives of object files
3. **Develop OS**: Leverage 64-bit addressing for kernel development
4. **Optimize Code**: Take advantage of 64-bit registers

## Contributing

This implementation follows Apple's Mach-O 64-bit specification and maintains compatibility with the Darwin toolchain architecture.

## References

- *The Art of Computer Programming* by Donald Knuth
- *MMIXware* - MMIX documentation and simulator
- Apple Mach-O File Format specification
- Darwin/XNU source code

## Status

**✅ PRODUCTION READY**

All standard Darwin toolchain operations work correctly with 64-bit MMIX binaries. The implementation is complete, tested, and documented.

---

**Last Updated**: 2025-10-25
**Version**: Darwin-0.3 with complete MMIX 64-bit support
**Branch**: claude/add-mmix-support-011CUT198Ha67uTFbXZqwn2A
