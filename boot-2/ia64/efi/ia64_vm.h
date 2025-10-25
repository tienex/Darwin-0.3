/*
 * IA64 Virtual Memory Definitions
 */

#ifndef _IA64_VM_H_
#define _IA64_VM_H_

#include <stdint.h>

/* Page sizes supported by IA64 */
#define IA64_PAGE_SIZE_4KB      0
#define IA64_PAGE_SIZE_8KB      1
#define IA64_PAGE_SIZE_16KB     2
#define IA64_PAGE_SIZE_64KB     3
#define IA64_PAGE_SIZE_256KB    4
#define IA64_PAGE_SIZE_1MB      5
#define IA64_PAGE_SIZE_4MB      6
#define IA64_PAGE_SIZE_16MB     7
#define IA64_PAGE_SIZE_64MB     8
#define IA64_PAGE_SIZE_256MB    9

/* TR/TLB permissions */
#define IA64_TR_READ            (1ULL << 0)
#define IA64_TR_WRITE           (1ULL << 1)
#define IA64_TR_EXECUTE         (1ULL << 2)
#define IA64_TR_ACCESSED        (1ULL << 5)
#define IA64_TR_DIRTY           (1ULL << 6)

/* Memory attributes */
#define IA64_MA_WB              0  /* Write-back cacheable */
#define IA64_MA_UC              4  /* Uncacheable */
#define IA64_MA_WC              5  /* Write-coalescing */

/* Function prototypes */
EFI_STATUS ia64_init_vhpt(void);
EFI_STATUS ia64_set_translation_register(uint32_t tr_num, uint64_t virt_addr,
                                         uint64_t phys_addr, uint64_t page_size,
                                         uint64_t permissions);
EFI_STATUS ia64_insert_tlb(uint64_t virt_addr, uint64_t phys_addr,
                           uint64_t page_size, uint64_t permissions);
void ia64_flush_tlb(void);
void ia64_enable_translation(void);

#endif /* _IA64_VM_H_ */
