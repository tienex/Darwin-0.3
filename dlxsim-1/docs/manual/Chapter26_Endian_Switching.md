# Chapter 26: Endian Switching

## 26.1 Overview

DLX supports dynamic endianness control:
- **Little-endian** (default on most systems)
- **Big-endian** (network byte order, legacy systems)
- **Per-privilege-level control**
- **Bi-endian** (runtime switching)

## 26.2 Endian Control Register

```c
typedef struct {
    uint32_t kernel_endian  : 1;    /* 0=LE, 1=BE */
    uint32_t super_endian   : 1;    /* Supervisor endianness */
    uint32_t hyper_endian   : 1;    /* Hypervisor endianness */
    uint32_t user_endian    : 1;    /* User endianness */
    uint32_t default_endian : 1;    /* Boot default */
    uint32_t switch_on_except : 1;  /* Auto-switch on exceptions */
    uint32_t reserved       : 26;
} endian_ctrl_t;

/* CSR address */
#define CSR_ENDIAN_CTRL 0x7E0
```

## 26.3 Endian Switching Instructions

```assembly
# Switch to big-endian
setend.be                       # Current level ← big-endian

# Switch to little-endian
setend.le                       # Current level ← little-endian

# Query current endianness
getend      rd                  # rd ← 0 (LE) or 1 (BE)

# Conditional code
getend      t0
bnez        t0, big_endian_path
little_endian_path:
    # LE-specific code
    j       done
big_endian_path:
    # BE-specific code
done:
```

## 26.4 Byte-Reversed Load/Store

Manual endian conversion:

```assembly
# Byte-reversed loads
lhbrx   rd, rs1, rs2            # Load halfword byte-reversed indexed
lwbrx   rd, rs1, rs2            # Load word byte-reversed indexed
ldbrx   rd, rs1, rs2            # Load doubleword byte-reversed indexed

# Byte-reversed stores
sthbrx  rs, rd1, rd2            # Store halfword byte-reversed indexed
stwbrx  rs, rd1, rd2            # Store word byte-reversed indexed
stdbrx  rs, rd1, rd2            # Store doubleword byte-reversed indexed

# Example: Read big-endian value on LE system
    la      t0, data_addr
    lwbrx   a0, t0, zero        # Load with byte reversal
```

## 26.5 Byte Reversal Instructions

```assembly
# Reverse bytes
rev8    rd, rs                  # Reverse all bytes (64-bit)
rev16   rd, rs                  # Reverse within halfwords
rev32   rd, rs                  # Reverse within words

# Reverse bits
rbit    rd, rs                  # Reverse all bits

# Example: Manual endian conversion
    lw      a0, data
    rev32   a0, a0              # Convert endianness
```

## 26.6 Per-Level Endianness

```
Ring 0 (Kernel):    Configurable (typically LE)
Ring 1 (Driver):    Configurable (typically LE)
Ring 2 (Service):   Configurable (typically LE)
Ring 3 (User):      Configurable (per-process)
```

### Endian Transition Example

```assembly
# User process in BE mode calls kernel in LE mode
syscall:
    ecall                       # Trap to kernel
    # Automatic endian switch: BE → LE
    # Kernel runs in LE
    # ...
    eret                        # Return from exception
    # Automatic endian switch: LE → BE
```

## 26.7 Network Byte Order

```assembly
# Host to network byte order
htonl   rd, rs                  # Host to network long (32-bit)
htons   rd, rs                  # Host to network short (16-bit)

# Network to host byte order
ntohl   rd, rs                  # Network to host long
ntohs   rd, rs                  # Network to host short

# Example usage
    lw      a0, ip_address      # Load IP address
    htonl   a0, a0              # Convert to network byte order
    sw      a0, packet_buffer
```

## 26.8 Mixed-Endian Data Structures

```c
/* Structure with endian-specific fields */
struct mixed_endian {
    uint32_t le_field;          /* Little-endian field */
    uint32_t be_field;          /* Big-endian field */
} __attribute__((packed));

/* Access with explicit conversions */
void read_mixed(struct mixed_endian *p) {
    uint32_t le_val = p->le_field;      /* Native LE */
    uint32_t be_val;
    asm("lwbrx %0, %1, zero" : "=r"(be_val) : "r"(&p->be_field));
}
```

## 26.9 Compiler Support

```c
/* GCC attributes */
__attribute__((scalar_storage_order("big-endian"))) struct {
    uint32_t field1;
    uint32_t field2;
} be_struct;

__attribute__((scalar_storage_order("little-endian"))) struct {
    uint32_t field1;
    uint32_t field2;
} le_struct;
```

## 26.10 Performance Considerations

- **Hardware swap**: Byte-reversed loads/stores ~1 cycle overhead
- **Software swap**: rev32 instruction ~1 cycle
- **Endian switching**: Automatic on privilege transitions (~0 cycles)
- **I-cache**: Separate caches for LE/BE code (optional)

## 26.11 Debugging Endian Issues

```assembly
# Debug print in both endiannesses
debug_print:
    getend  t0
    bnez    t0, print_be
print_le:
    lw      a0, value
    j       do_print
print_be:
    lwbrx   a0, value_addr, zero
do_print:
    call    printf
    ret
```

## 26.12 ELF Support

```c
/* ELF flags for endianness */
#define EF_DLX_BIG_ENDIAN    0x00000100
#define EF_DLX_LITTLE_ENDIAN 0x00000200

/* e_ident[EI_DATA] */
#define ELFDATA2LSB 1   /* Little-endian */
#define ELFDATA2MSB 2   /* Big-endian */
```
