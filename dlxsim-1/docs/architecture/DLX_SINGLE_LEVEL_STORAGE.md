# DLX Single-Level Storage - AS/400 Inspired Architecture

## Overview

This document specifies a DLX implementation of IBM AS/400-inspired single-level storage with 128-bit addresses, tagged memory, and object-based architecture. The design enables persistent objects, automatic paging, and capability-based security.

## AS/400 Architecture Background

The IBM AS/400 (now IBM i) introduced revolutionary concepts:
- **Single-Level Storage (SLS)**: Unified address space spanning RAM and disk
- **128-bit Pointers**: 64-bit object ID + 64-bit offset
- **Tagged Memory**: Hardware metadata bits for pointer validation
- **Object-Based**: All data encapsulated in typed objects
- **TIMI**: Technology-independent machine interface
- **Persistent Objects**: Survive across reboots
- **Capability-Based Security**: Pointer possession implies access rights

## 128-Bit Address Space Architecture

### Address Format

```c
/* 128-bit pointer format (AS/400-inspired) */
typedef struct {
    uint64_t object_id;     /* High 64 bits: Object identifier */
    uint64_t offset;        /* Low 64 bits: Offset within object */
} ptr128_t;

/* Object ID structure (64-bit) */
typedef struct {
    uint32_t context:16;    /* Bits 63-48: Context/domain ID */
    uint32_t type:16;       /* Bits 47-32: Object type */
    uint32_t index:32;      /* Bits 31-0: Object index in table */
} object_id_t;

/* Complete 128-bit address */
┌────────────────────────────────────────────────┬────────────────────────────────────────────────┐
│           Object ID (64 bits)                  │         Offset (64 bits)                       │
├────────┬────────┬───────────────────────────────┼────────────────────────────────────────────────┤
│Context │  Type  │       Index                   │        Offset within object                    │
│  16    │   16   │        32                     │              64                                │
└────────┴────────┴───────────────────────────────┴────────────────────────────────────────────────┘

Example:
Object ID = 0x0001002000000042 (Context 1, Type 32, Index 66)
Offset    = 0x0000000000001000 (Offset 4096 within object)
Full ptr  = 0x00010020000000420000000000001000
```

### Object Types

```c
/* AS/400-inspired object types */
typedef enum {
    OBJ_TYPE_SPACE      = 0x0001,   /* Storage space */
    OBJ_TYPE_PROGRAM    = 0x0201,   /* Executable program */
    OBJ_TYPE_FILE       = 0x0401,   /* Data file */
    OBJ_TYPE_QUEUE      = 0x0A01,   /* Message queue */
    OBJ_TYPE_LIBRARY    = 0x0421,   /* Library */
    OBJ_TYPE_CONTEXT    = 0x0801,   /* Context/domain */
    OBJ_TYPE_INDEX      = 0x0E01,   /* Index object */
    OBJ_TYPE_DEVICE     = 0x1001,   /* Device object */
    OBJ_TYPE_MEMORY     = 0x1201,   /* Memory region */
    OBJ_TYPE_JOURNAL    = 0x1401,   /* Journal */
} object_type_t;

/* Object header (stored at offset 0 of every object) */
typedef struct {
    uint64_t magic;             /* Magic number: 0x4153343030424A54 "AS400OBJ" */
    uint64_t object_id;         /* This object's ID */
    uint16_t type;              /* Object type */
    uint16_t subtype;           /* Object subtype */
    uint32_t size;              /* Object size in bytes */
    uint64_t creation_time;     /* Creation timestamp */
    uint64_t modification_time; /* Last modification */
    uint32_t owner_uid;         /* Owner user ID */
    uint32_t owner_gid;         /* Owner group ID */
    uint32_t permissions;       /* Object permissions */
    uint32_t flags;             /* Object flags */
    uint64_t parent_object_id;  /* Parent object (e.g., library) */
    char name[256];             /* Object name */
    uint8_t reserved[256];      /* Reserved for future use */
} object_header_t;
```

## Tagged Memory

### Tag Bit Architecture

```c
/*
 * Tagged Memory: 1 tag bit per 8 bytes (64 bits) of memory
 *
 * Memory organization:
 * - Main memory: 64-bit words
 * - Tag memory: 1 bit per 64-bit word
 * - Tag bit set (1) = word contains a pointer
 * - Tag bit clear (0) = word contains data
 *
 * Hardware enforces:
 * - Pointers can only be loaded with tag=1
 * - Non-pointer data cannot be used as pointer
 * - Prevents pointer forgery
 */

/* Tag memory layout (1 bit per 8 bytes = 1/64 overhead) */
#define TAG_BITS_PER_WORD   1
#define BYTES_PER_TAG       8

/* Tag memory is stored separately from main memory */
/* For 4GB main memory, need 64MB tag memory */

/* CP0 register for tag memory base */
#define SR_TAG_MEMORY_BASE  0x60

/* Enable tagged memory */
#define SR_TAG_ENABLE       0x61

typedef struct {
    uint32_t enable:1;          /* Bit 0: Enable tagged memory */
    uint32_t strict:1;          /* Bit 1: Strict tag checking */
    uint32_t trap_on_violation:1; /* Bit 2: Trap on tag violation */
    uint32_t log_violations:1;  /* Bit 3: Log violations */
    uint32_t reserved:28;
} tag_config_t;
```

### Tag Operations

```assembly
# Tagged load/store instructions (hardware enforced)

# Load with tag check (LWT = Load Word Tagged)
LWT     rd, (rs)            # Load word, check tag=1, trap if tag=0
LDT     rd, (rs)            # Load doubleword, check tag=1
LWTU    rd, (rs)            # Load word tagged, unsigned

# Store with tag set (SWT = Store Word Tagged)
SWT     rt, (rs)            # Store word, set tag=1
SDT     rt, (rs)            # Store doubleword, set tag=1

# Store without tag (normal data)
SW      rt, (rs)            # Store word, set tag=0 (normal store)
SD      rt, (rs)            # Store doubleword, set tag=0

# Query tag bit
GETTAG  rd, (rs)            # rd = tag_bit(mem[rs])
SETTAG  (rs), rt            # tag_bit(mem[rs]) = rt[0]

# Examples
    # Store pointer (sets tag bit)
    LI      r3, some_object
    SWT     r3, ptr_location    # Store with tag=1

    # Store data (clears tag bit)
    LI      r4, 12345
    SW      r4, data_location   # Store with tag=0

    # Load pointer (checks tag bit)
    LWT     r5, ptr_location    # OK: tag=1
    LW      r6, data_location   # OK: tag=0 (normal load)

    # This would trap:
    # LWT     r7, data_location  # TRAP: tag=0, expected tag=1
```

### Tag Bit Implementation

```c
/* Get tag bit for address */
static inline int get_tag_bit(uint32_t addr)
{
    uint32_t tag_base = read_cp0_reg(SR_TAG_MEMORY_BASE);
    uint32_t tag_offset = (addr >> 3);      /* Divide by 8 */
    uint32_t tag_byte_offset = tag_offset >> 3;
    uint32_t tag_bit_offset = tag_offset & 7;

    uint8_t tag_byte = *(uint8_t *)(tag_base + tag_byte_offset);
    return (tag_byte >> tag_bit_offset) & 1;
}

/* Set tag bit for address */
static inline void set_tag_bit(uint32_t addr, int value)
{
    uint32_t tag_base = read_cp0_reg(SR_TAG_MEMORY_BASE);
    uint32_t tag_offset = (addr >> 3);
    uint32_t tag_byte_offset = tag_offset >> 3;
    uint32_t tag_bit_offset = tag_offset & 7;

    uint8_t *tag_byte = (uint8_t *)(tag_base + tag_byte_offset);
    if (value)
        *tag_byte |= (1 << tag_bit_offset);
    else
        *tag_byte &= ~(1 << tag_bit_offset);
}

/* Hardware tag check on pointer load */
void hardware_tag_check_load(uint32_t addr)
{
    if (!get_tag_bit(addr)) {
        /* Tag violation: loading pointer from non-tagged memory */
        generate_exception(EXCEPT_TAG_VIOLATION, addr);
    }
}
```

## Single-Level Storage

### Storage Hierarchy

```
┌─────────────────────────────────────────────────────┐
│         Application View (128-bit addresses)        │
│  Single unified address space: 2^128 bytes          │
└──────────────────┬──────────────────────────────────┘
                   │
                   ▼
┌─────────────────────────────────────────────────────┐
│      Object Table (maps object IDs to locations)    │
│  - Object ID → Physical location mapping            │
│  - Object attributes (type, size, permissions)      │
│  - Residency status (in RAM, on disk, or both)      │
└──────────────────┬──────────────────────────────────┘
                   │
         ┌─────────┴─────────┐
         ▼                   ▼
┌─────────────────┐  ┌──────────────────┐
│   Main Memory   │  │   Backing Store  │
│   (RAM)         │  │   (Disk)         │
│   Fast access   │  │   Persistent     │
│   Volatile      │  │   Non-volatile   │
└─────────────────┘  └──────────────────┘

Automatic paging between RAM and disk (transparent to application)
```

### Object Table

```c
/* Object table entry (OTE) */
typedef struct {
    uint64_t object_id;         /* Object identifier */
    uint16_t type;              /* Object type */
    uint16_t flags;             /* Object flags */
    uint32_t size;              /* Object size in bytes */

    /* Location information */
    uint64_t ram_address;       /* Physical RAM address (0 if not in RAM) */
    uint64_t disk_block;        /* Disk block number (0 if not on disk) */

    /* Residency status */
    uint8_t in_ram:1;           /* Object is in RAM */
    uint8_t in_disk:1;          /* Object is on disk */
    uint8_t modified:1;         /* RAM copy modified (needs writeback) */
    uint8_t permanent:1;        /* Permanent object (survives reboot) */
    uint8_t locked:1;           /* Object locked in RAM (no pageout) */
    uint8_t accessed:1;         /* Recently accessed (for LRU) */
    uint8_t reserved:2;

    /* Access control */
    uint32_t owner_uid;
    uint32_t owner_gid;
    uint32_t permissions;

    /* Statistics */
    uint64_t access_count;
    uint64_t last_access_time;
    uint64_t creation_time;

    /* Linked list for LRU replacement */
    struct object_table_entry *lru_prev;
    struct object_table_entry *lru_next;
} object_table_entry_t;

/* Global object table */
#define MAX_OBJECTS  (1 << 24)  /* 16 million objects */

object_table_entry_t object_table[MAX_OBJECTS];

/* Object table base register */
#define SR_OBJECT_TABLE_BASE  0x62
#define SR_OBJECT_TABLE_SIZE  0x63
```

### Address Translation

```c
/* Translate 128-bit address to physical address */
uint32_t translate_128bit_address(ptr128_t ptr)
{
    object_id_t oid;
    object_table_entry_t *ote;
    uint32_t phys_addr;

    /* Extract object ID components */
    oid.context = (ptr.object_id >> 48) & 0xFFFF;
    oid.type = (ptr.object_id >> 32) & 0xFFFF;
    oid.index = ptr.object_id & 0xFFFFFFFF;

    /* Look up object in table */
    if (oid.index >= MAX_OBJECTS)
        return 0;  /* Invalid object */

    ote = &object_table[oid.index];

    /* Verify object exists and types match */
    if (ote->object_id != ptr.object_id)
        return 0;  /* Object not found or ID mismatch */

    if (ote->type != oid.type)
        return 0;  /* Type mismatch */

    /* Check if object is in RAM */
    if (!ote->in_ram) {
        /* Page fault: bring object from disk to RAM */
        page_in_object(ote);
    }

    /* Update access statistics */
    ote->accessed = 1;
    ote->access_count++;
    ote->last_access_time = get_system_time();

    /* Calculate physical address */
    phys_addr = ote->ram_address + ptr.offset;

    /* Bounds check */
    if (ptr.offset >= ote->size)
        return 0;  /* Offset out of bounds */

    return phys_addr;
}
```

### Automatic Paging

```c
/* Page in object from disk to RAM */
int page_in_object(object_table_entry_t *ote)
{
    void *ram_addr;
    uint64_t disk_block;
    uint32_t size;

    /* Check if already in RAM */
    if (ote->in_ram)
        return 0;

    /* Allocate RAM for object */
    ram_addr = allocate_object_memory(ote->size);
    if (!ram_addr) {
        /* Out of memory: need to page out another object */
        page_out_lru_object();
        ram_addr = allocate_object_memory(ote->size);
        if (!ram_addr)
            return -1;  /* Still no memory */
    }

    /* Read object from disk */
    disk_block = ote->disk_block;
    size = ote->size;

    disk_read(disk_block, ram_addr, size);

    /* Update OTE */
    ote->ram_address = (uint64_t)ram_addr;
    ote->in_ram = 1;
    ote->modified = 0;
    ote->accessed = 1;

    /* Add to LRU list */
    lru_add(ote);

    return 0;
}

/* Page out object from RAM to disk (LRU replacement) */
int page_out_lru_object(void)
{
    object_table_entry_t *ote;

    /* Find least recently used object */
    ote = lru_get_oldest();
    if (!ote)
        return -1;

    /* Don't page out locked objects */
    if (ote->locked)
        return -1;

    /* If modified, write back to disk */
    if (ote->modified) {
        disk_write(ote->disk_block, (void *)ote->ram_address, ote->size);
        ote->modified = 0;
    }

    /* Free RAM */
    free_object_memory((void *)ote->ram_address, ote->size);

    /* Update OTE */
    ote->ram_address = 0;
    ote->in_ram = 0;

    /* Remove from LRU list */
    lru_remove(ote);

    return 0;
}
```

## 128-Bit Pointer Instructions

### Load/Store with 128-bit Pointers

```assembly
# Load via 128-bit pointer (extended addressing)
# Pointer is stored in two consecutive 64-bit registers

LW128   rd, (rh:rl)         # Load word via 128-bit ptr (rh=high 64, rl=low 64)
LD128   rd, (rh:rl)         # Load doubleword via 128-bit ptr
LH128   rd, (rh:rl)         # Load halfword via 128-bit ptr
LB128   rd, (rh:rl)         # Load byte via 128-bit ptr

SW128   rt, (rh:rl)         # Store word via 128-bit ptr
SD128   rt, (rh:rl)         # Store doubleword via 128-bit ptr
SH128   rt, (rh:rl)         # Store halfword via 128-bit ptr
SB128   rt, (rh:rl)         # Store byte via 128-bit ptr

# Load 128-bit pointer
LDP128  rdh:rdl, (rs)       # Load 128-bit pointer from memory
STP128  rth:rtl, (rs)       # Store 128-bit pointer to memory

# Examples
    # Load word via 128-bit pointer in r4:r5
    LW128   r3, (r4:r5)     # r3 = mem[translate_128(r4:r5)]

    # Store word via 128-bit pointer
    SW128   r6, (r4:r5)     # mem[translate_128(r4:r5)] = r6

    # Load 128-bit pointer from memory
    LDP128  r8:r9, (r10)    # r8:r9 = 128-bit ptr at mem[r10]
```

### Pointer Arithmetic

```assembly
# 128-bit pointer arithmetic
ADDP128 rdh:rdl, rsh:rsl, offset   # ptr = ptr + offset
SUBP128 rdh:rdl, rsh:rsl, offset   # ptr = ptr - offset

# Get object ID from pointer
GETOID  rd, rh:rl           # rd = object_id(ptr)

# Get offset from pointer
GETOFF  rd, rh:rl           # rd = offset(ptr)

# Construct 128-bit pointer
MKPTR   rdh:rdl, oid, offset   # ptr = (oid << 64) | offset

# Examples
    # Add 16 to 128-bit pointer
    ADDP128 r4:r5, r4:r5, 16    # ptr += 16

    # Extract object ID
    GETOID  r6, r4:r5           # r6 = object_id(ptr)

    # Create pointer to object 42, offset 1000
    LI      r7, 42
    LI      r8, 1000
    MKPTR   r10:r11, r7, r8     # r10:r11 = ptr(42, 1000)
```

## Object Management

### Object Creation

```c
/* Create new object */
uint64_t create_object(uint16_t type, uint32_t size, int permanent)
{
    object_table_entry_t *ote;
    uint32_t index;
    uint64_t object_id;
    void *ram_addr;

    /* Allocate object table entry */
    index = allocate_object_index();
    if (index == 0)
        return 0;  /* Out of object slots */

    ote = &object_table[index];

    /* Generate object ID */
    object_id = ((uint64_t)get_current_context() << 48) |
                ((uint64_t)type << 32) |
                index;

    /* Allocate RAM for object */
    ram_addr = allocate_object_memory(size);
    if (!ram_addr) {
        free_object_index(index);
        return 0;
    }

    /* Initialize object header */
    object_header_t *header = (object_header_t *)ram_addr;
    header->magic = 0x4153343030424A54;  /* "AS400OBJ" */
    header->object_id = object_id;
    header->type = type;
    header->size = size;
    header->creation_time = get_system_time();
    header->owner_uid = get_current_uid();
    header->owner_gid = get_current_gid();
    header->permissions = 0644;

    /* Initialize OTE */
    ote->object_id = object_id;
    ote->type = type;
    ote->size = size;
    ote->ram_address = (uint64_t)ram_addr;
    ote->disk_block = 0;
    ote->in_ram = 1;
    ote->in_disk = 0;
    ote->modified = 1;
    ote->permanent = permanent;
    ote->locked = 0;
    ote->owner_uid = header->owner_uid;
    ote->owner_gid = header->owner_gid;
    ote->permissions = header->permissions;
    ote->creation_time = header->creation_time;

    /* If permanent, allocate disk space */
    if (permanent) {
        ote->disk_block = allocate_disk_blocks(size);
        ote->in_disk = 1;
    }

    return object_id;
}
```

### Object Deletion

```c
/* Delete object */
int delete_object(uint64_t object_id)
{
    object_table_entry_t *ote;
    uint32_t index;

    /* Look up object */
    index = object_id & 0xFFFFFFFF;
    if (index >= MAX_OBJECTS)
        return -1;

    ote = &object_table[index];

    /* Verify object ID */
    if (ote->object_id != object_id)
        return -1;

    /* Check permissions */
    if (!can_delete_object(ote))
        return -1;

    /* Free RAM if in memory */
    if (ote->in_ram) {
        free_object_memory((void *)ote->ram_address, ote->size);
        lru_remove(ote);
    }

    /* Free disk blocks if permanent */
    if (ote->in_disk) {
        free_disk_blocks(ote->disk_block, ote->size);
    }

    /* Free object table entry */
    memset(ote, 0, sizeof(*ote));
    free_object_index(index);

    return 0;
}
```

## Capability-Based Security

### Pointer Capabilities

```c
/* Pointer capability bits (encoded in unused bits of pointer) */
typedef struct {
    uint64_t object_id;         /* Object identifier */
    uint64_t offset:48;         /* Offset within object */
    uint64_t read:1;            /* Read permission */
    uint64_t write:1;           /* Write permission */
    uint64_t execute:1;         /* Execute permission */
    uint64_t delete:1;          /* Delete permission */
    uint64_t authorized:1;      /* Authorized pointer */
    uint64_t reserved:11;       /* Reserved for future use */
} capability_ptr_t;

/* Check pointer capabilities before access */
int check_pointer_capability(capability_ptr_t *ptr, int access_type)
{
    /* Verify pointer is authorized */
    if (!ptr->authorized)
        return -1;  /* Unauthorized pointer */

    /* Check requested access against capabilities */
    if ((access_type == ACCESS_READ) && !ptr->read)
        return -1;
    if ((access_type == ACCESS_WRITE) && !ptr->write)
        return -1;
    if ((access_type == ACCESS_EXECUTE) && !ptr->execute)
        return -1;

    return 0;
}
```

### Pointer Security Instructions

```assembly
# Authorize pointer (kernel only)
AUTHPTR rdh:rdl, rsh:rsl, permissions   # Authorize pointer with permissions

# Restrict pointer capabilities
RESTRICT rdh:rdl, rsh:rsl, mask         # Remove capabilities from pointer

# Check pointer authorization
CHKAUTH r1, rh:rl, access_type          # r1 = can_access(ptr, type)

# Examples
    # Authorize pointer for read-only access
    LI      r6, 0x1             # READ permission
    AUTHPTR r4:r5, r4:r5, r6    # Authorize with READ only

    # Check if pointer allows write
    LI      r7, 0x2             # WRITE access
    CHKAUTH r8, r4:r5, r7       # r8 = can_write(ptr)
    BEQZ    r8, .no_write       # Branch if no write permission
```

## Persistent Objects and Journaling

### Object Persistence

```c
/* Persist modified objects to disk */
void sync_objects_to_disk(void)
{
    object_table_entry_t *ote;
    int i;

    for (i = 0; i < MAX_OBJECTS; i++) {
        ote = &object_table[i];

        /* Skip empty slots */
        if (ote->object_id == 0)
            continue;

        /* Skip temporary objects */
        if (!ote->permanent)
            continue;

        /* Write modified objects to disk */
        if (ote->in_ram && ote->modified) {
            disk_write(ote->disk_block,
                      (void *)ote->ram_address,
                      ote->size);
            ote->modified = 0;
        }
    }
}

/* Restore objects from disk after reboot */
void restore_objects_from_disk(void)
{
    /* Scan disk for persistent objects */
    /* Rebuild object table from disk metadata */
    /* Objects are not loaded into RAM until first access */
}
```

### Journaling for Crash Recovery

```c
/* Journal entry for object modifications */
typedef struct {
    uint64_t sequence_number;
    uint64_t timestamp;
    uint64_t object_id;
    uint64_t offset;
    uint32_t length;
    uint8_t operation;      /* CREATE, MODIFY, DELETE */
    uint8_t data[];         /* Before/after images */
} journal_entry_t;

/* Write journal entry before modifying object */
void journal_write(uint64_t object_id, uint64_t offset,
                  void *data, uint32_t length)
{
    journal_entry_t *entry;

    entry = allocate_journal_entry(sizeof(*entry) + length * 2);
    entry->sequence_number = next_journal_sequence++;
    entry->timestamp = get_system_time();
    entry->object_id = object_id;
    entry->offset = offset;
    entry->length = length;
    entry->operation = JOURNAL_MODIFY;

    /* Save before image */
    memcpy(entry->data, (void *)(object_addr + offset), length);

    /* Save after image */
    memcpy(entry->data + length, data, length);

    /* Write journal entry to disk */
    write_journal_entry(entry);
}
```

## System Calls

### Single-Level Storage System Calls

```c
/* System call: Create object */
SYSCALL_DEFINE3(create_object, uint16_t, type, uint32_t, size, int, permanent)
{
    uint64_t object_id;

    /* Check permissions */
    if (!can_create_object(type))
        return -EPERM;

    object_id = create_object(type, size, permanent);
    if (object_id == 0)
        return -ENOMEM;

    return object_id;
}

/* System call: Map object to address space */
SYSCALL_DEFINE1(map_object, uint64_t, object_id)
{
    ptr128_t ptr;

    /* Create 128-bit pointer to object */
    ptr.object_id = object_id;
    ptr.offset = 0;

    /* Return pointer (split across two registers) */
    regs->r1 = ptr.object_id;
    regs->r2 = ptr.offset;

    return 0;
}

/* System call: Get object information */
SYSCALL_DEFINE2(get_object_info, uint64_t, object_id,
                struct object_info *, info)
{
    object_table_entry_t *ote;
    uint32_t index;

    index = object_id & 0xFFFFFFFF;
    if (index >= MAX_OBJECTS)
        return -EINVAL;

    ote = &object_table[index];
    if (ote->object_id != object_id)
        return -ENOENT;

    /* Copy object information to user space */
    info->type = ote->type;
    info->size = ote->size;
    info->permissions = ote->permissions;
    info->in_ram = ote->in_ram;
    info->in_disk = ote->in_disk;

    return 0;
}
```

## Performance Considerations

```c
/*
 * Single-Level Storage Performance:
 *
 * Benefits:
 * - No malloc/free overhead
 * - No explicit file I/O (automatic paging)
 * - Persistent data structures (no serialization)
 * - Simplified programming model
 * - Object-level protection
 *
 * Costs:
 * - Page fault overhead (10-1000 µs for disk access)
 * - Tag bit overhead (1/64 = 1.56% memory)
 * - Object table lookup (2-3 cycles with caching)
 * - Address translation (5-10 cycles)
 *
 * Optimization strategies:
 * - Cache frequently used objects in RAM
 * - Prefetch objects based on access patterns
 * - Group related objects for locality
 * - Use memory-mapped I/O for devices
 * - Batch disk I/O operations
 * - Lazy writeback of modified objects
 */
```

## Example Usage

### Creating and Using Objects

```c
/* Example: Create persistent data structure */
void example_create_persistent_array(void)
{
    uint64_t array_object_id;
    ptr128_t array_ptr;
    int i;

    /* Create permanent array object (1MB) */
    array_object_id = syscall(SYS_create_object,
                              OBJ_TYPE_MEMORY,
                              1024 * 1024,
                              1);  /* permanent */

    /* Map object to get 128-bit pointer */
    syscall(SYS_map_object, array_object_id);
    array_ptr.object_id = current->regs.r1;
    array_ptr.offset = current->regs.r2;

    /* Write data to array (automatically paged) */
    for (i = 0; i < 256000; i++) {
        uint32_t *addr = translate_128bit_address(array_ptr);
        *addr = i * i;
        array_ptr.offset += 4;
    }

    /* Object persists across reboots */
    /* No need to explicitly save/load */
}

/* Example: Access persistent object after reboot */
void example_access_after_reboot(uint64_t array_object_id)
{
    ptr128_t array_ptr;
    int i;

    /* Map existing object */
    syscall(SYS_map_object, array_object_id);
    array_ptr.object_id = current->regs.r1;
    array_ptr.offset = current->regs.r2;

    /* Data is still there! */
    for (i = 0; i < 10; i++) {
        uint32_t *addr = translate_128bit_address(array_ptr);
        printf("array[%d] = %u\n", i, *addr);
        array_ptr.offset += 4;
    }
}
```

## Instruction Encoding

```
┌────────┬────┬────┬────┬────┬──────────┐
│ Opcode │ rd │ rh │ rl │Func│ Reserved │  128-bit load/store
│   6    │ 5  │ 5  │ 5  │ 6  │    5     │
└────────┴────┴────┴────┴────┴──────────┘

Opcode: 0x3F (extended operations)
rh: High 64 bits of 128-bit pointer
rl: Low 64 bits of 128-bit pointer
Func:
  0x00: LW128  (load word)
  0x01: LH128  (load halfword)
  0x02: LB128  (load byte)
  0x03: LD128  (load doubleword)
  0x08: SW128  (store word)
  0x09: SH128  (store halfword)
  0x0A: SB128  (store byte)
  0x0B: SD128  (store doubleword)
  0x10: LDP128 (load 128-bit pointer)
  0x11: STP128 (store 128-bit pointer)
```

## Summary

**Key Features**:
- 128-bit address space (64-bit object ID + 64-bit offset)
- Single-level storage spanning RAM and disk
- Tagged memory with hardware pointer validation
- Object-based architecture with typed objects
- Automatic paging between RAM and disk
- Persistent objects surviving reboots
- Capability-based security model
- Journaling for crash recovery
- 16 million objects maximum
- 1.56% memory overhead for tag bits

**AS/400 Compatibility**:
- Inspired by IBM AS/400 architecture
- Similar object model and addressing
- PowerPC-AS style tagged memory
- Compatible pointer capabilities
- Persistent object semantics

**Use Cases**:
- Persistent data structures (databases, caches)
- Simplified file I/O (no explicit open/read/write)
- Object-oriented operating systems
- Capability-based security systems
- Transparent persistence across reboots
- Single address space for all storage

---

**Document Status**: Specification Complete
**Implementation Status**: Ready for kernel integration
**Complexity**: Very High - requires extensive OS support
**Testing Required**: Extensive - data persistence and recovery critical
**AS/400 Compatibility**: Architectural concepts adapted for DLX
