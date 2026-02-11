# Chapter 7: System Programming

## 7.1 Exception Handling

### Exception Types

DLX supports multiple exception handling mechanisms:

**1. Precise Exceptions**
- Synchronous events (divide by zero, illegal instruction)
- PC points to faulting instruction
- All prior instructions completed
- No subsequent instructions started

**2. Imprecise Exceptions**
- Asynchronous events (I/O interrupts)
- PC may not point to related instruction

### Exception Vectors

```c
#define EXC_VEC_RESET       0x00000000  /* Reset vector */
#define EXC_VEC_TLB_MISS    0x00000080  /* TLB miss */
#define EXC_VEC_GENERAL     0x00000180  /* General exception */
#define EXC_VEC_INTERRUPT   0x00000200  /* Interrupt */
#define EXC_VEC_NMI         0x00000280  /* Non-maskable interrupt */
```

### Exception Cause Codes

```c
#define EXC_INT         0   /* External interrupt */
#define EXC_MOD         1   /* TLB modification exception */
#define EXC_TLBL        2   /* TLB load exception */
#define EXC_TLBS        3   /* TLB store exception */
#define EXC_ADEL        4   /* Address error (load) */
#define EXC_ADES        5   /* Address error (store) */
#define EXC_IBE         6   /* Instruction bus error */
#define EXC_DBE         7   /* Data bus error */
#define EXC_SYS         8   /* System call (ecall) */
#define EXC_BP          9   /* Breakpoint (ebreak) */
#define EXC_RI          10  /* Reserved instruction */
#define EXC_CPU         11  /* Coprocessor unusable */
#define EXC_OV          12  /* Arithmetic overflow */
#define EXC_TR          13  /* Trap */
#define EXC_FPE         15  /* Floating-point exception */
```

### Exception Handler Entry

```assembly
exception_handler:
    # Save context
    csrrw   sp, sscratch, sp        # Swap sp with scratch
    addi    sp, sp, -320            # Allocate exception frame

    # Save all registers
    sd      x1, 8(sp)
    sd      x2, 16(sp)
    # ... save x3-x31

    # Read exception cause
    csrr    t0, scause              # Read cause
    csrr    t1, sepc                # Read exception PC
    csrr    t2, stval               # Read fault address

    # Dispatch to handler
    call    handle_exception

    # Restore context
    ld      x1, 8(sp)
    # ... restore all registers
    addi    sp, sp, 320
    csrrw   sp, sscratch, sp
    sret                            # Return from exception
```

## 7.2 System Calls

### System Call Interface

```assembly
# System call wrapper
write:
    li      a7, 64                  # Syscall number (write)
    # a0 = fd, a1 = buf, a2 = count
    ecall                           # Trap to kernel
    ret                             # Return with result in a0
```

### System Call Numbers (Linux-compatible)

```c
#define SYS_read        63
#define SYS_write       64
#define SYS_open        56
#define SYS_close       57
#define SYS_exit        93
#define SYS_brk         214
#define SYS_mmap        222
#define SYS_munmap      215
#define SYS_clone       220
#define SYS_execve      221
```

## 7.3 Dynamic Linking

### Dynamic Linker

The dynamic linker (`ld-dlx.so`) handles:
- Loading shared libraries
- Symbol resolution
- Relocation processing
- Lazy binding (PLT/GOT)

### Lazy Binding via PLT

```assembly
# PLT entry for function
function@PLT:
    auipc   t0, %pcrel_hi(.got.plt)
    ld      t1, %pcrel_lo(.got.plt)(t0)
    jr      t1

# First call goes to resolver
.plt_resolver:
    # Push relocation index
    auipc   t0, %pcrel_hi(.got.plt)
    ld      t1, 8(t0)               # Load link map
    ld      t2, 16(t0)              # Load resolver address
    jr      t2                      # Call resolver
```

### Symbol Resolution

```c
/* Dynamic symbol table entry */
typedef struct {
    uint32_t st_name;               /* Symbol name (string table index) */
    uint8_t  st_info;               /* Symbol type and binding */
    uint8_t  st_other;              /* Symbol visibility */
    uint16_t st_shndx;              /* Section index */
    uint64_t st_value;              /* Symbol value */
    uint64_t st_size;               /* Symbol size */
} Elf64_Sym;
```

## 7.4 Interrupt Handling

### Interrupt Controller

```c
/* APIC (Advanced Programmable Interrupt Controller) */
#define APIC_BASE       0xFFFE0000

struct apic_regs {
    uint32_t id;                    /* APIC ID */
    uint32_t version;               /* APIC version */
    uint32_t tpr;                   /* Task priority */
    uint32_t apr;                   /* Arbitration priority */
    uint32_t ppr;                   /* Processor priority */
    uint32_t eoi;                   /* End of interrupt */
    uint32_t ldr;                   /* Logical destination */
    uint32_t dfr;                   /* Destination format */
    uint32_t svr;                   /* Spurious interrupt vector */
    uint32_t isr[8];                /* In-service register */
    uint32_t tmr[8];                /* Trigger mode register */
    uint32_t irr[8];                /* Interrupt request register */
};
```

### Interrupt Service Routine

```assembly
irq_handler:
    # Save minimal context
    csrrw   sp, sscratch, sp
    addi    sp, sp, -64
    sd      ra, 56(sp)
    sd      t0, 48(sp)
    sd      t1, 40(sp)
    sd      a0, 32(sp)

    # Read interrupt vector
    li      t0, APIC_BASE
    lw      a0, APIC_IVR(t0)        # Get interrupt vector

    # Call C handler
    call    handle_irq

    # Send EOI
    li      t0, APIC_BASE
    sw      zero, APIC_EOI(t0)

    # Restore context
    ld      a0, 32(sp)
    ld      t1, 40(sp)
    ld      t0, 48(sp)
    ld      ra, 56(sp)
    addi    sp, sp, 64
    csrrw   sp, sscratch, sp
    sret
```

## 7.5 Context Switching

### Process Context

```c
struct task_context {
    uint64_t regs[32];              /* General-purpose registers */
    uint64_t fregs[32];             /* Floating-point registers */
    uint64_t pc;                    /* Program counter */
    uint64_t sp;                    /* Stack pointer */
    uint64_t status;                /* Status register */
    uint64_t satp;                  /* Page table pointer */
    uint64_t tp;                    /* Thread pointer (TLS) */
};
```

### Context Switch Code

```assembly
switch_to:
    # Save current context (a0 = old task)
    sd      sp, 0(a0)
    sd      ra, 8(a0)
    sd      s0, 16(a0)
    # ... save s1-s11, fs0-fs11

    # Load new context (a1 = new task)
    ld      sp, 0(a1)
    ld      ra, 8(a1)
    ld      s0, 16(a1)
    # ... restore s1-s11, fs0-fs11

    # Switch page tables
    ld      t0, TASK_SATP(a1)
    csrw    satp, t0
    sfence.vma                      # Flush TLB

    ret
```

## 7.6 Memory Management

### Page Table Entry

```c
typedef struct {
    uint64_t valid      : 1;        /* Valid bit */
    uint64_t readable   : 1;        /* Read permission */
    uint64_t writable   : 1;        /* Write permission */
    uint64_t executable : 1;        /* Execute permission */
    uint64_t user       : 1;        /* User accessible */
    uint64_t global     : 1;        /* Global mapping */
    uint64_t accessed   : 1;        /* Accessed bit */
    uint64_t dirty      : 1;        /* Dirty bit */
    uint64_t rsw        : 2;        /* Reserved for software */
    uint64_t ppn        : 44;       /* Physical page number */
    uint64_t reserved   : 10;
} pte_t;
```

### TLB Management

```assembly
# Flush entire TLB
sfence.vma  zero, zero

# Flush specific virtual address
sfence.vma  a0, zero

# Flush specific ASID
sfence.vma  zero, a1

# Flush specific VA in specific ASID
sfence.vma  a0, a1
```
