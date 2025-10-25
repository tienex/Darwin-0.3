# DLXSIM Architecture Analysis

Based on analysis of actual DLXSIM source code from:
- https://github.com/sgianelli/DLXOS-Virtual-Memory-Project

## Source Files Analyzed

- `dlx.h` - DLX processor flags and constants
- `memory.h` - Memory management definitions
- `traps.h` - Exception/trap definitions
- `dlxsim.cc` - Complete simulator implementation

## Key Findings

### 1. Dual MMU Architecture (**CONFIRMED**)

DLX supports **BOTH** page tables and TLB via status register flags:

```c
#define DLX_STATUS_PAGE_TABLE  0x100   // Set -> use a page table
#define DLX_STATUS_TLB         0x200   // Set -> use a software-loaded TLB
```

**These can be used independently OR together!**

- Page table only: Set 0x100
- TLB only: Set 0x200
- **Both (hybrid)**: Set 0x300

### 2. Page Table Implementation

**Two-level hierarchical page tables** with configurable page sizes:

- **Default configuration**:
  - L1 page size: 512KB (2^19 bits)
  - L2 page size: 8KB (2^13 bits)
  - Page mask: 0x1FFF (8KB - 1)

- **Special registers**:
  - `DLX_SREG_PGTBL_BASE` - Page table base address
  - `DLX_SREG_PGTBL_BITS` - Page size configuration (L1 bits | (L2 bits << 16))
  - `DLX_SREG_PGTBL_SIZE` - Page table size
  - `DLX_SREG_FAULT_ADDR` - Faulting address on exception

- **Virtual address translation**:
  ```
  VA bits 31-19: L1 index (13 bits for 512KB granularity)
  VA bits 18-13: L2 index (6 bits for 64 entries)
  VA bits 12-0:  Offset within 8KB page
  ```

- **PTE Format** (32-bit):
  ```c
  #define MEMORY_PTE_VALID       0x00000001  // Page is valid
  #define MEMORY_PTE_DIRTY       0x00000002  // Page has been modified
  #define MEMORY_PTE_REFERENCED  0x00000004  // Page has been accessed
  #define MEMORY_PTE_MASK        (~0x7)      // Physical address mask
  ```

### 3. TLB Implementation

**Fully associative software-managed TLB**:

- **Number of entries**: `DLX_TLB_NENTRIES` (typically 16-64)

- **TLB entry structure**:
  - `tlbVpage`: Virtual page number + entry flags
  - `tlbPpage`: Physical page number + page size encoding

- **Variable page sizes**:
  - Page size encoded in `tlbPpage` low bits
  - Supports different page sizes per entry
  - `DLX_TLB_ENTRY_PAGESIZE_MASK` extracts page size

- **TLB lookup**:
  - Fully associative search through all entries
  - Checks VA range: `(vaddr >= tlbVpage) && (vaddr < tlbVpage + pageSize)`
  - Validates `DLX_PTE_VALID` flag
  - Updates `DLX_PTE_REFERENCED` and `DLX_PTE_DIRTY` on access

- **TLB miss handling**:
  - Sets `DLX_SREG_FAULT_ADDR` to faulting address
  - Raises `DLX_EXC_TLBFAULT` exception
  - OS must handle miss and load TLB entry

### 4. Exception Types

From `traps.h`:

```c
#define TRAP_ILLEGALINST  0x1   // Illegal instruction
#define TRAP_ADDRESS      0x2   // Bad address alignment
#define TRAP_ACCESS       0x3   // Illegal memory access
#define TRAP_OVERFLOW     0x4   // Arithmetic overflow
#define TRAP_DIV0         0x5   // Divide by zero
#define TRAP_PRIVILEGE    0x6   // Privilege violation
#define TRAP_FORMAT       0x7   // Malformed instruction
#define TRAP_PAGEFAULT    0x20  // Page fault (page table mode)
#define TRAP_TLBFAULT     0x30  // TLB miss (TLB mode)
#define TRAP_TIMER        0x40  // Timer interrupt
#define TRAP_KBD          0x48  // Keyboard interrupt
```

### 5. System Registers

**Status Register Flags**:
```c
#define DLX_STATUS_INTRMASK    0x0f   // Interrupt mask (4 bits)
#define DLX_STATUS_FPTRUE      0x20   // FP comparison result
#define DLX_STATUS_SYSMODE     0x40   // System (kernel) mode
#define DLX_STATUS_PAGE_TABLE  0x100  // Enable page tables
#define DLX_STATUS_TLB         0x200  // Enable TLB
```

Additional status bits for translation control:
- `DLX_STATUS_XLATE_RD` - Translate system reads
- `DLX_STATUS_XLATE_WR` - Translate system writes

### 6. Memory Layout

- **User space**: Translated via page tables/TLB (when in user mode)
- **Kernel space**: Direct mapping (physical = virtual)
- **I/O space**: Memory-mapped I/O at fixed addresses
- **Special addresses**:
  - `DLX_MEMSIZE_ADDRESS` (0xffff0000) - System memory size
  - `DLX_TIMER_ADDRESS` (0xfff00010) - Timer control
  - `DLX_KBD_*` (0xfff00100+) - Keyboard I/O

### 7. Translation Process

**Page Table Mode**:
1. Check if translation needed (user mode or xlate bits set)
2. Extract L1 index from VA
3. Read L1 PTE, check for L2 table pointer
4. If L2 exists, read L2 PTE
5. Validate PTE (check VALID bit)
6. Update REFERENCED/DIRTY bits
7. Extract physical address and add offset
8. Raise `TRAP_PAGEFAULT` if invalid

**TLB Mode**:
1. Search all TLB entries for matching VA range
2. Check VALID flag on matching entry
3. Update REFERENCED/DIRTY flags
4. Extract physical address from matched entry
5. Raise `TRAP_TLBFAULT` if no match

**Hybrid Mode** (both flags set):
- Implementation-dependent
- Could check TLB first, fall back to page tables
- Or use TLB as cache over page tables

## Implementation Differences from Original

### What I Got Right:
✓ Dual MMU support (page tables + TLB)
✓ Two-level page tables
✓ Software-managed TLB
✓ PTE flags (VALID, DIRTY, REFERENCED)
✓ Exception handling for page/TLB faults
✓ ASID-like support for address spaces

### What Needs Correction:
✗ Page size: Should be 8KB (not 4KB)
✗ PTE bit positions: VALID=0x1, DIRTY=0x2, REF=0x4 (not 0x80000000...)
✗ TLB: Should be fully associative with variable page sizes
✗ Missing special registers (PGTBL_BASE, PGTBL_BITS, etc.)
✗ Page table structure: Configurable L1/L2 sizes

## Conclusion

The original implementation was **conceptually correct** but had wrong:
- Constants (page size, PTE bit positions)
- Special register definitions
- TLB implementation details (fully associative, variable page sizes)

The DLXSIM architecture DOES support the hybrid page table + TLB approach as originally implemented!

## Source Code Location

Full source available at:
```
/tmp/DLXOS-Virtual-Memory-Project/lab3_2/src/
├── dlx.h           # Processor flags
├── memory.h        # Memory management
├── traps.h         # Exception definitions
├── dlxsim.cc       # Complete simulator
└── memory.c        # Memory management implementation
```
