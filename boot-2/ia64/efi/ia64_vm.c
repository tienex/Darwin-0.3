/*
 * IA64 Virtual Memory Initialization
 *
 * Implements VHPT, TLB, and region-based address translation for Itanium
 */

#include "efi.h"
#include "ia64_vm.h"
#include "pal_sal.h"

/* VHPT (Virtual Hash Page Table) */
static uint64_t *vhpt_base = NULL;
static uint64_t vhpt_size = 0;

/* Page size definitions */
#define PAGE_SIZE_4KB    (1ULL << 12)
#define PAGE_SIZE_8KB    (1ULL << 13)
#define PAGE_SIZE_16KB   (1ULL << 14)
#define PAGE_SIZE_64KB   (1ULL << 16)
#define PAGE_SIZE_256KB  (1ULL << 18)
#define PAGE_SIZE_1MB    (1ULL << 20)
#define PAGE_SIZE_4MB    (1ULL << 22)
#define PAGE_SIZE_16MB   (1ULL << 24)
#define PAGE_SIZE_64MB   (1ULL << 26)
#define PAGE_SIZE_256MB  (1ULL << 28)

/* Page table entry format */
typedef struct {
    uint64_t present    : 1;    /* Page present */
    uint64_t reserved1  : 1;
    uint64_t ma         : 3;    /* Memory attribute */
    uint64_t accessed   : 1;    /* Accessed bit */
    uint64_t dirty      : 1;    /* Dirty bit */
    uint64_t pl         : 2;    /* Privilege level */
    uint64_t ar         : 3;    /* Access rights */
    uint64_t ppn        : 50;   /* Physical page number */
    uint64_t reserved2  : 2;
} IA64_PTE;

/*
 * Initialize VHPT (Virtual Hash Page Table)
 *
 * VHPT is a hardware-walked hash table for TLB misses
 * Itanium 2 supports both short and long format VHPT
 */
EFI_STATUS
ia64_init_vhpt(void)
{
    EFI_STATUS status;
    PAL_CACHE_INFO cache_info;
    uint64_t pta;  /* Page Table Address register */

    /* Determine optimal VHPT size based on memory and cache
     * Itanium 2 recommendation: 256KB - 2MB depending on L3 cache
     */

    /* Get L3 cache size */
    status = pal_get_cache_info(3, &cache_info);
    if (!EFI_ERROR(status) && cache_info.size >= 6*1024*1024) {
        /* Large L3 cache (6MB+) - use 2MB VHPT */
        vhpt_size = 2 * 1024 * 1024;
    } else {
        /* Smaller L3 - use 256KB VHPT */
        vhpt_size = 256 * 1024;
    }

    Print(L"  Allocating VHPT: %ld KB\n", vhpt_size / 1024);

    /* Allocate VHPT (must be naturally aligned) */
    status = gBS->AllocatePages(
        AllocateAnyPages,
        EfiLoaderData,
        vhpt_size / 4096,
        (EFI_PHYSICAL_ADDRESS *)&vhpt_base
    );

    if (EFI_ERROR(status)) {
        Print(L"ERROR: Failed to allocate VHPT: %r\n", status);
        return status;
    }

    /* Clear VHPT */
    gBS->SetMem(vhpt_base, vhpt_size, 0);

    /* Configure PTA (Page Table Address) register
     * PTA format:
     * - bits 0-7: VE (VHPT enable), Size
     * - bits 8-14: Reserved
     * - bits 15-63: Base address
     */
    pta = (uint64_t)vhpt_base & ~0x7FFFULL;  /* Base address */
    pta |= 0x01;  /* VE = 1 (VHPT enabled) */

    /* Set VHPT size field (log2 of size - 15) */
    uint32_t size_field = 0;
    uint64_t size = vhpt_size;
    while (size > 32768) {  /* 2^15 */
        size >>= 1;
        size_field++;
    }
    pta |= (size_field << 2);

    /* Write to cr.pta (PTA register) */
    __asm__ volatile (
        "mov cr.pta = %0\n"
        ";;\n"
        "srlz.d\n"
        :
        : "r" (pta)
        : "memory"
    );

    Print(L"  VHPT base: 0x%016llx\n", (uint64_t)vhpt_base);
    Print(L"  PTA register: 0x%016llx\n", pta);

    return EFI_SUCCESS;
}

/*
 * Set Translation Register (TR) entry
 *
 * TR provides guaranteed TLB entries (not subject to eviction)
 * Used for kernel code/data and other critical mappings
 */
EFI_STATUS
ia64_set_translation_register(
    uint32_t tr_num,
    uint64_t virt_addr,
    uint64_t phys_addr,
    uint64_t page_size,
    uint64_t permissions
)
{
    uint64_t itir;  /* Instruction Translation Insertion Register */
    uint64_t ifa;   /* Instruction Faulting Address */
    uint64_t pte;   /* Page Table Entry */

    /* Validate TR number (0-7) */
    if (tr_num >= 8) {
        return EFI_INVALID_PARAMETER;
    }

    /* Build ITIR (page size and key) */
    uint32_t ps = 0;  /* Page size field */
    switch (page_size) {
        case IA64_PAGE_SIZE_4KB:   ps = 12; break;
        case IA64_PAGE_SIZE_8KB:   ps = 13; break;
        case IA64_PAGE_SIZE_16KB:  ps = 14; break;
        case IA64_PAGE_SIZE_64KB:  ps = 16; break;
        case IA64_PAGE_SIZE_256KB: ps = 18; break;
        case IA64_PAGE_SIZE_1MB:   ps = 20; break;
        case IA64_PAGE_SIZE_4MB:   ps = 22; break;
        case IA64_PAGE_SIZE_16MB:  ps = 24; break;
        case IA64_PAGE_SIZE_256MB: ps = 28; break;
        default:
            return EFI_INVALID_PARAMETER;
    }

    itir = (ps << 2) | (0ULL << 8);  /* ps and protection key */

    /* Build PTE */
    pte = (phys_addr & ~0xFFFULL) | permissions | 0x01;  /* Present bit */

    /* Set virtual address */
    ifa = virt_addr;

    /* Insert TR entry
     * For instruction TR (iTR), use itr.i instruction
     * For data TR (dTR), use itr.d instruction
     */
    if (permissions & IA64_TR_EXECUTE) {
        /* Instruction TR */
        __asm__ volatile (
            "mov cr.itir = %1\n"
            "mov cr.ifa = %2\n"
            ";;\n"
            "itr.i itr[%0] = %3\n"
            ";;\n"
            "srlz.i\n"
            :
            : "r" (tr_num), "r" (itir), "r" (ifa), "r" (pte)
            : "memory"
        );
    } else {
        /* Data TR */
        __asm__ volatile (
            "mov cr.itir = %1\n"
            "mov cr.ifa = %2\n"
            ";;\n"
            "itr.d dtr[%0] = %3\n"
            ";;\n"
            "srlz.d\n"
            :
            : "r" (tr_num), "r" (itir), "r" (ifa), "r" (pte)
            : "memory"
        );
    }

    return EFI_SUCCESS;
}

/*
 * Insert TLB entry for dynamic mappings
 */
EFI_STATUS
ia64_insert_tlb(
    uint64_t virt_addr,
    uint64_t phys_addr,
    uint64_t page_size,
    uint64_t permissions
)
{
    uint64_t itir, ifa, pte;
    uint32_t ps = 14;  /* Default 16KB */

    /* Build ITIR, IFA, PTE (same as TR) */
    itir = (ps << 2);
    ifa = virt_addr;
    pte = (phys_addr & ~0x3FFFULL) | permissions | 0x01;

    /* Insert into TC (Translation Cache) using itc instruction */
    if (permissions & IA64_TR_EXECUTE) {
        __asm__ volatile (
            "mov cr.itir = %0\n"
            "mov cr.ifa = %1\n"
            ";;\n"
            "itc.i %2\n"
            ";;\n"
            "srlz.i\n"
            :
            : "r" (itir), "r" (ifa), "r" (pte)
            : "memory"
        );
    } else {
        __asm__ volatile (
            "mov cr.itir = %0\n"
            "mov cr.ifa = %1\n"
            ";;\n"
            "itc.d %2\n"
            ";;\n"
            "srlz.d\n"
            :
            : "r" (itir), "r" (ifa), "r" (pte)
            : "memory"
        );
    }

    return EFI_SUCCESS;
}

/*
 * Flush TLB
 */
void
ia64_flush_tlb(void)
{
    /* Flush TLB via PAL */
    pal_cache_flush_all();

    /* Serialize */
    __asm__ volatile (
        "srlz.i\n"
        ";;\n"
        "srlz.d\n"
        ::: "memory"
    );
}

/*
 * Enable address translation
 */
void
ia64_enable_translation(void)
{
    uint64_t psr;

    /* Read PSR */
    __asm__ volatile ("mov %0 = psr" : "=r" (psr));

    /* Set PSR.dt (data translation) and PSR.it (instruction translation) */
    psr |= (1ULL << 17);  /* PSR.dt */
    psr |= (1ULL << 36);  /* PSR.it */

    /* Write PSR */
    __asm__ volatile (
        "mov cr.ipsr = %0\n"
        ";;\n"
        "rfi\n"
        :
        : "r" (psr)
        : "memory"
    );
}
