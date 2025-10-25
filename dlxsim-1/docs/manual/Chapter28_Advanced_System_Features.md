# Chapter 28: Advanced System Features

## 28.1 Power Management

### Dynamic Voltage and Frequency Scaling (DVFS)

```assembly
# Set CPU frequency
csrw    cpu_freq, rs            # Frequency in MHz

# Set voltage
csrw    cpu_voltage, rs         # Voltage in mV

# Power states
wfi                             # Wait for interrupt (C1 state)
halt    C3                      # Deep sleep (C3 state)
halt    C6                      # Package C-state
```

### P-States (Performance States)
```c
struct pstate {
    uint32_t freq_mhz;          /* Frequency */
    uint32_t voltage_mv;        /* Voltage */
    uint32_t power_mw;          /* Power consumption */
};
```

### C-States (Sleep States)
```
C0  - Active
C1  - Clock gating (wfi)
C2  - Deeper clock gating
C3  - Cache flushed
C6  - Package sleep
```

## 28.2 Debug and Trace

### Hardware Breakpoints

```assembly
# Set instruction breakpoint
csrw    ibreak0, addr           # Breakpoint address
csrw    ibreak_ctrl, flags      # Enable with flags

# Set data breakpoint (watchpoint)
csrw    dbreak0, addr           # Watch address
csrw    dbreak_ctrl, flags      # Read/write/exec flags
```

### Instruction Trace

```c
struct trace_config {
    uint32_t enable      : 1;
    uint32_t filter_mode : 2;   /* User/kernel/all */
    uint32_t compress    : 1;   /* Compressed trace */
    uint64_t start_addr;        /* Trace start */
    uint64_t end_addr;          /* Trace end */
};
```

### Performance Trace

```assembly
# Enable branch tracing
csrw    trace_ctrl, TRACE_BRANCHES

# Enable memory tracing
csrw    trace_ctrl, TRACE_MEMORY

# Read trace buffer
csrr    rd, trace_data
```

## 28.3 Error Detection and Correction (ECC)

### Memory ECC

```c
struct ecc_status {
    uint32_t correctable_errors;    /* Single-bit errors */
    uint32_t uncorrectable_errors;  /* Multi-bit errors */
    uint64_t last_error_addr;       /* Address of last error */
};
```

### Cache ECC

```assembly
# Query cache ECC status
csrr    rd, cache_ecc_status

# Clear ECC error
csrw    cache_ecc_clear, rs
```

## 28.4 Machine Check Architecture (MCA)

```c
struct mca_bank {
    uint64_t status;            /* Error status */
    uint64_t addr;              /* Error address */
    uint64_t misc;              /* Additional info */
};

/* MCA error types */
#define MCA_CACHE_ERROR     0x01
#define MCA_TLB_ERROR       0x02
#define MCA_BUS_ERROR       0x03
#define MCA_INTERNAL_ERROR  0x04
```

## 28.5 Thermal Management

```assembly
# Read temperature
csrr    rd, cpu_temp            # Temperature in Celsius

# Set thermal threshold
csrw    thermal_threshold, rs   # Throttle threshold

# Thermal interrupt
csrr    rd, thermal_status      # Thermal status
```

## 28.6 Cache Management

### Cache Coherence Protocol

DLX implements MOESI protocol:
- **M**odified - Dirty, exclusive
- **O**wned - Dirty, shared
- **E**xclusive - Clean, exclusive
- **S**hared - Clean, shared
- **I**nvalid

```assembly
# Cache line state query
cstate  rd, addr                # Get cache line state

# Force cache line state
cinval  addr                    # Invalidate
cflush  addr                    # Flush (write-back + invalidate)
cclean  addr                    # Clean (write-back)
```

### Cache Allocation Technology (CAT)

```c
struct cat_config {
    uint32_t class_id;          /* QoS class */
    uint32_t way_mask;          /* Cache ways */
    uint32_t priority;          /* Allocation priority */
};
```

## 28.7 Reliability, Availability, Serviceability (RAS)

### Lockstep Execution

Dual-core lockstep for fault detection:
```c
struct lockstep_config {
    uint32_t enable      : 1;
    uint32_t core0_id    : 4;   /* Primary core */
    uint32_t core1_id    : 4;   /* Shadow core */
    uint32_t on_mismatch : 2;   /* Action on mismatch */
};
```

### Error Containment

```assembly
# Isolate faulty core
core.isolate core_id

# Recover from error
core.recover core_id
```

## 28.8 Virtualization Support

See Chapter 17 for nested virtualization details.

### IOMMU (I/O Memory Management Unit)

```assembly
# Map IOVA to physical address
iommu.map   iova, pa, size, prot

# Unmap IOVA
iommu.unmap iova, size

# Flush IOTLB
iommu.flush domain_id
```

## 28.9 Quality of Service (QoS)

### Memory Bandwidth Allocation

```c
struct mba_config {
    uint32_t class_id;          /* QoS class */
    uint32_t throttle;          /* Bandwidth throttle % */
    uint32_t priority;          /* Priority level */
};
```

### Cache QoS

```assembly
# Set cache allocation class
csrw    cache_qos_class, rs    # QoS class for current thread
```

## 28.10 Hardware-Assisted Verification

### Hardware Assertions

```assembly
# Runtime assertion check
assert  condition, error_code   # Trap if condition false

# Invariant check
invariant predicate             # Continuous checking
```

### Formal Verification Support

```c
/* Formal verification annotations */
__attribute__((verify_pre))     /* Precondition */
__attribute__((verify_post))    /* Postcondition */
__attribute__((verify_inv))     /* Invariant */
```
