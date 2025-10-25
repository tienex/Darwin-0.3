# MIPS Kernel Implementation - Critical Gaps Analysis

## Current Status Summary

### What We Have ✅ (23 files in kernel-7/machdep/mips/)

**Mach Layer (kernel-7/mach/mips/):** 9 files
- boolean.h, exception.h, kern_return.h, machine_types.defs
- ndr.h, simple_lock.h, syscall_sw.h, thread_status.h
- vm_param.h, vm_types.h

**Machdep Layer (kernel-7/machdep/mips/):** 15 files
- **Headers:** asm.h, mach_param.h, machspl.h, pcb.h, pmap.h, regs.h, thread.h, trap.h
- **C Implementation:** genassym.c, machdep.c, pcb.c, trap.c, vm_machdep.c
- **Kernel LibC:** 10 files (memset, memcpy, memmove, memcmp, strcmp, strcpy, strlen, strcat, strncmp, strncpy)

**BSD Layer (kernel-7/bsd/mips/):** 18 files (100% parity)
- All BSD headers complete (cpu.h, disklabel.h, endian.h, exec.h, etc.)

**Kernel Services (kernel-7/kernserv/mips/):** 2 files
- spl.h, time.h

### What's Missing ❌ - Critical Path to Bootable Kernel

## 1. CRITICAL: Assembly Exception Vectors and Entry Points

**Priority: HIGHEST** - Without these, the kernel cannot execute

### locore.s (~400-600 lines needed)
Must implement:

```assembly
# Exception vector table (must be at specific addresses)
.org 0x000      # TLB refill vector
    j   tlb_refill_handler
    nop

.org 0x080      # XTLB refill (MIPS64 only)
    j   xtlb_refill_handler
    nop

.org 0x100      # Cache error vector
    j   cache_error_handler
    nop

.org 0x180      # General exception vector
    j   general_exception
    nop

.org 0x200      # Interrupt vector
    j   interrupt_handler
    nop
```

**Functions needed:**
- `tlb_refill_handler` - Fast TLB miss handler (critical path)
- `general_exception` - Main exception dispatcher
- `interrupt_handler` - Interrupt dispatch
- `syscall_handler` - System call entry
- `context_switch` - Thread switching
- `kernel_entry` - Kernel bootstrap entry point
- `exception_return` - Return from exception with state restore

**Register Save/Restore:**
- Save all 32 GPRs + lo/hi/pc/cause/status/badvaddr
- Preserve kernel stack
- Handle kernel vs user mode entry

**Estimated effort:** 400-600 lines of MIPS assembly

---

## 2. CRITICAL: Physical Memory Management

### pmap.c (~1000-1500 lines needed)

**Current status:** Header only (pmap.h exists with structure definitions)

**Must implement these functions:**

```c
// Bootstrap and initialization
void pmap_bootstrap(vm_offset_t *vstart, vm_offset_t *vend);
void pmap_init(void);

// Pmap lifecycle
pmap_t pmap_create(vm_size_t size);
void pmap_destroy(pmap_t pmap);
void pmap_reference(pmap_t pmap);
void pmap_release(pmap_t pmap);

// Page mapping core operations
void pmap_enter(pmap_t pmap, vm_offset_t va, vm_offset_t pa,
                vm_prot_t prot, boolean_t wired);
void pmap_remove(pmap_t pmap, vm_offset_t sva, vm_offset_t eva);
void pmap_protect(pmap_t pmap, vm_offset_t sva, vm_offset_t eva,
                  vm_prot_t prot);
void pmap_page_protect(vm_offset_t pa, vm_prot_t prot);

// TLB management (MIPS-specific)
void mips_tlb_flush(void);                     // Flush entire TLB
void mips_tlb_flush_addr(vm_offset_t va);      // Flush single address
void mips_tlb_write(int index, tlb_entry_t *entry);
void mips_tlb_read(int index, tlb_entry_t *entry);

// ASID management
asid_t pmap_asid_alloc(pmap_t pmap);
void pmap_asid_free(asid_t asid);

// Page table operations
pt_entry_t *pmap_pte(pmap_t pmap, vm_offset_t va);
void pmap_zero_page(vm_offset_t pa);
void pmap_copy_page(vm_offset_t src, vm_offset_t dst);

// Attributes
boolean_t pmap_is_modified(vm_offset_t pa);
boolean_t pmap_is_referenced(vm_offset_t pa);
void pmap_clear_modify(vm_offset_t pa);
void pmap_clear_reference(vm_offset_t pa);

// Address translation
boolean_t pmap_extract(pmap_t pmap, vm_offset_t va, vm_offset_t *pa);
```

**Key algorithms needed:**
1. **TLB Refill:** Software TLB miss handler (called from locore.s)
2. **ASID Management:** Track 8-bit ASID space, recycle when exhausted
3. **Page Table Walking:** 2-level or 3-level page tables
4. **TLB Shootdown:** Multiprocessor TLB consistency

**Estimated effort:** 1000-1500 lines

---

## 3. HIGH: BSD Unix Integration

### unix_signal.c (~200-300 lines)

Signal delivery to user processes:

```c
void sendsig(sig_t catcher, int sig, int mask, unsigned long code);
void sigreturn(struct mips_thread_state *state, struct sigcontext *scp);
```

**Needs:**
- Build signal frame on user stack
- Save user context
- Set up signal trampoline
- Handle signal return

### unix_startup.c (~150-200 lines)

BSD subsystem initialization:

```c
void bsd_startup(void);
void bsd_init(void);
void bsd_autoconf(void);
```

**Estimated effort:** 350-500 lines total

---

## 4. HIGH: Clock and Timer Support

### machine_clock.c (~200-300 lines)

```c
void mips_clock_init(void);
void mips_clock_interrupt(void);
unsigned long mips_read_count(void);      // CP0 Count register
void mips_set_compare(unsigned long val); // CP0 Compare register
```

**MIPS timer uses CP0 Count/Compare registers:**
- Count increments every cycle (or every 2 cycles)
- Compare triggers interrupt when Count == Compare
- Provides periodic tick for scheduler

**Estimated effort:** 200-300 lines

---

## 5. MEDIUM: Additional Support Files

### sys_machdep.c (~100-150 lines)
Machine-dependent system calls

### kern_machdep.c (~100-150 lines)
Kernel machine-dependent initialization

### in_cksum.c (~50-100 lines)
Network checksum (for TCP/IP stack)

**Estimated effort:** 250-400 lines total

---

## 6. OPTIONAL: Debug and Development Support

### kdp_machdep.c (~150-200 lines)
Kernel debugger protocol support (for GDB remote debugging)

**Can be stubbed initially**

---

## Critical Path Priority Order

To get a **minimally bootable** MIPS Darwin kernel:

### Phase 1: Exception Handling (MUST DO FIRST)
1. **locore.s** - Exception vectors and handlers (600 lines)
2. **pmap.c** - Memory management implementation (1500 lines)

**Rationale:** Without these, the kernel cannot handle TLB misses or exceptions and will crash immediately.

### Phase 2: System Integration
3. **machine_clock.c** - Timer support (250 lines)
4. **unix_signal.c** - Signal delivery (250 lines)
5. **unix_startup.c** - BSD initialization (150 lines)

### Phase 3: Polish
6. **sys_machdep.c** - Machine syscalls (100 lines)
7. **kern_machdep.c** - Kernel init (100 lines)
8. **in_cksum.c** - Network checksum (75 lines)

---

## Effort Estimation

| Component | Lines | Priority | Complexity |
|-----------|-------|----------|------------|
| locore.s | 600 | CRITICAL | Very High |
| pmap.c | 1500 | CRITICAL | Very High |
| machine_clock.c | 250 | HIGH | Medium |
| unix_signal.c | 250 | HIGH | Medium |
| unix_startup.c | 150 | HIGH | Low |
| sys_machdep.c | 100 | MEDIUM | Low |
| kern_machdep.c | 100 | MEDIUM | Low |
| in_cksum.c | 75 | MEDIUM | Low |
| **TOTAL** | **~3025** | | |

**Total estimated effort to bootable kernel:** ~3000 lines of code

**Time estimate:**
- Experienced MIPS developer: 20-30 hours
- With existing Darwin knowledge: 15-25 hours
- With reference implementations: 10-20 hours

---

## What's NOT Needed Yet

These are i386-specific and not required for basic MIPS kernel:

- ❌ BIOS support (bios.c, bios_asm.s)
- ❌ GDT/LDT/IDT (x86 descriptor tables)
- ❌ VGA graphics (vga.h, vgapriv.h, Helvetica.14.c)
- ❌ ISA DMA (dma.c, dma_*.h)
- ❌ x86 FPU emulation (fp_emul/*.s)
- ❌ PC emulation support (pc_support/*.c)
- ❌ APM power management (APM_*.c/h)

These would be replaced with MIPS equivalents when needed:
- MIPS boot firmware interface (not BIOS)
- MIPS exception control (not GDT/IDT)
- Framebuffer console (not VGA)
- MIPS FPU handling

---

## Comparison: i386 vs MIPS machdep

| Category | i386 Files | MIPS Files | Gap |
|----------|------------|------------|-----|
| Core Headers | ~30 | 8 | -22 |
| Core C Files | ~25 | 4 | -21 |
| Assembly Files | ~5 | 0 | -5 |
| Kernel LibC | 18 | 10 | -8 |
| x86-Specific | ~50 | N/A | N/A |
| **Arch-Neutral Gap** | | | **~56 files** |

**But:** Many i386 files are x86-specific. For functional parity, MIPS needs ~20-25 additional files, not 56.

---

## Next Immediate Actions

Based on this analysis, the critical path is:

1. **Implement locore.s** with exception vectors and handlers
2. **Implement pmap.c** with TLB management
3. **Test** with a minimal kernel image

These two files alone will enable:
- Kernel bootstrap
- Exception handling
- Memory management
- Process creation (basic)

After that, adding clock/signal/startup support will enable full user-space execution.

---

## Current Progress Metrics

- ✅ **Foundation:** 100% (architecture definitions, types, constants)
- ✅ **BSD Layer:** 100% (18/18 headers)
- ✅ **Mach Layer:** 100% (9/9 headers)
- ✅ **Basic Machdep:** 60% (15 files, missing critical locore.s and pmap.c)
- ❌ **Exception Handling:** 0% (locore.s not started)
- ❌ **Memory Management:** 5% (pmap.h only, no pmap.c)
- ❌ **Clock/Timer:** 0% (no machine_clock.c)
- ❌ **Unix Integration:** 0% (no unix_*.c files)

**Overall Kernel Readiness:** ~70% foundation, ~30% implementation

**To bootable:** Need ~3000 more lines focused on exception handling and memory management.
