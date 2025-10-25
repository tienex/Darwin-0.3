/*
 * EFI Support Functions
 */

#ifndef _EFI_SUPPORT_H_
#define _EFI_SUPPORT_H_

#include "efi.h"

/* EFI global pointers (defined in efi_main.c) */
extern EFI_SYSTEM_TABLE *gST;
extern EFI_BOOT_SERVICES *gBS;
extern EFI_RUNTIME_SERVICES *gRT;
extern EFI_HANDLE gImageHandle;

/* Simplified Print function (using ConOut) */
#define Print(fmt, ...) \
    gST->ConOut->OutputString(gST->ConOut, fmt)

/* Helper macros */
#define CompareGuid(g1, g2) \
    (CompareMem((g1), (g2), sizeof(EFI_GUID)) == 0)

/* Function prototypes */
EFI_STATUS efi_load_file(CHAR16 *path, void **buffer, uint64_t *size);
EFI_STATUS efi_get_memory_info(void *boot_info);
uint64_t efi_find_acpi_rsdp(void);
EFI_STATUS efi_exit_boot_services(void);
void efi_wait_for_key(void);
EFI_STATUS relocate_macho_kernel(void *image, uint64_t size,
                                 uint64_t load_addr, uint64_t *entry);

/* Memory comparison */
static inline int
CompareMem(const void *s1, const void *s2, UINTN n)
{
    const unsigned char *p1 = s1, *p2 = s2;
    while (n--) {
        if (*p1 != *p2)
            return *p1 - *p2;
        p1++;
        p2++;
    }
    return 0;
}

#endif /* _EFI_SUPPORT_H_ */
