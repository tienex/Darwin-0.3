# DLX CHERI Extension - Capability Hardware Enhanced RISC Instructions

## Overview

This document specifies the CHERI (Capability Hardware Enhanced RISC Instructions) extension for DLX, providing hardware-enforced memory safety and compartmentalization. CHERI extends the DLX architecture with **capabilities** - unforgeable tokens of authority that govern memory access and control flow.

CHERI enables:
- **Spatial memory safety**: Bounds checking on every memory access
- **Temporal memory safety**: Use-after-free protection via capability revocation
- **Compartmentalization**: Fine-grained isolation between software components
- **Control-flow integrity**: Protected code pointers
- **Principle of least privilege**: Minimal authority for each software component

This specification is based on CHERI ISAv9 with DLX-specific adaptations.

## CHERI Capability Format

### 128-bit Compressed Capabilities

```c
/* CHERI-128 capability format (compressed bounds) */
typedef struct {
    /* Word 0: Metadata */
    uint64_t address    : 64;       /* Virtual address */

    /* Word 1: Permissions and bounds */
    uint64_t perms      : 16;       /* Permissions */
    uint64_t otype      : 18;       /* Object type */
    uint64_t bounds     : 27;       /* Compressed bounds */
    uint64_t tag        : 1;        /* Capability valid bit */
    uint64_t reserved   : 2;
} cap128_t;

/* Total: 128 bits + 1 tag bit (stored separately) */
```

### 256-bit Full Capabilities

```c
/* CHERI-256 capability format (full precision) */
typedef struct {
    uint64_t address;               /* Virtual address (64-bit) */
    uint64_t base;                  /* Base address (64-bit) */
    uint64_t length;                /* Length (64-bit) */
    uint32_t perms;                 /* Permissions (32-bit) */
    uint32_t otype;                 /* Object type (32-bit) */
    uint8_t  tag;                   /* Valid bit */
    uint8_t  sealed;                /* Sealed flag */
    uint16_t flags;                 /* Additional flags */
} cap256_t;

/* Total: 256 bits + metadata */
```

### Capability Permissions

```c
/* CHERI permission bits */
#define CAP_PERM_GLOBAL             0x0001  /* Global capability */
#define CAP_PERM_EXECUTE            0x0002  /* Execute code */
#define CAP_PERM_LOAD               0x0004  /* Load data */
#define CAP_PERM_STORE              0x0008  /* Store data */
#define CAP_PERM_LOAD_CAP           0x0010  /* Load capabilities */
#define CAP_PERM_STORE_CAP          0x0020  /* Store capabilities */
#define CAP_PERM_STORE_LOCAL        0x0040  /* Store local capabilities */
#define CAP_PERM_SEAL               0x0080  /* Seal capabilities */
#define CAP_PERM_INVOKE             0x0100  /* Domain crossing */
#define CAP_PERM_UNSEAL             0x0200  /* Unseal capabilities */
#define CAP_PERM_ACCESS_SYS_REGS    0x0400  /* Access system registers */
#define CAP_PERM_SETCID             0x0800  /* Set compartment ID */

/* Composite permissions */
#define CAP_PERM_RWX    (CAP_PERM_LOAD | CAP_PERM_STORE | CAP_PERM_EXECUTE)
#define CAP_PERM_RW     (CAP_PERM_LOAD | CAP_PERM_STORE)
#define CAP_PERM_RX     (CAP_PERM_LOAD | CAP_PERM_EXECUTE)
#define CAP_PERM_RO     (CAP_PERM_LOAD)
```

### Object Types (otype)

```c
/* Capability sealing */
#define CAP_OTYPE_UNSEALED      0x00000     /* Unsealed (usable) */
#define CAP_OTYPE_SENTRY        0x3FFFF     /* Sealed entry (special) */
#define CAP_OTYPE_RES0          0x3FFFE     /* Reserved */

/* User-defined types: 0x00001 - 0x3FFFD */
#define CAP_OTYPE_USER_BASE     0x00001
#define CAP_OTYPE_USER_MAX      0x3FFFD

/* Examples of object types */
#define CAP_OTYPE_FILE_DESC     0x00100     /* File descriptor */
#define CAP_OTYPE_SOCKET        0x00101     /* Network socket */
#define CAP_OTYPE_BUFFER        0x00102     /* Memory buffer */
#define CAP_OTYPE_FUNCTION      0x00103     /* Function pointer */
#define CAP_OTYPE_VTABLE        0x00104     /* Virtual table */
```

## CHERI Registers

### Capability Register File

```c
/* CHERI capability registers (32 × 128-bit or 256-bit) */

/* Special capability registers */
#define CAP_NULL        c0      /* Null capability (all zeros) */
#define CAP_DDC         c1      /* Default Data Capability */
#define CAP_PCC         c2      /* Program Counter Capability */
#define CAP_KCC         c3      /* Kernel Code Capability */
#define CAP_KDC         c4      /* Kernel Data Capability */
#define CAP_EPCC        c5      /* Exception PCC */
#define CAP_KR1C        c6      /* Kernel Reserved 1 */
#define CAP_KR2C        c7      /* Kernel Reserved 2 */

/* General capability registers */
#define CAP_C8_C15      /* Temporary capabilities */
#define CAP_C16_C23     /* Saved capabilities */
#define CAP_C24_C27     /* Temporary capabilities */
#define CAP_C28         /* Global capability (cgp) */
#define CAP_C29         /* Stack capability (csp) */
#define CAP_C30         /* Frame capability (cfp) */
#define CAP_C31         /* Return capability (cra) */

/* Mapping to integer registers */
/*
 * Capability mode: c-registers are primary
 * Hybrid mode: both c-registers and integer registers coexist
 * Legacy mode: only integer registers
 */
```

### Special Capability Registers (SCRs)

```c
/* System capability registers */
#define SCR_PCC         0       /* Program Counter Capability */
#define SCR_DDC         1       /* Default Data Capability */
#define SCR_UTCC        4       /* User TLS Capability */
#define SCR_UTDC        5       /* User TLS Data Capability */
#define SCR_KCC         29      /* Kernel Code Capability */
#define SCR_KDC         30      /* Kernel Data Capability */
#define SCR_EPCC        31      /* Exception Program Counter Capability */
```

### Capability Cause Register

```c
/* Capability exception causes */
#define CAP_CAUSE_NONE                  0x00
#define CAP_CAUSE_LENGTH_VIOLATION      0x01    /* Out of bounds */
#define CAP_CAUSE_TAG_VIOLATION         0x02    /* Tag bit not set */
#define CAP_CAUSE_SEAL_VIOLATION        0x03    /* Sealed capability used */
#define CAP_CAUSE_TYPE_VIOLATION        0x04    /* Wrong object type */
#define CAP_CAUSE_CALL_TRAP             0x05    /* CCall trap */
#define CAP_CAUSE_RETURN_TRAP           0x06    /* CReturn trap */
#define CAP_CAUSE_UNDERFLOW             0x07    /* Trust zone underflow */
#define CAP_CAUSE_USER_PERM_VIOLATION   0x08    /* User permission violation */
#define CAP_CAUSE_IMPRECISE_BASE        0x10    /* Imprecise exception */
#define CAP_CAUSE_GLOBAL_VIOLATION      0x18    /* Global violation */
#define CAP_CAUSE_PERMIT_EXECUTE        0x11    /* Execute permission */
#define CAP_CAUSE_PERMIT_LOAD           0x12    /* Load permission */
#define CAP_CAUSE_PERMIT_STORE          0x13    /* Store permission */
#define CAP_CAUSE_PERMIT_LOAD_CAP       0x14    /* Load cap permission */
#define CAP_CAUSE_PERMIT_STORE_CAP      0x15    /* Store cap permission */
#define CAP_CAUSE_PERMIT_STORE_LOCAL_CAP 0x16   /* Store local cap permission */
#define CAP_CAUSE_PERMIT_SEAL           0x17    /* Seal permission */
#define CAP_CAUSE_PERMIT_INVOKE         0x19    /* Invoke permission */
#define CAP_CAUSE_ACCESS_SYS_REGS       0x1a    /* Access sys regs permission */
#define CAP_CAUSE_PERMIT_UNSEAL         0x1b    /* Unseal permission */
#define CAP_CAUSE_PERMIT_SETCID         0x1c    /* SetCID permission */
```

## CHERI Instructions

### Capability Inspection

```assembly
# Get capability fields
CGETBASE    rd, cs          # rd = cs.base
CGETLEN     rd, cs          # rd = cs.length (top - base)
CGETPERM    rd, cs          # rd = cs.permissions
CGETTYPE    rd, cs          # rd = cs.otype
CGETTAG     rd, cs          # rd = cs.tag
CGETSEALED  rd, cs          # rd = cs.sealed
CGETADDR    rd, cs          # rd = cs.address
CGETOFFSET  rd, cs          # rd = cs.address - cs.base

# Get capability bounds
CGETTOP     rd, cs          # rd = cs.base + cs.length
CGETBOUNDS  rd_base, rd_top, cs  # Get both base and top

# Example: Check bounds
check_bounds:
    CGETLEN     t0, ca0         # t0 = length
    CGETOFFSET  t1, ca0         # t1 = offset
    BLTU        t1, t0, .ok     # if offset < length, OK
    J           .bounds_error
.ok:
```

### Capability Modification

```assembly
# Set capability address
CSETADDR    cd, cs, rt      # cd = cs with address = rt
CINCOFFSET  cd, cs, rt      # cd = cs with address += rt
CSETOFFSET  cd, cs, rt      # cd = cs with offset = rt

# Set capability bounds
CSETBOUNDS  cd, cs, rt      # cd = cs with length = rt
CSETBOUNDSEXACT cd, cs, rt  # Exact bounds (may fail)
CROUNDREPRESENTABLELENGTH rd, rs  # Round length to representable

# Clear permissions
CANDPERM    cd, cs, rt      # cd = cs with perms &= rt
CCLEARPERM  cd, cs, imm     # cd = cs with perms &= ~imm

# Seal/unseal
CSEAL       cd, cs, ct      # cd = seal(cs, ct.otype)
CUNSEAL     cd, cs, ct      # cd = unseal(cs, ct.otype)
CSEALENTRY  cd, cs          # cd = seal(cs, SENTRY)

# Build capability from address
CFROMPTR    cd, cs, rt      # cd = capability from address rt, DDC bounds
CTOPTR      rd, cs, ct      # rd = (cs - ct.base) if ct contains cs

# Example: Create bounded buffer capability
create_buffer:
    LI          t0, 1024        # Buffer size
    CMOVE       ca0, cddc       # Start with DDC
    CSETBOUNDS  ca0, ca0, t0    # Set bounds to 1024 bytes
    CANDPERM    ca0, ca0, CAP_PERM_RW  # Read/write only
    RET
```

### Capability Memory Access

```assembly
# Load via capability
CLB     rd, offset(cs)      # Load byte
CLH     rd, offset(cs)      # Load halfword
CLW     rd, offset(cs)      # Load word
CLD     rd, offset(cs)      # Load doubleword
CLBU    rd, offset(cs)      # Load byte unsigned
CLHU    rd, offset(cs)      # Load halfword unsigned
CLWU    rd, offset(cs)      # Load word unsigned

# Store via capability
CSB     rs, offset(ct)      # Store byte
CSH     rs, offset(ct)      # Store halfword
CSW     rs, offset(ct)      # Store word
CSD     rs, offset(ct)      # Store doubleword

# Load/store capabilities
CLC     cd, offset(cs)      # Load capability
CSC     cs, offset(ct)      # Store capability
CLCBI   cd, offset(cs)      # Load cap with capability index

# Example: Safe buffer access
safe_memcpy:
    # ca0 = dest capability
    # ca1 = src capability
    # a0 = size
    LI      t0, 0
.loop:
    BGE     t0, a0, .done
    CLB     t1, 0(ca1)          # Load byte via src cap (checked!)
    CSB     t1, 0(ca0)          # Store byte via dest cap (checked!)
    CINCOFFSET ca0, ca0, 1      # Advance dest
    CINCOFFSET ca1, ca1, 1      # Advance src
    ADDI    t0, t0, 1
    J       .loop
.done:
    RET
```

### Control Flow with Capabilities

```assembly
# Branch/Jump with capabilities
CJALR   cd, cs              # cd = return cap, jump to cs
CJR     cs                  # Jump to cs
CBNZ    rs, cs              # Branch to cs if rs != 0
CBEZ    rs, cs              # Branch to cs if rs == 0

# Special control flow
CCALL   cs, ct              # Cross-domain call (sealed caps)
CRETURN                     # Return from cross-domain call

# Example: Protected function call
call_function:
    # ca0 = function capability (sealed)
    # ca1 = data capability (sealed)
    CCALL   ca0, ca1            # Cross-domain call
    # Returns here after function completes
    RET

# Function entry (unsealed by CCALL)
function_entry:
    # ca0 and ca1 are now unsealed
    # Perform function work
    CRETURN                     # Return to caller
```

### Capability Building and Manipulation

```assembly
# Copy capabilities
CMOVE       cd, cs          # cd = cs
CCLEARTAG   cd, cs          # cd = cs with tag cleared

# Special capabilities
CSPECIALR   cd, scr         # cd = special_cap_reg[scr]
CSPECIALW   scr, cs         # special_cap_reg[scr] = cs

# Get PCC/DDC
CGETPCC     cd              # cd = PCC (current)
CGETDDC     cd              # cd = DDC

# Test capabilities
CTESTSUBSET rd, cs, ct      # rd = (cs subset of ct)

# Example: Check if capability is valid
is_valid_cap:
    CGETTAG     a0, ca0         # a0 = tag bit
    RET                         # Return tag value
```

## CHERI Memory Model

### Tagged Memory

```c
/*
 * CHERI requires 1 tag bit per capability-aligned word
 * For 128-bit capabilities: 1 bit per 16 bytes
 * For 256-bit capabilities: 1 bit per 32 bytes
 */

/* Tag storage (1 bit per 16 bytes = 1/129 overhead ~0.78%) */
static uint8_t *tag_memory;
static size_t tag_memory_size;

/* Get tag bit for address */
static inline int get_cap_tag(uintptr_t addr) {
    uintptr_t tag_index = addr >> 4;  /* Divide by 16 */
    uintptr_t byte_index = tag_index >> 3;
    uintptr_t bit_index = tag_index & 7;

    return (tag_memory[byte_index] >> bit_index) & 1;
}

/* Set tag bit */
static inline void set_cap_tag(uintptr_t addr, int tag) {
    uintptr_t tag_index = addr >> 4;
    uintptr_t byte_index = tag_index >> 3;
    uintptr_t bit_index = tag_index & 7;

    if (tag) {
        tag_memory[byte_index] |= (1 << bit_index);
    } else {
        tag_memory[byte_index] &= ~(1 << bit_index);
    }
}

/* Clear tags on memory write */
static inline void clear_tags_on_store(uintptr_t addr, size_t size) {
    /* Partial stores to capability-aligned words clear the tag */
    uintptr_t start = addr & ~15;
    uintptr_t end = (addr + size + 15) & ~15;

    for (uintptr_t a = start; a < end; a += 16) {
        if (get_cap_tag(a)) {
            /* Partial overwrite - clear tag */
            if (a < addr || (a + 16) > (addr + size)) {
                set_cap_tag(a, 0);
            }
        }
    }
}
```

### Capability Checking

```c
/*
 * Capability checks on every memory access
 */
static inline int check_cap_load(cap128_t *cap, uintptr_t addr, size_t size) {
    /* Check tag */
    if (!cap->tag) {
        raise_cap_exception(CAP_CAUSE_TAG_VIOLATION);
        return 0;
    }

    /* Check sealed */
    if (cap->otype != CAP_OTYPE_UNSEALED) {
        raise_cap_exception(CAP_CAUSE_SEAL_VIOLATION);
        return 0;
    }

    /* Check load permission */
    if (!(cap->perms & CAP_PERM_LOAD)) {
        raise_cap_exception(CAP_CAUSE_PERMIT_LOAD);
        return 0;
    }

    /* Check bounds */
    uint64_t base = get_cap_base(cap);
    uint64_t top = get_cap_top(cap);

    if (addr < base || (addr + size) > top) {
        raise_cap_exception(CAP_CAUSE_LENGTH_VIOLATION);
        return 0;
    }

    return 1;  /* OK */
}

static inline int check_cap_store(cap128_t *cap, uintptr_t addr, size_t size) {
    if (!cap->tag) {
        raise_cap_exception(CAP_CAUSE_TAG_VIOLATION);
        return 0;
    }

    if (cap->otype != CAP_OTYPE_UNSEALED) {
        raise_cap_exception(CAP_CAUSE_SEAL_VIOLATION);
        return 0;
    }

    if (!(cap->perms & CAP_PERM_STORE)) {
        raise_cap_exception(CAP_CAUSE_PERMIT_STORE);
        return 0;
    }

    uint64_t base = get_cap_base(cap);
    uint64_t top = get_cap_top(cap);

    if (addr < base || (addr + size) > top) {
        raise_cap_exception(CAP_CAUSE_LENGTH_VIOLATION);
        return 0;
    }

    /* Clear tags if partial store to capability */
    clear_tags_on_store(addr, size);

    return 1;
}

static inline int check_cap_execute(cap128_t *pcc, uintptr_t addr) {
    if (!pcc->tag) {
        raise_cap_exception(CAP_CAUSE_TAG_VIOLATION);
        return 0;
    }

    if (pcc->otype != CAP_OTYPE_UNSEALED && pcc->otype != CAP_OTYPE_SENTRY) {
        raise_cap_exception(CAP_CAUSE_SEAL_VIOLATION);
        return 0;
    }

    if (!(pcc->perms & CAP_PERM_EXECUTE)) {
        raise_cap_exception(CAP_CAUSE_PERMIT_EXECUTE);
        return 0;
    }

    uint64_t base = get_cap_base(pcc);
    uint64_t top = get_cap_top(pcc);

    if (addr < base || addr >= top) {
        raise_cap_exception(CAP_CAUSE_LENGTH_VIOLATION);
        return 0;
    }

    return 1;
}
```

## Capability Compression (CHERI-128)

### Compressed Bounds Representation

```c
/*
 * CHERI-128 uses compressed bounds encoding
 * Base and top are represented relative to address with exponent encoding
 */

/* Bounds compression format */
typedef struct {
    uint8_t exponent;       /* E: exponent (0-23) */
    uint32_t base_bits;     /* B: base bits (14-20 bits depending on E) */
    uint32_t top_bits;      /* T: top bits (14-20 bits) */
} compressed_bounds_t;

/* Decompress bounds */
static inline void decompress_bounds(cap128_t *cap,
                                     uint64_t *base_out,
                                     uint64_t *top_out) {
    uint64_t addr = cap->address;
    compressed_bounds_t cb = decode_bounds_field(cap->bounds);

    if (cb.exponent == 0) {
        /* Precise bounds */
        *base_out = addr - cb.base_bits;
        *top_out = addr + cb.top_bits;
    } else {
        /* Compressed bounds */
        uint64_t mask = (1ULL << cb.exponent) - 1;
        uint64_t alignment = 1ULL << cb.exponent;

        /* Align address */
        uint64_t aligned_addr = addr & ~mask;

        /* Reconstruct base and top */
        uint64_t base_approx = aligned_addr | (cb.base_bits << cb.exponent);
        uint64_t top_approx = aligned_addr | (cb.top_bits << cb.exponent);

        /* Adjust based on address position */
        if (base_approx > addr) {
            base_approx -= alignment;
        }
        if (top_approx <= addr) {
            top_approx += alignment;
        }

        *base_out = base_approx;
        *top_out = top_approx;
    }
}

/* Compress bounds */
static inline int compress_bounds(uint64_t base, uint64_t top,
                                  uint64_t address,
                                  compressed_bounds_t *cb_out) {
    uint64_t length = top - base;

    /* Find exponent needed */
    int exponent = 0;
    if (length > (1 << 14)) {
        exponent = 64 - __builtin_clzll(length - 1) - 14;
        if (exponent > 23) {
            return 0;  /* Unrepresentable */
        }
    }

    /* Encode base and top */
    uint64_t mask = (exponent == 0) ? 0 : ((1ULL << exponent) - 1);
    uint64_t aligned_base = base & ~mask;
    uint64_t aligned_top = top & ~mask;

    cb_out->exponent = exponent;
    cb_out->base_bits = (base >> exponent) & ((1 << 20) - 1);
    cb_out->top_bits = (top >> exponent) & ((1 << 20) - 1);

    return 1;  /* Success */
}
```

## Compartmentalization

### Domain Crossing with CCall/CReturn

```c
/*
 * Cross-domain calls use sealed capabilities
 */

/* Compartment structure */
struct compartment {
    cap256_t code_cap;          /* Code capability (sealed) */
    cap256_t data_cap;          /* Data capability (sealed) */
    uint32_t otype;             /* Object type for sealing */
};

/* Setup compartment */
void setup_compartment(struct compartment *comp,
                      void *code_base, size_t code_size,
                      void *data_base, size_t data_size) {
    /* Allocate unique object type */
    comp->otype = allocate_otype();

    /* Create code capability */
    cap256_t code_cap = {
        .address = (uint64_t)code_base,
        .base = (uint64_t)code_base,
        .length = code_size,
        .perms = CAP_PERM_LOAD | CAP_PERM_EXECUTE | CAP_PERM_INVOKE,
        .otype = CAP_OTYPE_UNSEALED,
        .tag = 1
    };

    /* Seal code capability */
    comp->code_cap = seal_cap(&code_cap, comp->otype);

    /* Create data capability */
    cap256_t data_cap = {
        .address = (uint64_t)data_base,
        .base = (uint64_t)data_base,
        .length = data_size,
        .perms = CAP_PERM_LOAD | CAP_PERM_STORE | CAP_PERM_LOAD_CAP | CAP_PERM_STORE_CAP,
        .otype = CAP_OTYPE_UNSEALED,
        .tag = 1
    };

    /* Seal data capability */
    comp->data_cap = seal_cap(&data_cap, comp->otype);
}

/* Perform cross-domain call */
void do_ccall(cap256_t *code_cap, cap256_t *data_cap) {
    /* Hardware (CCALL instruction) performs:
     * 1. Check both caps are sealed with same otype
     * 2. Check code cap has EXECUTE and INVOKE perms
     * 3. Unseal both capabilities
     * 4. Set PCC = unsealed code cap
     * 5. Set C1 (IDC) = unsealed data cap
     * 6. Save return information in special registers
     */

    if (!code_cap->tag || !data_cap->tag) {
        raise_exception(CAP_CAUSE_TAG_VIOLATION);
    }

    if (code_cap->otype != data_cap->otype) {
        raise_exception(CAP_CAUSE_TYPE_VIOLATION);
    }

    if (!(code_cap->perms & CAP_PERM_INVOKE)) {
        raise_exception(CAP_CAUSE_PERMIT_INVOKE);
    }

    /* Unseal and transfer control */
    cap256_t unsealed_code = *code_cap;
    cap256_t unsealed_data = *data_cap;
    unsealed_code.otype = CAP_OTYPE_UNSEALED;
    unsealed_data.otype = CAP_OTYPE_UNSEALED;

    /* Save return state */
    save_return_state();

    /* Switch to compartment */
    set_pcc(&unsealed_code);
    set_idc(&unsealed_data);
}
```

### Example: Protected Library Call

```c
/* Library exports a sealed capability pair */
struct library_entry {
    cap256_t entry_point;   /* Sealed code capability */
    cap256_t lib_data;      /* Sealed data capability */
};

/* Client code */
void call_library(struct library_entry *lib, int arg) {
    /* Setup arguments */
    set_register(a0, arg);

    /* Cross-domain call */
    asm volatile(
        "CLC    ca0, 0(%0)\n"       /* Load sealed code cap */
        "CLC    ca1, 16(%0)\n"      /* Load sealed data cap */
        "CCALL  ca0, ca1"           /* Cross-domain call */
        :: "r"(lib)
        : "ca0", "ca1", "memory"
    );
}

/* Library implementation */
__attribute__((cheri_ccall))
int library_function(int arg) {
    /* This function runs in library's compartment */
    /* Access to library's private data via IDC (ca1) */

    int result = process_arg(arg);

    /* Return via CRETURN */
    asm volatile("CRETURN");

    return result;  /* Actually unreachable */
}
```

## Integration with DLX Features

### CHERI + Register Banking

```c
/*
 * CHERI capabilities can be stored in banked registers
 * Each ring has its own capability register set
 */

struct dlx_cheri_bank {
    cap256_t caps[32];      /* 32 capability registers per bank */
    cap256_t pcc;           /* Program Counter Capability */
    cap256_t ddc;           /* Default Data Capability */
};

/* Banks for each ring */
struct dlx_cheri_bank cheri_bank[4];  /* 4 rings */

/* Ring transition automatically switches capability bank */
void ring_transition(int from_ring, int to_ring) {
    /* Save current bank */
    save_cheri_bank(&cheri_bank[from_ring]);

    /* Load new bank */
    restore_cheri_bank(&cheri_bank[to_ring]);

    /* Update PCC and DDC for new ring */
    set_pcc(&cheri_bank[to_ring].pcc);
    set_ddc(&cheri_bank[to_ring].ddc);
}
```

### CHERI + MMU

```c
/*
 * CHERI capabilities work alongside MMU
 * Page tables have additional capability metadata
 */

struct dlx_cheri_pte {
    /* Standard PTE fields */
    uint64_t pfn        : 40;
    uint64_t flags      : 10;
    uint64_t valid      : 1;

    /* CHERI extensions */
    uint64_t cap_load   : 1;    /* Allow loading capabilities */
    uint64_t cap_store  : 1;    /* Allow storing capabilities */
    uint64_t cap_exec   : 1;    /* Allow executing via capability */

    /* Capability bounds for page */
    uint64_t cap_base;          /* Base within page */
    uint64_t cap_top;           /* Top within page */
};

/* TLB with capability metadata */
void tlb_insert_cheri(uint64_t vaddr, uint64_t paddr,
                     uint64_t flags, uint64_t cap_base, uint64_t cap_top) {
    struct dlx_tlb_entry *entry = &tlb[get_tlb_index(vaddr)];

    entry->vaddr = vaddr & PAGE_MASK;
    entry->paddr = paddr & PAGE_MASK;
    entry->flags = flags;
    entry->cap_base = cap_base;
    entry->cap_top = cap_top;
    entry->valid = 1;
}
```

### CHERI + PAC (Pointer Authentication)

```c
/*
 * CHERI capabilities can incorporate PAC for additional security
 */

/* Capability with embedded PAC */
typedef struct {
    uint64_t address    : 56;   /* Address (reduced from 64) */
    uint64_t pac        : 8;    /* PAC in high bits */
    /* ... rest of capability ... */
} cap_with_pac_t;

/* Authenticate capability */
static inline int authenticate_cap(cap_with_pac_t *cap, uint64_t context) {
    uint8_t computed_pac = compute_pac(cap->address, context);

    if (computed_pac != cap->pac) {
        raise_exception(CAP_CAUSE_TAG_VIOLATION);  /* PAC failure */
        return 0;
    }

    return 1;
}
```

## CHERI Operating System Support

### Process Isolation

```c
/*
 * Each process has its own capability space
 */

struct cheri_process {
    cap256_t pcc;               /* Program code capability */
    cap256_t ddc;               /* Default data capability */
    cap256_t stack_cap;         /* Stack capability */
    cap256_t heap_cap;          /* Heap capability */
    cap256_t *cap_table;        /* Capability table */
    size_t cap_table_size;
};

/* Initialize process capability space */
void init_cheri_process(struct cheri_process *proc,
                       void *code_base, size_t code_size,
                       void *data_base, size_t data_size,
                       void *stack_base, size_t stack_size) {
    /* Code capability (RX) */
    proc->pcc = (cap256_t){
        .address = (uint64_t)code_base,
        .base = (uint64_t)code_base,
        .length = code_size,
        .perms = CAP_PERM_LOAD | CAP_PERM_EXECUTE | CAP_PERM_GLOBAL,
        .otype = CAP_OTYPE_UNSEALED,
        .tag = 1
    };

    /* Data capability (RW) */
    proc->ddc = (cap256_t){
        .address = (uint64_t)data_base,
        .base = (uint64_t)data_base,
        .length = data_size,
        .perms = CAP_PERM_LOAD | CAP_PERM_STORE | CAP_PERM_LOAD_CAP |
                 CAP_PERM_STORE_CAP | CAP_PERM_GLOBAL,
        .otype = CAP_OTYPE_UNSEALED,
        .tag = 1
    };

    /* Stack capability (RW) */
    proc->stack_cap = (cap256_t){
        .address = (uint64_t)stack_base + stack_size,  /* Stack grows down */
        .base = (uint64_t)stack_base,
        .length = stack_size,
        .perms = CAP_PERM_LOAD | CAP_PERM_STORE | CAP_PERM_STORE_LOCAL,
        .otype = CAP_OTYPE_UNSEALED,
        .tag = 1
    };

    /* Heap capability (RW) - initially zero length */
    proc->heap_cap = proc->ddc;  /* Share with data for now */
}
```

### System Calls with Capabilities

```c
/* System call with capability arguments */
long cheri_syscall(long nr, cap256_t *arg_caps, int nargs) {
    /* Validate capability arguments */
    for (int i = 0; i < nargs; i++) {
        if (!arg_caps[i].tag) {
            return -EINVAL;
        }
    }

    switch (nr) {
    case SYS_CHERI_MMAP: {
        /* Map memory and return capability */
        size_t length = arg_caps[0].length;
        int prot = *(int *)arg_caps[1].address;
        int flags = *(int *)arg_caps[2].address;

        void *addr = mmap(NULL, length, prot, flags, -1, 0);
        if (addr == MAP_FAILED) {
            return -ENOMEM;
        }

        /* Create and return capability */
        cap256_t *result_cap = (cap256_t *)arg_caps[3].address;
        *result_cap = (cap256_t){
            .address = (uint64_t)addr,
            .base = (uint64_t)addr,
            .length = length,
            .perms = prot_to_cap_perms(prot),
            .otype = CAP_OTYPE_UNSEALED,
            .tag = 1
        };

        return 0;
    }

    case SYS_CHERI_MUNMAP: {
        /* Unmap memory via capability */
        cap256_t *cap = &arg_caps[0];

        /* Revoke capability (clear tag in memory) */
        revoke_capability(cap);

        /* Unmap memory */
        munmap((void *)cap->base, cap->length);

        return 0;
    }

    default:
        return -ENOSYS;
    }
}
```

### Capability Revocation

```c
/*
 * Temporal safety: revoke capabilities to freed memory
 */

/* Sweep memory and clear tags for revoked regions */
void revoke_region(void *base, size_t length) {
    uintptr_t start = (uintptr_t)base;
    uintptr_t end = start + length;

    /* Clear all tags in region */
    for (uintptr_t addr = start; addr < end; addr += 16) {
        set_cap_tag(addr, 0);
    }

    /* Scan all memory for capabilities pointing into revoked region */
    for (uintptr_t scan = 0; scan < memory_size; scan += 16) {
        if (get_cap_tag(scan)) {
            cap128_t *cap = (cap128_t *)scan;
            uint64_t cap_base = get_cap_base(cap);
            uint64_t cap_top = get_cap_top(cap);

            /* Check if capability overlaps revoked region */
            if (cap_base < end && cap_top > start) {
                /* Clear tag - capability now invalid */
                set_cap_tag(scan, 0);
            }
        }
    }
}

/* Free with revocation */
void cheri_free(void *ptr) {
    /* Get allocation size */
    size_t size = get_allocation_size(ptr);

    /* Revoke all capabilities to this memory */
    revoke_region(ptr, size);

    /* Free memory */
    free(ptr);
}
```

## CHERI C/C++ Language Support

### Capability Pointers

```c
/* Capability pointer type */
typedef void * __capability cap_ptr;

/* Function taking capability pointers */
void safe_memcpy(cap_ptr dest, cap_ptr src, size_t n) {
    /* Compiler generates bounds-checked copy */
    for (size_t i = 0; i < n; i++) {
        ((char * __capability)dest)[i] = ((char * __capability)src)[i];
    }
}

/* Create bounded capability */
cap_ptr create_buffer(size_t size) {
    void *ptr = malloc(size);
    if (!ptr) return NULL;

    /* Compiler inserts CSetBounds */
    return (cap_ptr)__builtin_cheri_bounds_set(ptr, size);
}

/* Get capability properties */
void inspect_capability(cap_ptr p) {
    size_t base = __builtin_cheri_base_get(p);
    size_t length = __builtin_cheri_length_get(p);
    size_t perms = __builtin_cheri_perms_get(p);
    int tag = __builtin_cheri_tag_get(p);

    printf("Capability: base=%zx, length=%zu, perms=%zx, tag=%d\n",
           base, length, perms, tag);
}
```

### Compartmentalized Libraries

```c
/* Declare compartment boundary */
__attribute__((cheri_ccall))
int library_function(int arg, cap_ptr data) {
    /* This function runs in separate compartment */
    /* Can only access memory via 'data' capability */

    /* Read from data */
    int value = *(int * __capability)data;

    return value + arg;
}

/* Call compartmentalized library */
void caller(void) {
    int data = 42;
    cap_ptr data_cap = create_buffer(sizeof(int));
    *(int * __capability)data_cap = data;

    /* Cross-domain call */
    int result = library_function(10, data_cap);

    printf("Result: %d\n", result);  /* 52 */
}
```

## Performance Considerations

### Hardware Implementation

```c
/* Capability cache for fast lookup */
struct cap_cache_entry {
    uint64_t address;
    cap256_t capability;
    int valid;
};

#define CAP_CACHE_SIZE 64

struct cap_cache_entry cap_cache[CAP_CACHE_SIZE];

/* Cache lookup */
static inline cap256_t *cap_cache_lookup(uint64_t addr) {
    uint32_t index = (addr >> 4) % CAP_CACHE_SIZE;
    struct cap_cache_entry *entry = &cap_cache[index];

    if (entry->valid && entry->address == addr) {
        return &entry->capability;
    }

    return NULL;
}

/* Capability checking overhead:
 * - Without cache: ~20-30 cycles per memory access
 * - With cache: ~2-5 cycles per memory access
 * - Hardware optimization: ~0-1 cycles (parallel with address calculation)
 */
```

### Compressed Capabilities Benefit

```
Capability Size Comparison:
- Full capabilities (256-bit): 32 bytes per pointer (4× overhead)
- Compressed (128-bit): 16 bytes per pointer (2× overhead)
- Tagged memory overhead: ~0.78% (1 bit per 16 bytes)

Performance Impact:
- Memory bandwidth: 2× increase with CHERI-128
- Cache footprint: 2× increase
- Typical slowdown: 5-15% for memory-intensive code
- Benefit: Complete memory safety
```

## Summary

### CHERI Benefits

1. **Spatial Safety**: Bounds checking prevents buffer overflows
2. **Temporal Safety**: Capability revocation prevents use-after-free
3. **Compartmentalization**: Sealed capabilities enable fine-grained isolation
4. **Control-Flow Integrity**: Protected code pointers prevent ROP attacks
5. **Minimal TCB**: Hardware enforcement reduces trusted computing base

### Integration with DLX

- CHERI capabilities mapped to 32 capability registers (c0-c31)
- Compatible with existing DLX ISA (hybrid mode)
- Works with register banking (separate cap registers per ring)
- Integrates with MMU for page-level protection
- Supports PAC for additional security

### Use Cases

1. **Memory-safe C/C++**: Bounds checking without rewrites
2. **Secure compartments**: Isolate libraries and components
3. **Protected OS kernels**: Kernel modules with limited authority
4. **Embedded systems**: Security without heavyweight VMs

---

**Document Status**: Specification Complete
**Implementation Status**: Ready for hardware/simulator integration
**Complexity**: Very High - requires tagged memory and capability checking
**Performance**: 5-15% overhead with hardware support
**Security**: Comprehensive memory safety and compartmentalization
**Compatibility**: Can run legacy code in hybrid mode (DDC provides default bounds)
