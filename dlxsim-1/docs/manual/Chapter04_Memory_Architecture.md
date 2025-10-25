# Chapter 4: Memory Architecture

## 4.1 Address Spaces

### DLX64 Address Space (64-bit)
- **User space**: 0x0000_0000_0000_0000 to 0x0000_7FFF_FFFF_FFFF (128 TB)
- **Kernel space**: 0xFFFF_8000_0000_0000 to 0xFFFF_FFFF_FFFF_FFFF (128 TB)

### DLX128 Address Space (128-bit)
- **User space**: 2^96 bytes (practical limit: 79 billion TB)
- **Kernel space**: Upper 2^96 bytes
- **Device space**: Dedicated 2^96 byte region
- **Persistent storage**: Memory-mapped non-volatile storage

## 4.2 Endianness

DLX supports both little-endian and big-endian modes with **per-privilege-level control**:

### Endian Control Register
```c
typedef struct {
    uint32_t kernel_endian  : 1;    /* 0=LE, 1=BE */
    uint32_t super_endian   : 1;
    uint32_t hyper_endian   : 1;
    uint32_t user_endian    : 1;
    uint32_t default_endian : 1;    /* Boot default */
    uint32_t switch_on_except : 1;  /* Auto-switch on exceptions */
} endian_ctrl_t;
```

### Endian-Switching Instructions
```assembly
setend.be                       # Switch to big-endian
setend.le                       # Switch to little-endian
rev8    rd, rs                  # Reverse bytes (manual conversion)
```

## 4.3 Page-Based Virtual Memory

### Standard Page Sizes
- 4 KB (standard)
- 2 MB (large pages)
- 1 GB (huge pages)
- 16 GB (super-huge pages, DLX-specific)

### VMS-Style Page Protection (R Extension)
Advanced protection beyond standard RWX:

```c
typedef struct {
    uint64_t readable    : 1;
    uint64_t writable    : 1;
    uint64_t executable  : 1;
    uint64_t user        : 1;
    uint64_t global      : 1;
    uint64_t accessed    : 1;
    uint64_t dirty       : 1;

    /* VMS-style extended protection */
    uint64_t owner_read  : 1;   /* Owner can read */
    uint64_t owner_write : 1;   /* Owner can write */
    uint64_t owner_exec  : 1;   /* Owner can execute */
    uint64_t group_read  : 1;   /* Group can read */
    uint64_t group_write : 1;   /* Group can write */
    uint64_t group_exec  : 1;   /* Group can execute */
    uint64_t world_read  : 1;   /* World can read */
    uint64_t world_write : 1;   /* World can write */
    uint64_t world_exec  : 1;   /* World can execute */

    uint64_t pfn         : 44;  /* Physical frame number */
    uint64_t reserved    : 4;
} vms_pte_t;
```

## 4.4 4-Ring Protection Model (R Extension)

DLX supports x86-style 4-ring protection:

```
Ring 0: Kernel (most privileged)
Ring 1: Device drivers, OS services
Ring 2: OS servers, system services
Ring 3: User applications (least privileged)
```

### Ring Transition
```assembly
call.ring   target, new_ring    # Call with ring transition
ret.ring                        # Return to previous ring
```

### Ring Control Register
```c
typedef struct {
    uint32_t current_ring  : 2;    /* Current ring (0-3) */
    uint32_t previous_ring : 2;    /* Ring before transition */
    uint32_t ring_stack    : 8;    /* Ring transition stack */
    uint32_t enable_4ring  : 1;    /* Enable 4-ring mode */
} ring_ctrl_t;
```

## 4.5 Register Banking (R Extension)

Hardware context switching with separate register sets:

### Banking Modes
1. **Privilege-Based**: Bank per privilege level (User, Kernel, etc.)
2. **Exception-Based**: Different banks for exceptions/interrupts
3. **Manual**: Software-controlled bank switching
4. **Hybrid**: Combination of privilege + exception

### Bank Configuration
```c
typedef struct {
    uint32_t enable      : 1;    /* Enable register banking */
    uint32_t num_banks   : 2;    /* 2-4 banks */
    uint32_t auto_switch : 1;    /* Auto-switch on exceptions */
    uint32_t current_bank: 2;    /* Current active bank (0-3) */
    uint32_t bank_mode   : 2;    /* Banking mode */
} bank_config_t;
```

### Bank Switching
```assembly
setbank     bank_num            # Switch to bank (manual mode)
```

Each bank contains:
- Complete GPR set (r0-r31)
- Complete FPR set (f0-f31)
- Status snapshot
