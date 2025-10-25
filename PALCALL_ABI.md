# Alpha PALcode Call ABI for Darwin

## Overview

Darwin for Alpha provides a unified PALcode calling interface that works transparently across different firmware types (SRM/ARC) and PALcode variants (UNIX/NT). This is achieved through a kernel-managed trampoline page mapped into all user processes.

## PAL Call Page Location

The PAL call page is mapped at a fixed virtual address at the **very end of user address space**:

```
Address: 0x000003ffffffe000  (~4TB - 8KB)
Size:    8192 bytes (one Alpha page)
Access:  Read-only + Executable
Scope:   Present in all user address spaces (global mapping)
```

### Why the High Address?

- **No Conflicts**: Avoids low addresses used by programs, loaders, and `NULL` pointer detection
- **No Library Conflicts**: Well above typical shared library regions
- **No Heap Conflicts**: Heap grows upward from low addresses
- **No Stack Conflicts**: Stack grows downward from high addresses but stops well before this page
- **Clean Separation**: Clear boundary between user space and kernel space

## Trampoline Layout

The page contains 256 trampolines, one for each possible PALcode function:

```
Offset    Function    Content
------    --------    -------
0x0000    PAL 0       call_pal 0x00 ; ret ; nop ; nop
0x0010    PAL 1       call_pal 0x01 ; ret ; nop ; nop
0x0020    PAL 2       call_pal 0x02 ; ret ; nop ; nop
...
0x0FF0    PAL 255     call_pal 0xFF ; ret ; nop ; nop
```

Each trampoline is exactly 16 bytes (4 instructions).

## Calling Convention

### From C Code

Use the inline functions provided in `<architecture/alpha/pal_user.h>`:

```c
#include <architecture/alpha/pal_user.h>

/* Get thread-local storage pointer */
unsigned long tls = pal_rdunique();

/* Set thread-local storage pointer */
pal_wrunique(new_tls_value);

/* Set a breakpoint */
pal_bpt();

/* Generic PAL call (use with caution) */
unsigned long result = pal_call(function, arg0, arg1, arg2, arg3);
```

### From Assembly

```assembly
#include <architecture/alpha/pal_user.h>

/* Call rdunique (function 0x9E) */
lda     $27, PAL_CALL_ADDR(PAL_USER_rdunique)
jsr     $26, ($27), 0
/* Result in $0 */

/* Call wrunique (function 0x9F) */
bis     <value>, <value>, $16           /* Argument in a0 */
lda     $27, PAL_CALL_ADDR(PAL_USER_wrunique)
jsr     $26, ($27), 0
```

### Manual Calculation

```
PAL call address = 0x000003ffffffe000 + (function_number × 16)

Examples:
  rdunique (0x9E) → 0x000003ffffffe9E0
  wrunique (0x9F) → 0x000003ffffffe9F0
  bpt (0x80)      → 0x000003ffffffe800
```

## Allowed PAL Functions

Only specific PAL functions are allowed from userspace for security:

### UNIX PALcode (SRM Firmware)
- `0x80` - **bpt**: Breakpoint
- `0x81` - **bugchk**: Bugcheck/assert
- `0x9E` - **rdunique**: Read thread-local storage pointer
- `0x9F` - **wrunique**: Write thread-local storage pointer
- `0xAA` - **gentrap**: Generate software trap

### NT PALcode (ARC Firmware)
- `0x80` - **bpt**: Breakpoint
- `0x81` - **bugchk**: Bugcheck/assert
- `0xAA` - **gentrap**: Generate software trap

**Note**: Dangerous functions like `halt`, `swpctx`, `swpipl`, etc. will cause a kernel trap if called from userspace.

## Firmware Transparency

The kernel automatically detects which firmware/PALcode is in use:

- **SRM Firmware** → UNIX PALcode → trampolines use `call_pal` with UNIX function numbers
- **ARC Firmware** → NT PALcode → trampolines use `call_pal` with NT function numbers

Userspace programs don't need to know or care - they always call the same address and get the correct behavior.

## Thread-Local Storage

Alpha uses the PALcode unique value for thread-local storage:

```c
/* Get TLS pointer */
void *tls = (void *)alpha_get_tls();

/* Set TLS pointer */
alpha_set_tls((unsigned long)my_tls_block);
```

This is compatible with:
- Digital UNIX / Tru64 conventions
- GNU/Linux Alpha conventions
- Standard POSIX thread-local storage

## Memory Layout Example

```
0x0000000000000000  ← Program text (loaded by exec)
0x0000000000400000  ← Program data
0x0000000010000000  ← Heap (grows up) →
         ...
0x0000001000000000  ← Shared libraries (typical)
         ...
0x000003ff00000000  ← Stack (grows down) ↓
         ...
0x000003ffffffe000  ← PAL call page (8KB, read-only + executable)
0x0000040000000000  ← Kernel space begins (inaccessible to user)
```

## Performance

- **No System Call Overhead**: Direct call to PALcode, no kernel trap
- **No Context Switch**: Executes in current context
- **Branch Predictor Friendly**: Same addresses used repeatedly
- **Cache Friendly**: Page stays hot in I-cache

## Security Model

1. **Kernel controls the page**: Userspace cannot modify it
2. **Function filtering**: Only safe PAL functions allowed
3. **Privilege checking**: PALcode enforces kernel/user separation
4. **Auditing**: Kernel can intercept/log PAL calls if needed

## Compatibility

This ABI is compatible with:
- All Alpha CPU generations (EV3 through EV68)
- All firmware types (SRM, ARC)
- All PALcode variants (UNIX, NT, VMS with restrictions)
- All Alpha platforms (AlphaStation, AlphaServer, multia, etc.)

## Example Program

```c
#include <stdio.h>
#include <architecture/alpha/pal_user.h>

int main(void)
{
    unsigned long tls;

    /* Get current TLS pointer */
    tls = pal_rdunique();
    printf("Current TLS: 0x%016lx\n", tls);

    /* Set new TLS pointer */
    pal_wrunique(0x1234567890ABCDEF);

    /* Verify it was set */
    tls = pal_rdunique();
    printf("New TLS:     0x%016lx\n", tls);

    return 0;
}
```

## See Also

- `<architecture/alpha/pal_user.h>` - Userspace PAL call interface
- `<architecture/alpha/pal.h>` - Full PALcode function definitions
- `kernel-7/machdep/alpha/firmware.c` - Firmware abstraction implementation
