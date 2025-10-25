# Chapter 21: Advanced Security Features

## 21.1 Memory Tagging Extension (MTE)

```assembly
# Set memory tag
stg         rs1, offset(rs2)    # Set tag for address
ldg         rd, offset(rs)      # Load tag

# Tagged load/store
ldt.w       rd, offset(rs)      # Load with tag check
stt.w       rs1, offset(rs2)    # Store with tag check
```

## 21.2 Control-Flow Integrity (CFI)

### Forward-Edge CFI (Indirect Calls)
```assembly
# Landing pad instruction
endbr       type                # Valid indirect branch target
```

### Backward-Edge CFI (Return Address)
```assembly
# Shadow stack operations
sspush      rs                  # Push to shadow stack
sspop       rd                  # Pop from shadow stack
ssread      rd, offset          # Read shadow stack
```

## 21.3 Pointer Authentication

```assembly
# Sign pointer with key A
pac.ia      rd, rs, modifier    # Sign instruction address
pac.da      rd, rs, modifier    # Sign data address

# Authenticate pointer
aut.ia      rd, rs, modifier    # Authenticate instruction address
aut.da      rd, rs, modifier    # Authenticate data address

# Strip authentication code
strip.pac   rd, rs              # Remove PAC from pointer
```

## 21.4 Memory Encryption Engine (MEE)

```c
/* MEE configuration */
struct mee_config {
    uint32_t enable      : 1;   /* Enable encryption */
    uint32_t aes_mode    : 2;   /* AES-128/192/256 */
    uint32_t tweak_mode  : 2;   /* Tweak function */
    uint32_t reserved    : 27;
};

/* Encrypted memory region */
struct mee_region {
    uint64_t base;              /* Region base address */
    uint64_t size;              /* Region size */
    uint8_t  key[32];           /* AES-256 key */
    uint8_t  tweak[16];         /* Initial tweak */
    uint32_t perm;              /* Access permissions */
};
```

## 21.5 Secure Boot

### Boot ROM integrity
```c
#define BOOT_ROM_HASH   /* SHA-256 hash of boot ROM */
#define BOOT_KEY_HASH   /* Public key hash */

/* PCR (Platform Configuration Register) */
struct pcr {
    uint8_t pcr[24][32];        /* 24 × SHA-256 hashes */
    uint32_t sealed : 1;        /* Sealed/unsealed */
};
```

### Measured boot chain
1. Boot ROM validates Stage 1 bootloader
2. Stage 1 validates Stage 2
3. Stage 2 validates kernel
4. Each stage extends PCRs

## 21.6 Trusted Execution Environment (TEE)

```assembly
# World switching
tee.enter   secure_world        # Switch to secure world
tee.exit    normal_world        # Return to normal world

# SMC (Secure Monitor Call)
smc         function_id         # Call secure monitor
```

## 21.7 Cryptographic Coprocessor Interface

```assembly
# Hardware RNG
crypto.rng  rd                  # Read random number

# True random vs pseudo-random
crypto.trng rd                  # True RNG (hardware entropy)
crypto.prng rd                  # Pseudo RNG (DRBG)

# Key derivation
crypto.kdf  rd, key, salt, info # Key derivation function
```

## 21.8 Side-Channel Mitigations

### Constant-Time Operations
```assembly
# Constant-time select
ct.select   rd, rs1, rs2, cond  # rd ← cond ? rs1 : rs2 (constant-time)

# Constant-time comparison
ct.cmp.eq   rd, rs1, rs2        # Constant-time equality
```

### Cache Partitioning
```c
struct cache_partition {
    uint32_t partition_id;
    uint32_t way_mask;          /* Cache ways assigned */
    uint32_t capacity;          /* Capacity in KB */
};
```

## 21.9 Fault Injection Detection

```assembly
# Redundant execution
.fault_detect:
    # Execute critical operation twice
    critical_op a0
    mv      t0, a0              # Save result
    critical_op a0
    bne     t0, a0, fault_handler
```

## 21.10 Secure Debug

```c
/* Debug authentication */
struct debug_auth {
    uint32_t debug_enabled  : 1;
    uint32_t jtag_enabled   : 1;
    uint32_t trace_enabled  : 1;
    uint32_t requires_auth  : 1;
    uint8_t  auth_token[32];    /* Authentication token */
};
```
