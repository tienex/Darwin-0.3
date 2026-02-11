# DLX Binary Loaders - ELF, PE/COFF, Mach-O

## Overview

This document specifies the binary loader subsystem for DLX, supporting three major executable formats:
- **ELF** (Executable and Linkable Format) - Unix/Linux standard
- **PE/COFF** (Portable Executable / Common Object File Format) - Windows standard
- **Mach-O** (Mach Object) - macOS/iOS standard

The loader must handle:
- File format parsing and validation
- Program segment loading into memory
- Relocation processing
- Dynamic linking
- TLS (Thread-Local Storage) initialization
- Integration with DLX MMU and protection rings
- Support for both 32-bit and 64-bit formats

## Architecture Integration

### Memory Layout Strategy

```c
/* DLX Virtual Memory Layout (32-bit) */
#define USER_TEXT_BASE      0x00010000  /* Code segment */
#define USER_DATA_BASE      0x10000000  /* Data segment */
#define USER_HEAP_BASE      0x20000000  /* Heap (grows up) */
#define USER_MMAP_BASE      0x40000000  /* Memory mapped files */
#define USER_STACK_TOP      0x7FFFFFFF  /* Stack (grows down) */
#define KERNEL_BASE         0x80000000  /* Kernel space */

/* DLX Virtual Memory Layout (64-bit) */
#define USER_TEXT_BASE_64   0x0000000000400000ULL
#define USER_DATA_BASE_64   0x0000000010000000ULL
#define USER_HEAP_BASE_64   0x0000000020000000ULL
#define USER_MMAP_BASE_64   0x0000700000000000ULL
#define USER_STACK_TOP_64   0x00007FFFFFFFF000ULL
#define KERNEL_BASE_64      0xFFFF800000000000ULL
```

### Protection Ring Support

```c
/* Ring configuration for loaded binaries */
enum dlx_ring_mode {
    RING_MODE_2,        /* Standard 2-ring (user/kernel) */
    RING_MODE_4,        /* VMS-style 4-ring (user/supervisor/executive/kernel) */
    RING_MODE_BANK      /* Register banking mode */
};

/* Binary loading ring assignment */
struct binary_ring_config {
    uint32_t code_ring;     /* Ring for code execution */
    uint32_t data_ring;     /* Ring for data access */
    uint32_t stack_ring;    /* Ring for stack */
    uint32_t heap_ring;     /* Ring for heap */
    uint32_t use_banks;     /* Use register banking */
};
```

## ELF (Executable and Linkable Format)

### ELF File Structure

```c
/* ELF Header (32-bit) */
typedef struct {
    uint8_t  e_ident[16];       /* Magic: 0x7F 'E' 'L' 'F' */
    uint16_t e_type;            /* ET_EXEC, ET_DYN, ET_REL, ET_CORE */
    uint16_t e_machine;         /* EM_DLX (custom: 0x9999) */
    uint32_t e_version;         /* EV_CURRENT (1) */
    uint32_t e_entry;           /* Entry point virtual address */
    uint32_t e_phoff;           /* Program header table offset */
    uint32_t e_shoff;           /* Section header table offset */
    uint32_t e_flags;           /* Processor-specific flags */
    uint16_t e_ehsize;          /* ELF header size */
    uint16_t e_phentsize;       /* Program header entry size */
    uint16_t e_phnum;           /* Program header entry count */
    uint16_t e_shentsize;       /* Section header entry size */
    uint16_t e_shnum;           /* Section header entry count */
    uint16_t e_shstrndx;        /* Section name string table index */
} Elf32_Ehdr;

/* ELF Header (64-bit) */
typedef struct {
    uint8_t  e_ident[16];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint64_t e_entry;           /* 64-bit entry point */
    uint64_t e_phoff;
    uint64_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} Elf64_Ehdr;

/* DLX-specific e_flags */
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

/* Program Header (32-bit) */
typedef struct {
    uint32_t p_type;            /* PT_LOAD, PT_DYNAMIC, PT_INTERP, etc. */
    uint32_t p_offset;          /* File offset */
    uint32_t p_vaddr;           /* Virtual address */
    uint32_t p_paddr;           /* Physical address (unused) */
    uint32_t p_filesz;          /* Size in file */
    uint32_t p_memsz;           /* Size in memory */
    uint32_t p_flags;           /* PF_R, PF_W, PF_X */
    uint32_t p_align;           /* Alignment */
} Elf32_Phdr;

/* Program Header (64-bit) */
typedef struct {
    uint32_t p_type;
    uint32_t p_flags;           /* Flags moved earlier in 64-bit */
    uint64_t p_offset;
    uint64_t p_vaddr;
    uint64_t p_paddr;
    uint64_t p_filesz;
    uint64_t p_memsz;
    uint64_t p_align;
} Elf64_Phdr;

/* Section Header */
typedef struct {
    uint32_t sh_name;           /* Section name string table index */
    uint32_t sh_type;           /* SHT_PROGBITS, SHT_SYMTAB, etc. */
    uint32_t sh_flags;          /* SHF_WRITE, SHF_ALLOC, SHF_EXECINSTR */
    uint32_t sh_addr;           /* Virtual address */
    uint32_t sh_offset;         /* File offset */
    uint32_t sh_size;           /* Section size */
    uint32_t sh_link;           /* Link to another section */
    uint32_t sh_info;           /* Additional info */
    uint32_t sh_addralign;      /* Alignment */
    uint32_t sh_entsize;        /* Entry size for fixed-size entries */
} Elf32_Shdr;
```

### ELF Relocation Types for DLX

```c
/* DLX-specific relocation types */
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
#define R_DLX_TLS_DTPMOD32  11  /* TLS module ID */
#define R_DLX_TLS_DTPOFF32  12  /* TLS offset */
#define R_DLX_TLS_TPOFF32   13  /* TLS offset from TP */
#define R_DLX_HI20          14  /* High 20 bits */
#define R_DLX_LO12          15  /* Low 12 bits */
#define R_DLX_PCREL_HI20    16  /* PC-relative high 20 */
#define R_DLX_PCREL_LO12    17  /* PC-relative low 12 */

/* Relocation entry */
typedef struct {
    uint32_t r_offset;          /* Address */
    uint32_t r_info;            /* Type and symbol index */
} Elf32_Rel;

typedef struct {
    uint32_t r_offset;
    uint32_t r_info;
    int32_t  r_addend;          /* Explicit addend */
} Elf32_Rela;

#define ELF32_R_SYM(i)    ((i) >> 8)
#define ELF32_R_TYPE(i)   ((uint8_t)(i))
#define ELF32_R_INFO(s,t) (((s) << 8) + (uint8_t)(t))
```

### ELF Loader Implementation

```c
/*
 * load_elf_binary - Load an ELF executable
 * @filename: Path to ELF file
 * @ring_config: Ring configuration for loaded binary
 * @argc: Argument count
 * @argv: Argument vector
 * @envp: Environment vector
 *
 * Returns: Entry point address on success, 0 on failure
 */
uint32_t load_elf_binary(const char *filename,
                         struct binary_ring_config *ring_config,
                         int argc, char **argv, char **envp)
{
    int fd;
    Elf32_Ehdr ehdr;
    Elf32_Phdr *phdrs = NULL;
    uint32_t entry_point;
    int i;

    /* Open ELF file */
    fd = open(filename, O_RDONLY);
    if (fd < 0) {
        printf("Failed to open ELF file: %s\n", filename);
        return 0;
    }

    /* Read ELF header */
    if (read(fd, &ehdr, sizeof(ehdr)) != sizeof(ehdr)) {
        printf("Failed to read ELF header\n");
        goto error;
    }

    /* Verify ELF magic */
    if (ehdr.e_ident[EI_MAG0] != ELFMAG0 ||
        ehdr.e_ident[EI_MAG1] != ELFMAG1 ||
        ehdr.e_ident[EI_MAG2] != ELFMAG2 ||
        ehdr.e_ident[EI_MAG3] != ELFMAG3) {
        printf("Invalid ELF magic\n");
        goto error;
    }

    /* Verify DLX architecture */
    if (ehdr.e_machine != EM_DLX) {
        printf("Not a DLX binary (machine=%d)\n", ehdr.e_machine);
        goto error;
    }

    /* Verify ELF class (32-bit or 64-bit) */
    if (ehdr.e_ident[EI_CLASS] != ELFCLASS32) {
        printf("Only 32-bit ELF supported in this loader\n");
        goto error;
    }

    /* Read program headers */
    phdrs = malloc(ehdr.e_phnum * sizeof(Elf32_Phdr));
    if (!phdrs) {
        printf("Failed to allocate program headers\n");
        goto error;
    }

    lseek(fd, ehdr.e_phoff, SEEK_SET);
    if (read(fd, phdrs, ehdr.e_phnum * sizeof(Elf32_Phdr)) !=
        ehdr.e_phnum * sizeof(Elf32_Phdr)) {
        printf("Failed to read program headers\n");
        goto error;
    }

    /* Load program segments */
    for (i = 0; i < ehdr.e_phnum; i++) {
        if (phdrs[i].p_type == PT_LOAD) {
            if (!load_elf_segment(fd, &phdrs[i], ring_config)) {
                printf("Failed to load segment %d\n", i);
                goto error;
            }
        } else if (phdrs[i].p_type == PT_DYNAMIC) {
            /* Handle dynamic linking */
            if (!load_elf_dynamic(fd, &phdrs[i])) {
                printf("Failed to process dynamic segment\n");
                goto error;
            }
        } else if (phdrs[i].p_type == PT_TLS) {
            /* Handle TLS segment */
            if (!load_elf_tls(fd, &phdrs[i])) {
                printf("Failed to initialize TLS\n");
                goto error;
            }
        }
    }

    /* Process relocations if dynamic */
    if (ehdr.e_type == ET_DYN) {
        if (!process_elf_relocations(fd, &ehdr, phdrs)) {
            printf("Failed to process relocations\n");
            goto error;
        }
    }

    /* Setup stack with arguments */
    if (!setup_elf_stack(argc, argv, envp, &ehdr, phdrs)) {
        printf("Failed to setup stack\n");
        goto error;
    }

    entry_point = ehdr.e_entry;
    free(phdrs);
    close(fd);

    return entry_point;

error:
    if (phdrs) free(phdrs);
    close(fd);
    return 0;
}

/*
 * load_elf_segment - Load a single program segment
 */
static int load_elf_segment(int fd, Elf32_Phdr *phdr,
                           struct binary_ring_config *ring_config)
{
    void *vaddr = (void *)phdr->p_vaddr;
    uint32_t prot = 0;
    uint32_t flags = MAP_PRIVATE | MAP_FIXED;

    /* Convert ELF flags to memory protection */
    if (phdr->p_flags & PF_R) prot |= PROT_READ;
    if (phdr->p_flags & PF_W) prot |= PROT_WRITE;
    if (phdr->p_flags & PF_X) prot |= PROT_EXEC;

    /* Determine ring for this segment */
    uint32_t ring = ring_config->code_ring;
    if (phdr->p_flags & PF_W) {
        ring = ring_config->data_ring;  /* Writable = data ring */
    }

    /* Map memory for segment */
    if (phdr->p_filesz > 0) {
        /* Map file-backed region */
        void *mapped = mmap(vaddr, phdr->p_filesz, prot, flags, fd,
                           phdr->p_offset);
        if (mapped == MAP_FAILED) {
            printf("mmap failed for segment at 0x%08x\n", phdr->p_vaddr);
            return 0;
        }
    }

    /* Zero-fill BSS portion (p_memsz > p_filesz) */
    if (phdr->p_memsz > phdr->p_filesz) {
        void *bss_start = (void *)(phdr->p_vaddr + phdr->p_filesz);
        size_t bss_size = phdr->p_memsz - phdr->p_filesz;

        void *mapped = mmap(bss_start, bss_size, prot,
                           flags | MAP_ANONYMOUS, -1, 0);
        if (mapped == MAP_FAILED) {
            printf("mmap failed for BSS at 0x%p\n", bss_start);
            return 0;
        }
        memset(mapped, 0, bss_size);
    }

    /* Set ring protection for this segment */
    if (ring_config->use_banks || ring != 0) {
        set_memory_ring(phdr->p_vaddr, phdr->p_memsz, ring);
    }

    return 1;
}

/*
 * process_elf_relocations - Apply relocations for PIE/shared libraries
 */
static int process_elf_relocations(int fd, Elf32_Ehdr *ehdr,
                                   Elf32_Phdr *phdrs)
{
    /* Find dynamic segment */
    Elf32_Phdr *dyn_phdr = NULL;
    for (int i = 0; i < ehdr->e_phnum; i++) {
        if (phdrs[i].p_type == PT_DYNAMIC) {
            dyn_phdr = &phdrs[i];
            break;
        }
    }

    if (!dyn_phdr) {
        return 1;  /* No dynamic segment, nothing to relocate */
    }

    /* Read dynamic section */
    Elf32_Dyn *dyns = malloc(dyn_phdr->p_filesz);
    lseek(fd, dyn_phdr->p_offset, SEEK_SET);
    read(fd, dyns, dyn_phdr->p_filesz);

    /* Find relocation tables */
    uint32_t rela_addr = 0, rela_size = 0;
    uint32_t rel_addr = 0, rel_size = 0;
    uint32_t base_addr = phdrs[0].p_vaddr;  /* Load base */

    for (Elf32_Dyn *dyn = dyns; dyn->d_tag != DT_NULL; dyn++) {
        switch (dyn->d_tag) {
        case DT_RELA:
            rela_addr = dyn->d_un.d_ptr;
            break;
        case DT_RELASZ:
            rela_size = dyn->d_un.d_val;
            break;
        case DT_REL:
            rel_addr = dyn->d_un.d_ptr;
            break;
        case DT_RELSZ:
            rel_size = dyn->d_un.d_val;
            break;
        }
    }

    /* Process RELA relocations */
    if (rela_addr && rela_size) {
        Elf32_Rela *relas = (Elf32_Rela *)rela_addr;
        int count = rela_size / sizeof(Elf32_Rela);
        for (int i = 0; i < count; i++) {
            apply_elf_relocation_rela(&relas[i], base_addr);
        }
    }

    /* Process REL relocations */
    if (rel_addr && rel_size) {
        Elf32_Rel *rels = (Elf32_Rel *)rel_addr;
        int count = rel_size / sizeof(Elf32_Rel);
        for (int i = 0; i < count; i++) {
            apply_elf_relocation_rel(&rels[i], base_addr);
        }
    }

    free(dyns);
    return 1;
}

/*
 * apply_elf_relocation_rela - Apply a single RELA relocation
 */
static void apply_elf_relocation_rela(Elf32_Rela *rela, uint32_t base_addr)
{
    uint32_t *loc = (uint32_t *)(rela->r_offset + base_addr);
    uint32_t type = ELF32_R_TYPE(rela->r_info);
    uint32_t sym = ELF32_R_SYM(rela->r_info);
    int32_t addend = rela->r_addend;

    switch (type) {
    case R_DLX_NONE:
        break;

    case R_DLX_32:
        /* S + A */
        *loc = get_symbol_value(sym) + addend;
        break;

    case R_DLX_PC32:
        /* S + A - P */
        *loc = get_symbol_value(sym) + addend - (uint32_t)loc;
        break;

    case R_DLX_RELATIVE:
        /* B + A (for PIE) */
        *loc = base_addr + addend;
        break;

    case R_DLX_GLOB_DAT:
    case R_DLX_JUMP_SLOT:
        /* S (for dynamic linking) */
        *loc = get_symbol_value(sym);
        break;

    case R_DLX_COPY:
        /* Copy data from shared library */
        {
            void *src = (void *)get_symbol_value(sym);
            size_t size = get_symbol_size(sym);
            memcpy(loc, src, size);
        }
        break;

    default:
        printf("Unknown relocation type: %d\n", type);
        break;
    }
}

/*
 * setup_elf_stack - Setup initial stack with arguments and auxiliary vector
 */
static int setup_elf_stack(int argc, char **argv, char **envp,
                          Elf32_Ehdr *ehdr, Elf32_Phdr *phdrs)
{
    uint32_t stack_top = USER_STACK_TOP;
    uint32_t *sp = (uint32_t *)stack_top;

    /* Count environment variables */
    int envc = 0;
    if (envp) {
        while (envp[envc]) envc++;
    }

    /* Build auxiliary vector */
    struct {
        uint32_t type;
        uint32_t value;
    } auxv[] = {
        { AT_PHDR, (uint32_t)phdrs },
        { AT_PHENT, sizeof(Elf32_Phdr) },
        { AT_PHNUM, ehdr->e_phnum },
        { AT_PAGESZ, 4096 },
        { AT_BASE, 0 },
        { AT_FLAGS, 0 },
        { AT_ENTRY, ehdr->e_entry },
        { AT_UID, getuid() },
        { AT_EUID, geteuid() },
        { AT_GID, getgid() },
        { AT_EGID, getegid() },
        { AT_HWCAP, get_dlx_hwcaps() },
        { AT_CLKTCK, 100 },
        { AT_RANDOM, (uint32_t)get_random_bytes(16) },
        { AT_NULL, 0 }
    };

    /* Layout (from high to low addresses):
     * <random bytes>
     * <env strings>
     * <arg strings>
     * <null>
     * <auxv>
     * <null>
     * <envp>
     * <null>
     * <argv>
     * <argc>
     */

    /* Reserve space and copy strings */
    char *str_ptr = (char *)sp - 1024;  /* Start below top */
    char **arg_ptrs = malloc(argc * sizeof(char *));
    char **env_ptrs = malloc(envc * sizeof(char *));

    /* Copy argument strings */
    for (int i = argc - 1; i >= 0; i--) {
        size_t len = strlen(argv[i]) + 1;
        str_ptr -= len;
        memcpy(str_ptr, argv[i], len);
        arg_ptrs[i] = str_ptr;
    }

    /* Copy environment strings */
    for (int i = envc - 1; i >= 0; i--) {
        size_t len = strlen(envp[i]) + 1;
        str_ptr -= len;
        memcpy(str_ptr, envp[i], len);
        env_ptrs[i] = str_ptr;
    }

    /* Align stack pointer */
    sp = (uint32_t *)((uint32_t)str_ptr & ~15);

    /* Push auxiliary vector */
    for (int i = 0; auxv[i].type != AT_NULL; i++) {
        *(--sp) = auxv[i].value;
        *(--sp) = auxv[i].type;
    }
    *(--sp) = 0;  /* AT_NULL marker */

    /* Push environment pointers */
    *(--sp) = 0;  /* NULL terminator */
    for (int i = envc - 1; i >= 0; i--) {
        *(--sp) = (uint32_t)env_ptrs[i];
    }

    /* Push argument pointers */
    *(--sp) = 0;  /* NULL terminator */
    for (int i = argc - 1; i >= 0; i--) {
        *(--sp) = (uint32_t)arg_ptrs[i];
    }

    /* Push argc */
    *(--sp) = argc;

    /* Set stack pointer */
    set_stack_pointer((uint32_t)sp);

    free(arg_ptrs);
    free(env_ptrs);
    return 1;
}
```

## PE/COFF (Portable Executable)

### PE File Structure

```c
/* DOS Header (for backward compatibility) */
typedef struct {
    uint16_t e_magic;           /* MZ signature: 0x5A4D */
    uint16_t e_cblp;
    uint16_t e_cp;
    uint16_t e_crlc;
    uint16_t e_cparhdr;
    uint16_t e_minalloc;
    uint16_t e_maxalloc;
    uint16_t e_ss;
    uint16_t e_sp;
    uint16_t e_csum;
    uint16_t e_ip;
    uint16_t e_cs;
    uint16_t e_lfarlc;
    uint16_t e_ovno;
    uint16_t e_res[4];
    uint16_t e_oemid;
    uint16_t e_oeminfo;
    uint16_t e_res2[10];
    uint32_t e_lfanew;          /* Offset to PE header */
} IMAGE_DOS_HEADER;

/* PE Header */
typedef struct {
    uint32_t Signature;         /* PE signature: 0x00004550 ("PE\0\0") */
} IMAGE_NT_SIGNATURE;

/* COFF File Header */
typedef struct {
    uint16_t Machine;           /* 0x9999 for DLX */
    uint16_t NumberOfSections;
    uint32_t TimeDateStamp;
    uint32_t PointerToSymbolTable;
    uint32_t NumberOfSymbols;
    uint16_t SizeOfOptionalHeader;
    uint16_t Characteristics;
} IMAGE_FILE_HEADER;

/* PE Optional Header (32-bit) */
typedef struct {
    uint16_t Magic;             /* 0x010b (PE32) */
    uint8_t  MajorLinkerVersion;
    uint8_t  MinorLinkerVersion;
    uint32_t SizeOfCode;
    uint32_t SizeOfInitializedData;
    uint32_t SizeOfUninitializedData;
    uint32_t AddressOfEntryPoint;
    uint32_t BaseOfCode;
    uint32_t BaseOfData;
    uint32_t ImageBase;         /* Preferred load address */
    uint32_t SectionAlignment;
    uint32_t FileAlignment;
    uint16_t MajorOSVersion;
    uint16_t MinorOSVersion;
    uint16_t MajorImageVersion;
    uint16_t MinorImageVersion;
    uint16_t MajorSubsystemVersion;
    uint16_t MinorSubsystemVersion;
    uint32_t Win32VersionValue;
    uint32_t SizeOfImage;
    uint32_t SizeOfHeaders;
    uint32_t CheckSum;
    uint16_t Subsystem;         /* Console, GUI, etc. */
    uint16_t DllCharacteristics;
    uint32_t SizeOfStackReserve;
    uint32_t SizeOfStackCommit;
    uint32_t SizeOfHeapReserve;
    uint32_t SizeOfHeapCommit;
    uint32_t LoaderFlags;
    uint32_t NumberOfRvaAndSizes;
    IMAGE_DATA_DIRECTORY DataDirectory[16];
} IMAGE_OPTIONAL_HEADER32;

/* PE Optional Header (64-bit) */
typedef struct {
    uint16_t Magic;             /* 0x020b (PE32+) */
    uint8_t  MajorLinkerVersion;
    uint8_t  MinorLinkerVersion;
    uint32_t SizeOfCode;
    uint32_t SizeOfInitializedData;
    uint32_t SizeOfUninitializedData;
    uint32_t AddressOfEntryPoint;
    uint32_t BaseOfCode;
    uint64_t ImageBase;         /* 64-bit image base */
    uint32_t SectionAlignment;
    uint32_t FileAlignment;
    uint16_t MajorOSVersion;
    uint16_t MinorOSVersion;
    uint16_t MajorImageVersion;
    uint16_t MinorImageVersion;
    uint16_t MajorSubsystemVersion;
    uint16_t MinorSubsystemVersion;
    uint32_t Win32VersionValue;
    uint32_t SizeOfImage;
    uint32_t SizeOfHeaders;
    uint32_t CheckSum;
    uint16_t Subsystem;
    uint16_t DllCharacteristics;
    uint64_t SizeOfStackReserve;
    uint64_t SizeOfStackCommit;
    uint64_t SizeOfHeapReserve;
    uint64_t SizeOfHeapCommit;
    uint32_t LoaderFlags;
    uint32_t NumberOfRvaAndSizes;
    IMAGE_DATA_DIRECTORY DataDirectory[16];
} IMAGE_OPTIONAL_HEADER64;

/* Data Directory Indices */
#define IMAGE_DIRECTORY_ENTRY_EXPORT         0
#define IMAGE_DIRECTORY_ENTRY_IMPORT         1
#define IMAGE_DIRECTORY_ENTRY_RESOURCE       2
#define IMAGE_DIRECTORY_ENTRY_EXCEPTION      3
#define IMAGE_DIRECTORY_ENTRY_SECURITY       4
#define IMAGE_DIRECTORY_ENTRY_BASERELOC      5
#define IMAGE_DIRECTORY_ENTRY_DEBUG          6
#define IMAGE_DIRECTORY_ENTRY_ARCHITECTURE   7
#define IMAGE_DIRECTORY_ENTRY_GLOBALPTR      8
#define IMAGE_DIRECTORY_ENTRY_TLS            9
#define IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG   10
#define IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT  11
#define IMAGE_DIRECTORY_ENTRY_IAT           12
#define IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT  13
#define IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR 14

/* Section Header */
typedef struct {
    uint8_t  Name[8];           /* Section name */
    uint32_t VirtualSize;
    uint32_t VirtualAddress;
    uint32_t SizeOfRawData;
    uint32_t PointerToRawData;
    uint32_t PointerToRelocations;
    uint32_t PointerToLinenumbers;
    uint16_t NumberOfRelocations;
    uint16_t NumberOfLinenumbers;
    uint32_t Characteristics;
} IMAGE_SECTION_HEADER;

/* Section Characteristics */
#define IMAGE_SCN_CNT_CODE              0x00000020
#define IMAGE_SCN_CNT_INITIALIZED_DATA  0x00000040
#define IMAGE_SCN_CNT_UNINITIALIZED_DATA 0x00000080
#define IMAGE_SCN_MEM_EXECUTE           0x20000000
#define IMAGE_SCN_MEM_READ              0x40000000
#define IMAGE_SCN_MEM_WRITE             0x80000000

/* DLL Characteristics */
#define IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE    0x0040  /* ASLR */
#define IMAGE_DLLCHARACTERISTICS_NX_COMPAT       0x0100  /* DEP */
#define IMAGE_DLLCHARACTERISTICS_NO_SEH          0x0400
#define IMAGE_DLLCHARACTERISTICS_NO_BIND         0x0800
#define IMAGE_DLLCHARACTERISTICS_WDM_DRIVER      0x2000
#define IMAGE_DLLCHARACTERISTICS_TERMINAL_SERVER_AWARE 0x8000
```

### PE Base Relocations

```c
/* Base Relocation Block */
typedef struct {
    uint32_t VirtualAddress;    /* Page RVA */
    uint32_t SizeOfBlock;       /* Block size including this header */
    /* Followed by relocation entries */
} IMAGE_BASE_RELOCATION;

/* Relocation Entry (16-bit) */
#define IMAGE_REL_BASED_ABSOLUTE        0
#define IMAGE_REL_BASED_HIGH            1
#define IMAGE_REL_BASED_LOW             2
#define IMAGE_REL_BASED_HIGHLOW         3
#define IMAGE_REL_BASED_HIGHADJ         4
#define IMAGE_REL_BASED_DIR64           10

/* Extract type and offset from relocation entry */
#define IMAGE_REL_TYPE(e)      ((e) >> 12)
#define IMAGE_REL_OFFSET(e)    ((e) & 0xFFF)

/*
 * process_pe_relocations - Apply base relocations for PE
 */
static int process_pe_relocations(void *image_base,
                                  IMAGE_DATA_DIRECTORY *reloc_dir,
                                  uint64_t delta)
{
    if (reloc_dir->Size == 0) {
        return 1;  /* No relocations */
    }

    IMAGE_BASE_RELOCATION *reloc =
        (IMAGE_BASE_RELOCATION *)((uint8_t *)image_base + reloc_dir->VirtualAddress);
    IMAGE_BASE_RELOCATION *reloc_end =
        (IMAGE_BASE_RELOCATION *)((uint8_t *)reloc + reloc_dir->Size);

    while (reloc < reloc_end && reloc->SizeOfBlock > 0) {
        uint32_t page_rva = reloc->VirtualAddress;
        uint32_t num_entries = (reloc->SizeOfBlock - sizeof(*reloc)) / 2;
        uint16_t *entries = (uint16_t *)(reloc + 1);

        for (uint32_t i = 0; i < num_entries; i++) {
            uint16_t entry = entries[i];
            uint16_t type = IMAGE_REL_TYPE(entry);
            uint16_t offset = IMAGE_REL_OFFSET(entry);
            void *target = (uint8_t *)image_base + page_rva + offset;

            switch (type) {
            case IMAGE_REL_BASED_ABSOLUTE:
                /* No relocation */
                break;

            case IMAGE_REL_BASED_HIGHLOW:
                /* 32-bit relocation */
                *(uint32_t *)target += (uint32_t)delta;
                break;

            case IMAGE_REL_BASED_DIR64:
                /* 64-bit relocation */
                *(uint64_t *)target += delta;
                break;

            case IMAGE_REL_BASED_HIGH:
                /* High 16 bits */
                *(uint16_t *)target += (uint16_t)(delta >> 16);
                break;

            case IMAGE_REL_BASED_LOW:
                /* Low 16 bits */
                *(uint16_t *)target += (uint16_t)(delta & 0xFFFF);
                break;

            default:
                printf("Unknown PE relocation type: %d\n", type);
                return 0;
            }
        }

        reloc = (IMAGE_BASE_RELOCATION *)((uint8_t *)reloc + reloc->SizeOfBlock);
    }

    return 1;
}
```

### PE Import Directory

```c
/* Import Descriptor */
typedef struct {
    uint32_t OriginalFirstThunk;    /* RVA to Import Lookup Table */
    uint32_t TimeDateStamp;
    uint32_t ForwarderChain;
    uint32_t Name;                  /* RVA to DLL name */
    uint32_t FirstThunk;            /* RVA to Import Address Table */
} IMAGE_IMPORT_DESCRIPTOR;

/* Import by Name */
typedef struct {
    uint16_t Hint;                  /* Ordinal hint */
    uint8_t  Name[1];               /* Function name (null-terminated) */
} IMAGE_IMPORT_BY_NAME;

/*
 * process_pe_imports - Resolve imported functions
 */
static int process_pe_imports(void *image_base,
                              IMAGE_DATA_DIRECTORY *import_dir)
{
    if (import_dir->Size == 0) {
        return 1;  /* No imports */
    }

    IMAGE_IMPORT_DESCRIPTOR *import_desc =
        (IMAGE_IMPORT_DESCRIPTOR *)((uint8_t *)image_base +
                                    import_dir->VirtualAddress);

    for (; import_desc->Name != 0; import_desc++) {
        char *dll_name = (char *)image_base + import_desc->Name;
        void *dll_handle = load_dll(dll_name);
        if (!dll_handle) {
            printf("Failed to load DLL: %s\n", dll_name);
            return 0;
        }

        /* Process Import Address Table */
        uint32_t *thunk = (uint32_t *)((uint8_t *)image_base +
                                       import_desc->FirstThunk);
        uint32_t *lookup = import_desc->OriginalFirstThunk ?
            (uint32_t *)((uint8_t *)image_base + import_desc->OriginalFirstThunk) :
            thunk;

        for (; *lookup; lookup++, thunk++) {
            void *func_addr;

            if (*lookup & 0x80000000) {
                /* Import by ordinal */
                uint16_t ordinal = *lookup & 0xFFFF;
                func_addr = get_dll_export_by_ordinal(dll_handle, ordinal);
            } else {
                /* Import by name */
                IMAGE_IMPORT_BY_NAME *import_name =
                    (IMAGE_IMPORT_BY_NAME *)((uint8_t *)image_base + *lookup);
                func_addr = get_dll_export_by_name(dll_handle,
                                                   (char *)import_name->Name);
            }

            if (!func_addr) {
                printf("Failed to resolve import from %s\n", dll_name);
                return 0;
            }

            *thunk = (uint32_t)func_addr;
        }
    }

    return 1;
}
```

### PE Loader Implementation

```c
/*
 * load_pe_binary - Load a PE/COFF executable
 */
uint32_t load_pe_binary(const char *filename,
                        struct binary_ring_config *ring_config,
                        int argc, char **argv, char **envp)
{
    int fd;
    IMAGE_DOS_HEADER dos_hdr;
    IMAGE_NT_HEADERS32 nt_hdrs;
    void *image_base;
    uint64_t delta;

    fd = open(filename, O_RDONLY);
    if (fd < 0) {
        printf("Failed to open PE file: %s\n", filename);
        return 0;
    }

    /* Read DOS header */
    read(fd, &dos_hdr, sizeof(dos_hdr));
    if (dos_hdr.e_magic != 0x5A4D) {  /* "MZ" */
        printf("Invalid DOS header\n");
        close(fd);
        return 0;
    }

    /* Read NT headers */
    lseek(fd, dos_hdr.e_lfanew, SEEK_SET);
    read(fd, &nt_hdrs, sizeof(nt_hdrs));

    if (nt_hdrs.Signature != 0x00004550) {  /* "PE\0\0" */
        printf("Invalid PE signature\n");
        close(fd);
        return 0;
    }

    /* Verify machine type */
    if (nt_hdrs.FileHeader.Machine != 0x9999) {  /* DLX */
        printf("Not a DLX PE binary\n");
        close(fd);
        return 0;
    }

    /* Allocate image memory */
    image_base = mmap((void *)nt_hdrs.OptionalHeader.ImageBase,
                     nt_hdrs.OptionalHeader.SizeOfImage,
                     PROT_READ | PROT_WRITE | PROT_EXEC,
                     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (image_base == MAP_FAILED) {
        /* Try any address if preferred base unavailable */
        image_base = mmap(NULL, nt_hdrs.OptionalHeader.SizeOfImage,
                         PROT_READ | PROT_WRITE | PROT_EXEC,
                         MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (image_base == MAP_FAILED) {
            printf("Failed to allocate image memory\n");
            close(fd);
            return 0;
        }
    }

    /* Calculate relocation delta */
    delta = (uint64_t)image_base - nt_hdrs.OptionalHeader.ImageBase;

    /* Load sections */
    IMAGE_SECTION_HEADER *sections = malloc(nt_hdrs.FileHeader.NumberOfSections *
                                           sizeof(IMAGE_SECTION_HEADER));
    read(fd, sections, nt_hdrs.FileHeader.NumberOfSections *
         sizeof(IMAGE_SECTION_HEADER));

    for (int i = 0; i < nt_hdrs.FileHeader.NumberOfSections; i++) {
        void *section_base = (uint8_t *)image_base +
                            sections[i].VirtualAddress;

        if (sections[i].SizeOfRawData > 0) {
            lseek(fd, sections[i].PointerToRawData, SEEK_SET);
            read(fd, section_base, sections[i].SizeOfRawData);
        }

        /* Zero-fill remainder */
        if (sections[i].VirtualSize > sections[i].SizeOfRawData) {
            memset((uint8_t *)section_base + sections[i].SizeOfRawData, 0,
                   sections[i].VirtualSize - sections[i].SizeOfRawData);
        }

        /* Set section protection */
        uint32_t prot = 0;
        if (sections[i].Characteristics & IMAGE_SCN_MEM_READ)
            prot |= PROT_READ;
        if (sections[i].Characteristics & IMAGE_SCN_MEM_WRITE)
            prot |= PROT_WRITE;
        if (sections[i].Characteristics & IMAGE_SCN_MEM_EXECUTE)
            prot |= PROT_EXEC;

        mprotect(section_base, sections[i].VirtualSize, prot);
    }

    /* Process relocations */
    if (delta != 0) {
        process_pe_relocations(image_base,
            &nt_hdrs.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC],
            delta);
    }

    /* Process imports */
    process_pe_imports(image_base,
        &nt_hdrs.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT]);

    /* Setup TLS */
    if (nt_hdrs.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].Size > 0) {
        setup_pe_tls(image_base,
            &nt_hdrs.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS]);
    }

    free(sections);
    close(fd);

    return (uint32_t)image_base + nt_hdrs.OptionalHeader.AddressOfEntryPoint;
}
```

## Mach-O (Mach Object File Format)

### Mach-O File Structure

```c
/* Mach-O Header (32-bit) */
typedef struct {
    uint32_t magic;             /* 0xFEEDFACE (32-bit) or 0xFEEDFACF (64-bit) */
    uint32_t cputype;           /* CPU_TYPE_DLX (custom: 0x9999) */
    uint32_t cpusubtype;        /* CPU subtype */
    uint32_t filetype;          /* MH_EXECUTE, MH_DYLIB, etc. */
    uint32_t ncmds;             /* Number of load commands */
    uint32_t sizeofcmds;        /* Size of load commands */
    uint32_t flags;             /* Flags */
} mach_header;

/* Mach-O Header (64-bit) */
typedef struct {
    uint32_t magic;             /* 0xFEEDFACF */
    uint32_t cputype;
    uint32_t cpusubtype;
    uint32_t filetype;
    uint32_t ncmds;
    uint32_t sizeofcmds;
    uint32_t flags;
    uint32_t reserved;
} mach_header_64;

/* Mach-O Magic Numbers */
#define MH_MAGIC        0xFEEDFACE  /* 32-bit */
#define MH_MAGIC_64     0xFEEDFACF  /* 64-bit */
#define MH_CIGAM        0xCEFAEDFE  /* 32-bit byte-swapped */
#define MH_CIGAM_64     0xCFFAEDFE  /* 64-bit byte-swapped */

/* CPU Type */
#define CPU_TYPE_DLX    0x9999      /* DLX architecture */

/* File Types */
#define MH_OBJECT       0x1         /* Relocatable object */
#define MH_EXECUTE      0x2         /* Executable */
#define MH_DYLIB        0x6         /* Dynamic library */
#define MH_DYLINKER     0x7         /* Dynamic linker */
#define MH_BUNDLE       0x8         /* Bundle */

/* Flags */
#define MH_NOUNDEFS     0x1         /* No undefined references */
#define MH_PIE          0x200000    /* Position independent */
#define MH_NO_HEAP_EXECUTION 0x1000000  /* Heap NX */

/* Load Command Header */
typedef struct {
    uint32_t cmd;               /* Command type */
    uint32_t cmdsize;           /* Command size including this header */
} load_command;

/* Load Command Types */
#define LC_SEGMENT          0x1     /* Segment (32-bit) */
#define LC_SEGMENT_64       0x19    /* Segment (64-bit) */
#define LC_SYMTAB           0x2     /* Symbol table */
#define LC_DYSYMTAB         0xB     /* Dynamic symbol table */
#define LC_LOAD_DYLIB       0xC     /* Load dynamic library */
#define LC_ID_DYLIB         0xD     /* Dynamic library ID */
#define LC_LOAD_DYLINKER    0xE     /* Load dynamic linker */
#define LC_THREAD           0x4     /* Thread state */
#define LC_UNIXTHREAD       0x5     /* Unix thread (entry point) */
#define LC_MAIN             0x28    /* Entry point (LC_MAIN replaces LC_UNIXTHREAD) */
#define LC_DYLD_INFO        0x22    /* Dyld info */
#define LC_DYLD_INFO_ONLY   0x80000022  /* Dyld info only */
#define LC_ENCRYPTION_INFO  0x21    /* Encryption info */
#define LC_CODE_SIGNATURE   0x1D    /* Code signature */

/* Segment Command (32-bit) */
typedef struct {
    uint32_t cmd;               /* LC_SEGMENT */
    uint32_t cmdsize;
    char     segname[16];       /* Segment name */
    uint32_t vmaddr;            /* Virtual memory address */
    uint32_t vmsize;            /* Virtual memory size */
    uint32_t fileoff;           /* File offset */
    uint32_t filesize;          /* File size */
    uint32_t maxprot;           /* Maximum protection */
    uint32_t initprot;          /* Initial protection */
    uint32_t nsects;            /* Number of sections */
    uint32_t flags;             /* Flags */
} segment_command;

/* Segment Command (64-bit) */
typedef struct {
    uint32_t cmd;               /* LC_SEGMENT_64 */
    uint32_t cmdsize;
    char     segname[16];
    uint64_t vmaddr;
    uint64_t vmsize;
    uint64_t fileoff;
    uint64_t filesize;
    uint32_t maxprot;
    uint32_t initprot;
    uint32_t nsects;
    uint32_t flags;
} segment_command_64;

/* Section (32-bit) */
typedef struct {
    char     sectname[16];      /* Section name */
    char     segname[16];       /* Segment name */
    uint32_t addr;              /* Virtual address */
    uint32_t size;              /* Size */
    uint32_t offset;            /* File offset */
    uint32_t align;             /* Alignment (power of 2) */
    uint32_t reloff;            /* Relocation entries offset */
    uint32_t nreloc;            /* Number of relocation entries */
    uint32_t flags;             /* Section flags */
    uint32_t reserved1;
    uint32_t reserved2;
} section;

/* Section (64-bit) */
typedef struct {
    char     sectname[16];
    char     segname[16];
    uint64_t addr;
    uint64_t size;
    uint32_t offset;
    uint32_t align;
    uint32_t reloff;
    uint32_t nreloc;
    uint32_t flags;
    uint32_t reserved1;
    uint32_t reserved2;
    uint32_t reserved3;
} section_64;

/* Protection Bits */
#define VM_PROT_NONE    0x00
#define VM_PROT_READ    0x01
#define VM_PROT_WRITE   0x02
#define VM_PROT_EXECUTE 0x04

/* Entry Point Command (LC_MAIN) */
typedef struct {
    uint32_t cmd;               /* LC_MAIN */
    uint32_t cmdsize;
    uint64_t entryoff;          /* File offset of entry point */
    uint64_t stacksize;         /* Initial stack size */
} entry_point_command;

/* Dynamic Library Command */
typedef struct {
    uint32_t cmd;               /* LC_LOAD_DYLIB */
    uint32_t cmdsize;
    uint32_t name_offset;       /* Offset to library name */
    uint32_t timestamp;
    uint32_t current_version;
    uint32_t compatibility_version;
} dylib_command;

/* Dyld Info Command */
typedef struct {
    uint32_t cmd;               /* LC_DYLD_INFO or LC_DYLD_INFO_ONLY */
    uint32_t cmdsize;
    uint32_t rebase_off;        /* File offset to rebase info */
    uint32_t rebase_size;
    uint32_t bind_off;          /* File offset to binding info */
    uint32_t bind_size;
    uint32_t weak_bind_off;
    uint32_t weak_bind_size;
    uint32_t lazy_bind_off;
    uint32_t lazy_bind_size;
    uint32_t export_off;
    uint32_t export_size;
} dyld_info_command;
```

### Mach-O Relocations

```c
/* Relocation Entry */
typedef struct {
    int32_t  r_address;         /* Offset from start of section */
    uint32_t r_symbolnum:24,    /* Symbol index or section number */
             r_pcrel:1,         /* PC-relative */
             r_length:2,        /* 0=byte, 1=word, 2=long, 3=quad */
             r_extern:1,        /* External symbol */
             r_type:4;          /* Relocation type */
} relocation_info;

/* DLX-specific relocation types */
#define DLX_RELOC_VANILLA       0   /* Generic */
#define DLX_RELOC_PAIR          1   /* Second entry of a pair */
#define DLX_RELOC_SECTDIFF      2   /* Section difference */
#define DLX_RELOC_LOCAL_SECTDIFF 3  /* Local section difference */
#define DLX_RELOC_PB_LA_PTR     4   /* Prebound lazy pointer */
#define DLX_RELOC_BR24          5   /* 24-bit branch */
#define DLX_RELOC_BR14          6   /* 14-bit branch */
#define DLX_RELOC_GOT           7   /* GOT entry */
#define DLX_RELOC_TLS           8   /* TLS offset */
```

### Mach-O Dyld Binding

```c
/* Dyld Binding Opcodes */
#define BIND_OPCODE_DONE                    0x00
#define BIND_OPCODE_SET_DYLIB_ORDINAL_IMM   0x10
#define BIND_OPCODE_SET_SYMBOL_TRAILING_FLAGS_IMM 0x40
#define BIND_OPCODE_SET_TYPE_IMM            0x50
#define BIND_OPCODE_SET_ADDEND_SLEB         0x60
#define BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB 0x70
#define BIND_OPCODE_ADD_ADDR_ULEB           0x80
#define BIND_OPCODE_DO_BIND                 0x90
#define BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB   0xA0
#define BIND_OPCODE_DO_BIND_ADD_ADDR_IMM_SCALED 0xB0
#define BIND_OPCODE_DO_BIND_ULEB_TIMES_SKIPPING_ULEB 0xC0

/*
 * process_macho_bindings - Process dyld binding info
 */
static int process_macho_bindings(uint8_t *bind_info, uint32_t bind_size,
                                  segment_command_64 *segments, int nsegments)
{
    uint8_t *p = bind_info;
    uint8_t *end = bind_info + bind_size;

    int dylib_ordinal = 0;
    char *symbol_name = NULL;
    int symbol_flags = 0;
    int type = 0;
    int64_t addend = 0;
    int segment_index = 0;
    uint64_t segment_offset = 0;
    uint64_t address = 0;

    while (p < end) {
        uint8_t opcode = *p & BIND_OPCODE_MASK;
        uint8_t immediate = *p & BIND_IMMEDIATE_MASK;
        p++;

        switch (opcode) {
        case BIND_OPCODE_DONE:
            return 1;

        case BIND_OPCODE_SET_DYLIB_ORDINAL_IMM:
            dylib_ordinal = immediate;
            break;

        case BIND_OPCODE_SET_SYMBOL_TRAILING_FLAGS_IMM:
            symbol_name = (char *)p;
            symbol_flags = immediate;
            while (*p++) ;  /* Skip to null terminator */
            break;

        case BIND_OPCODE_SET_TYPE_IMM:
            type = immediate;
            break;

        case BIND_OPCODE_SET_ADDEND_SLEB:
            addend = read_sleb128(&p);
            break;

        case BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB:
            segment_index = immediate;
            segment_offset = read_uleb128(&p);
            address = segments[segment_index].vmaddr + segment_offset;
            break;

        case BIND_OPCODE_ADD_ADDR_ULEB:
            address += read_uleb128(&p);
            break;

        case BIND_OPCODE_DO_BIND:
            /* Perform binding */
            bind_symbol(address, dylib_ordinal, symbol_name, type, addend);
            address += sizeof(void *);
            break;

        case BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB:
            bind_symbol(address, dylib_ordinal, symbol_name, type, addend);
            address += read_uleb128(&p) + sizeof(void *);
            break;

        case BIND_OPCODE_DO_BIND_ADD_ADDR_IMM_SCALED:
            bind_symbol(address, dylib_ordinal, symbol_name, type, addend);
            address += immediate * sizeof(void *) + sizeof(void *);
            break;

        case BIND_OPCODE_DO_BIND_ULEB_TIMES_SKIPPING_ULEB: {
            uint64_t count = read_uleb128(&p);
            uint64_t skip = read_uleb128(&p);
            for (uint64_t i = 0; i < count; i++) {
                bind_symbol(address, dylib_ordinal, symbol_name, type, addend);
                address += skip + sizeof(void *);
            }
            break;
        }

        default:
            printf("Unknown bind opcode: 0x%02x\n", opcode);
            return 0;
        }
    }

    return 1;
}
```

### Mach-O Loader Implementation

```c
/*
 * load_macho_binary - Load a Mach-O executable
 */
uint32_t load_macho_binary(const char *filename,
                           struct binary_ring_config *ring_config,
                           int argc, char **argv, char **envp)
{
    int fd;
    mach_header_64 header;
    uint32_t entry_point = 0;
    uint64_t stack_size = 0;

    fd = open(filename, O_RDONLY);
    if (fd < 0) {
        printf("Failed to open Mach-O file: %s\n", filename);
        return 0;
    }

    /* Read Mach-O header */
    read(fd, &header, sizeof(header));

    /* Verify magic */
    if (header.magic != MH_MAGIC && header.magic != MH_MAGIC_64) {
        printf("Invalid Mach-O magic: 0x%08x\n", header.magic);
        close(fd);
        return 0;
    }

    /* Verify CPU type */
    if (header.cputype != CPU_TYPE_DLX) {
        printf("Not a DLX Mach-O binary\n");
        close(fd);
        return 0;
    }

    /* Read load commands */
    uint8_t *cmds = malloc(header.sizeofcmds);
    read(fd, cmds, header.sizeofcmds);

    load_command *cmd = (load_command *)cmds;
    for (uint32_t i = 0; i < header.ncmds; i++) {
        switch (cmd->cmd) {
        case LC_SEGMENT_64: {
            segment_command_64 *seg = (segment_command_64 *)cmd;
            if (!load_macho_segment(fd, seg, ring_config)) {
                printf("Failed to load segment: %s\n", seg->segname);
                goto error;
            }
            break;
        }

        case LC_MAIN: {
            entry_point_command *ep = (entry_point_command *)cmd;
            /* Entry point is file offset, need to convert to address */
            entry_point = find_entry_address(cmds, header.ncmds, ep->entryoff);
            stack_size = ep->stacksize;
            break;
        }

        case LC_UNIXTHREAD: {
            /* Legacy entry point in thread state */
            entry_point = extract_thread_entry(cmd);
            break;
        }

        case LC_LOAD_DYLIB: {
            dylib_command *dylib = (dylib_command *)cmd;
            char *dylib_name = (char *)cmd + dylib->name_offset;
            if (!load_dylib(dylib_name)) {
                printf("Failed to load dylib: %s\n", dylib_name);
                goto error;
            }
            break;
        }

        case LC_DYLD_INFO:
        case LC_DYLD_INFO_ONLY: {
            dyld_info_command *dyld_info = (dyld_info_command *)cmd;

            /* Process rebase info */
            if (dyld_info->rebase_size > 0) {
                uint8_t *rebase_info = malloc(dyld_info->rebase_size);
                lseek(fd, dyld_info->rebase_off, SEEK_SET);
                read(fd, rebase_info, dyld_info->rebase_size);
                process_macho_rebasing(rebase_info, dyld_info->rebase_size);
                free(rebase_info);
            }

            /* Process binding info */
            if (dyld_info->bind_size > 0) {
                uint8_t *bind_info = malloc(dyld_info->bind_size);
                lseek(fd, dyld_info->bind_off, SEEK_SET);
                read(fd, bind_info, dyld_info->bind_size);
                process_macho_bindings(bind_info, dyld_info->bind_size,
                                      get_loaded_segments(), get_nsegments());
                free(bind_info);
            }

            /* Process lazy binding info */
            if (dyld_info->lazy_bind_size > 0) {
                setup_lazy_binding(fd, dyld_info);
            }
            break;
        }

        case LC_SYMTAB:
            /* Symbol table - typically not needed for execution */
            break;

        case LC_DYSYMTAB:
            /* Dynamic symbol table */
            break;

        default:
            /* Ignore unknown commands */
            break;
        }

        cmd = (load_command *)((uint8_t *)cmd + cmd->cmdsize);
    }

    /* Setup stack */
    if (stack_size == 0) stack_size = 8 * 1024 * 1024;  /* Default 8MB */
    setup_macho_stack(stack_size, argc, argv, envp);

    free(cmds);
    close(fd);

    return entry_point;

error:
    free(cmds);
    close(fd);
    return 0;
}

/*
 * load_macho_segment - Load a single Mach-O segment
 */
static int load_macho_segment(int fd, segment_command_64 *seg,
                              struct binary_ring_config *ring_config)
{
    /* Skip zero-sized segments */
    if (seg->vmsize == 0) {
        return 1;
    }

    /* Convert Mach-O protection to mmap protection */
    int prot = 0;
    if (seg->initprot & VM_PROT_READ) prot |= PROT_READ;
    if (seg->initprot & VM_PROT_WRITE) prot |= PROT_WRITE;
    if (seg->initprot & VM_PROT_EXECUTE) prot |= PROT_EXEC;

    /* Map segment */
    void *addr = mmap((void *)seg->vmaddr, seg->vmsize, prot,
                     MAP_PRIVATE | MAP_FIXED | MAP_ANONYMOUS, -1, 0);
    if (addr == MAP_FAILED) {
        printf("Failed to map segment %s at 0x%llx\n",
               seg->segname, seg->vmaddr);
        return 0;
    }

    /* Load file data */
    if (seg->filesize > 0) {
        lseek(fd, seg->fileoff, SEEK_SET);
        read(fd, addr, seg->filesize);
    }

    /* Zero-fill remainder */
    if (seg->vmsize > seg->filesize) {
        memset((uint8_t *)addr + seg->filesize, 0,
               seg->vmsize - seg->filesize);
    }

    /* Load sections within segment */
    section_64 *sections = (section_64 *)(seg + 1);
    for (uint32_t i = 0; i < seg->nsects; i++) {
        load_macho_section(fd, &sections[i]);
    }

    /* Set ring protection */
    uint32_t ring = (seg->initprot & VM_PROT_EXECUTE) ?
        ring_config->code_ring : ring_config->data_ring;
    if (ring_config->use_banks || ring != 0) {
        set_memory_ring(seg->vmaddr, seg->vmsize, ring);
    }

    return 1;
}
```

## Universal Binary Format (Fat Mach-O)

```c
/* Fat Header */
typedef struct {
    uint32_t magic;             /* FAT_MAGIC (0xCAFEBABE) */
    uint32_t nfat_arch;         /* Number of architectures */
} fat_header;

/* Fat Architecture */
typedef struct {
    uint32_t cputype;           /* CPU type */
    uint32_t cpusubtype;        /* CPU subtype */
    uint32_t offset;            /* File offset to architecture */
    uint32_t size;              /* Size of architecture slice */
    uint32_t align;             /* Alignment (power of 2) */
} fat_arch;

#define FAT_MAGIC   0xCAFEBABE
#define FAT_CIGAM   0xBEBAFECA  /* Byte-swapped */

/*
 * load_fat_binary - Load appropriate slice from universal binary
 */
uint32_t load_fat_binary(const char *filename,
                         struct binary_ring_config *ring_config,
                         int argc, char **argv, char **envp)
{
    int fd;
    fat_header header;
    fat_arch *archs;

    fd = open(filename, O_RDONLY);
    if (fd < 0) {
        return 0;
    }

    read(fd, &header, sizeof(header));
    if (header.magic != FAT_MAGIC) {
        close(fd);
        return 0;
    }

    /* Read architecture descriptors */
    archs = malloc(header.nfat_arch * sizeof(fat_arch));
    read(fd, archs, header.nfat_arch * sizeof(fat_arch));

    /* Find DLX architecture */
    fat_arch *dlx_arch = NULL;
    for (uint32_t i = 0; i < header.nfat_arch; i++) {
        if (archs[i].cputype == CPU_TYPE_DLX) {
            dlx_arch = &archs[i];
            break;
        }
    }

    if (!dlx_arch) {
        printf("No DLX slice in universal binary\n");
        free(archs);
        close(fd);
        return 0;
    }

    /* Extract DLX slice to temporary file */
    char temp_file[] = "/tmp/dlx_slice_XXXXXX";
    int temp_fd = mkstemp(temp_file);
    if (temp_fd < 0) {
        free(archs);
        close(fd);
        return 0;
    }

    /* Copy slice */
    lseek(fd, dlx_arch->offset, SEEK_SET);
    char buffer[4096];
    uint32_t remaining = dlx_arch->size;
    while (remaining > 0) {
        uint32_t to_read = remaining < sizeof(buffer) ? remaining : sizeof(buffer);
        read(fd, buffer, to_read);
        write(temp_fd, buffer, to_read);
        remaining -= to_read;
    }

    close(temp_fd);
    close(fd);
    free(archs);

    /* Load extracted slice */
    uint32_t entry = load_macho_binary(temp_file, ring_config, argc, argv, envp);
    unlink(temp_file);

    return entry;
}
```

## Summary

### Binary Format Support Matrix

| Feature | ELF | PE/COFF | Mach-O |
|---------|-----|---------|--------|
| 32-bit support | ✓ | ✓ | ✓ |
| 64-bit support | ✓ | ✓ | ✓ |
| Position-independent code | ✓ | ✓ | ✓ |
| Dynamic linking | ✓ | ✓ | ✓ |
| TLS support | ✓ | ✓ | ✓ |
| Code signing | △ | ✓ | ✓ |
| Universal binaries | - | - | ✓ |
| Ring protection | ✓ | ✓ | ✓ |

### DLX-Specific Extensions

All three formats support DLX-specific features through:
- Custom machine type identifiers
- Extended flags for ring modes
- ISA extension markers (vector, crypto, quantum, etc.)
- Endianness specification
- Register banking mode indicators

### Loader Integration

The loaders integrate with:
- DLX MMU and page tables
- Protection ring system (2-ring, 4-ring, banking)
- TLB management
- Cache coherency
- Memory tagging (for single-level storage)
- Endian switching mechanisms

---

**Document Status**: Specification Complete
**Implementation Status**: Ready for kernel integration
**Complexity**: High - requires careful binary parsing and memory management
**Testing Required**: Extensive - must handle malformed binaries safely
**Cross-Platform**: Supports Unix/Linux (ELF), Windows (PE), macOS (Mach-O) binaries
