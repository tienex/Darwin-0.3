# IA64 Quick Reference Guide

**Quick reference for developers working with IA64 (Itanium) support in Darwin 0.3**

## At a Glance

| Property | Value |
|----------|-------|
| Architecture | Intel Itanium (IA64) |
| Word Size | 64-bit |
| Data Model | LP64 |
| Endianness | Little-endian |
| Alignment | Natural (hardware-supported) |
| Pointer Size | 64 bits |
| `long` Size | 64 bits |

## Key Directories

```
architecture-1/ia64/          # Architecture headers
Libc-1/gen.subproj/ia64.subproj/      # General library functions
Libc-1/sys.subproj/ia64.subproj/      # System call wrappers
Libc-1/threads.subproj/ia64.subproj/  # Threading support
Csu-1/start.s                 # Startup code (see #ifdef ia64)
```

## Important Headers

### alignment.h
```c
__inline__ static unsigned long get_align_long(void *ivalue);
__inline__ static unsigned long put_align_long(unsigned long ivalue, void *ovalue);
```
IA64 handles unaligned access in hardware, so these are simple pass-through.

### ansi.h - LP64 Type Definitions
```c
#define _BSD_PTRDIFF_T_   long           // 64-bit
#define _BSD_SIZE_T_      unsigned long  // 64-bit
#define _BSD_SSIZE_T_     long           // 64-bit
```

### limits.h - 64-bit Limits
```c
#define LONG_MAX   9223372036854775807L
#define LONG_MIN   (-9223372036854775807L-1)
#define ULONG_MAX  0xffffffffffffffffUL
```

### byte_order.h
```c
NXSwapShort(unsigned short)      // 16-bit swap
NXSwapInt(unsigned int)          // 32-bit swap
NXSwapLong(unsigned long)        // 64-bit swap (!)
NXSwapLongLong(unsigned long long) // 64-bit swap
```
⚠️ Note: On IA64, `NXSwapLong()` swaps 64 bits (different from i386)

## Build Configuration

### Environment Variables
```bash
export RC_ARCHS="ia64"           # Build for IA64 only
export RC_ARCHS="ppc i386 ia64"  # Multi-arch build
```

### Makefile Integration
```makefile
# In subproject Makefile.preamble:
INCLUDED_ARCHS = ia64
PUBLIC_HEADER_DIR_SUFFIX = /ia64
PRIVATE_HEADER_DIR_SUFFIX = /ia64
```

## Data Type Sizes

| Type | i386 | ppc | ia64 |
|------|------|-----|------|
| `char` | 8 | 8 | 8 |
| `short` | 16 | 16 | 16 |
| `int` | 32 | 32 | 32 |
| `long` | 32 | 32 | **64** ⭐ |
| `long long` | 64 | 64 | 64 |
| `pointer` | 32 | 32 | **64** ⭐ |
| `size_t` | 32 | 32 | **64** ⭐ |
| `ptrdiff_t` | 32 | 32 | **64** ⭐ |

## Calling Convention

### Register Usage
```
out0-out7    Function arguments (first 8)
ret0-ret3    Return values
loc0-loc127  Local registers (saved across calls)
r0           Always zero
rp           Return pointer (branch register)
gp           Global pointer
sp           Stack pointer
```

### Function Prologue Pattern
```assembly
.proc function_name
function_name:
    alloc loc0 = ar.pfs, inputs, locals, outputs, rotating
    mov   loc1 = rp        // Save return pointer
    mov   loc2 = gp        // Save global pointer
    ...
```

### Function Epilogue Pattern
```assembly
    mov   rp = loc1        // Restore return pointer
    mov   ar.pfs = loc0    // Restore previous frame state
    br.ret.sptk.many rp    // Return
.endp function_name
```

## System Call Interface

### SYS.h Macros
Located in: `Libc-1/sys.subproj/ia64.subproj/SYS.h`

Example system call stub:
```assembly
#include "SYS.h"

LEAF(_open)
    // IA64-specific syscall implementation
    br.ret.sptk.many rp
END(_open)
```

### Error Handling
```assembly
// On error, jump to cerror
// cerror sets errno and returns -1
br.cond.sptk cerror
```

## Common Patterns

### Pointer Arithmetic (64-bit)
```c
// For i386/ppc (32-bit):
ptr + (n * 4)         // Add n words

// For IA64 (64-bit):
ptr + (n * 8)         // Add n words (!)
```

### Stack Frame
```assembly
// Stack layout (grows downward):
//
// High Address
// +-------------+
// | STRING AREA |
// +-------------+
// |      0      |
// +-------------+
// |   env[n]    |
// |     ...     |
// |   env[0]    |
// +-------------+
// |      0      |
// +-------------+
// |  argv[n-1]  |
// |     ...     |
// |  argv[0]    |
// +-------------+
// |    argc     | <- sp (8 bytes on IA64)
// +-------------+
// Low Address
```

### Loading Arguments
```assembly
ld8   out0 = [sp]           // argc (64-bit load)
adds  out1 = 8, sp          // argv (ptr is 8 bytes)
shladd out2 = r14, 3, out1  // envp (shift left 3 = multiply by 8)
```

## Preprocessor Checks

### Architecture Detection
```c
#ifdef ia64
    // IA64-specific code
#elif defined(i386)
    // i386-specific code
#elif defined(ppc)
    // ppc-specific code
#endif
```

### Combined Checks
```c
#if defined(m68k) || defined(i386) || defined(ppc) || defined(ia64)
    // Code for multiple architectures
#endif
```

## Common Pitfalls

### ⚠️ Size Assumptions
```c
// WRONG - assumes pointer is 32-bit
int ptr_value = (int)pointer;

// RIGHT - use proper type
long ptr_value = (long)pointer;  // or intptr_t
```

### ⚠️ Format Strings
```c
// WRONG
printf("%d", sizeof(long));        // Wrong for 64-bit

// RIGHT
printf("%ld", sizeof(long));       // Use %ld for long
printf("%zu", sizeof(long));       // Or %zu for size_t
```

### ⚠️ Array Indexing
```c
// Array of pointers on IA64:
char *argv[];

// WRONG - assumes 4-byte pointers
ptr = (char *)(argv + (n * 4));

// RIGHT - compiler handles pointer arithmetic
ptr = argv[n];  // Let compiler do the math
```

## Assembly Syntax

### Instruction Format
```assembly
// IA64 uses bundled instructions (3 per bundle)
{ .mii
    ld8   r1 = [r2]        // Memory instruction
    add   r3 = r4, r5      // Integer instruction
    mov   r6 = r7          // Integer instruction
}
```

### Common Instructions
```assembly
ld8    r1 = [r2]           // Load 8 bytes
st8    [r1] = r2           // Store 8 bytes
adds   r1 = imm, r2        // Add small immediate
shladd r1 = r2, n, r3      // r1 = (r2 << n) + r3
alloc  loc0 = ar.pfs, i, l, o, r  // Allocate register frame
br.ret.sptk.many rp        // Return from function
br.call.sptk.many rp = f   // Call function
```

## Debugging Tips

### Check Architecture
```bash
# Verify ia64 is in build configuration
grep RC_ARCHS CoreOSMakefiles-1/ReleaseControl/Common.make

# Check for ia64-specific code
grep -r "ifdef ia64" .
```

### Verify Headers
```bash
# Check header installation
ls -la /System/Library/Frameworks/System.framework/Versions/B/Headers/architecture/ia64/

# Verify types
grep -A5 "_BSD_SIZE_T_" architecture-1/ia64/ansi.h
```

### Build Verification
```bash
# Build for IA64 only
make RC_ARCHS=ia64

# Verify object files
file *.o  # Should show "IA-64"
```

## Quick Reference: File Locations

| Component | File Path |
|-----------|-----------|
| Build config | `CoreOSMakefiles-1/ReleaseControl/Common.make:48` |
| Headers | `architecture-1/ia64/*.h` |
| Startup | `Csu-1/start.s:141-172` |
| Gen library | `Libc-1/gen.subproj/ia64.subproj/` |
| System calls | `Libc-1/sys.subproj/ia64.subproj/` |
| Threading | `Libc-1/threads.subproj/ia64.subproj/` |
| Documentation | `IA64_IMPLEMENTATION_SUMMARY.md` |

## Additional Resources

- **Full Documentation**: See `IA64_IMPLEMENTATION_SUMMARY.md`
- **PR Description**: See `PULL_REQUEST_DESCRIPTION.md`
- **Intel Manual**: IA-64 Architecture Software Developer's Manual
- **Linux IA64**: https://www.kernel.org/doc/html/latest/ia64/

## Cheat Sheet Commands

```bash
# Build for IA64
export RC_ARCHS=ia64
make install

# Find IA64-specific files
find . -path "*/ia64.subproj/*" -o -path "*/ia64/*"

# Count system calls
ls Libc-1/sys.subproj/ia64.subproj/*.s | wc -l

# Check for architecture guards
grep -n "ifdef ia64" Csu-1/start.s

# View commit history
git log --oneline --grep="IA64\|ia64"
```

---

**Version:** Darwin 0.3 IA64 Support
**Last Updated:** 2025-10-25
**Commits:** 2b68b509, 02af7452, 37524114
