/*
 * PAL/SAL Definitions for IA64
 */

#ifndef _PAL_SAL_H_
#define _PAL_SAL_H_

#include <stdint.h>

/* SAL System Table GUID */
#define SAL_SYSTEM_TABLE_GUID \
    { 0xeb9d2d32, 0x2d88, 0x11d3, \
      { 0x9a, 0x16, 0x0, 0x90, 0x27, 0x3f, 0xc1, 0x4d } }

/* SAL System Table structure */
typedef struct {
    char     signature[4];      /* "SST_" */
    uint32_t length;
    uint8_t  sal_rev_minor;
    uint8_t  sal_rev_major;
    uint16_t entry_count;
    uint8_t  checksum;
    uint8_t  reserved[7];
    uint16_t sal_a_version;
    uint16_t sal_b_version;
    char     oem_id[32];
    char     product_id[32];
    uint64_t reserved2[2];
    uint64_t sal_a;             /* SAL_A entry point (physical) */
    uint64_t sal_b;             /* SAL_B entry point */
    uint64_t pal_proc;          /* PAL entry point (physical) */
} SAL_SYSTEM_TABLE;

/* SAL return structure */
typedef struct {
    int64_t  status;
    uint64_t r9;
    uint64_t r10;
    uint64_t r11;
    uint64_t r14;
    uint64_t r15;
    uint64_t r16;
    uint64_t r17;
} SAL_RETURN;

/* PAL procedure indices */
#define PAL_VERSION                 0
#define PAL_CACHE_FLUSH             1
#define PAL_CACHE_INFO              2
#define PAL_CACHE_SUMMARY           4
#define PAL_MEM_ATTRIB              5
#define PAL_PROC_GET_FEATURES       17
#define PAL_PROC_SET_FEATURES       18
#define PAL_VM_SUMMARY              8
#define PAL_VM_PAGE_SIZE            34

/* SAL function indices */
#define SAL_GET_VERSION             0x00
#define SAL_SET_VECTORS             0x01
#define SAL_GET_STATE_INFO          0x02
#define SAL_GET_STATE_INFO_SIZE     0x03
#define SAL_CLEAR_STATE_INFO        0x04
#define SAL_MC_RENDEZ               0x05
#define SAL_MC_SET_PARAMS           0x06
#define SAL_CACHE_FLUSH             0x08
#define SAL_CACHE_INIT              0x09
#define SAL_PCI_CONFIG_READ         0x0a
#define SAL_PCI_CONFIG_WRITE        0x0b
#define SAL_FREQ_BASE               0x0d
#define SAL_UPDATE_PAL              0x0e
#define SAL_NUM_PROCESSORS          0x0f

/* Cache information structure */
typedef struct {
    uint32_t size;
    uint8_t  line_size;
    uint8_t  associativity;
} PAL_CACHE_INFO;

/* Function prototypes */
EFI_STATUS init_pal_sal(void *boot_info);
uint32_t sal_get_cpu_count(void);
EFI_STATUS pal_get_cache_info(uint32_t level, PAL_CACHE_INFO *info);
void pal_cache_flush_all(void);
EFI_STATUS pal_set_features(uint64_t features);

#endif /* _PAL_SAL_H_ */
