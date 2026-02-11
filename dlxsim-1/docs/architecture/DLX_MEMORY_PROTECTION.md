# DLX Advanced Memory Protection and Register Banking

## Overview

This document specifies advanced memory protection, register banking, and flexible page size support for the DLX architecture, enabling:

- **Register Banking**: Hardware context switching with separate register sets per privilege level
- **4-Ring Protection**: Fine-grained privilege levels (Ring 0-3) like x86
- **VMS-Style Page Protection**: Advanced per-page protection for porting OpenVMS
- **Multiple Page Sizes**: From 4KB to 16GB for performance and flexibility

All features are **optional** and controlled via configuration bits in CP0 control registers.

## 1. Register Banking Architecture

### Register Bank Configuration

Register banking provides hardware-accelerated context switching by maintaining separate physical register sets for different execution contexts.

```c
/* CP0 Register: Bank Configuration (CP0 reg 30, sel 0) */
typedef struct {
    uint32_t enable:1;          /* Bit 0: Enable register banking */
    uint32_t num_banks:2;       /* Bits 1-2: Number of banks (2-4) */
    uint32_t auto_switch:1;     /* Bit 3: Auto-switch on exception/eret */
    uint32_t current_bank:2;    /* Bits 4-5: Current active bank */
    uint32_t bank_mode:2;       /* Bits 6-7: Banking mode */
#define BANK_MODE_PRIVILEGE     0   /* Bank per privilege level */
#define BANK_MODE_EXCEPTION     1   /* Bank per exception type */
#define BANK_MODE_MANUAL        2   /* Manual bank selection */
#define BANK_MODE_HYBRID        3   /* Privilege + interrupt bank */
    uint32_t reserved:24;       /* Bits 8-31 */
} dlx_bank_config_t;

/* CP0 Register: Bank Status (CP0 reg 30, sel 1) */
typedef struct {
    uint32_t bank0_valid:1;     /* Bank 0 initialized */
    uint32_t bank1_valid:1;     /* Bank 1 initialized */
    uint32_t bank2_valid:1;     /* Bank 2 initialized */
    uint32_t bank3_valid:1;     /* Bank 3 initialized */
    uint32_t bank0_dirty:1;     /* Bank 0 modified */
    uint32_t bank1_dirty:1;     /* Bank 1 modified */
    uint32_t bank2_dirty:1;     /* Bank 2 modified */
    uint32_t bank3_dirty:1;     /* Bank 3 modified */
    uint32_t last_bank:2;       /* Previous bank (for nesting) */
    uint32_t reserved:22;
} dlx_bank_status_t;
```

### Register Bank Layout

Each bank contains a complete copy of general-purpose registers:

```
Bank 0 (Kernel/Ring 0):
  r0-r31: Full register set
  FPR0-FPR31: Floating-point registers
  Special: Status snapshot

Bank 1 (Interrupt/Exception):
  r0-r31: Full register set
  FPR0-FPR31: Floating-point registers
  Special: Pre-interrupt state

Bank 2 (User/Ring 3):
  r0-r31: Full register set
  FPR0-FPR31: Floating-point registers
  Special: User context

Bank 3 (Guest/Hypervisor):
  r0-r31: Full register set
  FPR0-FPR31: Floating-point registers
  Special: Guest state
```

### Register Bank Mapping Modes

#### Mode 0: Privilege-Based Banking
```c
/* Automatic bank selection based on privilege level */
Bank 0: Kernel mode (KUC = 0, Ring = 0)
Bank 1: Supervisor mode (KUC = 0, Ring = 1)
Bank 2: User mode (KUC = 1, Ring = 3)
Bank 3: Guest mode (Guest bit set)

/* Example privilege transition */
void handle_syscall(void) {
    /* Hardware automatically:
     * 1. Saves current bank number
     * 2. Switches to Bank 0 (kernel)
     * 3. Sets KUC = 0
     */

    /* Kernel code executes with Bank 0 registers */
    do_syscall();

    /* ERET instruction:
     * 1. Restores previous bank
     * 2. Restores KUC bit
     * 3. Returns to user code with Bank 2 registers
     */
}
```

#### Mode 1: Exception-Based Banking
```c
/* Different banks for different exception types */
Bank 0: Normal kernel execution
Bank 1: Interrupts (fast context save)
Bank 2: TLB refill (minimal overhead)
Bank 3: User mode

void handle_interrupt(void) {
    /* Hardware switches to Bank 1 automatically */
    /* All registers from Bank 0 preserved */

    /* Handle interrupt with fresh register set */
    service_interrupt();

    /* ERET restores Bank 0 */
}
```

#### Mode 2: Manual Banking
```c
/* Software-controlled bank switching */
INSTRUTION: SETBANK bank_num

/* Explicitly save/restore banks */
void context_switch(task_t *from, task_t *to) {
    /* Save current bank to task structure */
    asm("mfc0 %0, $30, 1" : "=r"(from->bank_status));

    /* Switch to target bank */
    asm("setbank %0" :: "r"(to->bank_num));

    /* All registers now from target bank */
}
```

#### Mode 3: Hybrid (Privilege + Interrupt)
```c
/* Combines privilege-based with interrupt banking */
Bank 0: Kernel (Ring 0)
Bank 1: Interrupt (any privilege level)
Bank 2: User (Ring 3)
Bank 3: Reserved/Guest

/* Nested interrupts */
void nested_interrupt_handler(void) {
    /* First interrupt: Bank 0 → Bank 1 */
    /* Interrupt context saved in Bank 1 */

    if (allow_nested) {
        enable_interrupts();

        /* Second interrupt: Bank 1 → Bank 1 (different instance) */
        /* Hardware pushes Bank 1 state to shadow bank */
    }
}
```

### Register Bank Instructions

```assembly
# SETBANK - Set active register bank (privileged)
SETBANK  imm3              # Switch to bank 0-3

# GETBANK - Read current bank number
GETBANK  rd                # rd = current bank

# SAVEBANK - Save bank to memory
SAVEBANK bank, (rs)        # Save bank to memory address

# LOADBANK - Load bank from memory
LOADBANK bank, (rs)        # Load bank from memory address

# SWAPBANK - Atomic bank swap
SWAPBANK bank              # Switch bank and return old bank in $v0
```

### Register Bank Shadow Stack

For nested interrupts/exceptions with banking enabled:

```c
/* Shadow bank stack (4 levels deep) */
typedef struct {
    uint32_t regs[32];      /* General-purpose registers */
    uint32_t fpr[32];       /* Floating-point registers */
    uint32_t status;        /* Status register snapshot */
    uint32_t pc;            /* PC at bank switch */
    uint32_t cause;         /* Cause register */
} bank_shadow_frame_t;

typedef struct {
    bank_shadow_frame_t stack[4];
    int depth;
    int overflow;           /* Overflow flag */
} bank_shadow_stack_t;
```

## 2. Four-Ring Protection Scheme

### Ring Architecture

The DLX four-ring protection scheme provides fine-grained privilege separation similar to x86, controlled via CP0 configuration.

```c
/* CP0 Register: Ring Configuration (CP0 reg 31, sel 0) */
typedef struct {
    uint32_t enable:1;          /* Bit 0: Enable 4-ring protection */
    uint32_t current_ring:2;    /* Bits 1-2: Current ring (0-3) */
    uint32_t max_user_ring:2;   /* Bits 3-4: Maximum user ring */
    uint32_t ring_crossings:1;  /* Bit 5: Allow non-adjacent transitions */
    uint32_t call_gate_enable:1; /* Bit 6: Enable call gates */
    uint32_t iopl:2;            /* Bits 7-8: I/O Privilege Level */
    uint32_t reserved:23;
} dlx_ring_config_t;

/* Ring Privilege Levels */
#define RING_0_KERNEL       0   /* Kernel/supervisor */
#define RING_1_DRIVER       1   /* Device drivers */
#define RING_2_SERVICE      2   /* System services */
#define RING_3_USER         3   /* User applications */
```

### Ring Privilege Rules

```c
/* Memory Access Rules */
typedef struct {
    int ring;

    /* Can access pages with ring >= this level */
    int can_read_ring[4];       /* Ring 0 can read all, Ring 3 only Ring 3 */
    int can_write_ring[4];      /* Ring 0 can write all */
    int can_execute_ring[4];    /* Code execution rules */
} ring_access_rules_t;

/* Example: Ring 1 (Device Driver) */
ring_access_rules_t ring1 = {
    .ring = 1,
    .can_read_ring = {1, 1, 1, 1},      /* Can read Ring 1-3 */
    .can_write_ring = {0, 1, 1, 1},     /* Can write Ring 1-3, not Ring 0 */
    .can_execute_ring = {0, 1, 1, 0},   /* Can execute Ring 1-2 code */
};
```

### Ring Transitions

#### Call Gates

Call gates provide controlled transitions between rings:

```c
/* Call Gate Descriptor */
typedef struct {
    uint32_t target_pc;         /* Target procedure address */
    uint16_t target_ring:2;     /* Destination ring (0-3) */
    uint16_t param_count:5;     /* Number of parameters to copy */
    uint16_t valid:1;           /* Gate is valid */
    uint16_t gate_type:2;       /* Call, interrupt, trap, task */
#define GATE_TYPE_CALL      0
#define GATE_TYPE_INTERRUPT 1
#define GATE_TYPE_TRAP      2
#define GATE_TYPE_TASK      3
    uint16_t reserved:6;
    uint32_t selector;          /* Segment/page selector */
} call_gate_t;

/* Call Gate Table (CGT) */
typedef struct {
    call_gate_t gates[256];     /* Up to 256 call gates */
    uint32_t base_addr;         /* CGT base address */
    uint16_t limit;             /* Number of valid gates */
} call_gate_table_t;
```

#### CALLGATE Instruction

```assembly
# CALLGATE - Call through gate (ring transition)
# Format: CALLGATE gate_num
#
# Hardware operation:
#   1. Validate gate number and permissions
#   2. Check current_ring <= gate.target_ring (can only go to higher privilege)
#   3. Save return ring and PC
#   4. Copy parameters to target stack
#   5. Switch to target ring
#   6. Jump to target_pc

CALLGATE 5                  # Call through gate 5

# Example: User (Ring 3) calling kernel service (Ring 0)
# User code:
    lw      r4, (r10)       # Load parameter
    callgate 10             # Call kernel through gate 10
    # Returns here after kernel service

# Kernel service (Ring 0):
kernel_service_10:
    # Automatically in Ring 0
    # Parameters copied from user stack
    ...
    retgate                 # Return through gate
```

#### RETGATE Instruction

```assembly
# RETGATE - Return through call gate
# Hardware operation:
#   1. Restore previous ring level
#   2. Restore previous PC
#   3. Clean up stack
#   4. Validate return is to lower privilege

RETGATE
```

### Ring-Based Page Protection

Page table entries extended with ring information:

```c
/* Page Table Entry with Ring Protection */
typedef struct {
    uint32_t pfn:20;            /* Physical frame number */
    uint32_t ring:2;            /* Required ring (0-3) */
    uint32_t ring_exact:1;      /* Exact match vs >= */
    uint32_t valid:1;
    uint32_t writable:1;
    uint32_t executable:1;
    uint32_t user:1;            /* Legacy user bit */
    uint32_t global:1;
    uint32_t dirty:1;
    uint32_t accessed:1;
    uint32_t reserved:2;
} pte_ring_t;

/* Access check */
int check_ring_access(pte_ring_t *pte, int current_ring, int access_type) {
    if (!pte->ring_exact) {
        /* Current ring must be >= page ring (more privileged) */
        if (current_ring > pte->ring)
            return ACCESS_DENIED;
    } else {
        /* Exact ring match required */
        if (current_ring != pte->ring)
            return ACCESS_DENIED;
    }

    /* Additional checks for write/execute */
    if (access_type == ACCESS_WRITE && !pte->writable)
        return ACCESS_DENIED;
    if (access_type == ACCESS_EXECUTE && !pte->executable)
        return ACCESS_DENIED;

    return ACCESS_GRANTED;
}
```

### Ring Crossing Detection

```c
/* CP0 Register: Ring Crossing Counter (CP0 reg 31, sel 1) */
typedef struct {
    uint32_t ring_0_to_1:8;     /* Transitions Ring 0 → Ring 1 */
    uint32_t ring_1_to_2:8;     /* Ring 1 → Ring 2 */
    uint32_t ring_2_to_3:8;     /* Ring 2 → Ring 3 */
    uint32_t ring_violations:8; /* Illegal transitions */
} dlx_ring_stats_t;

/* Ring violation causes exception */
#define EXCEPT_RING_VIOLATION   0x15
```

## 3. VMS-Style Advanced Page Protection

### OpenVMS Protection Model

OpenVMS uses a sophisticated 4-bit protection field per page with owner/group/world granularity. We extend this for DLX.

```c
/* Page Protection Bits (VMS-style) */
typedef struct {
    /* Owner protection (bits 0-3) */
    uint32_t owner_read:1;
    uint32_t owner_write:1;
    uint32_t owner_execute:1;
    uint32_t owner_delete:1;

    /* Group protection (bits 4-7) */
    uint32_t group_read:1;
    uint32_t group_write:1;
    uint32_t group_execute:1;
    uint32_t group_delete:1;

    /* World protection (bits 8-11) */
    uint32_t world_read:1;
    uint32_t world_write:1;
    uint32_t world_execute:1;
    uint32_t world_delete:1;

    /* System protection (bits 12-15) */
    uint32_t system_read:1;
    uint32_t system_write:1;
    uint32_t system_execute:1;
    uint32_t system_delete:1;

    /* Additional VMS features (bits 16-31) */
    uint32_t no_access_copy:1;      /* Cannot copy page */
    uint32_t copy_on_write:1;       /* COW bit */
    uint32_t demand_zero:1;         /* Zero-fill on access */
    uint32_t page_locked:1;         /* Cannot be paged out */
    uint32_t page_shared:1;         /* Shared memory */
    uint32_t page_cacheable:1;      /* Cache policy */
    uint32_t change_mode_handler:1; /* Has change-mode handler */
    uint32_t owner_uid:8;           /* Owner UID (0-255) */
    uint32_t group_gid:8;           /* Group GID (0-255) */
} vms_page_protection_t;

/* Protection codes (VMS-style octal) */
#define VMS_PROT_NONE   0x0     /* No access */
#define VMS_PROT_R      0x1     /* Read */
#define VMS_PROT_W      0x2     /* Write */
#define VMS_PROT_RW     0x3     /* Read/Write */
#define VMS_PROT_E      0x4     /* Execute */
#define VMS_PROT_RE     0x5     /* Read/Execute */
#define VMS_PROT_WE     0x6     /* Write/Execute */
#define VMS_PROT_RWE    0x7     /* Read/Write/Execute */
#define VMS_PROT_D      0x8     /* Delete */
#define VMS_PROT_RWED   0xF     /* Full access */
```

### Enhanced Page Table Entry

```c
/* Full PTE with VMS protection */
typedef struct {
    /* Physical address (bits 0-19) */
    uint32_t pfn:20;            /* Physical frame number */

    /* Basic flags (bits 20-27) */
    uint32_t valid:1;
    uint32_t global:1;
    uint32_t dirty:1;
    uint32_t accessed:1;
    uint32_t cache_policy:2;    /* UC, WB, WT, WC */
    uint32_t page_size:2;       /* 4K, 16K, 64K, 256K, etc. */

    /* Protection mode (bits 28-29) */
    uint32_t prot_mode:2;
#define PROT_MODE_SIMPLE    0   /* Simple R/W/X */
#define PROT_MODE_RING      1   /* Ring-based */
#define PROT_MODE_VMS       2   /* VMS-style */
#define PROT_MODE_EXTENDED  3   /* Full extended */

    /* Ring protection (bits 30-31, if PROT_MODE_RING) */
    uint32_t ring:2;

    /* VMS protection pointer (if PROT_MODE_VMS) */
    /* Points to separate protection descriptor */
    uint32_t vms_prot_index;

    /* Extended attributes */
    uint64_t extended_pte;      /* Additional 64-bit PTE data */
} dlx_pte_full_t;

/* Protection Descriptor Table (PDT) */
typedef struct {
    vms_page_protection_t prot[4096]; /* Protection descriptors */
    uint32_t base_addr;                /* PDT base address */
    uint16_t limit;                    /* Number of descriptors */
} protection_desc_table_t;
```

### Access Control Lists (ACL) for Pages

```c
/* Page ACL Entry */
typedef struct {
    uint32_t uid;               /* User ID */
    uint32_t gid;               /* Group ID */
    uint32_t rights:16;         /* Access rights bitmap */
#define ACL_READ        (1 << 0)
#define ACL_WRITE       (1 << 1)
#define ACL_EXECUTE     (1 << 2)
#define ACL_DELETE      (1 << 3)
#define ACL_MODIFY_ACL  (1 << 4)
#define ACL_CHANGE_PROT (1 << 5)
#define ACL_LOCK        (1 << 6)
#define ACL_UNLOCK      (1 << 7)
    uint32_t deny:1;            /* Deny entry (override) */
    uint32_t inherited:1;       /* Inherited from parent */
    uint32_t reserved:14;
} page_acl_entry_t;

/* Page ACL */
typedef struct {
    page_acl_entry_t entries[16]; /* Up to 16 ACL entries */
    int num_entries;
    uint32_t owner_uid;
    uint32_t owner_gid;
    int acl_enabled;
} page_acl_t;
```

### VMS-Style Protection Check

```c
/* Check VMS protection */
int check_vms_protection(dlx_pte_full_t *pte, int access_type,
                        uint32_t uid, uint32_t gid) {
    vms_page_protection_t *prot;

    /* Get protection descriptor */
    if (pte->prot_mode != PROT_MODE_VMS)
        return check_simple_protection(pte, access_type);

    prot = &pdt.prot[pte->vms_prot_index];

    /* Check system access first (highest priority) */
    if (current_ring == RING_0_KERNEL) {
        if (access_type == ACCESS_READ && prot->system_read)
            return ACCESS_GRANTED;
        if (access_type == ACCESS_WRITE && prot->system_write)
            return ACCESS_GRANTED;
        if (access_type == ACCESS_EXECUTE && prot->system_execute)
            return ACCESS_GRANTED;
        if (access_type == ACCESS_DELETE && prot->system_delete)
            return ACCESS_GRANTED;
    }

    /* Check owner access */
    if (uid == prot->owner_uid) {
        if (access_type == ACCESS_READ && prot->owner_read)
            return ACCESS_GRANTED;
        if (access_type == ACCESS_WRITE && prot->owner_write)
            return ACCESS_GRANTED;
        if (access_type == ACCESS_EXECUTE && prot->owner_execute)
            return ACCESS_GRANTED;
        if (access_type == ACCESS_DELETE && prot->owner_delete)
            return ACCESS_GRANTED;
    }

    /* Check group access */
    if (gid == prot->group_gid) {
        if (access_type == ACCESS_READ && prot->group_read)
            return ACCESS_GRANTED;
        if (access_type == ACCESS_WRITE && prot->group_write)
            return ACCESS_GRANTED;
        if (access_type == ACCESS_EXECUTE && prot->group_execute)
            return ACCESS_GRANTED;
        if (access_type == ACCESS_DELETE && prot->group_delete)
            return ACCESS_GRANTED;
    }

    /* Check world access */
    if (access_type == ACCESS_READ && prot->world_read)
        return ACCESS_GRANTED;
    if (access_type == ACCESS_WRITE && prot->world_write)
        return ACCESS_GRANTED;
    if (access_type == ACCESS_EXECUTE && prot->world_execute)
        return ACCESS_GRANTED;
    if (access_type == ACCESS_DELETE && prot->world_delete)
        return ACCESS_GRANTED;

    /* Check ACL if enabled */
    if (pte->extended_pte & PTE_EXT_ACL_ENABLE) {
        return check_page_acl(pte, access_type, uid, gid);
    }

    return ACCESS_DENIED;
}
```

### Change Mode Handlers

VMS-style change mode handlers allow secure transitions:

```c
/* Change Mode Descriptor */
typedef struct {
    uint32_t handler_pc;        /* Handler entry point */
    uint32_t target_mode:2;     /* Target privilege mode */
    uint32_t arg_count:5;       /* Number of arguments */
    uint32_t valid:1;
    uint32_t gate_type:2;
    uint32_t reserved:22;
} change_mode_desc_t;

/* CHME - Change Mode to Executive (Ring 1) */
/* CHMK - Change Mode to Kernel (Ring 0) */
/* CHMS - Change Mode to Supervisor (Ring 2) */
/* CHMU - Change Mode to User (Ring 3) */

asm("chme $5");     /* Change to executive mode via gate 5 */
asm("chmk $10");    /* Change to kernel mode via gate 10 */
```

## 4. Multiple Page Size Support

### Supported Page Sizes

DLX supports multiple page sizes for performance and flexibility:

```c
/* Page Size Configuration */
typedef enum {
    PAGE_SIZE_4KB    = 0,   /* 12-bit offset: 4,096 bytes */
    PAGE_SIZE_16KB   = 1,   /* 14-bit offset: 16,384 bytes */
    PAGE_SIZE_64KB   = 2,   /* 16-bit offset: 65,536 bytes */
    PAGE_SIZE_256KB  = 3,   /* 18-bit offset: 262,144 bytes */
    PAGE_SIZE_1MB    = 4,   /* 20-bit offset: 1,048,576 bytes */
    PAGE_SIZE_4MB    = 5,   /* 22-bit offset: 4,194,304 bytes */
    PAGE_SIZE_16MB   = 6,   /* 24-bit offset: 16,777,216 bytes */
    PAGE_SIZE_64MB   = 7,   /* 26-bit offset: 67,108,864 bytes */
    PAGE_SIZE_256MB  = 8,   /* 28-bit offset: 268,435,456 bytes */
    PAGE_SIZE_1GB    = 9,   /* 30-bit offset: 1,073,741,824 bytes */
    PAGE_SIZE_4GB    = 10,  /* 32-bit offset: 4,294,967,296 bytes (DLX64 only) */
    PAGE_SIZE_16GB   = 11,  /* 34-bit offset: 17,179,869,184 bytes (DLX64 only) */
} page_size_t;

/* CP0 Register: Page Size Configuration (CP0 reg 5, sel 2) */
typedef struct {
    uint32_t enable_multiple:1;     /* Enable multiple page sizes */
    uint32_t default_size:4;        /* Default page size */
    uint32_t supported_mask:12;     /* Bitmap of supported sizes */
    uint32_t min_size:4;            /* Minimum supported */
    uint32_t max_size:4;            /* Maximum supported */
    uint32_t alignment_check:1;     /* Enforce alignment */
    uint32_t reserved:6;
} dlx_pagesize_config_t;
```

### Multi-Level Page Table with Variable Sizes

```c
/* Variable-size page table hierarchy */
typedef struct {
    /* Level 0: Page Directory (1024 entries) */
    /* Each PDE can point to:
     *   - Level 1 page table (4KB pages)
     *   - Directly to 4MB superpage
     *   - Directly to 1GB huge page
     */
    uint32_t pde[1024];

    /* Level 1: Page Tables (1024 entries each) */
    /* Each PTE can point to:
     *   - 4KB page
     *   - 64KB large page
     *   - 256KB page
     */
    uint32_t pte[1024][1024];
} variable_page_table_t;

/* PDE/PTE format with page size */
typedef struct {
    uint32_t pfn:20;            /* Physical frame number */
    uint32_t page_size:4;       /* Page size selector */
    uint32_t present:1;
    uint32_t writable:1;
    uint32_t user:1;
    uint32_t pwt:1;             /* Page write-through */
    uint32_t pcd:1;             /* Page cache disable */
    uint32_t accessed:1;
    uint32_t dirty:1;
    uint32_t pat:1;             /* Page attribute table */
} pde_pte_variable_t;
```

### Page Size Translation

```c
/* Virtual address breakdown for different page sizes */

/* 4KB pages (standard) */
typedef struct {
    uint32_t offset:12;         /* Bits 0-11: Offset (4096) */
    uint32_t pte_index:10;      /* Bits 12-21: PTE index */
    uint32_t pde_index:10;      /* Bits 22-31: PDE index */
} va_4kb_t;

/* 4MB superpages */
typedef struct {
    uint32_t offset:22;         /* Bits 0-21: Offset (4MB) */
    uint32_t pde_index:10;      /* Bits 22-31: PDE index */
} va_4mb_t;

/* 1GB huge pages */
typedef struct {
    uint32_t offset:30;         /* Bits 0-29: Offset (1GB) */
    uint32_t pdpe_index:2;      /* Bits 30-31: PDPE index */
} va_1gb_t;

/* Translation function */
uint64_t translate_variable_page(uint32_t va, pde_pte_variable_t *pde) {
    int page_size = pde->page_size;
    int offset_bits = 12 + (page_size * 2);  /* Simplified */
    uint32_t offset_mask = (1 << offset_bits) - 1;

    uint32_t offset = va & offset_mask;
    uint64_t pfn = pde->pfn;

    /* Adjust PFN based on page size */
    pfn = pfn & ~((1 << (offset_bits - 12)) - 1);

    return (pfn << 12) | offset;
}
```

### TLB Support for Multiple Page Sizes

```c
/* TLB Entry with page size */
typedef struct {
    uint32_t vpn;               /* Virtual page number */
    uint32_t pfn:20;            /* Physical frame number */
    uint32_t page_size:4;       /* Page size code */
    uint32_t asid:8;            /* Address space ID */
    uint32_t valid:1;
    uint32_t global:1;
    uint32_t dirty:1;
    uint32_t writable:1;
    uint32_t user:1;
    uint32_t cached:1;

    /* Computed fields */
    uint32_t page_mask;         /* Mask for this page size */
    uint32_t offset_bits;       /* Number of offset bits */
} tlb_entry_variable_t;

/* TLB lookup with variable page sizes */
tlb_entry_variable_t *tlb_lookup_variable(uint32_t va, uint32_t asid) {
    for (int i = 0; i < TLB_ENTRIES; i++) {
        tlb_entry_variable_t *entry = &tlb[i];

        if (!entry->valid)
            continue;

        /* Calculate VPN based on page size */
        uint32_t vpn_mask = ~((1 << entry->offset_bits) - 1);
        uint32_t vpn = (va & vpn_mask) >> entry->offset_bits;

        if (entry->vpn == vpn &&
            (entry->global || entry->asid == asid)) {
            return entry;
        }
    }

    return NULL;  /* TLB miss */
}
```

### Page Size Selection Policy

```c
/* Automatic page size selection */
typedef struct {
    /* Size thresholds for promotion */
    uint64_t promote_to_64kb;   /* Promote if ≥ 64KB contiguous */
    uint64_t promote_to_1mb;    /* Promote if ≥ 1MB contiguous */
    uint64_t promote_to_4mb;    /* Promote if ≥ 4MB contiguous */
    uint64_t promote_to_1gb;    /* Promote if ≥ 1GB contiguous */

    /* Usage statistics */
    struct {
        uint64_t pages_4kb;
        uint64_t pages_64kb;
        uint64_t pages_1mb;
        uint64_t pages_4mb;
        uint64_t pages_1gb;
        uint64_t pages_16gb;
    } stats;

    /* Fragmentation tracking */
    int fragmentation_4kb;      /* % fragmentation at 4KB */
    int fragmentation_large;    /* % fragmentation for large pages */
} page_size_policy_t;

/* Transparent huge pages (THP) */
int try_promote_to_huge_page(uint32_t va_start, uint32_t length) {
    /* Check if region is contiguous and aligned */
    if (length >= (1 << 30) && IS_ALIGNED(va_start, 1 << 30)) {
        /* Promote to 1GB page */
        return promote_page_size(va_start, PAGE_SIZE_1GB);
    } else if (length >= (1 << 22) && IS_ALIGNED(va_start, 1 << 22)) {
        /* Promote to 4MB page */
        return promote_page_size(va_start, PAGE_SIZE_4MB);
    } else if (length >= (1 << 16) && IS_ALIGNED(va_start, 1 << 16)) {
        /* Promote to 64KB page */
        return promote_page_size(va_start, PAGE_SIZE_64KB);
    }

    return 0;  /* Keep as 4KB pages */
}
```

### Huge Page Support

```c
/* Huge Page Allocation */
typedef struct {
    void *base_addr;
    uint64_t size;              /* In bytes */
    page_size_t page_size;
    int num_pages;

    /* Huge page flags */
    int transparent;            /* THP - transparent to app */
    int explicit;               /* Explicitly requested */
    int locked;                 /* Cannot be split */

    /* NUMA */
    int numa_node;
    int numa_policy;
} huge_page_region_t;

/* System calls for huge pages */
void *mmap_hugepage(void *addr, size_t length, int prot, int flags) {
    /* Align to huge page boundary */
    if (length >= (1 << 30))
        return mmap_1gb_hugepage(addr, length, prot, flags);
    else if (length >= (1 << 21))
        return mmap_2mb_hugepage(addr, length, prot, flags);
    else
        return mmap_normal(addr, length, prot, flags);
}
```

## 5. Dual Page Tables for 4GB/4GB Split

### Motivation

On 32-bit systems, the address space is typically split between user and kernel (e.g., 3GB/1GB on x86-32). This limits either user processes (< 3GB) or kernel (< 1GB). **Dual page tables** allow a 4GB/4GB split: each user process gets a full 4GB address space, and the kernel has its own separate 4GB space.

This is inspired by the Linux x86 4G/4G patch and eliminates the need for continuous TLB flushing on kernel entry/exit.

### Dual Page Table Configuration

```c
/* CP0 Register: Dual Page Table Configuration (CP0 reg 5, sel 3) */
typedef struct {
    uint32_t enable:1;          /* Bit 0: Enable dual page tables */
    uint32_t auto_switch:1;     /* Bit 1: Auto-switch on privilege change */
    uint32_t current_table:1;   /* Bit 2: Current active table (0=user, 1=kernel) */
    uint32_t tlb_mode:2;        /* Bits 3-4: TLB handling mode */
#define TLB_MODE_SEPARATE       0   /* Separate TLBs (no flush) */
#define TLB_MODE_TAGGED         1   /* Tagged TLB (ASID-like) */
#define TLB_MODE_FLUSH          2   /* Flush on switch */
#define TLB_MODE_LAZY           3   /* Lazy flush */

    uint32_t reserved:27;
} dlx_dual_pgtable_config_t;
```

### Page Table Base Registers

```c
/* Two separate page table base registers */

/* CP0 Register: User Page Table Base (CP0 reg 2, sel 0) - existing */
/* This becomes the user-mode page table base */
uint32_t user_pgtable_base;     /* User mode page directory */

/* CP0 Register: Kernel Page Table Base (CP0 reg 2, sel 1) - NEW */
/* This is the kernel-mode page table base */
uint32_t kernel_pgtable_base;   /* Kernel mode page directory */

/* CP0 Register: Current Page Table Base (CP0 reg 2, sel 2) - Read-only */
/* Returns the currently active page table base */
uint32_t current_pgtable_base;  /* Active page table (auto-selected) */
```

### Address Space Layout

```
User Mode (4GB address space):
0x00000000 - 0xFFFFFFFF    User process virtual memory
                          (uses user_pgtable_base)

Kernel Mode (separate 4GB address space):
0x00000000 - 0x7FFFFFFF    Kernel direct-mapped memory
0x80000000 - 0xBFFFFFFF    Kernel vmalloc area
0xC0000000 - 0xFFFFFFFF    Kernel modules, device mappings
                          (uses kernel_pgtable_base)

No overlap - completely separate address spaces!
```

### Automatic Page Table Switching

```c
/* Hardware automatically switches page tables on privilege change */

void handle_syscall(void) {
    /* On syscall entry (user → kernel):
     * 1. Save current_pgtable_base → user_pgtable_base (implicit)
     * 2. current_pgtable_base = kernel_pgtable_base
     * 3. If TLB_MODE_SEPARATE: no TLB flush needed!
     * 4. If TLB_MODE_FLUSH: flush TLB
     */

    /* Kernel code executes with kernel page tables */
    do_syscall();

    /* On return (kernel → user):
     * 1. current_pgtable_base = user_pgtable_base
     * 2. Return to user space
     */
}
```

### TLB Handling Modes

#### Mode 0: Separate TLBs

```c
/* Separate TLB entries for user and kernel */
/* No flush needed on context switch */

typedef struct {
    uint32_t vpn;
    uint32_t pfn:20;
    uint32_t mode:1;        /* 0=user, 1=kernel */
    uint32_t valid:1;
    uint32_t writable:1;
    uint32_t user:1;
    /* ... */
} tlb_entry_dual_t;

/* TLB lookup considers current mode */
tlb_entry_dual_t *tlb_lookup_dual(uint32_t va) {
    int current_mode = (status & STATUS_KUC) ? 0 : 1;

    for (int i = 0; i < TLB_ENTRIES; i++) {
        if (tlb[i].valid &&
            tlb[i].vpn == (va >> 12) &&
            tlb[i].mode == current_mode) {
            return &tlb[i];
        }
    }

    return NULL;  /* TLB miss */
}
```

#### Mode 1: Tagged TLB (ASID-like)

```c
/* Use ASID to distinguish user/kernel entries */
/* ASID 0 = kernel, ASID 1-255 = user processes */

#define KERNEL_ASID  0

/* On privilege change, update EntryHi ASID */
void switch_to_kernel(void) {
    uint32_t entryhi;
    asm("mfc0 %0, $ENTRYHI" : "=r"(entryhi));
    entryhi = (entryhi & ~0xFF) | KERNEL_ASID;
    asm("mtc0 %0, $ENTRYHI" :: "r"(entryhi));

    /* TLB entries with ASID=0 (kernel) now match */
}
```

### Page Table Walk with Dual Tables

```c
uint64_t translate_dual_pgtable(uint32_t va, int access_type) {
    uint32_t *pgtable_base;
    int is_kernel_mode = !(status & STATUS_KUC);

    /* Select page table based on current mode */
    if (is_kernel_mode) {
        pgtable_base = (uint32_t *)kernel_pgtable_base;
    } else {
        pgtable_base = (uint32_t *)user_pgtable_base;
    }

    /* Walk selected page table */
    uint32_t pde_index = (va >> 22) & 0x3FF;
    uint32_t pte_index = (va >> 12) & 0x3FF;

    uint32_t *pde = &pgtable_base[pde_index];
    if (!(*pde & PTE_PRESENT))
        return page_fault(va, access_type);

    uint32_t *pte_table = (uint32_t *)(*pde & ~0xFFF);
    uint32_t *pte = &pte_table[pte_index];

    if (!(*pte & PTE_PRESENT))
        return page_fault(va, access_type);

    /* Check permissions */
    if (access_type == ACCESS_WRITE && !(*pte & PTE_WRITABLE))
        return page_fault(va, ACCESS_WRITE);

    /* Return physical address */
    return (*pte & ~0xFFF) | (va & 0xFFF);
}
```

### Benefits of Dual Page Tables

```c
/* Performance improvements */

// 1. No TLB flush on syscall/return
//    Before: ~100-200 cycles per syscall
//    After: ~10-20 cycles

// 2. Full 4GB user address space
//    User processes can use entire 32-bit address space

// 3. Full 4GB kernel address space
//    Kernel not limited to 1GB

// 4. Better cache utilization
//    TLB entries remain valid across privilege transitions

/* Example: Syscall overhead reduction */
void benchmark_syscall() {
    // Without dual page tables:
    // - TLB flush: ~150 cycles
    // - Refill TLB on return: ~50 cycles
    // Total overhead: ~200 cycles

    // With dual page tables (TLB_MODE_SEPARATE):
    // - Page table switch: ~5 cycles
    // - No TLB flush needed
    // Total overhead: ~5 cycles

    // **40x speedup!**
}
```

### Kernel Direct Mapping

```c
/* Kernel page table setup for direct-mapped region */
void setup_kernel_pgtable(void) {
    uint32_t *kernel_pgd = allocate_pgd();

    /* Direct-map first 2GB of physical memory to kernel VA 0-2GB */
    for (uint64_t pa = 0; pa < 0x80000000; pa += PAGE_SIZE_4MB) {
        uint32_t va = pa;  /* Identity mapping */
        map_kernel_page_4mb(kernel_pgd, va, pa,
                           PTE_PRESENT | PTE_WRITABLE | PTE_KERNEL);
    }

    /* Set kernel page table base */
    asm("mtc0 %0, $PGTABLE, 1" :: "r"(kernel_pgd));

    /* Enable dual page tables */
    uint32_t config;
    asm("mfc0 %0, $PGTABLE, 3" : "=r"(config));
    config |= (1 << 0) | (1 << 1);  /* enable | auto_switch */
    config |= (TLB_MODE_SEPARATE << 3);
    asm("mtc0 %0, $PGTABLE, 3" :: "r"(config));
}
```

### User-Kernel Data Sharing

```c
/* Special handling for shared data between user and kernel */

/* Option 1: Duplicate mappings */
void map_shared_region(void *kernel_va, void *user_va, uint64_t pa, size_t size) {
    /* Map in kernel page table */
    map_pages(kernel_pgtable_base, kernel_va, pa, size, KERNEL_FLAGS);

    /* Map same physical pages in user page table */
    map_pages(user_pgtable_base, user_va, pa, size, USER_FLAGS);
}

/* Option 2: Temporary mapping */
void *kernel_access_user_memory(void *user_va, size_t len) {
    /* Temporarily map user pages into kernel address space */
    uint64_t pa = translate_user_va(user_va);
    void *kernel_va = alloc_kernel_va(len);

    map_pages(kernel_pgtable_base, kernel_va, pa, len, KERNEL_FLAGS);
    return kernel_va;
}
```

### Context Switch with Dual Page Tables

```c
void context_switch(task_t *prev, task_t *next) {
    /* Save previous user page table base */
    prev->pgtable_base = user_pgtable_base;

    /* Load next user page table base */
    user_pgtable_base = next->pgtable_base;
    asm("mtc0 %0, $PGTABLE, 0" :: "r"(next->pgtable_base));

    /* Kernel page table remains the same! */
    /* No need to modify kernel_pgtable_base */

    /* TLB handling depends on mode */
    if (tlb_mode == TLB_MODE_FLUSH) {
        flush_tlb_user();  /* Flush only user TLB entries */
        /* Kernel TLB entries remain */
    } else if (tlb_mode == TLB_MODE_TAGGED) {
        /* Update ASID */
        set_asid(next->asid);
    }
    /* TLB_MODE_SEPARATE: no action needed */
}
```

### Copy-on-Write with Dual Tables

```c
/* COW is easier with dual page tables */
void handle_cow_fault(uint32_t va) {
    uint32_t *pte;

    /* Get PTE from correct page table */
    if (is_kernel_mode())
        pte = get_pte(kernel_pgtable_base, va);
    else
        pte = get_pte(user_pgtable_base, va);

    if (*pte & PTE_COW) {
        /* Allocate new page */
        uint64_t new_pa = alloc_page();

        /* Copy old page to new page */
        uint64_t old_pa = *pte & ~0xFFF;
        memcpy((void *)new_pa, (void *)old_pa, PAGE_SIZE);

        /* Update PTE */
        *pte = new_pa | PTE_PRESENT | PTE_WRITABLE;
        *pte &= ~PTE_COW;

        /* Flush TLB entry for this VA */
        flush_tlb_entry(va);
    }
}
```

### Compatibility Mode

```c
/* Disable dual page tables for compatibility */
void disable_dual_pgtables(void) {
    uint32_t config;

    /* Disable dual page table mode */
    asm("mfc0 %0, $PGTABLE, 3" : "=r"(config));
    config &= ~1;  /* Clear enable bit */
    asm("mtc0 %0, $PGTABLE, 3" :: "r"(config));

    /* Revert to traditional 3GB/1GB split */
    /* Single page table base (CP0 $2, sel 0) */
}
```

### Summary

**Dual Page Table Benefits:**
- ✓ Full 4GB user address space (not 3GB)
- ✓ Full 4GB kernel address space (not 1GB)
- ✓ No TLB flush on syscall/return (huge performance win)
- ✓ Better TLB utilization
- ✓ Simpler kernel memory management
- ✓ Compatible with register banking

**Trade-offs:**
- ✗ Slightly more complex page table management
- ✗ Requires additional CP0 register (kernel_pgtable_base)
- ✗ Potential memory overhead (two sets of page tables)

**Use Cases:**
- 32-bit systems needing > 3GB per process
- High-frequency syscall workloads
- Database servers
- Java VMs (large heap)
- Scientific computing

### Alternate Page Table Load/Store Instructions

DLX provides specialized load/store instructions that explicitly access memory using alternate page table contexts. When combined with 4-ring protection, this becomes four separate page table registers.

#### Base Instructions (Dual Page Tables)

```assembly
# Load from alternate page table
LDALT   rd, (rs)            # Load word using alternate page table
LHALT   rd, (rs)            # Load halfword using alternate page table
LBALT   rd, (rs)            # Load byte using alternate page table

# Store to alternate page table
STALT   rt, (rs)            # Store word using alternate page table
SHALT   rt, (rs)            # Store halfword using alternate page table
SBALT   rt, (rs)            # Store byte using alternate page table

# If current mode is kernel: uses user_pgtable_base
# If current mode is user: uses kernel_pgtable_base
```

#### Example: Kernel accessing user memory

```c
/* Safe kernel access to user memory */
int copy_from_user(void *kernel_dst, const void *user_src, size_t len) {
    uint8_t *kdst = kernel_dst;
    const uint8_t *usrc = user_src;

    /* Verify user pointer is valid */
    if (!access_ok(VERIFY_READ, user_src, len))
        return -EFAULT;

    /* Use LDALT to load from user space using user page table */
    for (size_t i = 0; i < len; i++) {
        asm volatile("ldalt %0, (%1)"
                    : "=r"(kdst[i])
                    : "r"(&usrc[i]));
    }

    return 0;
}

int copy_to_user(void *user_dst, const void *kernel_src, size_t len) {
    uint8_t *udst = user_dst;
    const uint8_t *ksrc = kernel_src;

    if (!access_ok(VERIFY_WRITE, user_dst, len))
        return -EFAULT;

    /* Use STALT to store to user space using user page table */
    for (size_t i = 0; i < len; i++) {
        asm volatile("stalt %0, (%1)"
                    :: "r"(ksrc[i]), "r"(&udst[i]));
    }

    return 0;
}
```

#### Four-Ring Extended Mode

When 4-ring protection is enabled, the alternate load/store instructions can access any of the four ring page tables:

```assembly
# Extended format with ring selector
LDRING  rd, (rs), ring      # Load from specified ring's page table
STORING rt, (rs), ring      # Store to specified ring's page table

# ring: 0-3 (Ring 0 = kernel, Ring 3 = user)

# Examples:
LDRING  r1, (r2), 0         # Load using Ring 0 (kernel) page table
LDRING  r3, (r4), 3         # Load using Ring 3 (user) page table
STORING r5, (r6), 1         # Store using Ring 1 (driver) page table
```

#### Page Table Registers (4-Ring Mode)

```c
/* CP0 Page Table Base Registers (when 4-ring enabled) */

/* CP0 reg 2, sel 0: Ring 0 (Kernel) Page Table */
uint32_t ring0_pgtable_base;

/* CP0 reg 2, sel 1: Ring 1 (Driver) Page Table */
uint32_t ring1_pgtable_base;

/* CP0 reg 2, sel 2: Ring 2 (Service) Page Table */
uint32_t ring2_pgtable_base;

/* CP0 reg 2, sel 3: Ring 3 (User) Page Table */
uint32_t ring3_pgtable_base;

/* CP0 reg 2, sel 4: Current Page Table Base (read-only) */
/* Returns the active page table for current ring */
uint32_t current_pgtable_base;
```

#### Automatic Page Table Selection

```c
/* Hardware selects page table based on current ring */
uint32_t *get_current_pgtable(void) {
    uint32_t ring_config;
    asm("mfc0 %0, $RING, 0" : "=r"(ring_config));

    int current_ring = (ring_config >> 1) & 0x3;

    switch (current_ring) {
    case RING_0_KERNEL:  return (uint32_t *)ring0_pgtable_base;
    case RING_1_DRIVER:  return (uint32_t *)ring1_pgtable_base;
    case RING_2_SERVICE: return (uint32_t *)ring2_pgtable_base;
    case RING_3_USER:    return (uint32_t *)ring3_pgtable_base;
    }
}
```

#### Cross-Ring Memory Access

```c
/* Driver (Ring 1) accessing kernel (Ring 0) memory */
void driver_call_kernel(void *kernel_func, void *args) {
    /* Ring 1 code */

    /* Read kernel function pointer using Ring 0 page table */
    void *func_ptr;
    asm volatile("ldring %0, (%1), 0"  /* ring 0 */
                : "=r"(func_ptr)
                : "r"(kernel_func));

    /* Call kernel function */
    ((void (*)(void *))func_ptr)(args);
}

/* Service (Ring 2) accessing user (Ring 3) memory */
int service_read_user_data(void *user_buf, size_t len) {
    uint8_t buffer[256];

    /* Ring 2 code */
    /* Read from user space using Ring 3 page table */
    for (size_t i = 0; i < len && i < sizeof(buffer); i++) {
        asm volatile("ldring %0, (%1), 3"  /* ring 3 */
                    : "=r"(buffer[i])
                    : "r"((uint8_t *)user_buf + i));
    }

    return process_data(buffer, len);
}
```

#### Privilege Checking

```c
/* Hardware enforces ring access rules */
int check_cross_ring_access(int current_ring, int target_ring, int access_type) {
    /* Rule 1: Can only access equal or less privileged rings */
    if (current_ring > target_ring) {
        /* Cannot access more privileged ring's page table */
        generate_exception(EXCEPTION_PRIVILEGE_VIOLATION);
        return -EPERM;
    }

    /* Rule 2: Ring 0 can access all */
    if (current_ring == RING_0_KERNEL)
        return 0;  /* Allowed */

    /* Rule 3: Ring 1-3 can access same or lower privilege */
    if (target_ring >= current_ring)
        return 0;  /* Allowed */

    return -EPERM;
}
```

#### Implementation Examples

```c
/* Example 1: Debugger (Ring 0) reading any ring */
void debugger_read_memory(int target_ring, void *addr, void *buf, size_t len) {
    /* Ring 0 privilege - can access any ring */
    for (size_t i = 0; i < len; i++) {
        asm volatile("ldring %0, (%1), %2"
                    : "=r"(((uint8_t *)buf)[i])
                    : "r"((uint8_t *)addr + i), "i"(target_ring));
    }
}

/* Example 2: System call from user to kernel */
void syscall_handler(int syscall_num, void *user_args) {
    /* Now in Ring 0 (kernel) */

    /* Read arguments from user space (Ring 3) */
    struct syscall_args args;
    for (size_t i = 0; i < sizeof(args); i++) {
        asm volatile("ldring %0, (%1), 3"
                    : "=r"(((uint8_t *)&args)[i])
                    : "r"((uint8_t *)user_args + i));
    }

    /* Process syscall */
    int result = do_syscall(syscall_num, &args);

    /* Write result back to user space (Ring 3) */
    asm volatile("storing %0, (%1), 3"
                :: "r"(result), "r"(user_args));
}

/* Example 3: Device driver (Ring 1) DMA */
void driver_dma_to_user(void *user_buf, void *dma_buf, size_t len) {
    /* Ring 1 code */

    /* DMA completed, copy to user buffer using Ring 3 page table */
    for (size_t i = 0; i < len; i++) {
        uint8_t byte = ((uint8_t *)dma_buf)[i];
        asm volatile("storing %0, (%1), 3"
                    :: "r"(byte), "r"((uint8_t *)user_buf + i));
    }
}
```

#### TLB Tagging for Multi-Ring

```c
/* TLB entries tagged with ring number */
typedef struct {
    uint32_t vpn;
    uint32_t pfn:20;
    uint32_t ring:2;        /* Source ring (0-3) */
    uint32_t asid:8;        /* Address space ID */
    uint32_t valid:1;
    uint32_t writable:1;
    /* ... */
} tlb_entry_multi_ring_t;

/* TLB lookup with ring consideration */
tlb_entry_multi_ring_t *tlb_lookup_ring(uint32_t va, int ring) {
    for (int i = 0; i < TLB_ENTRIES; i++) {
        if (tlb[i].valid &&
            tlb[i].vpn == (va >> 12) &&
            tlb[i].ring == ring) {
            return &tlb[i];
        }
    }

    return NULL;
}
```

#### Performance Considerations

```c
/* Alternate page table instructions are slightly slower */

// Regular load: 1 cycle (L1 cache hit)
LD   r1, (r2)

// Alternate page table load: 2-3 cycles
// - 1 cycle: Select alternate page table base
// - 1 cycle: TLB lookup (may miss if different ASID)
// - 1 cycle: Load data
LDALT r1, (r2)

// Cross-ring load (4-ring mode): 3-4 cycles
// - 1 cycle: Select ring's page table base
// - 1 cycle: Privilege check
// - 1 cycle: TLB lookup
// - 1 cycle: Load data
LDRING r1, (r2), 3

/* Use regular loads when possible */
/* Use alternate loads only when necessary */
```

#### Atomic Operations Across Rings

```assembly
# Atomic compare-and-swap across rings
CASRING rd, rs, rt, (ra), ring
# Atomically:
#   temp = MEM[ra] (using ring's page table)
#   if (temp == rs) MEM[ra] = rt
#   rd = temp

# Fetch-and-add across rings
FAADDRING rd, rs, (rt), ring
# Atomically:
#   temp = MEM[rt] (using ring's page table)
#   MEM[rt] = temp + rs
#   rd = temp
```

#### Security Implications

```c
/* Alternate page table instructions require careful use */

/* GOOD: Kernel validating user pointer before access */
int safe_copy_from_user(void *kdst, void *usrc, size_t len) {
    /* Validate user address range */
    if (!is_user_address(usrc, len))
        return -EFAULT;

    /* Use LDALT to safely read user memory */
    for (size_t i = 0; i < len; i++) {
        asm("ldalt %0, (%1)" : "=r"(((uint8_t *)kdst)[i])
                            : "r"((uint8_t *)usrc + i));
    }

    return 0;
}

/* BAD: Driver directly accessing kernel memory (security violation) */
void bad_driver_code(void *kernel_secret) {
    /* Ring 1 trying to read Ring 0 memory */
    uint32_t secret;
    asm("ldring %0, (%1), 0" : "=r"(secret) : "r"(kernel_secret));
    /* This will FAIL - Ring 1 cannot access Ring 0! */
    /* Hardware generates EXCEPTION_PRIVILEGE_VIOLATION */
}
```

## 6. Configuration and Control

### CP0 Register Summary

```c
/* Memory Protection Configuration Registers */

/* CP0 $30, sel 0: Bank Configuration */
BANKCONFIG  = mfc0(30, 0);
mtc0(30, 0, new_config);

/* CP0 $30, sel 1: Bank Status */
BANKSTATUS  = mfc0(30, 1);

/* CP0 $31, sel 0: Ring Configuration */
RINGCONFIG  = mfc0(31, 0);
mtc0(31, 0, ring_enable | (iopl << 7));

/* CP0 $31, sel 1: Ring Statistics */
RINGSTATS   = mfc0(31, 1);

/* CP0 $5, sel 2: Page Size Configuration */
PAGESIZECONFIG = mfc0(5, 2);

/* CP0 $16, sel 5: Protection Mode */
PROTMODE    = mfc0(16, 5);
mtc0(16, 5, PROT_MODE_VMS);
```

### Feature Detection

```c
/* CPUID-style feature detection */
void detect_memory_features(void) {
    uint32_t features = cpuid(CPUID_MEMORY_FEATURES);

    if (features & MEM_FEAT_REG_BANKING)
        printf("Register banking: supported\n");

    if (features & MEM_FEAT_4RING)
        printf("4-ring protection: supported\n");

    if (features & MEM_FEAT_VMS_PROT)
        printf("VMS protection: supported\n");

    if (features & MEM_FEAT_MULTI_PAGESIZE)
        printf("Multiple page sizes: supported\n");

    /* Query supported page sizes */
    uint32_t page_sizes = cpuid(CPUID_PAGE_SIZES);
    if (page_sizes & (1 << PAGE_SIZE_1GB))
        printf("1GB huge pages: supported\n");
}

/* Feature bits */
#define MEM_FEAT_REG_BANKING    (1 << 0)
#define MEM_FEAT_4RING          (1 << 1)
#define MEM_FEAT_VMS_PROT       (1 << 2)
#define MEM_FEAT_MULTI_PAGESIZE (1 << 3)
#define MEM_FEAT_CALL_GATES     (1 << 4)
#define MEM_FEAT_PAGE_ACL       (1 << 5)
#define MEM_FEAT_HUGE_PAGES     (1 << 6)
#define MEM_FEAT_THP            (1 << 7)
```

## 6. Use Cases and Examples

### Example 1: VMS-Style Operating System

```c
/* Initialize VMS protection for page */
void vms_init_page(uint32_t va, uint32_t pa, uint32_t owner_uid) {
    dlx_pte_full_t *pte = get_pte(va);

    /* Set VMS protection mode */
    pte->prot_mode = PROT_MODE_VMS;
    pte->pfn = pa >> 12;
    pte->valid = 1;

    /* Allocate protection descriptor */
    int prot_index = alloc_prot_descriptor();
    pte->vms_prot_index = prot_index;

    /* Set VMS protection (owner=RWED, group=RE, world=R) */
    vms_page_protection_t *prot = &pdt.prot[prot_index];
    prot->owner_uid = owner_uid;
    prot->owner_read = 1;
    prot->owner_write = 1;
    prot->owner_execute = 1;
    prot->owner_delete = 1;

    prot->group_read = 1;
    prot->group_execute = 1;

    prot->world_read = 1;

    prot->system_read = 1;
    prot->system_write = 1;
    prot->system_execute = 1;
    prot->system_delete = 1;
}
```

### Example 2: Microkernel with Ring Protection

```c
/* Ring 0: Microkernel */
void microkernel_init(void) {
    /* Enable 4-ring protection */
    uint32_t ring_config = (1 << 0) |      /* Enable */
                          (0 << 1) |      /* Current ring = 0 */
                          (1 << 6);       /* Enable call gates */
    mtc0(31, 0, ring_config);

    /* Setup call gates for ring transitions */
    setup_call_gate(0, syscall_handler, RING_0_KERNEL);
    setup_call_gate(1, driver_entry, RING_1_DRIVER);
}

/* Ring 1: Device Driver */
void driver_init(void) {
    /* Running in Ring 1 */
    /* Can access Ring 1-3 memory */
    /* Need call gate to access Ring 0 */
}

/* Ring 3: User Application */
void user_app(void) {
    /* Call into kernel via call gate */
    asm("callgate 0");
}
```

### Example 3: Database with Huge Pages

```c
/* Allocate 1GB huge page for database buffer pool */
void init_database_buffer_pool(void) {
    /* Enable multiple page sizes */
    uint32_t ps_config = (1 << 0) |                    /* Enable */
                        (PAGE_SIZE_1GB << 8) |        /* Support 1GB */
                        ((1 << PAGE_SIZE_1GB) << 4);  /* Mask */
    mtc0(5, 2, ps_config);

    /* Allocate 16GB of 1GB huge pages for buffer pool */
    void *buffer = mmap_hugepage(NULL, 16ULL << 30,
                                PROT_READ | PROT_WRITE,
                                MAP_ANONYMOUS | MAP_HUGETLB);

    /* Only 16 TLB entries needed for 16GB! */
    /* vs 4,194,304 entries with 4KB pages */
}
```

---

**Document Status**: Specification Complete
**Implementation Status**: Not Started
**Target**: DLX Architecture Extensions
**Complexity**: Very High - core architectural changes
**Estimated LOC**: ~15,000 lines (kernel/simulator changes)
**Key Features**: Register banking, 4-ring protection, VMS compatibility, huge pages
**OS Compatibility**: Enables porting of VMS, microkernel architectures, modern Linux
