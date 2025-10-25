/*
 * PAL (Processor Abstraction Layer) and SAL (System Abstraction Layer) Support
 *
 * PAL provides processor-specific firmware services
 * SAL provides platform-specific firmware services
 */

#include "efi.h"
#include "pal_sal.h"

/* PAL and SAL entry points (discovered from EFI) */
static uint64_t pal_entry_point = 0;
static uint64_t sal_entry_point = 0;

/* EFI GUID for SAL System Table */
static EFI_GUID SalSystemTableGuid = SAL_SYSTEM_TABLE_GUID;

/*
 * Make PAL call
 * Uses static registers (r8-r11 for input, r8-r11 for output)
 */
static inline void
pal_call(uint64_t index, uint64_t arg1, uint64_t arg2, uint64_t arg3,
         uint64_t *ret0, uint64_t *ret1, uint64_t *ret2, uint64_t *ret3)
{
    struct {
        uint64_t status;
        uint64_t v0;
        uint64_t v1;
        uint64_t v2;
    } result;

    typedef void (*pal_func_t)(uint64_t, uint64_t, uint64_t, uint64_t,
                               uint64_t, uint64_t, uint64_t, uint64_t);

    pal_func_t pal_proc = (pal_func_t)pal_entry_point;

    /* Call PAL with physical mode enabled */
    __asm__ volatile (
        "mov r8 = %4\n"      /* PAL procedure index */
        "mov r9 = %5\n"      /* arg1 */
        "mov r10 = %6\n"     /* arg2 */
        "mov r11 = %7\n"     /* arg3 */
        ";;\n"
        "mov b6 = %8\n"      /* PAL entry point */
        ";;\n"
        "br.call.sptk.many rp = b6\n"
        ";;\n"
        "mov %0 = r8\n"      /* status */
        "mov %1 = r9\n"      /* return value 0 */
        "mov %2 = r10\n"     /* return value 1 */
        "mov %3 = r11\n"     /* return value 2 */
        : "=r" (result.status), "=r" (result.v0),
          "=r" (result.v1), "=r" (result.v2)
        : "r" (index), "r" (arg1), "r" (arg2), "r" (arg3),
          "r" (pal_proc)
        : "r8", "r9", "r10", "r11", "b6", "rp", "memory"
    );

    if (ret0) *ret0 = result.status;
    if (ret1) *ret1 = result.v0;
    if (ret2) *ret2 = result.v1;
    if (ret3) *ret3 = result.v2;
}

/*
 * Make SAL call
 */
static inline void
sal_call(uint64_t index, uint64_t arg1, uint64_t arg2, uint64_t arg3,
         uint64_t arg4, uint64_t arg5, uint64_t arg6, uint64_t arg7,
         SAL_RETURN *result)
{
    typedef void (*sal_func_t)(uint64_t, uint64_t, uint64_t, uint64_t,
                               uint64_t, uint64_t, uint64_t, uint64_t);

    sal_func_t sal_proc = (sal_func_t)sal_entry_point;

    __asm__ volatile (
        "mov r8 = %8\n"      /* SAL function index */
        "mov r9 = %9\n"
        "mov r10 = %10\n"
        "mov r11 = %11\n"
        "mov r14 = %12\n"
        "mov r15 = %13\n"
        "mov r16 = %14\n"
        "mov r17 = %15\n"
        ";;\n"
        "mov b6 = %16\n"     /* SAL entry point */
        ";;\n"
        "br.call.sptk.many rp = b6\n"
        ";;\n"
        "mov %0 = r8\n"
        "mov %1 = r9\n"
        "mov %2 = r10\n"
        "mov %3 = r11\n"
        "mov %4 = r14\n"
        "mov %5 = r15\n"
        "mov %6 = r16\n"
        "mov %7 = r17\n"
        : "=r" (result->status), "=r" (result->r9),
          "=r" (result->r10), "=r" (result->r11),
          "=r" (result->r14), "=r" (result->r15),
          "=r" (result->r16), "=r" (result->r17)
        : "r" (index), "r" (arg1), "r" (arg2), "r" (arg3),
          "r" (arg4), "r" (arg5), "r" (arg6), "r" (arg7),
          "r" (sal_proc)
        : "r8", "r9", "r10", "r11", "r14", "r15", "r16", "r17",
          "b6", "rp", "memory"
    );
}

/*
 * Initialize PAL and SAL from EFI
 */
EFI_STATUS
init_pal_sal(IA64_BOOT_INFO *boot_info)
{
    EFI_STATUS status;
    SAL_SYSTEM_TABLE *sal_table;
    uint64_t pal_version, pal_vendor;
    uint64_t sal_version;

    /* Find SAL System Table in EFI configuration tables */
    sal_table = NULL;
    for (UINTN i = 0; i < gST->NumberOfTableEntries; i++) {
        if (CompareGuid(&gST->ConfigurationTable[i].VendorGuid,
                        &SalSystemTableGuid)) {
            sal_table = (SAL_SYSTEM_TABLE *)gST->ConfigurationTable[i].VendorTable;
            break;
        }
    }

    if (!sal_table) {
        Print(L"ERROR: SAL System Table not found\n");
        return EFI_NOT_FOUND;
    }

    /* Validate SAL table signature */
    if (CompareMem(sal_table->signature, "SST_", 4) != 0) {
        Print(L"ERROR: Invalid SAL table signature\n");
        return EFI_LOAD_ERROR;
    }

    /* Extract SAL and PAL entry points */
    sal_entry_point = sal_table->sal_a;
    pal_entry_point = sal_table->pal_proc;

    boot_info->sal_entry = sal_entry_point;
    boot_info->pal_entry = pal_entry_point;

    Print(L"  SAL entry point: 0x%016llx\n", sal_entry_point);
    Print(L"  PAL entry point: 0x%016llx\n", pal_entry_point);

    /* Get PAL version */
    pal_call(PAL_VERSION, 0, 0, 0,
             NULL, &pal_version, &pal_vendor, NULL);
    Print(L"  PAL version: %d.%d\n",
          (pal_version >> 8) & 0xFF,
          pal_version & 0xFF);

    /* Get SAL version via SAL_GET_VERSION */
    SAL_RETURN ret;
    sal_call(SAL_GET_VERSION, 0, 0, 0, 0, 0, 0, 0, &ret);
    if (ret.status == 0) {
        sal_version = ret.r9;
        Print(L"  SAL version: %d.%d\n",
              (sal_version >> 8) & 0xFF,
              sal_version & 0xFF);
    }

    return EFI_SUCCESS;
}

/*
 * Get CPU count via SAL
 */
uint32_t
sal_get_cpu_count(void)
{
    SAL_RETURN ret;

    /* SAL_NUM_PROCESSORS returns processor count */
    sal_call(SAL_NUM_PROCESSORS, 0, 0, 0, 0, 0, 0, 0, &ret);

    if (ret.status == 0) {
        return (uint32_t)ret.r9;  /* Number of processors */
    }

    return 1;  /* Default to 1 if call fails */
}

/*
 * Get cache information via PAL
 */
EFI_STATUS
pal_get_cache_info(uint32_t level, PAL_CACHE_INFO *info)
{
    uint64_t status, cache_info1, cache_info2;

    /* PAL_CACHE_INFO - get cache characteristics */
    pal_call(PAL_CACHE_INFO, level, 0, 0,
             &status, &cache_info1, &cache_info2, NULL);

    if (status != 0) {
        return EFI_NOT_FOUND;
    }

    /* Parse cache info (simplified) */
    info->size = cache_info1 & 0xFFFFFFFF;
    info->line_size = (cache_info1 >> 32) & 0xFF;
    info->associativity = (cache_info2 >> 8) & 0xFF;

    return EFI_SUCCESS;
}

/*
 * Flush all caches via PAL
 */
void
pal_cache_flush_all(void)
{
    /* PAL_CACHE_FLUSH - flush all caches */
    pal_call(PAL_CACHE_FLUSH, 3, 0, 0, NULL, NULL, NULL, NULL);
}

/*
 * Set processor features via PAL
 */
EFI_STATUS
pal_set_features(uint64_t features)
{
    uint64_t status;

    /* PAL_PROC_SET_FEATURES - set processor features */
    pal_call(PAL_PROC_SET_FEATURES, features, 0, 0,
             &status, NULL, NULL, NULL);

    return (status == 0) ? EFI_SUCCESS : EFI_UNSUPPORTED;
}
