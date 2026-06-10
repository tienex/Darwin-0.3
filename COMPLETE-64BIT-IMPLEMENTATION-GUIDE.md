# Darwin-0.3 Complete 64-bit MMIX Implementation Guide

## Overview

This document provides a comprehensive guide for implementing 64-bit Mach-O support across all Darwin-0.3 components.

**Date**: 2025-10-26
**Branch**: `claude/add-mmix-support-011CUT198Ha67uTFbXZqwn2A`
**Status**: Implementation Guide

---

## Current Status

### ✅ **COMPLETED** Components

1. **Toolchain (cctools-2)** - 100% Complete
   - ✅ Assembler (as) - generates 64-bit objects
   - ✅ Linker (ld) - reads/writes 64-bit executables
   - ✅ All tools (otool, nm, size, strip, strings, ar, lipo, file) - full 64-bit support
   - ✅ Core library (libstuff/ofile.c) - 64-bit detection and parsing
   - ✅ Byte swapping functions - complete 64-bit support

2. **Kernel (kernel-7)** - 100% Complete
   - ✅ CPU type definition (CPU_TYPE_MMIX with CPU_ARCH_ABI64)
   - ✅ Mach-O loader - MH_MAGIC_64 detection
   - ✅ LC_SEGMENT_64 support - load_segment_64() function
   - ✅ 64-bit header size handling
   - ✅ 64-bit virtual memory mapping

### 🔨 **PENDING** Components

3. **Dynamic Linker (dyld)** - Needs Implementation
4. **Boot Loader (boot-2)** - Needs Implementation
5. **C Library (Libc-1)** - Needs Implementation
6. **Kernel Extensions** - Needs Implementation
7. **Objective-C Runtime (libobjc)** - Needs Implementation

---

## Implementation Patterns

All components follow the same basic pattern established in toolchain and kernel:

```c
// 1. Detect 64-bit files
if (magic == MH_MAGIC_64 || magic == SWAP_LONG(MH_MAGIC_64))
    is_64bit = TRUE;

// 2. Calculate correct header size
hdr_size = is_64bit ?
    sizeof(struct mach_header_64) :
    sizeof(struct mach_header);

// 3. Handle both load command types
switch(lcp->cmd) {
case LC_SEGMENT:
    // Handle 32-bit segment
    break;
case LC_SEGMENT_64:
    // Handle 64-bit segment
    break;
}
```

---

## Component-by-Component Implementation Guide

## 3. Dynamic Linker (dyld)

**Location**: `cctools-2/dyld/`

**Purpose**: Loads shared libraries at runtime and resolves symbols

### Key Files to Modify

#### `images.c` (~2500 lines)
**Current Issues**:
- Line 863, 886, 891, 1344: Only checks MH_MAGIC, not MH_MAGIC_64
- Line 856, 879, 1337: Uses sizeof(struct mach_header) instead of dynamic size
- Line 302, 1676: Fixed header size calculations

**Changes Needed**:
1. Add MH_MAGIC_64 detection everywhere MH_MAGIC is checked
2. Add is_64bit tracking variable to image structures
3. Calculate header size dynamically based on magic
4. Add LC_SEGMENT_64 handling in load command parsing

**Implementation**:
```c
// Add to image structure (images.h)
struct image {
    // ... existing fields ...
    enum bool is_64bit;  // NEW
};

// Update map_image() function
if (mh->magic == MH_MAGIC)
    image->is_64bit = FALSE;
else if (mh->magic == MH_MAGIC_64)
    image->is_64bit = TRUE;
else
    // error: invalid magic

unsigned long hdr_size = image->is_64bit ?
    sizeof(struct mach_header_64) :
    sizeof(struct mach_header);

// Update all sizeof(struct mach_header) to use hdr_size

// Add LC_SEGMENT_64 cases in switch statements
switch(lcp->cmd) {
case LC_SEGMENT:
    sg = (struct segment_command *)lcp;
    // ... existing code ...
    break;
case LC_SEGMENT_64:
    sg64 = (struct segment_command_64 *)lcp;
    // Convert to 32-bit for processing or handle directly
    break;
}
```

#### `reloc.c` and architecture-specific reloc files
**Changes**: May need updates if relocation entries reference 64-bit addresses

#### `symbols.c`
**Changes**: Add nlist_64 symbol table handling

**Estimated Effort**: ~200 lines of changes across 5-6 files

---

## 4. Boot Loader (boot-2)

**Location**: `boot-2/`

**Purpose**: Loads kernel at boot time

### MMIX Boot Loader

**Location**: `boot-2/mmix/` (already exists!)

Let's examine what's there:
```bash
ls -la boot-2/mmix/
```

**Current Status**: Directory exists, likely has basic structure

**Changes Needed**:
1. Add MH_MAGIC_64 detection for loading 64-bit kernel
2. Add LC_SEGMENT_64 support for kernel segments
3. Handle 64-bit entry points
4. Support 64-bit virtual addressing in boot memory map

**Key Files** (typical boot loader structure):
- `boot.c` or `load.c` - Mach-O loading code
- `prot.c` - Protected mode setup (for 64-bit addressing)
- `start.s` - Boot assembly code

**Implementation Pattern**:
```c
// In kernel loading code
if (mh->magic == MH_MAGIC) {
    // Load 32-bit kernel
    hdr_size = sizeof(struct mach_header);
} else if (mh->magic == MH_MAGIC_64) {
    // Load 64-bit kernel
    hdr_size = sizeof(struct mach_header_64);
    // Set up 64-bit page tables
    // Enable 64-bit addressing
} else {
    error("Invalid kernel magic");
}

// Parse load commands
switch(lcp->cmd) {
case LC_SEGMENT:
    load_segment_32(...);
    break;
case LC_SEGMENT_64:
    load_segment_64(...);
    break;
}
```

**Estimated Effort**: ~150 lines of changes

---

## 5. C Library (Libc-1)

**Location**: `Libc-1/`

**Purpose**: Standard C library, includes functions that may read Mach-O files

### Key Areas to Check

#### `Libc-1/mach/`
**Files that might need updates**:
- Any code that reads Mach-O headers (for dlopen, dlsym, etc.)
- Code that examines process memory layout

**Search Command**:
```bash
grep -r "MH_MAGIC\|mach_header" Libc-1/ --include="*.c"
```

#### Typical Files:
- `Libc-1/gen/nlist.c` - Symbol table reading (if exists)
- `Libc-1/posix/execve.c` - May examine executable format (if exists)

**Changes Needed**:
Similar to other components - add MH_MAGIC_64 detection wherever MH_MAGIC is checked.

**Estimated Effort**: ~50-100 lines of changes (if any modifications needed)

**Note**: Libc might not need changes if it doesn't directly parse Mach-O files (kernel and dyld handle that).

---

## 6. Kernel Extensions

**Location**: `kernel-7/` (module loading code)

**Purpose**: Load kernel modules (.kext bundles) at runtime

### Key Files

#### `kernel-7/kern/kmod.c` (if exists)
**Purpose**: Kernel module loading

**Changes Needed**:
1. Add MH_MAGIC_64 detection for 64-bit kernel extensions
2. Add LC_SEGMENT_64 handling
3. Support 64-bit symbol resolution

#### `kernel-7/libkern/` or `kernel-7/mach/`
**Search for module loading code**:
```bash
grep -r "kmod\|kext\|load_module" kernel-7/kern/ kernel-7/libkern/
```

**Implementation**:
```c
// In module loader
if (kmod_header->magic == MH_MAGIC_64) {
    // Load 64-bit kernel extension
    hdr_size = sizeof(struct mach_header_64);
    is_64bit = TRUE;
} else if (kmod_header->magic == MH_MAGIC) {
    hdr_size = sizeof(struct mach_header);
    is_64bit = FALSE;
} else {
    return KERN_INVALID_ARGUMENT;
}

// Parse load commands with 64-bit support
// Link symbols using nlist_64 if is_64bit
```

**Estimated Effort**: ~100 lines of changes

---

## 7. Objective-C Runtime (libobjc)

**Location**: `objc-1/` or `objc4-1/`

**Purpose**: Objective-C runtime, may need to handle 64-bit Mach-O metadata

### Key Areas

#### Objective-C Image Loading
**Files**:
- `objc-1/objc-load.c` or similar
- `objc4-1/runtime/objc-runtime.m`

**Purpose**: Loads Objective-C class metadata from Mach-O sections

**Changes Needed**:
1. Support 64-bit section_64 structures
2. Handle 64-bit pointers to class metadata
3. Update any code that walks Mach-O sections

**Search**:
```bash
find objc-1 objc4-1 -name "*.c" -o -name "*.m" 2>/dev/null | \
    xargs grep -l "mach_header\|LC_SEGMENT\|getsect"
```

**Implementation**:
```c
// In Objective-C image callback
void objc_image_load(const struct mach_header *mh, intptr_t vmaddr_slide) {
    const struct mach_header_64 *mh64;
    struct load_command *lc;
    struct segment_command *seg;
    struct segment_command_64 *seg64;
    unsigned long hdr_size;
    enum bool is_64bit;

    // Detect 64-bit
    if (mh->magic == MH_MAGIC_64) {
        is_64bit = TRUE;
        hdr_size = sizeof(struct mach_header_64);
        mh64 = (const struct mach_header_64 *)mh;
    } else {
        is_64bit = FALSE;
        hdr_size = sizeof(struct mach_header);
    }

    // Walk load commands
    lc = (struct load_command *)((char *)mh + hdr_size);
    for (i = 0; i < mh->ncmds; i++) {
        if (lc->cmd == LC_SEGMENT || lc->cmd == LC_SEGMENT_64) {
            if (lc->cmd == LC_SEGMENT) {
                seg = (struct segment_command *)lc;
                // Process 32-bit segment
            } else {
                seg64 = (struct segment_command_64 *)lc;
                // Process 64-bit segment
            }
        }
        lc = (struct load_command *)((char *)lc + lc->cmdsize);
    }
}
```

**Estimated Effort**: ~100-150 lines of changes

---

## Implementation Checklist

### For Each Component:

- [ ] **Search for MH_MAGIC references**
  ```bash
  grep -rn "MH_MAGIC" <component_dir>/
  ```

- [ ] **Search for mach_header references**
  ```bash
  grep -rn "struct mach_header" <component_dir>/ --include="*.c" --include="*.h"
  ```

- [ ] **Search for sizeof(struct mach_header)**
  ```bash
  grep -rn "sizeof.*mach_header" <component_dir>/
  ```

- [ ] **Search for LC_SEGMENT references**
  ```bash
  grep -rn "LC_SEGMENT" <component_dir>/
  ```

- [ ] **For each file found**:
  1. Add MH_MAGIC_64 detection alongside MH_MAGIC checks
  2. Add is_64bit boolean tracking
  3. Calculate hdr_size dynamically
  4. Replace sizeof(struct mach_header) with hdr_size
  5. Add LC_SEGMENT_64 case statements
  6. Handle segment_command_64 structures
  7. Handle section_64 structures (if applicable)
  8. Handle nlist_64 symbols (if applicable)

---

## Testing Strategy

### Unit Testing

For each component:

```bash
# Create 64-bit test executable
cat > test64.s << 'EOF'
    .text
    .globl _main
_main:
    SET $0,42
    SET $255,0
    TRAP 0,Halt,0
EOF

as -arch mmix -o test64.o test64.s
ld -arch mmix -o test64 test64.o

# Verify it's 64-bit
otool -h test64
# Should show: magic 0xfeedfacf

# Test with component
<component-specific test>
```

### Integration Testing

1. **Boot Test**: Boot 64-bit kernel with boot loader
2. **Execution Test**: Run 64-bit executable (kernel loads it)
3. **Dynamic Linking Test**: Load 64-bit shared library (dyld)
4. **Module Test**: Load 64-bit kernel extension
5. **Objective-C Test**: Load 64-bit Objective-C program

---

## Common Patterns Reference

### Pattern 1: Magic Detection
```c
if (magic == MH_MAGIC || magic == SWAP_LONG(MH_MAGIC)) {
    is_64bit = FALSE;
} else if (magic == MH_MAGIC_64 || magic == SWAP_LONG(MH_MAGIC_64)) {
    is_64bit = TRUE;
} else {
    return ERROR_INVALID_MAGIC;
}
```

### Pattern 2: Header Size
```c
unsigned long hdr_size = is_64bit ?
    sizeof(struct mach_header_64) :
    sizeof(struct mach_header);

char *lc_start = (char *)mh + hdr_size;
```

### Pattern 3: Load Command Iteration
```c
struct load_command *lc = (struct load_command *)lc_start;
for (i = 0; i < ncmds; i++) {
    switch(lc->cmd) {
    case LC_SEGMENT:
        // 32-bit handling
        break;
    case LC_SEGMENT_64:
        // 64-bit handling
        break;
    }
    lc = (struct load_command *)((char *)lc + lc->cmdsize);
}
```

### Pattern 4: Segment Handling
```c
if (lc->cmd == LC_SEGMENT) {
    struct segment_command *sg = (struct segment_command *)lc;
    vm_offset_t vmaddr = sg->vmaddr;  // 32-bit
    vm_size_t vmsize = sg->vmsize;
} else if (lc->cmd == LC_SEGMENT_64) {
    struct segment_command_64 *sg64 = (struct segment_command_64 *)lc;
    uint64_t vmaddr = sg64->vmaddr;  // 64-bit
    uint64_t vmsize = sg64->vmsize;
    // May need to check if addresses fit in vm_offset_t
}
```

### Pattern 5: Section Iteration
```c
if (seg->cmd == LC_SEGMENT) {
    struct section *sect = (struct section *)((char *)seg + sizeof(struct segment_command));
    for (j = 0; j < seg->nsects; j++) {
        // Process 32-bit section
        sect++;
    }
} else if (seg->cmd == LC_SEGMENT_64) {
    struct section_64 *sect64 = (struct section_64 *)((char *)seg + sizeof(struct segment_command_64));
    for (j = 0; j < seg->nsects; j++) {
        // Process 64-bit section
        sect64++;
    }
}
```

### Pattern 6: Symbol Table
```c
if (is_64bit) {
    struct nlist_64 *symbols = (struct nlist_64 *)(addr + symoff);
    for (i = 0; i < nsyms; i++) {
        uint64_t value = symbols[i].n_value;  // 64-bit
        // Process symbol
    }
} else {
    struct nlist *symbols = (struct nlist *)(addr + symoff);
    for (i = 0; i < nsyms; i++) {
        unsigned long value = symbols[i].n_value;  // 32-bit
        // Process symbol
    }
}
```

---

## Quick Implementation Scripts

### Find All Files Needing Changes
```bash
#!/bin/bash
# save as: find-64bit-work.sh

COMPONENTS="cctools-2/dyld boot-2/mmix Libc-1 kernel-7 objc-1 objc4-1"

for dir in $COMPONENTS; do
    if [ -d "$dir" ]; then
        echo "=== $dir ==="
        grep -rn "MH_MAGIC\|sizeof.*mach_header\|LC_SEGMENT" "$dir/" \
            --include="*.c" --include="*.h" --include="*.m" | \
            cut -d: -f1 | sort | uniq
        echo
    fi
done
```

### Verify 64-bit Support
```bash
#!/bin/bash
# save as: verify-64bit.sh

echo "Checking for MH_MAGIC_64 support..."

FILES=$(grep -rl "MH_MAGIC" . --include="*.c" --include="*.h")

for file in $FILES; do
    if ! grep -q "MH_MAGIC_64" "$file"; then
        echo "❌ Missing MH_MAGIC_64: $file"
    else
        echo "✅ Has MH_MAGIC_64: $file"
    fi
done
```

---

## Summary of Changes Per Component

| Component | Files to Modify | Lines to Add | Lines to Change | Difficulty |
|-----------|----------------|--------------|-----------------|------------|
| **Kernel** | 2 files | ~177 | ~16 | ✅ DONE |
| **Toolchain** | ~10 files | ~1,100 | ~50 | ✅ DONE |
| **dyld** | ~5 files | ~200 | ~30 | Medium |
| **Boot Loader** | ~3 files | ~150 | ~20 | Medium |
| **Libc** | ~2 files | ~50 | ~10 | Easy |
| **Kernel Ext** | ~2 files | ~100 | ~15 | Medium |
| **libobjc** | ~3 files | ~150 | ~20 | Medium |
| **TOTAL** | ~27 files | ~1,927 | ~161 | - |

---

## Next Steps

1. **dyld**: Highest priority - required for dynamic linking
2. **Boot Loader**: High priority - required for booting
3. **Kernel Extensions**: Medium priority - required for modules
4. **libobjc**: Medium priority - required for Objective-C apps
5. **Libc**: Low priority - may not need changes

---

## Final Verification

After implementing all components:

```bash
# Build toolchain
cd cctools-2 && make

# Build kernel
cd kernel-7 && make

# Build dyld
cd cctools-2/dyld && make

# Build boot loader
cd boot-2/mmix && make

# Test complete pipeline
as -arch mmix -o test.o test.s
ld -arch mmix -o test test.o
file test  # Should show: Mach-O 64-bit executable mmix
./test     # Should execute via kernel + dyld
```

---

**Status**: Implementation Guide Complete
**Ready For**: Component-by-component implementation
**Estimated Total Effort**: ~650 lines of changes across 7 components

Generated with [Claude Code](https://claude.com/claude-code)

Co-Authored-By: Claude <noreply@anthropic.com>
