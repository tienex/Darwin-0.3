/*
 * Copyright (c) 1999 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * Portions Copyright (c) 1999 Apple Computer, Inc.  All Rights
 * Reserved.  This file contains Original Code and/or Modifications of
 * Original Code as defined in and that are subject to the Apple Public
 * Source License Version 1.1 (the "License").
 *
 * @APPLE_LICENSE_HEADER_END@
 */

/*
 * IA64 EFI Bootloader - Main Entry Point
 *
 * This is the EFI application entry point for the Darwin IA64 bootloader.
 * It runs in EFI environment and loads the Darwin kernel.
 */

#include "efi.h"
#include "efi_support.h"
#include "pal_sal.h"
#include "ia64_vm.h"

/* EFI System Table and Boot Services */
EFI_SYSTEM_TABLE *gST = NULL;
EFI_BOOT_SERVICES *gBS = NULL;
EFI_RUNTIME_SERVICES *gRT = NULL;
EFI_HANDLE gImageHandle = NULL;

/* Darwin kernel load address (Region 5 - kernel virtual) */
#define KERNEL_LOAD_ADDRESS  0xA000000000000000ULL

/* Boot information passed to kernel */
typedef struct {
    uint64_t    mem_size;           /* Total memory size */
    uint64_t    pal_entry;          /* PAL entry point */
    uint64_t    sal_entry;          /* SAL entry point */
    uint64_t    acpi_rsdp;          /* ACPI RSDP pointer */
    uint64_t    smbios_table;       /* SMBIOS table pointer */
    uint32_t    cpu_count;          /* Number of CPUs */
    uint32_t    cpu_family;         /* Itanium family (1=Itanium, 2=Itanium2) */
    uint64_t    cpu_features;       /* CPU feature flags */
    uint64_t    kernel_entry;       /* Kernel entry point */
    uint64_t    initrd_base;        /* Initial ramdisk base */
    uint64_t    initrd_size;        /* Initial ramdisk size */
} IA64_BOOT_INFO;

/*
 * Detect Itanium processor family and features
 */
static void
detect_cpu_info(IA64_BOOT_INFO *boot_info)
{
    uint64_t cpuid[4];

    /* Read CPUID register 3 (processor info) */
    __asm__ volatile (
        "mov r8 = 3\n"
        ";;\n"
        "mov %0 = cpuid[r8]\n"
        : "=r" (cpuid[0])
        :
        : "r8"
    );

    /* Extract family, model, revision */
    boot_info->cpu_family = (cpuid[0] >> 8) & 0xFF;

    /* Itanium 2 family features:
     * - Madison: 130nm, 1.5GHz, 6MB L3
     * - Montecito: 90nm, dual-core, 12MB L3
     * - Montvale: 90nm, dual-core, 12MB L3, improved
     * - Tukwila: 65nm, quad-core, 24MB L3
     */
    if (boot_info->cpu_family >= 2) {
        Print(L"Detected Itanium 2 processor\n");

        /* Check for VT-i (hardware virtualization) */
        uint64_t feature_reg;
        __asm__ volatile ("mov %0 = cpuid[4]" : "=r" (feature_reg));
        if (feature_reg & (1ULL << 2)) {
            boot_info->cpu_features |= (1ULL << 0); /* VT-i support */
            Print(L"  VT-i (Virtualization) supported\n");
        }
    }

    /* Count CPUs via SAL */
    boot_info->cpu_count = sal_get_cpu_count();
    Print(L"  CPU count: %d\n", boot_info->cpu_count);
}

/*
 * Initialize IA64 virtual memory system
 */
static EFI_STATUS
init_virtual_memory(IA64_BOOT_INFO *boot_info)
{
    Print(L"Initializing IA64 virtual memory...\n");

    /* Configure Translation Registers (TR) for kernel region
     * TR entries provide guaranteed TLB entries for critical code/data
     */

    /* TR[0] - Kernel code (Region 5) */
    ia64_set_translation_register(
        0,                          /* TR index */
        KERNEL_LOAD_ADDRESS,        /* Virtual address */
        0,                          /* Physical address (identity mapped initially) */
        IA64_PAGE_SIZE_16MB,        /* Page size */
        IA64_TR_EXECUTE | IA64_TR_READ  /* Permissions */
    );

    /* TR[1] - Kernel data (Region 5) */
    ia64_set_translation_register(
        1,
        KERNEL_LOAD_ADDRESS + 0x01000000ULL,
        0x01000000ULL,
        IA64_PAGE_SIZE_16MB,
        IA64_TR_READ | IA64_TR_WRITE
    );

    /* Initialize VHPT (Virtual Hash Page Table) */
    if (ia64_init_vhpt() != EFI_SUCCESS) {
        Print(L"ERROR: Failed to initialize VHPT\n");
        return EFI_LOAD_ERROR;
    }

    Print(L"  Using 16KB page size\n");
    Print(L"  VHPT initialized\n");
    Print(L"  TR[0-1] configured for kernel\n");

    return EFI_SUCCESS;
}

/*
 * Load Darwin kernel from filesystem
 */
static EFI_STATUS
load_darwin_kernel(IA64_BOOT_INFO *boot_info)
{
    EFI_STATUS status;
    void *kernel_image;
    uint64_t kernel_size;

    Print(L"Loading Darwin kernel...\n");

    /* Load kernel file (mach_kernel or mach_kernel.ia64) */
    status = efi_load_file(L"\\mach_kernel.ia64", &kernel_image, &kernel_size);
    if (EFI_ERROR(status)) {
        /* Try default name */
        status = efi_load_file(L"\\mach_kernel", &kernel_image, &kernel_size);
        if (EFI_ERROR(status)) {
            Print(L"ERROR: Failed to load kernel: %r\n", status);
            return status;
        }
    }

    Print(L"  Kernel size: %ld bytes\n", kernel_size);

    /* Parse Mach-O header and relocate to Region 5 */
    status = relocate_macho_kernel(kernel_image, kernel_size,
                                    KERNEL_LOAD_ADDRESS,
                                    &boot_info->kernel_entry);
    if (EFI_ERROR(status)) {
        Print(L"ERROR: Failed to relocate kernel: %r\n", status);
        return status;
    }

    Print(L"  Kernel entry: 0x%016llx\n", boot_info->kernel_entry);

    return EFI_SUCCESS;
}

/*
 * Main EFI bootloader entry point
 */
EFI_STATUS
EFIAPI
efi_main(
    EFI_HANDLE ImageHandle,
    EFI_SYSTEM_TABLE *SystemTable
)
{
    EFI_STATUS status;
    IA64_BOOT_INFO boot_info;
    void (*kernel_entry)(IA64_BOOT_INFO *);

    /* Initialize global EFI pointers */
    gImageHandle = ImageHandle;
    gST = SystemTable;
    gBS = SystemTable->BootServices;
    gRT = SystemTable->RuntimeServices;

    /* Clear boot info structure */
    gBS->SetMem(&boot_info, sizeof(boot_info), 0);

    /* Display banner */
    gST->ConOut->ClearScreen(gST->ConOut);
    Print(L"\n");
    Print(L"Darwin IA64 EFI Bootloader\n");
    Print(L"==========================\n");
    Print(L"\n");

    /* Initialize PAL and SAL */
    Print(L"Initializing PAL/SAL firmware interfaces...\n");
    status = init_pal_sal(&boot_info);
    if (EFI_ERROR(status)) {
        Print(L"ERROR: PAL/SAL initialization failed: %r\n", status);
        goto error;
    }

    /* Detect CPU */
    detect_cpu_info(&boot_info);

    /* Get memory map */
    status = efi_get_memory_info(&boot_info);
    if (EFI_ERROR(status)) {
        Print(L"ERROR: Failed to get memory info: %r\n", status);
        goto error;
    }
    Print(L"  Total memory: %ld MB\n", boot_info.mem_size / (1024*1024));

    /* Find ACPI tables */
    boot_info.acpi_rsdp = efi_find_acpi_rsdp();
    if (boot_info.acpi_rsdp) {
        Print(L"  Found ACPI RSDP at 0x%016llx\n", boot_info.acpi_rsdp);
    }

    /* Initialize virtual memory */
    status = init_virtual_memory(&boot_info);
    if (EFI_ERROR(status)) {
        goto error;
    }

    /* Load Darwin kernel */
    status = load_darwin_kernel(&boot_info);
    if (EFI_ERROR(status)) {
        goto error;
    }

    /* Exit EFI boot services */
    Print(L"\nExiting EFI boot services...\n");
    status = efi_exit_boot_services();
    if (EFI_ERROR(status)) {
        Print(L"ERROR: Failed to exit boot services: %r\n", status);
        goto error;
    }

    /* Jump to kernel */
    Print(L"Starting Darwin kernel...\n\n");

    kernel_entry = (void (*)(IA64_BOOT_INFO *))boot_info.kernel_entry;
    kernel_entry(&boot_info);

    /* Should never return */
    return EFI_SUCCESS;

error:
    Print(L"\nPress any key to exit...\n");
    efi_wait_for_key();
    return status;
}
