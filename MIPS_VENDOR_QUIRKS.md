# MIPS Vendor Quirks and Hardware Variations

## Overview

MIPS processors from different vendors have various quirks, bugs, and extensions that require special handling in the Darwin kernel. This document catalogs all known vendor-specific behaviors and the workarounds implemented.

---

## Loongson/Godson (Chinese Academy of Sciences)

### Loongson 2E

**PRId:** 0x6302 (Company: 0x42, Implementation: 0x63, Revision: 0x02)

**Features:**
- MIPS64 (Release 2 compatible)
- 64KB I-cache, 64KB D-cache (4-way set associative, 32-byte lines)
- 64-entry TLB
- FPU included
- 800 MHz - 1.0 GHz

**Quirks:**

1. **LL/SC Atomic Operation Bug** (`MIPS_QUIRK_LOONGSON2_LLSC`)
   - **Problem:** SC (Store-Conditional) may succeed even when it should fail
   - **Impact:** Atomic operations can corrupt data in SMP systems
   - **Workaround:**
     ```c
     // Insert extra SYNC before LL/SC pair
     __asm__ volatile("sync");
     ll/sc sequence...
     __asm__ volatile("sync");
     ```
   - **Applied:** All spinlock and atomic operations

2. **Cache Coherency Issues** (`MIPS_QUIRK_LOONGSON2_CACHE`)
   - **Problem:** Write combining can cause cache inconsistency
   - **Impact:** Data corruption on DMA operations
   - **Workaround:** Disable write combining for I/O regions
   - **Applied:** pmap.c, DMA buffer allocation

3. **Branch Target Address Cache** (`MIPS_QUIRK_LOONGSON2_BTAC`)
   - **Problem:** BTAC and RAS disabled by default
   - **Impact:** Poor branch prediction performance
   - **Workaround:** Enable via CP0 DiagControl register (0x22)
     ```assembly
     mfc0 $t0, $22          # Read DiagControl
     ori  $t0, $t0, 0x0c    # Enable BTAC + RAS
     mtc0 $t0, $22          # Write back
     ```
   - **Applied:** Early boot initialization

### Loongson 2F

**PRId:** 0x6303-0x6305 (Revision: 0x03-0x05)

**Features:**
- MIPS64 Release 2
- 64KB I-cache, 64KB D-cache (4-way)
- 512KB L2 cache (4-way, 32-byte lines)
- 64-entry TLB
- 800 MHz - 1.0 GHz (later revisions 1.2 GHz)

**Quirks:**

1. **Early Revision LL/SC Bug** (Revision <= 0x04)
   - Same as Loongson 2E
   - Fixed in revision 0x05 and later

2. **L2 Cache Issues**
   - **Problem:** L2 cache line size mismatch with L1
   - **Workaround:** Explicit cache flushes between levels

3. **Improved Features:**
   - Better cache coherency than 2E
   - More robust BTAC
   - Hardware prefetching support

**Laptop Compatibility:**
- Used in Lemote YeeLoong 8089 laptop
- Requires ACPI workarounds for power management
- BIOS compatibility mode needed

### Loongson 3A/3B

**PRId:** 0x6305+ (Revision >= 0x05)

**Features:**
- MIPS64 Release 2
- Quad-core (3A) or dual-core (3B)
- 64KB I-cache, 64KB D-cache per core
- 4MB shared L2 cache
- 1.0-1.5 GHz
- Hardware cache coherency (HyperTransport)

**Quirks:**

1. **Improved LL/SC** (`MIPS_QUIRK_LOONGSON3_LLSC`)
   - No LL/SC bug
   - Full support for SMP atomics
   - Hardware cache coherency via HT bus

2. **GS464 Core Extensions**
   - Custom multimedia instructions
   - X86 translation extensions (for emulation)
   - Not documented - avoid using

**Multiprocessor Considerations:**
- Full SMP support
- Cache coherency protocol works correctly
- Inter-processor interrupts via HT

---

## Broadcom BMIPS

### BMIPS32/3300

**PRId:** 0x0200xx

**Features:**
- MIPS32 Release 1
- Used in BCM33xx, BCM63xx SoCs
- 16KB I-cache, 16KB D-cache
- 32-entry TLB

**Quirks:**

1. **Cache Flush Required** (`MIPS_QUIRK_BMIPS_CACHE`)
   - **Problem:** Write buffer not automatically flushed
   - **Workaround:** Explicit SYNC after cache operations
   - **Applied:** All cache management functions

2. **Interrupt Controller Differences**
   - Non-standard interrupt routing
   - Requires vendor-specific IRQ driver

### BMIPS4380

**PRId:** 0x0240xx

**Features:**
- MIPS32 Release 1
- Dual-core (threaded)
- 32KB I-cache, 32KB D-cache per core
- 64-entry TLB

**Quirks:**

1. **Thread Synchronization**
   - Two hardware threads share TLB
   - Requires careful TLB management in SMP

2. **Cache Coherency**
   - Hardware coherency between threads
   - Still needs SYNC for proper ordering

### BMIPS5000

**PRId:** 0x0250xx

**Features:**
- MIPS32 Release 1 + extensions
- Dual-core
- 32KB I-cache, 32KB D-cache
- 256KB L2 cache
- 128-entry TLB

**Quirks:**
- Generally well-behaved
- Standard cache operations work

---

## Cavium Octeon (Network Processors)

### CN38XX

**PRId:** 0x0D00xx

**Features:**
- MIPS64 Release 2
- 16-core capable
- 32KB I-cache, 16KB D-cache per core
- 128KB L2 cache shared
- Custom network acceleration

**Quirks:**

1. **Errata in Early Silicon** (`MIPS_QUIRK_CAVIUM_CN38XX`)
   - **Problem:** TLB flush may not complete
   - **Workaround:** Double TLB write with NOP delay
     ```assembly
     tlbwi
     nop; nop; nop; nop
     tlbwi
     ```

2. **Custom CP0 Registers**
   - CVMCtl ($9, select 7) - Cavium control
   - CVMMemCtl ($11, select 7) - Memory controller
   - Must preserve on context switch

3. **Hardware Packet Processing**
   - Custom instructions for network acceleration
   - Not used in general-purpose code

**TLB Configuration:**
- 128-entry TLB (larger than standard)
- Fast TLB refill required for line-rate packet processing

### CN50XX/CN58XX/CN63XX

**Improvements over CN38XX:**
- Fixed TLB errata
- Better cache coherency
- More cores (up to 32 on CN68XX)
- Improved power management

**Same Quirks:**
- Custom CP0 registers
- Non-standard interrupt routing
- Packet acceleration instructions

---

## Ingenic JZ47xx

### JZ4740/JZ4750/JZ4770

**PRId:** 0xE102xx

**Features:**
- MIPS32 Release 1
- Single core
- 16KB I-cache, 16KB D-cache
- 32-entry TLB
- Used in handheld devices (Dingoo, GCW Zero)

**Quirks:**

1. **Cache Operations** (`MIPS_QUIRK_INGENIC_CACHE`)
   - **Problem:** Standard cache ops may fail
   - **Workaround:** Use Ingenic-specific cache index operations
     ```c
     // Use Index_Writeback_Inv instead of Hit_Writeback_Inv
     cache_op(Index_Writeback_Inv_D, addr);
     ```

2. **Low-Power Features**
   - Aggressive clock gating
   - Requires wake-up delays after idle
   - WAIT instruction may hang - use polling instead

3. **FPU Issues**
   - FPU present but slow
   - Consider software floating point

**Multimedia Extensions:**
- Custom SIMD instructions (undocumented)
- Audio/video acceleration
- Not portable - avoid in kernel

---

## MIPS Technologies (Official Cores)

### 4Kc/4Km/4Kp

**PRId:** 0x0180xx

**Features:**
- MIPS32 Release 1/2
- Synthesizable core
- Configurable caches
- 16-entry TLB (minimum)

**Quirks:**
- Generally compliant
- Configuration-dependent features
- Check Config registers for actual configuration

### 24K/34K/74K

**PRId:** 0x0193xx, 0x0195xx, 0x0196xx

**Features:**
- MIPS32 Release 2
- 24K: Single core
- 34K: Multi-threading (2-4 VPEs)
- 74K: Dual-core
- DSP ASE support

**Quirks:**

1. **Multi-Threading (34K/74K)**
   - VPEs (Virtual Processing Elements) share TLB
   - Requires MT ASE support in kernel
   - ASID allocation per-VPE

2. **DSP ASE**
   - Additional registers (AC0-AC3, Hi1-Hi3, Lo1-Lo3)
   - Must save/restore in context switch
   - Disabled by default in SR.MX

### P5600/I6400/P6600 (Warrior/Aptiv)

**PRId:** 0x01A8xx, 0x01A9xx, 0x01AAxx

**Features:**
- MIPS32/64 Release 5+
- EVA (Enhanced Virtual Addressing)
- HTW (Hardware TLB Walker)
- Virtualization support

**Quirks:**

1. **EVA Configuration**
   - Allows user/kernel address overlap
   - Requires segment control setup
   - Changes memory map layout

2. **Hardware TLB Walker**
   - Can fill TLB automatically from page tables
   - Must configure PWBase/PWSize registers
   - Faster than software TLB refill

---

## Legacy MIPS

### R2000/R3000

**PRId:** 0x0001xx, 0x0002xx

**Quirks:**

1. **No Write Buffer** (`MIPS_QUIRK_NO_WRITE_COMBINE`)
   - All stores are blocking
   - Performance impact on I/O

2. **Limited TLB**
   - 64 entries, paired (odd/even)
   - No ASID support on R2000

### R4000/R4400

**PRId:** 0x0004xx

**Quirks:**

1. **Cache Bug** (`MIPS_QUIRK_R4000_SC`)
   - **Problem:** Secondary cache parity errors
   - **Workaround:** Disable S-cache or use ECC memory
   - **Applied:** Conditional on silicon revision

2. **Load Delay Slot**
   - Load result not available for 2 cycles
   - Compiler must insert NOPs

3. **TLB Size**
   - 48 entries (smaller than R10000's 64)

### R5432

**Quirks:**

1. **CP0 Hazards** (`MIPS_QUIRK_R5432_CP0`)
   - **Problem:** CP0 writes take multiple cycles
   - **Workaround:** Insert NOPs after MTC0
     ```assembly
     mtc0 $t0, $12
     nop; nop; nop
     ```

---

## Workaround Summary Table

| Vendor | Model | Critical Quirks | Impact | Workaround Complexity |
|--------|-------|----------------|--------|---------------------|
| Loongson | 2E/2F | LL/SC bug | **HIGH** | Medium (extra SYNC) |
| Loongson | 2E/2F | Cache coherency | **HIGH** | Low (disable WC) |
| Loongson | 3A/3B | (none major) | Low | N/A |
| Broadcom | BMIPS32 | Cache flush | Medium | Low (extra SYNC) |
| Broadcom | BMIPS4380 | TLB sharing | Medium | Medium (SMP aware) |
| Cavium | CN38XX | TLB errata | **HIGH** | Medium (double write) |
| Cavium | CN50XX+ | (fixed) | Low | N/A |
| Ingenic | JZ47xx | Cache ops | Medium | Medium (alt ops) |
| MIPS | 34K/74K | MT support | Medium | High (VPE handling) |
| MIPS | P5600+ | EVA/HTW | Low | High (config complex) |
| Legacy | R4000 | S-cache bug | Medium | Medium (disable cache) |

---

## Detection and Initialization

### Boot Sequence

1. **Early Detection** (locore.s)
   ```assembly
   mfc0 $t0, $15        # Read PRId
   # Minimal quirk handling for boot
   ```

2. **Full Detection** (cpu_quirks.c)
   ```c
   void mips_cpu_detect(void) {
       unsigned int prid = read_prid();
       // Parse company, implementation, revision
       // Set features and quirks flags
   }
   ```

3. **Quirk Initialization**
   ```c
   void mips_cpu_quirks_init(void) {
       if (mips_cpu.quirks & MIPS_QUIRK_LOONGSON2_LLSC)
           setup_loongson2_llsc_war();
       // ... other quirks
   }
   ```

### Runtime Checks

Throughout kernel code:

```c
if (cpu_has_loongson2_bug) {
    /* Use workaround */
    __asm__ volatile("sync");
    ll_sc_operation();
    __asm__ volatile("sync");
} else {
    /* Normal path */
    ll_sc_operation();
}
```

---

## Cache Operation Variations

### Standard MIPS

```c
#define CACHE_OP(op, addr) \
    __asm__ volatile("cache %0, 0(%1)" : : "i" (op), "r" (addr))
```

### Loongson 2E/2F

```c
/* Requires extra SYNC */
static inline void loongson2_cache_op(int op, void *addr) {
    __asm__ volatile("sync");
    __asm__ volatile("cache %0, 0(%1)" : : "i" (op), "r" (addr));
    __asm__ volatile("sync");
}
```

### Ingenic JZ47xx

```c
/* Must use Index operations */
static inline void ingenic_cache_flush(void *addr) {
    unsigned long index = (unsigned long)addr & 0x1fff;
    __asm__ volatile("cache 0x01, 0(%0)" : : "r" (index)); /* Index_Writeback_Inv_D */
}
```

---

## TLB Size Variations

| CPU | TLB Entries | VTLB | FTLB | Notes |
|-----|-------------|------|------|-------|
| R2000 | 64 | N/A | N/A | Paired entries |
| R3000 | 64 | N/A | N/A | Paired entries |
| R4000 | 48 | N/A | N/A | Smaller than typical |
| R10000 | 64 | N/A | N/A | Standard |
| 4Kc | 16 | 16 | 0 | Configurable minimum |
| 24K | 32 | 32 | 0 | Standard small core |
| 74K | 64 | 64 | 0 | Larger core |
| Loongson 2/3 | 64 | 64 | 0 | Standard |
| Cavium Octeon | 128 | 128 | 0 | Large for networking |
| Ingenic JZ | 32 | 32 | 0 | Small embedded |

**Kernel Handling:**

```c
/* Detect TLB size */
void tlb_size_detect(void) {
    unsigned int config1 = read_config1();
    mips_cpu.tlb_entries = ((config1 >> 25) & 0x3f) + 1;

    /* Override for known quirks */
    if (mips_cpu.company == MIPS_PRID_COMP_CAVIUM)
        mips_cpu.tlb_entries = 128;
}
```

---

## Testing Recommendations

### Loongson 2E/2F

1. **LL/SC Stress Test**
   ```c
   /* Run on SMP, verify no data corruption */
   parallel_spinlock_test();
   atomic_increment_test(1000000);
   ```

2. **Cache Coherency Test**
   ```c
   /* DMA + CPU access to same memory */
   dma_coherency_test();
   ```

### General Vendor Testing

1. **TLB Refill Performance**
   - Measure TLB miss latency
   - Verify quirk workarounds don't slow critical path

2. **Cache Performance**
   - Memory bandwidth tests
   - Cache miss penalty measurements

3. **Atomic Operation Correctness**
   - SMP stress tests
   - Lock contention scenarios

---

## Future Vendor Support

### Adding New Vendor

1. Add PRId constants to `cpu_quirks.h`
2. Create detect_XXX_cpu() function in `cpu_quirks.c`
3. Add quirk flags as needed
4. Implement workarounds
5. Update documentation
6. Test on actual hardware

### Example Template

```c
static void detect_newvendor_cpu(void)
{
    mips_cpu.vendor_name = "NewVendor";
    mips_cpu.cpu_name = "NewCPU";

    /* Set features */
    mips_cpu.features = MIPS_CPU_FPU | MIPS_CPU_LLSC;

    /* Set quirks if needed */
    if (needs_workaround)
        mips_cpu.quirks |= MIPS_QUIRK_NEWVENDOR_BUG;

    /* Set cache/TLB info */
    mips_cpu.icache_size = 32 * 1024;
    mips_cpu.tlb_entries = 64;
}
```

---

## References

- Loongson 2E/2F User Manual (Chinese)
- MIPS Architecture For Programmers (MIPS Technologies)
- Broadcom BMIPS documentation
- Cavium Octeon Software Development Kit
- Linux kernel arch/mips/kernel/cpu-probe.c

---

*This documentation is based on public specifications, Linux kernel source code, and empirical testing. Some vendor-specific details are undocumented and reverse-engineered.*

**Last Updated:** 2025-10-25
**Status:** Complete vendor quirks framework implemented
