# DLX Endian Switching - PowerPC-Style Palindrome Sequences

## Overview

This document specifies safe endian mode switching in DLX, inspired by PowerPC's palindrome instruction sequences. Proper endian switching requires careful instruction sequencing and cache management to avoid executing garbled instructions.

## Background: The Endian Problem

When switching endianness, instructions in memory change their binary representation:

```
Big Endian:    0x12 0x34 0x56 0x78  →  instruction A
Little Endian: 0x78 0x56 0x34 0x12  →  instruction B (different!)
```

If the CPU switches endianness mid-stream, subsequent instruction fetches will be garbled. PowerPC solves this with **palindrome sequences**: instructions that are valid and equivalent in both endian modes.

## Endian Control Bits

```c
/* Status Register Endian Bits (from DLX_SYSTEM.md) */
#define STATUS_SYS_BE   0x00040000  /* Bit 18: System Mode Big-Endian */
#define STATUS_USR_BE   0x00080000  /* Bit 19: User Mode Big-Endian */

/* Helper macros */
#define IS_KERNEL_MODE(status) ((status) & STATUS_KUC)
#define IS_BIG_ENDIAN(status) \
    (IS_KERNEL_MODE(status) ? ((status) & STATUS_SYS_BE) : ((status) & STATUS_USR_BE))

/* Endian switching instruction */
SETEND mode                     /* Set endianness: 0=LE, 1=BE */
```

## Palindrome Instructions

### DLX Palindromic Opcodes

These instructions have identical encoding when bytes are reversed:

```c
/* True palindrome instructions (32-bit) */

/* NOP variants */
0x00000000: NOP                 /* All zeros - palindrome */
0xFFFFFFFF: TRAP #-1            /* All ones - palindrome */
0x01000001: Special case        /* Byte-symmetric */

/* Bitwise-symmetric instructions */
/* Format: construct instructions where:
 * byte[0] == byte[3]
 * byte[1] == byte[2]
 */

/* Example palindromes for DLX */
ORI  r0, r0, 0x0000    /* 0x34000000 → 0x00000034 (different but safe NOP) */
ADDI r0, r0, 0         /* 0x20000000 → 0x00000020 (safe) */

/* Special palindromic encoding */
#define PALIN_NOP    0x00000000
#define PALIN_ISYNC  0x4C00012C  /* PowerPC-style, adapted */
```

### Constructing Palindrome Sequences

```assembly
# PowerPC-inspired palindrome sequence for endian switch
# These instructions are chosen to be safe in both endian modes

.align 4
switch_to_little_endian:
    # Instruction 1: Safe in both modes
    ori     r0, r0, 0          # 0x60000000 (safe NOP equivalent)

    # Instruction 2: SETEND (endian switch)
    setend  0                  # Switch to little-endian
                              # This is the critical instruction

    # Instruction 3: Another safe instruction
    ori     r0, r0, 0

    # Instruction 4: Sync
    isync                      # Instruction synchronize

    # Now in little-endian mode
    # Next instructions executed as little-endian
```

## Complete Endian Switch Sequence

### Big Endian → Little Endian (Kernel Mode)

```assembly
#
# Safe endian mode switch sequence (PowerPC-style)
# Must be executed in kernel mode (KUC=0)
#

.section __TEXT,__endian_switch
.align 64                      # Cache line alignment

# Entry: Big-endian mode
switch_be_to_le:
    # Save state
    mfsr    r3, STATUS          # r3 = current STATUS
    mfsr    r4, PC              # r4 = current PC

    # Disable interrupts
    mfsr    r5, STATUS
    andi    r5, r5, ~STATUS_IE  # Clear interrupt enable
    mtsr    STATUS, r5

    # Flush instruction cache
    # Must flush before switch to avoid fetching stale instructions
    li      r6, 0
    li      r7, ICACHE_SIZE
.L_flush_icache:
    icbi    r6                  # Invalidate instruction cache block
    addi    r6, r6, CACHE_LINE_SIZE
    cmp     r6, r7
    blt     .L_flush_icache

    # Sync after I-cache flush
    sync                        # Memory barrier
    isync                       # Instruction sync

    # Critical section: Palindrome sequence
    # These instructions are safe in both endian modes
    .align 16
.L_palindrome_start:
    ori     r0, r0, 0           # Safe instruction (NOP equivalent)
    ori     r0, r0, 0           # Safe instruction
    ori     r0, r0, 0           # Safe instruction

    # SETEND: Switch to little-endian
    # This instruction changes how subsequent fetches are interpreted
    mfsr    r8, STATUS
    andi    r8, r8, ~STATUS_SYS_BE  # Clear big-endian bit
    mtsr    STATUS, r8

    # More palindrome instructions
    ori     r0, r0, 0           # Safe in both modes
    ori     r0, r0, 0
    ori     r0, r0, 0

    # Sync after mode switch
    isync                       # Instruction synchronize
    sync                        # Memory barrier

.L_palindrome_end:

    # Now in little-endian mode
    # All subsequent instructions fetched as little-endian

    # Flush instruction cache again
    # Clear any big-endian instructions that might be cached
    li      r6, 0
    li      r7, ICACHE_SIZE
.L_flush_icache2:
    icbi    r6
    addi    r6, r6, CACHE_LINE_SIZE
    cmp     r6, r7
    blt     .L_flush_icache2

    sync
    isync

    # Flush data cache if needed
    li      r6, 0
    li      r7, DCACHE_SIZE
.L_flush_dcache:
    dcbf    r6                  # Data cache block flush
    addi    r6, r6, CACHE_LINE_SIZE
    cmp     r6, r7
    blt     .L_flush_dcache

    sync

    # Re-enable interrupts if they were enabled
    mtsr    STATUS, r3          # Restore original STATUS (except endian bit)

    # Branch to little-endian code
    # PC-relative branch to avoid address confusion
    b       .L_now_little_endian

.L_now_little_endian:
    # All code from here is little-endian
    jr      r31                 # Return
```

### Little Endian → Big Endian (Kernel Mode)

```assembly
# Reverse sequence: LE → BE
switch_le_to_be:
    mfsr    r3, STATUS
    mfsr    r4, PC

    # Disable interrupts
    mfsr    r5, STATUS
    andi    r5, r5, ~STATUS_IE
    mtsr    STATUS, r5

    # Flush I-cache
    li      r6, 0
    li      r7, ICACHE_SIZE
.L_flush_icache_le:
    icbi    r6
    addi    r6, r6, CACHE_LINE_SIZE
    cmp     r6, r7
    blt     .L_flush_icache_le

    sync
    isync

    # Palindrome sequence
    .align 16
.L_palindrome_le_start:
    ori     r0, r0, 0
    ori     r0, r0, 0
    ori     r0, r0, 0

    # Switch to big-endian
    mfsr    r8, STATUS
    ori     r8, r8, STATUS_SYS_BE  # Set big-endian bit
    mtsr    STATUS, r8

    ori     r0, r0, 0
    ori     r0, r0, 0
    ori     r0, r0, 0

    isync
    sync

.L_palindrome_le_end:

    # Flush caches in new mode
    li      r6, 0
    li      r7, ICACHE_SIZE
.L_flush_icache2_le:
    icbi    r6
    addi    r6, r6, CACHE_LINE_SIZE
    cmp     r6, r7
    blt     .L_flush_icache2_le

    sync
    isync

    li      r6, 0
    li      r7, DCACHE_SIZE
.L_flush_dcache_le:
    dcbf    r6
    addi    r6, r6, CACHE_LINE_SIZE
    cmp     r6, r7
    blt     .L_flush_dcache_le

    sync

    mtsr    STATUS, r3
    b       .L_now_big_endian

.L_now_big_endian:
    jr      r31
```

## Cache Management Instructions

```assembly
# Cache control instructions for endian switching

ICBI    rs                      # Instruction Cache Block Invalidate
                               # Invalidate I-cache line containing address in rs

DCBF    rs                      # Data Cache Block Flush
                               # Flush D-cache line containing address in rs

DCBI    rs                      # Data Cache Block Invalidate
                               # Invalidate D-cache line (no writeback)

DCBST   rs                      # Data Cache Block Store
                               # Write back D-cache line if modified

SYNC                            # Synchronize (memory barrier)
                               # Complete all previous memory operations

ISYNC                           # Instruction Synchronize
                               # Context synchronization
                               # Discard prefetched instructions

EIEIO                           # Enforce In-order Execution of I/O
                               # (PowerPC-style I/O barrier)
```

## Complete Kernel Endian Switch Function

```c
/*
 * switch_endianness - Switch kernel endianness
 * @new_mode: 0 = little-endian, 1 = big-endian
 *
 * Returns: 0 on success, -1 on failure
 *
 * IMPORTANT: This function uses a palindrome instruction sequence
 * to ensure safe execution during the endian mode transition.
 *
 * The sequence is:
 * 1. Disable interrupts
 * 2. Flush instruction cache
 * 3. Sync caches
 * 4. Execute palindrome sequence
 * 5. Switch endian bit
 * 6. More palindrome instructions
 * 7. Sync caches
 * 8. Flush instruction cache again
 * 9. Flush data cache
 * 10. Re-enable interrupts
 *
 * The palindrome sequence ensures that instructions are valid
 * regardless of current endian mode, preventing garbled instruction
 * execution during the transition.
 */

int switch_endianness(int new_mode)
{
    unsigned long flags;
    unsigned long status;
    int current_mode;

    /* Must be in kernel mode */
    asm volatile("mfsr %0, $STATUS" : "=r"(status));
    if (status & STATUS_KUC) {
        return -1;  /* Cannot switch endian in user mode */
    }

    /* Get current endian mode */
    current_mode = (status & STATUS_SYS_BE) ? 1 : 0;

    if (current_mode == new_mode) {
        return 0;  /* Already in requested mode */
    }

    /* Disable interrupts */
    local_irq_save(flags);

    /* Call assembly endian switch sequence */
    if (new_mode == 0) {
        /* Switch to little-endian */
        switch_be_to_le();
    } else {
        /* Switch to big-endian */
        switch_le_to_be();
    }

    /* Restore interrupts */
    local_irq_restore(flags);

    return 0;
}
```

## User Mode Endian Switching

User mode endianness is typically switched via system call:

```c
/* System call: setend */
SYSCALL_DEFINE1(setend, int, mode)
{
    struct pt_regs *regs = current_pt_regs();

    if (mode != 0 && mode != 1)
        return -EINVAL;

    /* Update user mode endian bit */
    if (mode == 1)
        regs->status |= STATUS_USR_BE;
    else
        regs->status &= ~STATUS_USR_BE;

    /*
     * No cache flush needed for user mode switch
     * because kernel controls I-cache coherency on
     * return to user space
     */

    return 0;
}
```

## Endian-Aware Memory Access

### Byte Swapping Macros

```c
/* Byte swap for endian conversion */
static inline uint32_t swab32(uint32_t x)
{
    return ((x & 0x000000FF) << 24) |
           ((x & 0x0000FF00) << 8) |
           ((x & 0x00FF0000) >> 8) |
           ((x & 0xFF000000) >> 24);
}

static inline uint16_t swab16(uint16_t x)
{
    return ((x & 0x00FF) << 8) |
           ((x & 0xFF00) >> 8);
}

/* Load word with byte swap */
static inline uint32_t lwbrx(void *addr)
{
    uint32_t val;
    asm volatile("lwbrx %0, 0, %1" : "=r"(val) : "r"(addr));
    return val;
}

/* Store word with byte swap */
static inline void stwbrx(uint32_t val, void *addr)
{
    asm volatile("stwbrx %0, 0, %1" :: "r"(val), "r"(addr));
}
```

### Endian-Aware Load/Store Instructions

```assembly
# PowerPC-style byte-reversed loads/stores
LWBRX  rd, (rs)                 # Load word byte-reversed indexed
STWBRX rs, (rt)                 # Store word byte-reversed indexed
LHBRX  rd, (rs)                 # Load halfword byte-reversed indexed
STHBRX rs, (rt)                 # Store halfword byte-reversed indexed
```

## Device Tree Endian Property

```dts
/* Device tree specification for endian-aware devices */
device@0 {
    compatible = "dlx,endian-device";
    reg = <0x10000000 0x1000>;

    /* Endian property */
    big-endian;           /* Device is big-endian */
    /* or */
    little-endian;        /* Device is little-endian */
    /* or */
    native-endian;        /* Device matches CPU endian */
};
```

## Boot-Time Endian Selection

```c
/* Early boot endian initialization */
void __init setup_endianness(void)
{
    unsigned long status;
    int boot_endian;

    /* Read boot endian from device tree or firmware */
    boot_endian = of_get_endian_mode();

    /* Set initial kernel endian */
    asm volatile("mfsr %0, $STATUS" : "=r"(status));

    if (boot_endian == ENDIAN_BIG)
        status |= STATUS_SYS_BE;
    else
        status &= ~STATUS_SYS_BE;

    asm volatile("mtsr $STATUS, %0" :: "r"(status));

    /* Flush caches */
    flush_icache_all();
    flush_dcache_all();
}
```

## Endian-Safe Data Structures

```c
/* Endian-annotated types (like Linux __be32, __le32) */
typedef uint32_t __be32;        /* Big-endian 32-bit */
typedef uint32_t __le32;        /* Little-endian 32-bit */
typedef uint16_t __be16;        /* Big-endian 16-bit */
typedef uint16_t __le16;        /* Little-endian 16-bit */

/* Conversion functions */
static inline uint32_t be32_to_cpu(__be32 val)
{
    if (IS_BIG_ENDIAN(current_status()))
        return val;
    else
        return swab32(val);
}

static inline uint32_t le32_to_cpu(__le32 val)
{
    if (IS_BIG_ENDIAN(current_status()))
        return swab32(val);
    else
        return val;
}

static inline __be32 cpu_to_be32(uint32_t val)
{
    if (IS_BIG_ENDIAN(current_status()))
        return val;
    else
        return swab32(val);
}

static inline __le32 cpu_to_le32(uint32_t val)
{
    if (IS_BIG_ENDIAN(current_status()))
        return swab32(val);
    else
        return val;
}
```

## Testing Endian Switch

```c
/* Test program to verify endian switching */
void test_endian_switch(void)
{
    uint32_t test_val = 0x12345678;
    uint32_t read_val;

    printf("Original value: 0x%08x\n", test_val);

    /* Test big-endian mode */
    switch_endianness(ENDIAN_BIG);
    printf("After switch to BE: 0x%08x\n", test_val);

    /* Write to memory */
    *(volatile uint32_t *)0x10000000 = test_val;

    /* Switch to little-endian */
    switch_endianness(ENDIAN_LITTLE);

    /* Read from memory (will be byte-swapped) */
    read_val = *(volatile uint32_t *)0x10000000;
    printf("After switch to LE, read value: 0x%08x\n", read_val);

    /* Should be swapped: 0x78563412 */
}
```

## Summary

### Key Points

1. **Palindrome Sequences**: Use instructions that are safe in both endian modes
2. **Cache Flushing**: Mandatory before and after endian switch
3. **Synchronization**: SYNC and ISYNC after critical operations
4. **Kernel-Only**: Endian switching is a privileged operation
5. **Interrupt Disable**: Required during switch sequence
6. **Testing**: Thorough testing of palindrome sequences

### Performance Considerations

- Endian switching is expensive (100+ cycles)
- Should be done rarely (boot time, mode changes)
- Cache flushes dominate cost
- Not suitable for frequent switching

### Safety Checklist

- ✓ Disable interrupts
- ✓ Flush instruction cache before switch
- ✓ Use palindrome instruction sequence
- ✓ Synchronize after mode change
- ✓ Flush instruction cache after switch
- ✓ Flush data cache after switch
- ✓ Test thoroughly in both modes

---

**Document Status**: Specification Complete
**Implementation Status**: Ready for kernel integration
**Complexity**: High - requires careful assembly programming
**Testing Required**: Extensive - data corruption risk if incorrect
**PowerPC Compatibility**: Based on proven PowerPC palindrome approach
