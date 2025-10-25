# DLX Nested Virtualization and Encrypted Page Tables

## Overview

The DLX architecture provides comprehensive virtualization support including nested virtualization (hypervisor running inside a VM), paravirtualization interfaces, two-dimensional page tables (guest and nested), and hardware memory encryption for virtual machines. This enables efficient multi-level virtualization scenarios such as cloud environments running nested VMs.

## Table of Contents

1. [Virtualization Architecture](#virtualization-architecture)
2. [VM Control Structures (VMCS)](#vm-control-structures-vmcs)
3. [Two-Dimensional Paging](#two-dimensional-paging)
4. [Nested Page Tables with Encryption](#nested-page-tables-with-encryption)
5. [VM Entry and Exit](#vm-entry-and-exit)
6. [Nested Virtualization](#nested-virtualization)
7. [Paravirtualization](#paravirtualization)
8. [Virtual Interrupts](#virtual-interrupts)
9. [Instructions](#instructions)
10. [Programming Examples](#programming-examples)

---

## 1. Virtualization Architecture

### Virtualization Modes

```
┌────────────────────────────────────────────────────────┐
│                   Privilege Levels                      │
├────────────────────────────────────────────────────────┤
│  Ring -1 (VMX Root)        Hypervisor (Host)           │
│  Ring 0  (VMX Non-root)    Guest OS Kernel             │
│  Ring 1-2                  Guest OS Services           │
│  Ring 3                    Guest Applications          │
└────────────────────────────────────────────────────────┘

Nested Virtualization:
┌────────────────────────────────────────────────────────┐
│  L0: Host Hypervisor (VMX Root)                        │
│    ┌──────────────────────────────────────────────┐   │
│    │  L1: Guest Hypervisor (VMX Non-root)         │   │
│    │    ┌────────────────────────────────────┐    │   │
│    │    │  L2: Nested Guest OS               │    │   │
│    │    │    ┌──────────────────────────┐    │    │   │
│    │    │    │  L2 Applications         │    │    │   │
│    │    │    └──────────────────────────┘    │    │   │
│    │    └────────────────────────────────────┘    │   │
│    └──────────────────────────────────────────────┘   │
└────────────────────────────────────────────────────────┘
```

### Virtualization Extensions

```c
/* Virtualization mode register */
typedef struct {
    uint32_t vmx_enable      : 1;   /* VMX operation enabled */
    uint32_t vmx_root_mode   : 1;   /* 1=root, 0=non-root */
    uint32_t vmx_nested      : 1;   /* Nested virtualization enabled */
    uint32_t ept_enable      : 1;   /* Extended page tables */
    uint32_t vpid_enable     : 1;   /* Virtual processor IDs */
    uint32_t apicv_enable    : 1;   /* Virtual APIC */
    uint32_t mee_guest       : 1;   /* Memory encryption for guests */
    uint32_t nested_level    : 3;   /* Current nesting level (0-7) */
    uint32_t pv_interface    : 1;   /* Paravirt interface enabled */
    uint32_t reserved        : 21;
} vmx_mode_reg_t;

/* System registers for virtualization */
#define SR_VMX_MODE         0x300   /* VMX mode register */
#define SR_VMCS_PTR         0x301   /* Current VMCS pointer */
#define SR_VMCS_LINK        0x302   /* Linked VMCS pointer */
#define SR_EPT_PTR          0x303   /* Extended page table pointer */
#define SR_VPID             0x304   /* Virtual processor ID */
#define SR_VMFUNC_CTRL      0x305   /* VM function control */
#define SR_ENCLS_EXITING    0x306   /* Enclave exit bitmap */
```

---

## 2. VM Control Structures (VMCS)

### VMCS Layout

The VMCS (Virtual Machine Control Structure) contains all state for a virtual machine.

```c
/* VMCS structure (4KB aligned) */
typedef struct {
    /* Header */
    uint32_t revision_id;               /* VMCS revision */
    uint32_t abort_indicator;           /* VMX abort indicator */

    /* Guest state area */
    struct {
        /* General purpose registers */
        uint64_t gpr[32];               /* r0-r31 */
        uint64_t pc;                    /* Program counter */
        uint64_t status;                /* Status register */

        /* Control registers */
        uint64_t cr[8];                 /* Control registers */
        uint64_t ring;                  /* Current ring level */

        /* Segment registers (for x86 compat) */
        uint64_t cs_base, cs_limit, cs_ar;
        uint64_t ds_base, ds_limit, ds_ar;
        uint64_t es_base, es_limit, es_ar;
        uint64_t ss_base, ss_limit, ss_ar;

        /* System registers */
        uint64_t idtr_base, idtr_limit;
        uint64_t gdtr_base, gdtr_limit;

        /* Debug registers */
        uint64_t dr[8];

        /* Pending interrupts */
        uint64_t pending_interrupts;
        uint64_t interrupt_status;

        /* Activity state */
        uint32_t activity_state;        /* Active, HLT, shutdown, wait-for-SIPI */
        uint32_t interruptibility_state;

        /* Page table */
        uint64_t page_table_base;       /* Guest page table */
        uint64_t asid;                  /* Address space ID */
    } guest_state;

    /* Host state area (for VM exit) */
    struct {
        uint64_t gpr[32];
        uint64_t pc;
        uint64_t status;
        uint64_t cr[8];
        uint64_t idtr_base, idtr_limit;
        uint64_t gdtr_base, gdtr_limit;
        uint64_t page_table_base;
    } host_state;

    /* VM execution controls */
    struct {
        /* Primary processor-based controls */
        uint32_t interrupt_window_exit  : 1;
        uint32_t use_tsc_offsetting     : 1;
        uint32_t hlt_exit               : 1;
        uint32_t invlpg_exit            : 1;
        uint32_t mwait_exit             : 1;
        uint32_t rdpmc_exit             : 1;
        uint32_t rdtsc_exit             : 1;
        uint32_t cr3_load_exit          : 1;
        uint32_t cr3_store_exit         : 1;
        uint32_t cr8_load_exit          : 1;
        uint32_t cr8_store_exit         : 1;
        uint32_t use_tpr_shadow         : 1;
        uint32_t nmi_window_exit        : 1;
        uint32_t mov_dr_exit            : 1;
        uint32_t unconditional_io_exit  : 1;
        uint32_t use_io_bitmaps         : 1;
        uint32_t monitor_trap_flag      : 1;
        uint32_t use_msr_bitmaps        : 1;
        uint32_t monitor_exit           : 1;
        uint32_t pause_exit             : 1;
        uint32_t activate_secondary     : 1;

        /* Secondary processor-based controls */
        uint32_t virtualize_apic        : 1;
        uint32_t enable_ept             : 1;
        uint32_t descriptor_table_exit  : 1;
        uint32_t enable_rdtscp          : 1;
        uint32_t virtualize_x2apic      : 1;
        uint32_t enable_vpid            : 1;
        uint32_t wbinvd_exit            : 1;
        uint32_t unrestricted_guest     : 1;
        uint32_t apic_register_virt     : 1;
        uint32_t virtual_interrupt_del  : 1;
        uint32_t pause_loop_exit        : 1;
        uint32_t rdrand_exit            : 1;
        uint32_t enable_invpcid         : 1;
        uint32_t enable_vmfunc          : 1;
        uint32_t vmcs_shadowing         : 1;
        uint32_t enable_encls_exit      : 1;
        uint32_t rdseed_exit            : 1;
        uint32_t enable_pml             : 1;
        uint32_t ept_violation_ve       : 1;
        uint32_t conceal_vmx_from_pt    : 1;
        uint32_t enable_xsaves_xrstors  : 1;
        uint32_t mode_based_exec_ctrl   : 1;
        uint32_t enable_ept_encryption  : 1;  /* DLX extension */

        /* Exception bitmap */
        uint32_t exception_bitmap;      /* Bitmap of exceptions to intercept */

        /* I/O bitmap addresses */
        uint64_t io_bitmap_a;           /* I/O bitmap A (0x0000-0x7FFF) */
        uint64_t io_bitmap_b;           /* I/O bitmap B (0x8000-0xFFFF) */

        /* MSR bitmaps */
        uint64_t msr_bitmap;            /* MSR intercept bitmap */

        /* EPT pointer */
        uint64_t ept_pointer;           /* Extended page table */
        uint16_t vpid;                  /* Virtual processor ID */

        /* Virtual APIC */
        uint64_t virtual_apic_page;     /* Virtual APIC page */
        uint64_t apic_access_page;      /* APIC access page */

        /* TSC offset */
        uint64_t tsc_offset;            /* Guest TSC = Host TSC + offset */

        /* Pause loop exiting */
        uint32_t ple_gap;               /* PLE gap */
        uint32_t ple_window;            /* PLE window */
    } exec_controls;

    /* VM exit controls */
    struct {
        uint32_t save_debug_controls    : 1;
        uint32_t host_address_space_64  : 1;
        uint32_t load_ia32_perf_global  : 1;
        uint32_t ack_interrupt_on_exit  : 1;
        uint32_t save_ia32_pat          : 1;
        uint32_t load_ia32_pat          : 1;
        uint32_t save_ia32_efer         : 1;
        uint32_t load_ia32_efer         : 1;
        uint32_t save_vmx_preempt_timer : 1;
        uint32_t clear_ia32_bndcfgs     : 1;
        uint32_t conceal_vmx_from_pt    : 1;
    } exit_controls;

    /* VM entry controls */
    struct {
        uint32_t load_debug_controls    : 1;
        uint32_t ia32e_mode_guest       : 1;
        uint32_t entry_to_smm           : 1;
        uint32_t deactivate_dual_mon    : 1;
        uint32_t load_ia32_perf_global  : 1;
        uint32_t load_ia32_pat          : 1;
        uint32_t load_ia32_efer         : 1;
        uint32_t load_ia32_bndcfgs      : 1;
        uint32_t conceal_vmx_from_pt    : 1;
    } entry_controls;

    /* VM exit information */
    struct {
        uint32_t exit_reason;           /* Why did we exit? */
        uint32_t exit_qualification;    /* Additional info */
        uint64_t guest_linear_addr;     /* Faulting address */
        uint64_t guest_physical_addr;   /* Guest physical addr */
        uint32_t vm_instruction_error;  /* Error code */
        uint32_t exit_instruction_len;  /* Instruction length */
        uint64_t exit_instruction_info; /* Instruction info */
    } exit_info;

    /* Nested virtualization */
    struct {
        uint64_t vmcs12_ptr;            /* L1's VMCS for L2 */
        uint64_t shadow_vmcs;           /* Shadow VMCS */
        uint32_t nested_level;          /* Nesting depth */
    } nested;

    /* Memory encryption */
    struct {
        uint64_t encryption_key_id;     /* Key ID for this VM */
        uint64_t encrypted_regions[16]; /* Encrypted memory regions */
        uint32_t num_encrypted_regions;
    } encryption;

    /* Statistics */
    struct {
        uint64_t vm_entries;
        uint64_t vm_exits;
        uint64_t cycles_in_guest;
    } stats;

    uint8_t padding[4096 - sizeof(struct {})]; /* Pad to 4KB */
} __attribute__((aligned(4096))) vmcs_t;

/* VM exit reasons */
#define EXIT_REASON_EXCEPTION           0
#define EXIT_REASON_EXTERNAL_INT        1
#define EXIT_REASON_TRIPLE_FAULT        2
#define EXIT_REASON_INIT_SIGNAL         3
#define EXIT_REASON_SIPI                4
#define EXIT_REASON_IO_SMI              5
#define EXIT_REASON_OTHER_SMI           6
#define EXIT_REASON_INT_WINDOW          7
#define EXIT_REASON_NMI_WINDOW          8
#define EXIT_REASON_TASK_SWITCH         9
#define EXIT_REASON_CPUID               10
#define EXIT_REASON_GETSEC              11
#define EXIT_REASON_HLT                 12
#define EXIT_REASON_INVD                13
#define EXIT_REASON_INVLPG              14
#define EXIT_REASON_RDPMC               15
#define EXIT_REASON_RDTSC               16
#define EXIT_REASON_RSM                 17
#define EXIT_REASON_VMCALL              18
#define EXIT_REASON_VMCLEAR             19
#define EXIT_REASON_VMLAUNCH            20
#define EXIT_REASON_VMPTRLD             21
#define EXIT_REASON_VMPTRST             22
#define EXIT_REASON_VMREAD              23
#define EXIT_REASON_VMRESUME            24
#define EXIT_REASON_VMWRITE             25
#define EXIT_REASON_VMXOFF              26
#define EXIT_REASON_VMXON               27
#define EXIT_REASON_CR_ACCESS           28
#define EXIT_REASON_DR_ACCESS           29
#define EXIT_REASON_IO_INSTRUCTION      30
#define EXIT_REASON_MSR_READ            31
#define EXIT_REASON_MSR_WRITE           32
#define EXIT_REASON_INVALID_STATE       33
#define EXIT_REASON_MSR_LOAD_FAIL       34
#define EXIT_REASON_MWAIT               36
#define EXIT_REASON_MONITOR_TRAP        37
#define EXIT_REASON_MONITOR             39
#define EXIT_REASON_PAUSE               40
#define EXIT_REASON_MCE_DURING_ENTRY    41
#define EXIT_REASON_TPR_BELOW_THRESHOLD 43
#define EXIT_REASON_APIC_ACCESS         44
#define EXIT_REASON_EOI_INDUCED         45
#define EXIT_REASON_GDTR_IDTR           46
#define EXIT_REASON_LDTR_TR             47
#define EXIT_REASON_EPT_VIOLATION       48
#define EXIT_REASON_EPT_MISCONFIG       49
#define EXIT_REASON_INVEPT              50
#define EXIT_REASON_RDTSCP              51
#define EXIT_REASON_PREEMPT_TIMER       52
#define EXIT_REASON_INVVPID             53
#define EXIT_REASON_WBINVD              54
#define EXIT_REASON_XSETBV              55
#define EXIT_REASON_APIC_WRITE          56
#define EXIT_REASON_RDRAND              57
#define EXIT_REASON_INVPCID             58
#define EXIT_REASON_VMFUNC              59
#define EXIT_REASON_ENCLS               60
#define EXIT_REASON_RDSEED              61
#define EXIT_REASON_PML_FULL            62
#define EXIT_REASON_XSAVES              63
#define EXIT_REASON_XRSTORS             64
#define EXIT_REASON_PCOMMIT             65

/* DLX-specific exit reasons */
#define EXIT_REASON_DLX_SYSCALL         128
#define EXIT_REASON_DLX_CAPABILITY      129
#define EXIT_REASON_DLX_ENCLAVE         130
#define EXIT_REASON_DLX_NESTED_EXIT     131
```

---

## 3. Two-Dimensional Paging

### Address Translation with EPT/NPT

```
Guest Virtual Address (GVA)
        ↓
    [Guest Page Tables - First Dimension]
        ↓
Guest Physical Address (GPA)
        ↓
    [Extended/Nested Page Tables (EPT/NPT) - Second Dimension]
        ↓
Host Physical Address (HPA)
```

### EPT Structure

The Extended Page Table provides second-level address translation.

```c
/* EPT page table entry (64-bit) */
typedef struct {
    uint64_t read        : 1;       /* Read permission */
    uint64_t write       : 1;       /* Write permission */
    uint64_t exec        : 1;       /* Execute permission */
    uint64_t memory_type : 3;       /* Memory type (0=UC, 6=WB) */
    uint64_t ignore_pat  : 1;       /* Ignore PAT */
    uint64_t large_page  : 1;       /* 1=2MB/1GB page */
    uint64_t accessed    : 1;       /* Accessed bit */
    uint64_t dirty       : 1;       /* Dirty bit */
    uint64_t user_exec   : 1;       /* User-mode execute */
    uint64_t reserved1   : 1;
    uint64_t pfn         : 40;      /* Page frame number */
    uint64_t reserved2   : 11;
    uint64_t encrypted   : 1;       /* Page is encrypted (DLX ext) */
} ept_entry_t;

/* EPT pointer (loaded into SR_EPT_PTR) */
typedef struct {
    uint64_t memory_type : 3;       /* EPT page walk memory type */
    uint64_t page_walk_len : 3;     /* Page walk length - 1 */
    uint64_t enable_access_dirty : 1; /* Enable A/D bits */
    uint64_t enable_encryption : 1; /* Enable page encryption */
    uint64_t reserved    : 5;
    uint64_t pfn         : 51;      /* Page frame of EPT PML4 */
} ept_pointer_t;

/* Memory types */
#define EPT_MEMTYPE_UC      0       /* Uncacheable */
#define EPT_MEMTYPE_WC      1       /* Write-combining */
#define EPT_MEMTYPE_WT      4       /* Write-through */
#define EPT_MEMTYPE_WP      5       /* Write-protected */
#define EPT_MEMTYPE_WB      6       /* Write-back */
```

### EPT Page Table Walk

```c
/* Walk EPT to translate GPA → HPA */
uint64_t ept_walk(uint64_t ept_base, uint64_t gpa, uint32_t *error) {
    ept_entry_t *pml4, *pdpt, *pd, *pt;
    uint64_t hpa;

    /* Extract page table indices */
    uint32_t pml4_idx = (gpa >> 39) & 0x1FF;
    uint32_t pdpt_idx = (gpa >> 30) & 0x1FF;
    uint32_t pd_idx   = (gpa >> 21) & 0x1FF;
    uint32_t pt_idx   = (gpa >> 12) & 0x1FF;
    uint32_t offset   = gpa & 0xFFF;

    /* PML4 */
    pml4 = (ept_entry_t *)ept_base;
    if (!pml4[pml4_idx].read) {
        *error = EPT_VIOLATION_READ;
        return 0;
    }

    /* PDPT */
    pdpt = (ept_entry_t *)(pml4[pml4_idx].pfn << 12);
    if (!pdpt[pdpt_idx].read) {
        *error = EPT_VIOLATION_READ;
        return 0;
    }

    /* Check for 1GB page */
    if (pdpt[pdpt_idx].large_page) {
        hpa = (pdpt[pdpt_idx].pfn << 30) | (gpa & 0x3FFFFFFF);
        return hpa;
    }

    /* PD */
    pd = (ept_entry_t *)(pdpt[pdpt_idx].pfn << 12);
    if (!pd[pd_idx].read) {
        *error = EPT_VIOLATION_READ;
        return 0;
    }

    /* Check for 2MB page */
    if (pd[pd_idx].large_page) {
        hpa = (pd[pd_idx].pfn << 21) | (gpa & 0x1FFFFF);
        return hpa;
    }

    /* PT */
    pt = (ept_entry_t *)(pd[pd_idx].pfn << 12);
    if (!pt[pt_idx].read) {
        *error = EPT_VIOLATION_READ;
        return 0;
    }

    /* 4KB page */
    hpa = (pt[pt_idx].pfn << 12) | offset;

    /* Mark accessed */
    pt[pt_idx].accessed = 1;

    return hpa;
}

/* EPT violation errors */
#define EPT_VIOLATION_READ      0x01
#define EPT_VIOLATION_WRITE     0x02
#define EPT_VIOLATION_EXEC      0x04
#define EPT_VIOLATION_GPA_READABLE      0x08
#define EPT_VIOLATION_GPA_WRITABLE      0x10
#define EPT_VIOLATION_GPA_EXECUTABLE    0x20
```

---

## 4. Nested Page Tables with Encryption

### Encrypted Guest Memory

DLX extends EPT to support per-VM memory encryption using the Memory Encryption Engine (MEE).

```c
/* VM encryption context */
typedef struct {
    uint8_t  encryption_key[32];    /* AES-256 key for this VM */
    uint64_t key_id;                /* Hardware key ID (0-255) */
    uint32_t encryption_enabled;    /* Encryption active */

    /* Integrity tree for encrypted pages */
    struct {
        void    *tree_root;         /* Merkle tree root */
        uint64_t tree_size;         /* Tree size in bytes */
        uint8_t  root_hash[32];     /* SHA-256 root hash */
    } integrity;

    /* Encrypted memory regions */
    struct {
        uint64_t gpa_start;         /* Guest physical start */
        uint64_t gpa_end;           /* Guest physical end */
        uint64_t hpa_start;         /* Host physical start */
    } encrypted_regions[16];
    uint32_t num_encrypted_regions;
} vm_encryption_ctx_t;

/* EPT entry with encryption */
typedef struct {
    ept_entry_t pte;                /* Base EPT entry */
    uint64_t nonce;                 /* Encryption nonce */
    uint8_t  mac[16];               /* AES-GCM MAC tag */
    uint64_t version;               /* Anti-replay counter */
} ept_encrypted_entry_t;
```

### Encrypted Page Access

```c
/* Read encrypted guest page */
int ept_read_encrypted_page(vm_encryption_ctx_t *ctx, uint64_t gpa,
                           void *buffer, size_t size) {
    ept_encrypted_entry_t *entry;
    uint8_t ciphertext[4096];
    uint8_t computed_mac[16];

    /* Walk EPT to find entry */
    entry = ept_get_encrypted_entry(gpa);
    if (!entry || !entry->pte.encrypted)
        return -1;

    /* Read encrypted data from HPA */
    uint64_t hpa = entry->pte.pfn << 12;
    memcpy(ciphertext, (void *)hpa, size);

    /* Decrypt with AES-256-GCM */
    aes256_gcm_decrypt(
        ciphertext, size,                   /* Ciphertext */
        ctx->encryption_key,                /* Key */
        (uint8_t *)&entry->nonce, 8,        /* Nonce */
        (uint8_t *)&gpa, 8,                 /* AAD (GPA) */
        buffer,                             /* Plaintext output */
        computed_mac                        /* Computed MAC */
    );

    /* Verify MAC */
    if (memcmp(computed_mac, entry->mac, 16) != 0) {
        /* Integrity violation! */
        trigger_ept_violation(gpa, EPT_VIOLATION_INTEGRITY);
        return -1;
    }

    /* Verify version counter (anti-replay) */
    if (!verify_version_counter(ctx, gpa, entry->version))
        return -1;

    return 0;
}

/* Write encrypted guest page */
int ept_write_encrypted_page(vm_encryption_ctx_t *ctx, uint64_t gpa,
                            const void *buffer, size_t size) {
    ept_encrypted_entry_t *entry;
    uint8_t ciphertext[4096];

    entry = ept_get_encrypted_entry(gpa);
    if (!entry || !entry->pte.encrypted)
        return -1;

    /* Generate new nonce */
    entry->nonce = atomic_inc(&ctx->nonce_counter);
    entry->version++;

    /* Encrypt with AES-256-GCM */
    aes256_gcm_encrypt(
        buffer, size,                       /* Plaintext */
        ctx->encryption_key,                /* Key */
        (uint8_t *)&entry->nonce, 8,        /* Nonce */
        (uint8_t *)&gpa, 8,                 /* AAD (GPA) */
        ciphertext,                         /* Ciphertext output */
        entry->mac                          /* MAC tag */
    );

    /* Write to HPA */
    uint64_t hpa = entry->pte.pfn << 12;
    memcpy((void *)hpa, ciphertext, size);

    /* Update integrity tree */
    update_integrity_tree(ctx, gpa, entry->mac, entry->version);

    /* Mark dirty */
    entry->pte.dirty = 1;

    return 0;
}

#define EPT_VIOLATION_INTEGRITY     0x80
```

### VM Encryption Management

```c
/* Initialize VM encryption */
int vm_encryption_init(vmcs_t *vmcs, vm_encryption_ctx_t *ctx) {
    /* Generate random encryption key */
    get_random_bytes(ctx->encryption_key, 32);

    /* Allocate hardware key ID */
    ctx->key_id = allocate_vm_key_id();

    /* Load key into MEE */
    mee_load_vm_key(ctx->key_id, ctx->encryption_key);

    /* Initialize integrity tree */
    ctx->integrity.tree_size = calculate_tree_size(vmcs);
    ctx->integrity.tree_root = allocate_integrity_tree(ctx->integrity.tree_size);

    /* Enable encryption in VMCS */
    vmcs->exec_controls.enable_ept_encryption = 1;
    vmcs->encryption.encryption_key_id = ctx->key_id;

    ctx->encryption_enabled = 1;

    return 0;
}

/* Encrypt guest memory region */
int vm_encrypt_memory_region(vm_encryption_ctx_t *ctx,
                            uint64_t gpa_start, uint64_t gpa_end) {
    if (ctx->num_encrypted_regions >= 16)
        return -1;

    /* Walk EPT and mark pages as encrypted */
    for (uint64_t gpa = gpa_start; gpa < gpa_end; gpa += 4096) {
        ept_entry_t *pte = ept_get_entry(gpa);
        if (pte) {
            pte->encrypted = 1;

            /* Encrypt existing data in-place */
            encrypt_guest_page(ctx, gpa);
        }
    }

    /* Record encrypted region */
    ctx->encrypted_regions[ctx->num_encrypted_regions].gpa_start = gpa_start;
    ctx->encrypted_regions[ctx->num_encrypted_regions].gpa_end = gpa_end;
    ctx->num_encrypted_regions++;

    return 0;
}
```

---

## 5. VM Entry and Exit

### VMLAUNCH / VMRESUME

```c
/* VM entry */
int vm_enter(vmcs_t *vmcs) {
    /* Validate VMCS */
    if (!vmcs_is_valid(vmcs))
        return -1;

    /* Load guest state */
    load_guest_state(&vmcs->guest_state);

    /* Load EPT pointer */
    if (vmcs->exec_controls.enable_ept) {
        asm volatile("mtsr $EPT_PTR, %0" : : "r"(vmcs->exec_controls.ept_pointer));
    }

    /* Load VPID */
    if (vmcs->exec_controls.enable_vpid) {
        asm volatile("mtsr $VPID, %0" : : "r"(vmcs->exec_controls.vpid));
    }

    /* Enable non-root mode */
    uint32_t vmx_mode = 0;
    vmx_mode |= (1 << 0);   /* VMX enabled */
    vmx_mode |= (0 << 1);   /* Non-root mode */
    asm volatile("mtsr $VMX_MODE, %0" : : "r"(vmx_mode));

    /* Jump to guest PC */
    vmcs->stats.vm_entries++;
    asm volatile("jr %0" : : "r"(vmcs->guest_state.pc));

    /* Should not return (VM exit will return to host) */
    return 0;
}

/* VM exit handler */
void vm_exit_handler(void) {
    vmcs_t *vmcs = get_current_vmcs();

    /* Save guest state */
    save_guest_state(&vmcs->guest_state);

    /* Read exit reason */
    uint32_t exit_reason;
    asm volatile("mfsr %0, $VMX_EXIT_REASON" : "=r"(exit_reason));
    vmcs->exit_info.exit_reason = exit_reason;

    /* Read exit qualification */
    asm volatile("mfsr %0, $VMX_EXIT_QUAL" : "=r"(vmcs->exit_info.exit_qualification));

    /* Switch to root mode */
    uint32_t vmx_mode = 0;
    vmx_mode |= (1 << 0);   /* VMX enabled */
    vmx_mode |= (1 << 1);   /* Root mode */
    asm volatile("mtsr $VMX_MODE, %0" : : "r"(vmx_mode));

    /* Load host state */
    load_host_state(&vmcs->host_state);

    /* Statistics */
    vmcs->stats.vm_exits++;

    /* Handle exit reason */
    handle_vm_exit(vmcs);
}
```

### Exit Handling

```c
/* Handle VM exit */
void handle_vm_exit(vmcs_t *vmcs) {
    switch (vmcs->exit_info.exit_reason) {
    case EXIT_REASON_CPUID:
        handle_cpuid_exit(vmcs);
        break;

    case EXIT_REASON_HLT:
        handle_hlt_exit(vmcs);
        break;

    case EXIT_REASON_IO_INSTRUCTION:
        handle_io_exit(vmcs);
        break;

    case EXIT_REASON_MSR_READ:
    case EXIT_REASON_MSR_WRITE:
        handle_msr_exit(vmcs);
        break;

    case EXIT_REASON_EPT_VIOLATION:
        handle_ept_violation(vmcs);
        break;

    case EXIT_REASON_VMCALL:
        handle_hypercall(vmcs);
        break;

    case EXIT_REASON_EXTERNAL_INT:
        handle_external_interrupt(vmcs);
        break;

    case EXIT_REASON_DLX_CAPABILITY:
        handle_capability_exit(vmcs);
        break;

    case EXIT_REASON_DLX_ENCLAVE:
        handle_enclave_exit(vmcs);
        break;

    case EXIT_REASON_DLX_NESTED_EXIT:
        handle_nested_vm_exit(vmcs);
        break;

    default:
        /* Inject exception to guest */
        inject_exception(vmcs, EXCEPTION_INVALID_OPCODE);
        break;
    }
}

/* EPT violation handler */
void handle_ept_violation(vmcs_t *vmcs) {
    uint64_t gpa = vmcs->exit_info.guest_physical_addr;
    uint32_t qual = vmcs->exit_info.exit_qualification;

    /* Check if this is encrypted memory access */
    ept_entry_t *pte = ept_get_entry(gpa);
    if (pte && pte->encrypted) {
        /* Handle encrypted page fault */
        handle_encrypted_page_fault(vmcs, gpa, qual);
        return;
    }

    /* Check read/write/execute permissions */
    if (qual & EPT_VIOLATION_READ) {
        /* Map page with read permission */
        ept_map_page(gpa, get_hpa_for_gpa(gpa), EPT_READ);
    } else if (qual & EPT_VIOLATION_WRITE) {
        /* Handle copy-on-write, etc. */
        handle_ept_write_fault(vmcs, gpa);
    } else if (qual & EPT_VIOLATION_EXEC) {
        /* Handle execute-only pages */
        handle_ept_exec_fault(vmcs, gpa);
    }
}
```

---

## 6. Nested Virtualization

### Three-Level Address Translation

In nested virtualization, address translation becomes three-dimensional:

```
L2 Guest Virtual Address (GVA)
        ↓
    [L2 Guest Page Tables]
        ↓
L2 Guest Physical Address (GPA)  [= L1 GVA]
        ↓
    [L1 Guest Page Tables - from L1's perspective]
        ↓
L1 Guest Physical Address (GPA)  [= L0 GPA]
        ↓
    [L0 Extended Page Tables (EPT)]
        ↓
L0 Host Physical Address (HPA)
```

### Shadow VMCS

```c
/* Nested virtualization context */
typedef struct {
    vmcs_t *vmcs01;             /* L0's VMCS for L1 */
    vmcs_t *vmcs02;             /* L0's VMCS for L2 (shadow) */
    vmcs_t *vmcs12;             /* L1's VMCS for L2 */

    /* Cached L1 state */
    struct {
        uint64_t ept_pointer;   /* L1's EPT for L2 */
        uint32_t exec_controls;
        uint32_t exit_controls;
        uint32_t entry_controls;
    } l1_state;

    /* Nested EPT */
    uint64_t nested_ept_base;   /* Combined EPT for L2 */
} nested_virt_ctx_t;

/* Prepare nested VM entry (L1 → L2) */
int prepare_nested_vm_entry(nested_virt_ctx_t *ctx) {
    /* Copy L1's VMCS12 to our shadow VMCS02 */
    vmcs_copy_fields(ctx->vmcs12, ctx->vmcs02);

    /* Merge execution controls */
    merge_vmcs_controls(ctx->vmcs01, ctx->vmcs12, ctx->vmcs02);

    /* Build combined EPT: GPA(L2) → GPA(L1) → HPA(L0) */
    build_nested_ept(ctx->l1_state.ept_pointer,
                    ctx->vmcs01->exec_controls.ept_pointer,
                    &ctx->nested_ept_base);

    /* Set nested EPT in VMCS02 */
    ctx->vmcs02->exec_controls.ept_pointer = ctx->nested_ept_base;

    /* Set nesting level */
    ctx->vmcs02->nested.nested_level = 2;

    return 0;
}

/* Merge EPT tables for nested virtualization */
int build_nested_ept(uint64_t ept12_base, uint64_t ept01_base,
                    uint64_t *ept02_base) {
    ept_entry_t *ept12 = (ept_entry_t *)ept12_base;  /* L1's EPT for L2 */
    ept_entry_t *ept01 = (ept_entry_t *)ept01_base;  /* L0's EPT for L1 */
    ept_entry_t *ept02;                              /* Combined EPT */

    /* Allocate combined EPT */
    ept02 = allocate_ept_table();
    *ept02_base = (uint64_t)ept02;

    /* Walk EPT12 and translate each GPA(L1) to HPA(L0) using EPT01 */
    for (uint64_t gpa_l2 = 0; gpa_l2 < (1ULL << 48); gpa_l2 += 4096) {
        /* Translate GPA(L2) → GPA(L1) using EPT12 */
        uint64_t gpa_l1 = ept_walk(ept12_base, gpa_l2, NULL);
        if (!gpa_l1)
            continue;

        /* Translate GPA(L1) → HPA(L0) using EPT01 */
        uint64_t hpa_l0 = ept_walk(ept01_base, gpa_l1, NULL);
        if (!hpa_l0)
            continue;

        /* Install direct mapping GPA(L2) → HPA(L0) in EPT02 */
        ept_install_mapping(ept02, gpa_l2, hpa_l0);
    }

    return 0;
}
```

### Nested VM Exit Handling

```c
/* Handle VM exit from L2 */
void handle_nested_vm_exit(vmcs_t *vmcs02) {
    nested_virt_ctx_t *ctx = get_nested_context();
    uint32_t exit_reason = vmcs02->exit_info.exit_reason;

    /* Determine if L0 or L1 should handle this exit */
    if (should_exit_to_l0(exit_reason)) {
        /* L0 handles directly (e.g., host interrupts) */
        handle_vm_exit(vmcs02);
        return;
    }

    /* Exit to L1 */
    /* Copy exit info from VMCS02 to VMCS12 */
    ctx->vmcs12->exit_info = vmcs02->exit_info;

    /* Reflect exit to L1 by entering L1 */
    vm_enter(ctx->vmcs01);
}

/* Check if exit should go to L0 */
static inline int should_exit_to_l0(uint32_t exit_reason) {
    switch (exit_reason) {
    case EXIT_REASON_EXTERNAL_INT:      /* Physical interrupts → L0 */
    case EXIT_REASON_NMI_WINDOW:
    case EXIT_REASON_MCE_DURING_ENTRY:
        return 1;

    default:
        return 0;  /* Reflect to L1 */
    }
}
```

---

## 7. Paravirtualization

### Hypercall Interface

Paravirtualization allows guests to make explicit calls to the hypervisor for better performance.

```c
/* Hypercall numbers */
#define HYPERCALL_CONSOLE_OUTPUT    0
#define HYPERCALL_YIELD             1
#define HYPERCALL_NOTIFY            2
#define HYPERCALL_MAP_GRANT         3
#define HYPERCALL_UNMAP_GRANT       4
#define HYPERCALL_EVENT_CHANNEL     5
#define HYPERCALL_VCPU_OP           6
#define HYPERCALL_MEMORY_OP         7
#define HYPERCALL_SCHED_OP          8
#define HYPERCALL_GRANT_TABLE_OP    9
#define HYPERCALL_PHYSDEV_OP        10

/* DLX-specific hypercalls */
#define HYPERCALL_DLX_FAST_TLB_FLUSH    128
#define HYPERCALL_DLX_FAST_IPI          129
#define HYPERCALL_DLX_ENCRYPT_MEMORY    130
#define HYPERCALL_DLX_CAPABILITY_OP     131

/* Hypercall ABI */
typedef struct {
    uint64_t hypercall_num;     /* a0 */
    uint64_t arg1;              /* a1 */
    uint64_t arg2;              /* a2 */
    uint64_t arg3;              /* a3 */
    uint64_t arg4;              /* a4 */
    uint64_t arg5;              /* a5 */
    uint64_t result;            /* v0 */
} hypercall_args_t;

/* Make hypercall from guest */
static inline uint64_t hypercall(uint64_t num, uint64_t arg1, uint64_t arg2,
                                uint64_t arg3, uint64_t arg4, uint64_t arg5) {
    uint64_t result;

    asm volatile(
        "mv     a0, %1\n"       /* Hypercall number */
        "mv     a1, %2\n"
        "mv     a2, %3\n"
        "mv     a3, %4\n"
        "mv     a4, %5\n"
        "mv     a5, %6\n"
        "vmcall\n"              /* Trigger VM exit */
        "mv     %0, v0\n"       /* Get result */
        : "=r"(result)
        : "r"(num), "r"(arg1), "r"(arg2), "r"(arg3), "r"(arg4), "r"(arg5)
        : "a0", "a1", "a2", "a3", "a4", "a5", "v0"
    );

    return result;
}

/* Handle hypercall in hypervisor */
void handle_hypercall(vmcs_t *vmcs) {
    hypercall_args_t args;

    /* Get arguments from guest registers */
    args.hypercall_num = vmcs->guest_state.gpr[4];  /* a0 */
    args.arg1 = vmcs->guest_state.gpr[5];           /* a1 */
    args.arg2 = vmcs->guest_state.gpr[6];           /* a2 */
    args.arg3 = vmcs->guest_state.gpr[7];           /* a3 */
    args.arg4 = vmcs->guest_state.gpr[8];           /* a4 */
    args.arg5 = vmcs->guest_state.gpr[9];           /* a5 */

    /* Dispatch hypercall */
    switch (args.hypercall_num) {
    case HYPERCALL_CONSOLE_OUTPUT:
        args.result = hypercall_console_output(vmcs, args.arg1, args.arg2);
        break;

    case HYPERCALL_YIELD:
        args.result = hypercall_yield(vmcs);
        break;

    case HYPERCALL_DLX_FAST_TLB_FLUSH:
        args.result = hypercall_fast_tlb_flush(vmcs, args.arg1, args.arg2);
        break;

    case HYPERCALL_DLX_ENCRYPT_MEMORY:
        args.result = hypercall_encrypt_memory(vmcs, args.arg1, args.arg2);
        break;

    default:
        args.result = -ENOSYS;
        break;
    }

    /* Return result in v0 */
    vmcs->guest_state.gpr[2] = args.result;

    /* Advance guest PC past VMCALL */
    vmcs->guest_state.pc += vmcs->exit_info.exit_instruction_len;
}
```

### Paravirtualized Devices

```c
/* Shared memory structure for paravirt devices */
typedef struct {
    /* Ring buffer for communication */
    volatile uint32_t req_prod;     /* Guest producer index */
    volatile uint32_t req_cons;     /* Host consumer index */
    volatile uint32_t rsp_prod;     /* Host producer index */
    volatile uint32_t rsp_cons;     /* Guest consumer index */

    /* Requests */
    struct {
        uint32_t id;
        uint32_t operation;
        uint64_t sector;
        uint64_t buffer_gpa;
        uint32_t num_sectors;
    } requests[256];

    /* Responses */
    struct {
        uint32_t id;
        uint32_t status;
    } responses[256];

    /* Event channel for notifications */
    volatile uint32_t event_pending;
} paravirt_ring_t;

/* Paravirt block device */
int pv_block_read(paravirt_ring_t *ring, uint64_t sector,
                 void *buffer, uint32_t num_sectors) {
    uint32_t idx = ring->req_prod % 256;

    /* Fill request */
    ring->requests[idx].id = ring->req_prod;
    ring->requests[idx].operation = BLOCK_OP_READ;
    ring->requests[idx].sector = sector;
    ring->requests[idx].buffer_gpa = virt_to_gpa(buffer);
    ring->requests[idx].num_sectors = num_sectors;

    /* Update producer index */
    __sync_synchronize();
    ring->req_prod++;

    /* Notify hypervisor */
    hypercall(HYPERCALL_NOTIFY, 0, 0, 0, 0, 0);

    /* Wait for response */
    while (ring->rsp_cons == ring->rsp_prod)
        cpu_relax();

    /* Get response */
    uint32_t rsp_idx = ring->rsp_cons % 256;
    uint32_t status = ring->responses[rsp_idx].status;
    ring->rsp_cons++;

    return status;
}
```

---

## 8. Virtual Interrupts

### Virtual APIC

```c
/* Virtual APIC page (4KB, mapped to guest) */
typedef struct {
    /* APIC registers */
    uint32_t apic_id;                   /* 0x020 */
    uint32_t apic_version;              /* 0x030 */
    uint32_t tpr;                       /* 0x080 Task Priority */
    uint32_t apr;                       /* 0x090 Arbitration Priority */
    uint32_t ppr;                       /* 0x0A0 Processor Priority */
    uint32_t eoi;                       /* 0x0B0 End of Interrupt */
    uint32_t ldr;                       /* 0x0D0 Logical Destination */
    uint32_t dfr;                       /* 0x0E0 Destination Format */
    uint32_t svr;                       /* 0x0F0 Spurious Interrupt Vector */
    uint32_t isr[8];                    /* 0x100-0x170 In-Service */
    uint32_t tmr[8];                    /* 0x180-0x1F0 Trigger Mode */
    uint32_t irr[8];                    /* 0x200-0x270 Interrupt Request */
    uint32_t esr;                       /* 0x280 Error Status */
    uint32_t icr_low;                   /* 0x300 Interrupt Command (low) */
    uint32_t icr_high;                  /* 0x310 Interrupt Command (high) */
    uint32_t lvt_timer;                 /* 0x320 LVT Timer */
    uint32_t lvt_thermal;               /* 0x330 LVT Thermal */
    uint32_t lvt_perf;                  /* 0x340 LVT Performance */
    uint32_t lvt_lint0;                 /* 0x350 LVT LINT0 */
    uint32_t lvt_lint1;                 /* 0x360 LVT LINT1 */
    uint32_t lvt_error;                 /* 0x370 LVT Error */
    uint32_t timer_icr;                 /* 0x380 Timer Initial Count */
    uint32_t timer_ccr;                 /* 0x390 Timer Current Count */
    uint32_t timer_dcr;                 /* 0x3E0 Timer Divide Config */
} __attribute__((packed)) virtual_apic_page_t;

/* Post interrupt to guest */
void post_virtual_interrupt(vmcs_t *vmcs, uint32_t vector) {
    virtual_apic_page_t *vapic = (virtual_apic_page_t *)vmcs->exec_controls.virtual_apic_page;

    /* Set bit in IRR (Interrupt Request Register) */
    uint32_t irr_idx = vector / 32;
    uint32_t irr_bit = vector % 32;
    vapic->irr[irr_idx] |= (1 << irr_bit);

    /* Request interrupt window exit if guest interrupts disabled */
    if (!(vmcs->guest_state.status & STATUS_IE)) {
        vmcs->exec_controls.interrupt_window_exit = 1;
    } else {
        /* Inject interrupt immediately */
        inject_virtual_interrupt(vmcs, vector);
    }
}

/* Inject virtual interrupt */
void inject_virtual_interrupt(vmcs_t *vmcs, uint32_t vector) {
    virtual_apic_page_t *vapic = (virtual_apic_page_t *)vmcs->exec_controls.virtual_apic_page;

    /* Move from IRR to ISR */
    uint32_t irr_idx = vector / 32;
    uint32_t irr_bit = vector % 32;
    vapic->irr[irr_idx] &= ~(1 << irr_bit);
    vapic->isr[irr_idx] |= (1 << irr_bit);

    /* Update interrupt status */
    vmcs->guest_state.pending_interrupts |= (1ULL << vector);
}
```

---

## 9. Instructions

### Virtualization Instructions (New ISA Mnemonics)

```assembly
# VMX operations
vmxon       addr                    # Enable VMX operation
vmxoff                              # Disable VMX operation

vmclear     vmcs_addr               # Clear VMCS
vmptrld     vmcs_addr               # Load VMCS pointer
vmptrst     mem_addr                # Store VMCS pointer

vmread      rd, field               # Read VMCS field
vmwrite     field, rs               # Write VMCS field

vmlaunch                            # Launch VM (initial entry)
vmresume                            # Resume VM (re-entry)

vmcall                              # Hypercall from guest

# EPT operations
invept      type, descriptor        # Invalidate EPT
invvpid     type, descriptor        # Invalidate VPID

# VM functions (called from guest)
vmfunc      function_num            # VM function (fast switching)

# Nested operations
vmptrld.nested  vmcs_addr           # Load nested VMCS
vmlaunch.nested                     # Launch nested VM
```

### System Register Access

```assembly
# Load/store virtualization state
mfsr        rd, $VMX_MODE           # Read VMX mode
mtsr        $VMCS_PTR, rs           # Set VMCS pointer
mfsr        rd, $EPT_PTR            # Read EPT pointer
mtsr        $EPT_PTR, rs            # Set EPT pointer
```

---

## 10. Programming Examples

### Example 1: Basic VM Creation

```c
/* Create and run a simple VM */
int create_simple_vm(void) {
    vmcs_t *vmcs;
    uint64_t guest_mem;

    /* Allocate VMCS */
    vmcs = aligned_alloc(4096, sizeof(vmcs_t));
    memset(vmcs, 0, sizeof(vmcs_t));

    vmcs->revision_id = get_vmcs_revision();

    /* Allocate guest memory (16MB) */
    guest_mem = (uint64_t)allocate_guest_memory(16 * 1024 * 1024);

    /* Setup guest state */
    vmcs->guest_state.pc = guest_mem;          /* Start at base */
    vmcs->guest_state.gpr[29] = guest_mem + (8 * 1024 * 1024);  /* Stack */
    vmcs->guest_state.status = STATUS_IE;       /* Interrupts enabled */
    vmcs->guest_state.ring = 0;                 /* Ring 0 */

    /* Setup EPT */
    uint64_t ept_base = setup_ept_for_guest(guest_mem, 16 * 1024 * 1024);
    vmcs->exec_controls.enable_ept = 1;
    vmcs->exec_controls.ept_pointer = ept_base;
    vmcs->exec_controls.vpid = allocate_vpid();
    vmcs->exec_controls.enable_vpid = 1;

    /* Setup execution controls */
    vmcs->exec_controls.hlt_exit = 1;
    vmcs->exec_controls.rdtsc_exit = 0;         /* Allow RDTSC */
    vmcs->exec_controls.activate_secondary = 1;
    vmcs->exec_controls.unrestricted_guest = 1;

    /* Setup exit controls */
    vmcs->exit_controls.ack_interrupt_on_exit = 1;

    /* Setup host state (for VM exit) */
    vmcs->host_state.pc = (uint64_t)vm_exit_handler;
    vmcs->host_state.gpr[29] = (uint64_t)get_host_stack();
    vmcs->host_state.page_table_base = get_host_page_table();

    /* Load guest code */
    load_guest_code(guest_mem, "guest.bin");

    /* Enable VMX */
    asm volatile("vmxon %0" : : "m"(get_vmxon_region()));

    /* Load VMCS */
    asm volatile("vmptrld %0" : : "m"(vmcs));

    /* Launch VM */
    asm volatile("vmlaunch");

    /* Should not return (will exit to vm_exit_handler) */
    return 0;
}
```

### Example 2: Encrypted VM

```c
/* Create VM with encrypted memory */
int create_encrypted_vm(void) {
    vmcs_t *vmcs;
    vm_encryption_ctx_t encryption_ctx;

    vmcs = create_basic_vm();

    /* Initialize encryption */
    vm_encryption_init(vmcs, &encryption_ctx);

    /* Encrypt guest memory region (0-16MB) */
    vm_encrypt_memory_region(&encryption_ctx, 0, 16 * 1024 * 1024);

    /* Enable EPT encryption */
    vmcs->exec_controls.enable_ept_encryption = 1;

    /* Launch VM */
    vm_enter(vmcs);

    return 0;
}
```

### Example 3: Nested Hypervisor

```c
/* Setup nested virtualization (L0 hypervisor) */
int setup_nested_hypervisor(void) {
    nested_virt_ctx_t ctx;

    /* Create VMCS01 for L1 guest (which will be a hypervisor) */
    ctx.vmcs01 = create_basic_vm();

    /* Enable nested VMX in L1 */
    ctx.vmcs01->exec_controls.vmcs_shadowing = 1;
    ctx.vmcs01->exec_controls.enable_vmfunc = 1;

    /* Expose VMX capability to L1 */
    expose_vmx_to_guest(ctx.vmcs01);

    /* Handle L1's VMLAUNCH (when L1 launches L2) */
    register_exit_handler(EXIT_REASON_VMLAUNCH, handle_nested_vmlaunch);

    /* Enter L1 */
    vm_enter(ctx.vmcs01);

    return 0;
}

/* Handle L1 launching L2 */
void handle_nested_vmlaunch(vmcs_t *vmcs01) {
    nested_virt_ctx_t *ctx = get_nested_context();

    /* Read L1's VMCS12 from guest memory */
    uint64_t vmcs12_gpa = vmcs01->guest_state.gpr[4];  /* a0 */
    ctx->vmcs12 = gpa_to_hva(vmcs12_gpa);

    /* Create shadow VMCS02 */
    ctx->vmcs02 = allocate_vmcs();

    /* Prepare nested VM entry */
    prepare_nested_vm_entry(ctx);

    /* Enter L2 using VMCS02 */
    vm_enter(ctx->vmcs02);
}
```

### Example 4: Paravirtualized Guest

```c
/* Paravirtualized guest OS initialization */
void pv_guest_init(void) {
    paravirt_ring_t *block_ring;
    paravirt_ring_t *net_ring;

    /* Detect paravirtualization */
    if (!detect_hypervisor())
        return;

    /* Setup console hypercalls */
    console_init_pv();

    /* Setup paravirt block device */
    block_ring = setup_paravirt_device(DEVICE_TYPE_BLOCK);
    register_block_device(block_ring);

    /* Setup paravirt network device */
    net_ring = setup_paravirt_device(DEVICE_TYPE_NET);
    register_net_device(net_ring);

    /* Use hypercalls for TLB flushing */
    tlb_flush_single_page = pv_tlb_flush_single;
    tlb_flush_all = pv_tlb_flush_all;

    printf("Paravirtualization enabled\n");
}

/* Paravirtualized TLB flush */
void pv_tlb_flush_single(uint64_t vaddr) {
    hypercall(HYPERCALL_DLX_FAST_TLB_FLUSH, vaddr, 1, 0, 0, 0);
}

void pv_tlb_flush_all(void) {
    hypercall(HYPERCALL_DLX_FAST_TLB_FLUSH, 0, 0, 1, 0, 0);
}
```

---

## Summary

The DLX nested virtualization architecture provides:

1. **Nested virtualization**: Up to 8 levels of nesting (L0-L7)
2. **Two-dimensional paging**: EPT/NPT with guest and host page tables
3. **Memory encryption**: Per-VM AES-256-GCM encryption with integrity
4. **VMCS**: Complete VM control structures with 4KB layout
5. **VM entry/exit**: Hardware-accelerated transitions
6. **Paravirtualization**: Hypercall interface for performance
7. **Virtual interrupts**: Virtual APIC with interrupt posting
8. **Shadow VMCS**: Optimized nested VM execution
9. **VPIDs**: TLB tagging to avoid flushes on context switch
10. **Security**: Integration with enclaves and CHERI capabilities

These features enable cloud environments, container orchestration, sandboxing, and secure multi-tenancy with minimal performance overhead.
