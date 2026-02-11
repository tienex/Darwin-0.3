# Chapter 5: Binary Formats

> **Note**: This chapter describes system-level binary format support, not ISA instructions. Binary loaders are implemented by operating systems and simulators, not by the CPU hardware.

## 5.1 Supported Binary Formats

DLX systems support three major executable formats:
- **ELF** (Executable and Linkable Format) - Unix/Linux standard
- **PE/COFF** (Portable Executable / Common Object File Format) - Windows standard
- **Mach-O** (Mach Object) - macOS/iOS standard

## 5.2 Memory Layout

### DLX32 Virtual Memory Layout
```c
#define USER_TEXT_BASE      0x00010000  /* Code segment */
#define USER_DATA_BASE      0x10000000  /* Data segment */
#define USER_HEAP_BASE      0x20000000  /* Heap (grows up) */
#define USER_MMAP_BASE      0x40000000  /* Memory mapped files */
#define USER_STACK_TOP      0x7FFFFFFF  /* Stack (grows down) */
#define KERNEL_BASE         0x80000000  /* Kernel space */
```

### DLX64 Virtual Memory Layout
```c
#define USER_TEXT_BASE_64   0x0000000000400000ULL
#define USER_DATA_BASE_64   0x0000000010000000ULL
#define USER_HEAP_BASE_64   0x0000000020000000ULL
#define USER_MMAP_BASE_64   0x0000700000000000ULL
#define USER_STACK_TOP_64   0x00007FFFFFFFF000ULL
#define KERNEL_BASE_64      0xFFFF800000000000ULL
```

## 5.3 ELF Format

### ELF Header (64-bit)
```c
typedef struct {
    uint8_t  e_ident[16];       /* Magic: 0x7F 'E' 'L' 'F' */
    uint16_t e_type;            /* ET_EXEC, ET_DYN, ET_REL */
    uint16_t e_machine;         /* EM_DLX (0x9999) */
    uint32_t e_version;         /* EV_CURRENT (1) */
    uint64_t e_entry;           /* Entry point address */
    uint64_t e_phoff;           /* Program header offset */
    uint64_t e_shoff;           /* Section header offset */
    uint32_t e_flags;           /* DLX-specific flags */
    uint16_t e_ehsize;          /* ELF header size */
    uint16_t e_phentsize;       /* Program header size */
    uint16_t e_phnum;           /* Program header count */
    uint16_t e_shentsize;       /* Section header size */
    uint16_t e_shnum;           /* Section header count */
    uint16_t e_shstrndx;        /* String table index */
} Elf64_Ehdr;
```

### DLX-Specific ELF Flags
```c
#define EF_DLX_ABI_2RING    0x00000001  /* Uses 2-ring ABI */
#define EF_DLX_ABI_4RING    0x00000002  /* Uses 4-ring ABI */
#define EF_DLX_ABI_BANK     0x00000004  /* Uses register banking */
#define EF_DLX_SOFT_FLOAT   0x00000008  /* Software floating point */
#define EF_DLX_HARD_FLOAT   0x00000010  /* Hardware floating point */
#define EF_DLX_VECTOR       0x00000020  /* Uses vector extensions */
#define EF_DLX_CRYPTO       0x00000040  /* Uses crypto extensions */
#define EF_DLX_QUANTUM      0x00000080  /* Uses quantum extensions */
#define EF_DLX_BIG_ENDIAN   0x00000100  /* Big-endian code */
#define EF_DLX_LITTLE_ENDIAN 0x00000200 /* Little-endian code */
```

### DLX Relocation Types
```c
#define R_DLX_NONE          0   /* No relocation */
#define R_DLX_32            1   /* Direct 32-bit */
#define R_DLX_64            2   /* Direct 64-bit */
#define R_DLX_PC32          3   /* PC-relative 32-bit */
#define R_DLX_PC64          4   /* PC-relative 64-bit */
#define R_DLX_GOT32         5   /* 32-bit GOT entry */
#define R_DLX_PLT32         6   /* 32-bit PLT entry */
#define R_DLX_COPY          7   /* Copy symbol at runtime */
#define R_DLX_GLOB_DAT      8   /* Create GOT entry */
#define R_DLX_JUMP_SLOT     9   /* Create PLT entry */
#define R_DLX_RELATIVE      10  /* Adjust by program base */
#define R_DLX_TLS_DTPMOD64  11  /* TLS module ID */
#define R_DLX_TLS_DTPOFF64  12  /* TLS offset */
#define R_DLX_TLS_TPOFF64   13  /* TLS offset from TP */
#define R_DLX_HI20          14  /* High 20 bits */
#define R_DLX_LO12          15  /* Low 12 bits */
```

## 5.4 PE/COFF Format

### COFF Header
```c
typedef struct {
    uint16_t Machine;           /* IMAGE_FILE_MACHINE_DLX (0x9999) */
    uint16_t NumberOfSections;
    uint32_t TimeDateStamp;
    uint32_t PointerToSymbolTable;
    uint32_t NumberOfSymbols;
    uint16_t SizeOfOptionalHeader;
    uint16_t Characteristics;
} IMAGE_FILE_HEADER;
```

### PE Optional Header
```c
typedef struct {
    uint16_t Magic;             /* PE32 or PE32+ */
    uint8_t  MajorLinkerVersion;
    uint8_t  MinorLinkerVersion;
    uint32_t SizeOfCode;
    uint32_t SizeOfInitializedData;
    uint32_t SizeOfUninitializedData;
    uint32_t AddressOfEntryPoint;
    uint32_t BaseOfCode;
    uint64_t ImageBase;         /* DLX_DEFAULT_IMAGE_BASE */
    uint32_t SectionAlignment;
    uint32_t FileAlignment;
    /* ... additional fields */
} IMAGE_OPTIONAL_HEADER64;
```

## 5.5 Mach-O Format

### Mach-O Header
```c
struct mach_header_64 {
    uint32_t magic;             /* MH_MAGIC_64 (0xfeedfacf) */
    cpu_type_t cputype;         /* CPU_TYPE_DLX (0x0000009999) */
    cpu_subtype_t cpusubtype;   /* CPU_SUBTYPE_DLX_ALL */
    uint32_t filetype;          /* MH_EXECUTE, MH_DYLIB, etc. */
    uint32_t ncmds;             /* Number of load commands */
    uint32_t sizeofcmds;        /* Size of load commands */
    uint32_t flags;             /* MH_* flags */
    uint32_t reserved;
};
```

### DLX-Specific Mach-O Flags
```c
#define MH_DLX_2RING        0x01000000  /* Uses 2-ring mode */
#define MH_DLX_4RING        0x02000000  /* Uses 4-ring mode */
#define MH_DLX_BANKING      0x04000000  /* Uses register banking */
#define MH_DLX_VECTOR       0x08000000  /* Uses vector extensions */
```
