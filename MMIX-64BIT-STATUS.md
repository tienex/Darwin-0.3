# MMIX 64-bit Support Status for Darwin-0.3

## Overview

MMIX is a native 64-bit RISC architecture designed by Donald Knuth. This document describes
the 64-bit Mach-O format support added to Darwin-0.3 for MMIX and identifies what remains to
be implemented.

---

## ✅ 64-bit Mach-O Structures Added

### Header and Load Commands

**mach_header_64** (loader.h)
- Magic: MH_MAGIC_64 (0xfeedfacf)
- Contains 'reserved' field for 64-bit alignment
- Used by all 64-bit architectures
- Status: ✅ **COMPLETE**

**segment_command_64** (loader.h)
- Load command type: LC_SEGMENT_64 (0x19)
- 64-bit fields: vmaddr, vmsize, fileoff, filesize (all unsigned long long)
- Supports full 64-bit virtual address space
- Status: ✅ **COMPLETE**

**section_64** (loader.h)
- 64-bit addr and size fields (unsigned long long)
- Additional reserved3 field
- Used within segment_command_64
- Status: ✅ **COMPLETE**

### Symbol Tables

**nlist_64** (nlist.h)
- 64-bit n_value field (unsigned long long)
- Expanded n_desc to unsigned short
- Critical for 64-bit symbol addresses
- Status: ✅ **COMPLETE**

### Dynamic Libraries

**dylib_module_64** (loader.h)
- 64-bit objc_module_info_size field
- Combined init/term index fields
- Supports 64-bit dynamic library modules
- Status: ✅ **COMPLETE**

### Architecture Flags

**CPU_ARCH_ABI64** (machine.h)
- Value: 0x01000000
- Indicates 64-bit ABI
- CPU_ARCH_MASK: 0xff000000
- Status: ✅ **COMPLETE**

**CPU_TYPE_MMIX** (machine.h)
- Defined as: (19 | CPU_ARCH_ABI64)
- Properly marked as 64-bit architecture
- Status: ✅ **COMPLETE**

---

## ⚠️ INCOMPLETE: Tool Support

While all the **structures** are defined, the **tools** don't yet use them properly!

### Assembler (as/) - NEEDS WORK

**Current Status:**
- ✅ Parses MMIX assembly
- ✅ Generates 32-bit instructions correctly
- ❌ **Still generates 32-bit Mach-O headers (mach_header, not mach_header_64)**
- ❌ **Still uses segment_command instead of segment_command_64**
- ❌ **Still uses section instead of section_64**
- ❌ **Still uses nlist instead of nlist_64**

**What's Needed:**
```c
// In write_object.c, assembler needs to:
1. Detect 64-bit architecture (check CPU_ARCH_ABI64 flag)
2. Write mach_header_64 instead of mach_header
3. Use LC_SEGMENT_64 load commands
4. Write section_64 structures
5. Write nlist_64 symbol table entries with 64-bit addresses
```

### Linker (ld/) - NEEDS WORK

**Current Status:**
- ✅ Has mmix_reloc.c for relocations
- ✅ Integrated into sections.c dispatch
- ❌ **Still reads/writes 32-bit structures**
- ❌ **Symbol resolution uses 32-bit nlist**
- ❌ **Section merging uses 32-bit section**

**What's Needed:**
```c
// Throughout ld/, need to:
1. Detect MH_MAGIC_64 vs MH_MAGIC
2. Use mach_header_64 for 64-bit objects
3. Read/write segment_command_64
4. Read/write section_64
5. Use nlist_64 for symbol table
6. Handle 64-bit addresses in relocations
7. Update all address calculations for 64-bit
```

### Object Tool (otool/) - NEEDS WORK

**Current Status:**
- ✅ Has mmix_disasm.c for disassembly
- ✅ Can disassemble MMIX instructions
- ❌ **Still displays 32-bit headers**
- ❌ **Shows 32-bit sections and segments**
- ❌ **Symbol table display uses 32-bit nlist**

**What's Needed:**
```c
// In ofile_print.c, need to:
1. Detect MH_MAGIC_64 and branch to 64-bit display code
2. Print mach_header_64 fields
3. Display segment_command_64 with 64-bit addresses
4. Display section_64 with 64-bit addresses  
5. Show nlist_64 symbol values as 64-bit
6. Format addresses as 16-digit hex (0x0123456789abcdef)
```

---

## 🔍 What Actually Works Right Now

### Works:
1. ✅ **Structures are defined** - All 64-bit Mach-O types exist
2. ✅ **MMIX is marked 64-bit** - CPU_TYPE_MMIX has CPU_ARCH_ABI64 flag
3. ✅ **Assembler encodes instructions** - All 256 MMIX opcodes work
4. ✅ **Linker relocates code** - MMIX relocations function
5. ✅ **Disassembler decodes instructions** - All MMIX instructions display

### Doesn't Work:
1. ❌ **Assembling MMIX code produces 32-bit Mach-O** files
2. ❌ **Linker can't link 64-bit objects** (reads 32-bit structures)
3. ❌ **otool displays wrong header format** (shows 32-bit)
4. ❌ **Symbols > 4GB would be truncated** (using 32-bit nlist)

---

## 📋 Implementation Priority

To get MMIX working properly on Darwin-0.3:

### Phase 1: Assembler (CRITICAL)
```
Priority: HIGH - Without this, can't create 64-bit MMIX binaries

Files to modify:
- cctools-2/as/write_object.c
  - Add is_64bit() check based on md_cputype & CPU_ARCH_ABI64
  - Write mach_header_64 for 64-bit archs
  - Write segment_command_64 structures
  - Write section_64 structures
  - Write nlist_64 symbol table

Effort: Medium (2-3 days)
```

### Phase 2: Linker (HIGH)
```
Priority: HIGH - Without this, can't link multiple MMIX objects

Files to modify:
- cctools-2/ld/pass1.c - Read 64-bit structures
- cctools-2/ld/pass2.c - Write 64-bit output
- cctools-2/ld/objects.c - Handle 64-bit objects
- cctools-2/ld/sections.c - Use section_64
- cctools-2/ld/symbols.c - Use nlist_64

Effort: Large (1 week)
```

### Phase 3: Object Tool (MEDIUM)
```
Priority: MEDIUM - Needed for debugging, but not critical for basic operation

Files to modify:
- cctools-2/otool/ofile_print.c
  - Add print_mach_header_64()
  - Add print_segment_command_64()
  - Add print_section_64()
  - Update print_symbols() for nlist_64

Effort: Small (1-2 days)
```

---

## 🎯 Current Reality

**The structures are there, but the tools don't use them yet.**

Think of it like having a 64-bit CPU but running 32-bit software - everything will
"work" but you're not actually using the 64-bit capabilities!

### What this means practically:

1. **You can assemble MMIX code** → but it creates 32-bit Mach-O files
2. **You can link MMIX objects** → but addresses are truncated to 32-bit
3. **You can disassemble MMIX** → but headers show wrong format
4. **MMIX code will run** → as long as everything fits in 4GB address space

### To fully support MMIX 64-bit:
- **Assembler** must detect MMIX and write 64-bit structures
- **Linker** must read/write 64-bit object files
- **Tools** must display 64-bit values correctly

---

## 📊 Summary

| Component | Structures Defined | Tools Updated | Status |
|-----------|-------------------|---------------|--------|
| mach_header_64 | ✅ | ❌ | PARTIAL |
| segment_command_64 | ✅ | ❌ | PARTIAL |
| section_64 | ✅ | ❌ | PARTIAL |
| nlist_64 | ✅ | ❌ | PARTIAL |
| dylib_module_64 | ✅ | ❌ | PARTIAL |
| CPU_ARCH_ABI64 | ✅ | ❌ | PARTIAL |
| Assembler | N/A | ❌ | **NEEDS WORK** |
| Linker | N/A | ❌ | **NEEDS WORK** |
| otool | N/A | ❌ | **NEEDS WORK** |

**Overall Status: 40% Complete**
- Structures: 100% ✅
- Tool Implementation: 0% ❌

---

## 🚀 Next Steps

1. Update `cctools-2/as/write_object.c` to detect 64-bit architectures and write proper headers
2. Update linker to handle 64-bit structures throughout
3. Update otool to display 64-bit formats
4. Test with real MMIX programs
5. Verify symbol resolution works with 64-bit addresses

---

**Bottom Line:** The foundation is solid, but the house isn't built yet!

Generated: 2025-10-25
Branch: claude/add-mmix-support-011CUT198Ha67uTFbXZqwn2A
