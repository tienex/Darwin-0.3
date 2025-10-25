# Chapter 25: Memory Protection and Register Banking

## 25.1 Protection Ring Model

DLX supports flexible protection with 2-ring or 4-ring modes:

```
Ring 0: Kernel (highest privilege)
Ring 1: Device drivers, OS services
Ring 2: System services, middleware
Ring 3: User applications (lowest privilege)
```

### Ring Control
```assembly
call.ring   target, new_ring    # Call with ring transition
ret.ring                        # Return to previous ring

# Get/set current ring
mfcr        rd, ring            # rd ← current ring
mtcr        ring, rs            # current ring ← rs (privileged)
```

## 25.2 VMS-Style Page Protection

Extended protection beyond standard RWX:

```c
typedef struct {
    uint64_t valid       : 1;
    uint64_t readable    : 1;
    uint64_t writable    : 1;
    uint64_t executable  : 1;
    uint64_t user        : 1;
    uint64_t global      : 1;
    uint64_t accessed    : 1;
    uint64_t dirty       : 1;

    /* VMS-style extended protection */
    uint64_t owner_read  : 1;   /* Owner can read */
    uint64_t owner_write : 1;   /* Owner can write */
    uint64_t owner_exec  : 1;   /* Owner can execute */
    uint64_t group_read  : 1;   /* Group can read */
    uint64_t group_write : 1;   /* Group can write */
    uint64_t group_exec  : 1;   /* Group can execute */
    uint64_t world_read  : 1;   /* World can read */
    uint64_t world_write : 1;   /* World can write */
    uint64_t world_exec  : 1;   /* World can execute */

    uint64_t owner_id    : 16;  /* Owner process ID */
    uint64_t group_id    : 8;   /* Group ID */
    uint64_t pfn         : 28;  /* Physical frame number */
} vms_pte_t;
```

## 25.3 Register Banking

Hardware context switching with separate register sets:

### Banking Modes

**1. Privilege-Based Banking**
- Separate register bank for each ring
- Automatic switching on ring transitions

**2. Exception-Based Banking**
- Separate banks for normal vs exception
- Fast interrupt handling

**3. Manual Banking**
- Software-controlled bank switching
- Flexible context management

**4. Hybrid Banking**
- Combination of privilege + exception

### Bank Configuration

```c
typedef struct {
    uint32_t enable      : 1;    /* Enable register banking */
    uint32_t num_banks   : 2;    /* 2-4 banks */
    uint32_t auto_switch : 1;    /* Auto-switch on exceptions */
    uint32_t current_bank: 2;    /* Current active bank (0-3) */
    uint32_t bank_mode   : 2;    /* Banking mode */
    uint32_t shared_mask : 32;   /* Shared register bitmask */
} bank_config_t;
```

### Bank Switching

```assembly
# Manual bank switch
setbank     bank_num            # Switch to bank

# Query current bank
getbank     rd                  # rd ← current bank number
```

### Bank Layout

Each bank contains:
- 32 general-purpose registers (r0-r31)
- 32 floating-point registers (f0-f31)
- Select CSRs (status, epc, cause, etc.)

Typically shared across banks:
- r0 (zero) - hardwired
- r2 (sp) - stack pointer
- r3 (gp) - global pointer
- r4 (tp) - thread pointer

## 25.4 Domain Protection

Memory domains with access control:

```c
struct domain_descriptor {
    uint64_t base_addr;         /* Domain base */
    uint64_t size;              /* Domain size */
    uint32_t permissions;       /* Access permissions */
    uint32_t ring_mask;         /* Which rings can access */
    uint32_t domain_id;         /* Domain identifier */
};
```

### Domain Switching

```assembly
# Enter protected domain
domain.enter domain_id, entry_point

# Exit domain
domain.exit return_value

# Cross-domain call
domain.call  domain_id, function, args
```

## 25.5 Capability-Based Protection (CHERI Integration)

Combines ring protection with capabilities:
- Rings control privilege level
- Capabilities control memory access
- Both must be satisfied for access

## 25.6 Multiple Page Sizes

DLX supports multiple page sizes for efficiency:

```
4 KB    - Standard pages
2 MB    - Large pages (reduces TLB pressure)
1 GB    - Huge pages (large memory regions)
16 GB   - Super-huge pages (DLX-specific)
```

### Page Size Selection

```c
/* Page table entry page size field */
#define PTE_PS_4KB      0
#define PTE_PS_2MB      1
#define PTE_PS_1GB      2
#define PTE_PS_16GB     3   /* DLX extension */
```

## 25.7 Memory Protection Keys (MPK)

Protection keys for user-space memory protection:

```assembly
# Set protection key for page
pkey.set    addr, key           # Assign protection key

# Modify protection key rights
pkey.mprotect addr, len, prot, key

# Read/write PKRU (Protection Key Rights for Userspace)
rdpkru      rd                  # Read PKRU
wrpkru      rs                  # Write PKRU
```

## 25.8 Guard Pages

Automatic guard page insertion:

```c
struct guard_config {
    uint32_t enable_stack_guard : 1;  /* Stack overflow protection */
    uint32_t enable_heap_guard  : 1;  /* Heap overflow protection */
    uint32_t guard_size         : 30; /* Guard page size */
};
```

## 25.9 Execute-Only Memory (XOM)

Pages that can be executed but not read:

```c
/* Page table entry flags */
#define PTE_X       (1 << 3)    /* Executable */
#define PTE_R       (1 << 1)    /* Readable */
#define PTE_W       (1 << 2)    /* Writable */
#define PTE_XOM     (PTE_X)     /* Execute-only (X without R) */
```

## 25.10 Performance Considerations

- **Banking overhead**: 1-2 cycles for automatic switch
- **TLB pressure**: Multiple page sizes reduce TLB misses
- **Domain crossings**: ~10-20 cycles overhead
- **Capability checks**: ~2-3 cycles when enabled
