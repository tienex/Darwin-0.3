/*
 * Minimal EFI Definitions for IA64 Bootloader
 */

#ifndef _EFI_H_
#define _EFI_H_

#include <stdint.h>

/* Basic types */
typedef uint64_t UINTN;
typedef int64_t INTN;
typedef uint16_t CHAR16;
typedef uint8_t BOOLEAN;
typedef void VOID;

#define TRUE  1
#define FALSE 0

/* EFI Status codes */
typedef UINTN EFI_STATUS;
typedef UINTN EFI_PHYSICAL_ADDRESS;
typedef void *EFI_HANDLE;
typedef void *EFI_EVENT;

#define EFI_SUCCESS               0
#define EFI_LOAD_ERROR            1
#define EFI_INVALID_PARAMETER     2
#define EFI_UNSUPPORTED           3
#define EFI_NOT_FOUND             14
#define EFI_ERROR(Status)         (((INTN)(Status)) < 0)

/* EFI GUID */
typedef struct {
    uint32_t Data1;
    uint16_t Data2;
    uint16_t Data3;
    uint8_t  Data4[8];
} EFI_GUID;

/* EFI Table Header */
typedef struct {
    uint64_t Signature;
    uint32_t Revision;
    uint32_t HeaderSize;
    uint32_t CRC32;
    uint32_t Reserved;
} EFI_TABLE_HEADER;

/* EFI Configuration Table */
typedef struct {
    EFI_GUID VendorGuid;
    void     *VendorTable;
} EFI_CONFIGURATION_TABLE;

/* Memory types */
typedef enum {
    EfiReservedMemoryType,
    EfiLoaderCode,
    EfiLoaderData,
    EfiBootServicesCode,
    EfiBootServicesData,
    EfiRuntimeServicesCode,
    EfiRuntimeServicesData,
    EfiConventionalMemory,
    EfiUnusableMemory,
    EfiACPIReclaimMemory,
    EfiACPIMemoryNVS,
    EfiMemoryMappedIO,
    EfiMemoryMappedIOPortSpace,
    EfiPalCode,
    EfiMaxMemoryType
} EFI_MEMORY_TYPE;

/* Memory allocation type */
typedef enum {
    AllocateAnyPages,
    AllocateMaxAddress,
    AllocateAddress,
    MaxAllocateType
} EFI_ALLOCATE_TYPE;

/* Forward declarations */
typedef struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL;
typedef struct _EFI_BOOT_SERVICES EFI_BOOT_SERVICES;
typedef struct _EFI_RUNTIME_SERVICES EFI_RUNTIME_SERVICES;
typedef struct _EFI_SYSTEM_TABLE EFI_SYSTEM_TABLE;

/* Simple Text Output Protocol */
struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL {
    void *Reset;
    EFI_STATUS (*OutputString)(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
                               CHAR16 *String);
    void *TestString;
    void *QueryMode;
    void *SetMode;
    void *SetAttribute;
    EFI_STATUS (*ClearScreen)(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This);
    void *SetCursorPosition;
    void *EnableCursor;
    void *Mode;
};

/* Boot Services Table */
struct _EFI_BOOT_SERVICES {
    EFI_TABLE_HEADER Hdr;
    void *RaiseTPL;
    void *RestoreTPL;
    EFI_STATUS (*AllocatePages)(EFI_ALLOCATE_TYPE Type,
                                EFI_MEMORY_TYPE MemoryType,
                                UINTN Pages,
                                EFI_PHYSICAL_ADDRESS *Memory);
    void *FreePages;
    void *GetMemoryMap;
    void *AllocatePool;
    void *FreePool;
    void *CreateEvent;
    void *SetTimer;
    void *WaitForEvent;
    void *SignalEvent;
    void *CloseEvent;
    void *CheckEvent;
    void *InstallProtocolInterface;
    void *ReinstallProtocolInterface;
    void *UninstallProtocolInterface;
    void *HandleProtocol;
    void *Reserved;
    void *RegisterProtocolNotify;
    void *LocateHandle;
    void *LocateDevicePath;
    void *InstallConfigurationTable;
    void *LoadImage;
    void *StartImage;
    void *Exit;
    void *UnloadImage;
    void *ExitBootServices;
    void *GetNextMonotonicCount;
    void *Stall;
    void (*SetMem)(void *Buffer, UINTN Size, uint8_t Value);
    void *CopyMem;
    void *SetWatchdogTimer;
};

/* Runtime Services Table */
struct _EFI_RUNTIME_SERVICES {
    EFI_TABLE_HEADER Hdr;
    /* Time services */
    void *GetTime;
    void *SetTime;
    void *GetWakeupTime;
    void *SetWakeupTime;
    /* Virtual memory services */
    void *SetVirtualAddressMap;
    void *ConvertPointer;
    /* Variable services */
    void *GetVariable;
    void *GetNextVariableName;
    void *SetVariable;
    /* Other */
    void *GetNextHighMonotonicCount;
    void *ResetSystem;
};

/* System Table */
struct _EFI_SYSTEM_TABLE {
    EFI_TABLE_HEADER                Hdr;
    CHAR16                          *FirmwareVendor;
    uint32_t                        FirmwareRevision;
    EFI_HANDLE                      ConsoleInHandle;
    void                            *ConIn;
    EFI_HANDLE                      ConsoleOutHandle;
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *ConOut;
    EFI_HANDLE                      StandardErrorHandle;
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *StdErr;
    EFI_RUNTIME_SERVICES            *RuntimeServices;
    EFI_BOOT_SERVICES               *BootServices;
    UINTN                           NumberOfTableEntries;
    EFI_CONFIGURATION_TABLE         *ConfigurationTable;
};

/* EFI calling convention */
#define EFIAPI

#endif /* _EFI_H_ */
