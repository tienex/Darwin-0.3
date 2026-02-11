# DLX Architecture Support for Darwin

## Overview

This directory contains the DLX architecture support for Darwin, based on the ETH DLXSIM architecture. DLX is a RISC processor architecture designed by John L. Hennessy and David A. Patterson for their computer architecture textbooks.

## Key Features

### Hybrid MMU Support

The DLX implementation supports **both page tables and TLB** (Translation Lookaside Buffer) for memory management, with three operational modes:

1. **Page Tables Only Mode** (`DLX_MMU_MODE_PAGE_TABLES_ONLY`)
   - Traditional two-level hierarchical page tables
   - No TLB usage
   - Slower but simpler

2. **TLB Only Mode** (`DLX_MMU_MODE_TLB_ONLY`)
   - Software-managed TLB with 64 entries
   - No page table backing
   - Faster but limited to TLB capacity

3. **Both Mode** (`DLX_MMU_MODE_BOTH`) - **Default**
   - Uses both page tables and TLB
   - TLB provides fast lookups
   - Page tables provide backing store
   - Best performance with flexibility

## Architecture Specifications

### Virtual Memory Layout

- **User Space**: 0x00000000 - 0x7FFFFFFF (2GB)
- **Kernel Space**: 0x80000000 - 0xFFFFFFFF (2GB)

### Page Size

- 4KB (4096 bytes) pages
- Page shift: 12 bits

### Page Table Structure

Two-level hierarchical page table:

- **Page Directory**: 1024 entries (10 bits)
- **Page Table**: 1024 entries (10 bits)
- **Page Offset**: 4096 bytes (12 bits)

Virtual address breakdown:
```
31        22 21        12 11         0
+-----------+------------+------------+
|    PD     |     PT     |   Offset   |
| (10 bits) | (10 bits)  | (12 bits)  |
+-----------+------------+------------+
```

### Page Table Entry (PTE) Format

32-bit PTE with the following flags:

| Bit(s)  | Field        | Description                    |
|---------|--------------|--------------------------------|
| 31      | VALID        | Entry is valid                 |
| 30      | DIRTY        | Page has been modified         |
| 29      | REFERENCE    | Page has been referenced       |
| 28      | WRITE        | Page is writable               |
| 27      | USER         | User accessible                |
| 26      | GLOBAL       | Global (not ASID-specific)     |
| 25      | CACHED       | Page is cacheable              |
| 19-0    | PFN          | Physical frame number          |

### TLB (Translation Lookaside Buffer)

- **Entries**: 64 TLB entries
- **Management**: Software-managed
- **Replacement**: Round-robin policy
- **ASID Support**: 12-bit Address Space ID (4096 address spaces)

#### TLB Entry Structure

Each TLB entry consists of:

1. **Virtual Page**:
   - Bits 31-12: Virtual page number (VPN)
   - Bits 11-0: Address Space ID (ASID)

2. **Physical Page**:
   - Same format as PTE
   - Includes physical frame number and flags

### Address Space ID (ASID) Management

- **Kernel ASID**: 0 (reserved)
- **User ASIDs**: 1-4095
- **Purpose**: Avoid TLB flush on context switch
- **Global Pages**: Can be accessed by any ASID

## File Organization

### Headers (mach/dlx/)

- `vm_types.h` - VM type definitions
- `vm_param.h` - VM parameters and address space layout
- `boolean.h` - Boolean type definition
- `exception.h` - Exception type definitions

### Implementation (machdep/dlx/)

- `pmap.h` - Physical map interface and structures
- `pmap.c` - Physical map implementation with TLB and page table support
- `vm_machdep.c` - VM machine-dependent functions
- `exception.c` - Exception handlers for TLB misses and faults
- `README.md` - This file

## Key Functions

### TLB Management

```c
void dlx_tlb_init(void);                    // Initialize TLB
void dlx_tlb_flush(void);                   // Flush entire TLB
void dlx_tlb_flush_entry(vm_offset_t va);   // Flush specific entry
void dlx_tlb_flush_asid(unsigned int asid); // Flush entries for ASID
int  dlx_tlb_lookup(vm_offset_t va, unsigned int asid, dlx_tlb_entry_t *entry);
void dlx_tlb_insert(vm_offset_t va, unsigned int asid, pt_entry_t pte);
void dlx_tlb_update(vm_offset_t va, unsigned int asid, pt_entry_t pte);
```

### Exception Handlers

```c
void dlx_tlb_miss_handler(vm_offset_t va, int is_write);  // Handle TLB miss
void dlx_tlb_mod_handler(vm_offset_t va);                 // Handle TLB modification
void dlx_exception_handler(int type, vm_offset_t va, int is_write, void *ctx);
```

### Page Table Management

```c
pt_entry_t *dlx_pte_lookup(pmap_t pmap, vm_offset_t va);    // Find PTE
pt_entry_t *dlx_pte_allocate(pmap_t pmap, vm_offset_t va);  // Allocate PTE
```

### PMAP Operations

```c
void dlx_pmap_activate(pmap_t pmap, thread_t th, int cpu);    // Activate pmap
void dlx_pmap_deactivate(pmap_t pmap, thread_t th, int cpu);  // Deactivate pmap
```

### ASID Management

```c
unsigned int dlx_asid_alloc(pmap_t pmap);      // Allocate ASID
void dlx_asid_free(unsigned int asid);         // Free ASID
```

## Exception Handling

### TLB Miss Exceptions

When a TLB miss occurs:

1. Hardware raises TLB miss exception
2. `dlx_tlb_miss_handler()` is called
3. Handler looks up page table entry
4. If PTE is valid:
   - Set reference/dirty bits as needed
   - Insert entry into TLB (if using TLB)
   - Return to retry instruction
5. If PTE is invalid:
   - Call VM fault handler to page in
   - Retry instruction

### TLB Modification Exception

When writing to a read-only page:

1. Hardware raises TLB modification exception
2. `dlx_tlb_mod_handler()` is called
3. Handler checks if page is writable
4. If writable:
   - Set dirty bit in PTE and TLB
   - Return to retry instruction
5. If not writable:
   - Call VM protection fault handler

## Configuration

The MMU mode can be configured by setting the `dlx_mmu_mode` variable:

```c
extern dlx_mmu_mode_t dlx_mmu_mode;

// Set to use both page tables and TLB (default)
dlx_mmu_mode = DLX_MMU_MODE_BOTH;

// Set to use only page tables
dlx_mmu_mode = DLX_MMU_MODE_PAGE_TABLES_ONLY;

// Set to use only TLB
dlx_mmu_mode = DLX_MMU_MODE_TLB_ONLY;
```

## Performance Considerations

### TLB Hit Rate

- TLB has 64 entries covering 256KB of address space
- Round-robin replacement policy
- ASID tagging reduces context switch overhead

### Statistics

The implementation maintains statistics:

- `dlx_tlb_hits` - Number of successful TLB lookups
- `dlx_tlb_misses` - Number of TLB misses
- `dlx_tlb_replacements` - Number of TLB replacements

## Integration with Darwin VM System

The DLX pmap integrates with Darwin's VM system through:

- Standard pmap interface (pmap_enter, pmap_remove, etc.)
- VM fault handling for page faults
- Copy-on-write support
- Memory protection enforcement
- Page reference and modification tracking

## References

- *Computer Architecture: A Quantitative Approach* by Hennessy & Patterson
- ETH Zurich DLXSIM documentation
- Darwin kernel source code
- Mach VM system documentation

## Author

Implementation for Darwin-0.3 based on ETH DLXSIM architecture specification.
