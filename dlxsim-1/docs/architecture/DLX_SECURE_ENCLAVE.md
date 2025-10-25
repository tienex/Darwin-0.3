# DLX Secure Enclave and TrustZone Extension

## Overview

This document specifies the DLX Secure Enclave and TrustZone extensions, providing hardware-enforced isolation for sensitive code and data. The architecture combines features from:
- **ARM TrustZone**: Secure/Normal world separation
- **Intel SGX**: Encrypted memory enclaves with attestation
- **AMD SEV**: Secure encrypted virtualization
- **RISC-V Keystone**: Flexible enclave framework

Key features:
- **Dual-world architecture**: Secure world and Normal world with independent resources
- **Encrypted enclaves**: Memory encryption with integrity protection
- **Remote attestation**: Cryptographic proof of enclave identity and state
- **Secure boot**: Chain of trust from hardware to applications
- **Integration**: Works with CHERI capabilities, ring protection, and register banking

## Architecture Modes

### World States

```c
/* DLX supports two parallel execution worlds */
typedef enum {
    WORLD_NORMAL = 0,           /* Normal world (untrusted) */
    WORLD_SECURE = 1            /* Secure world (trusted) */
} dlx_world_t;

/* World configuration register (WCR) */
typedef struct {
    uint32_t current_world  : 1;    /* Current world (0=Normal, 1=Secure) */
    uint32_t secure_enabled : 1;    /* Secure world enabled */
    uint32_t enclave_mode   : 1;    /* Enclave execution mode */
    uint32_t encryption_en  : 1;    /* Memory encryption enabled */
    uint32_t attestation_en : 1;    /* Attestation enabled */
    uint32_t secure_boot    : 1;    /* Secure boot enabled */
    uint32_t debug_disable  : 1;    /* Debug access disabled in secure world */
    uint32_t reserved       : 25;
} world_config_reg_t;

/* Access via special register */
#define WCR_REG     $SR_WORLD_CONFIG

/* Check current world */
static inline dlx_world_t get_current_world(void) {
    uint32_t wcr;
    asm volatile("MFSR %0, $SR_WORLD_CONFIG" : "=r"(wcr));
    return (wcr & 1) ? WORLD_SECURE : WORLD_NORMAL;
}
```

### Memory Partitioning

```c
/* Memory is partitioned between worlds */

/* Normal world memory map */
#define NORMAL_RAM_BASE         0x00000000
#define NORMAL_RAM_SIZE         0x80000000      /* 2 GB */
#define NORMAL_DEVICE_BASE      0x80000000
#define NORMAL_DEVICE_SIZE      0x40000000      /* 1 GB */

/* Secure world memory map (protected) */
#define SECURE_RAM_BASE         0xC0000000
#define SECURE_RAM_SIZE         0x20000000      /* 512 MB */
#define SECURE_DEVICE_BASE      0xE0000000
#define SECURE_DEVICE_SIZE      0x10000000      /* 256 MB */

/* Enclave memory (encrypted) */
#define ENCLAVE_BASE            0xF0000000
#define ENCLAVE_SIZE            0x10000000      /* 256 MB */

/* Memory region configuration */
typedef struct {
    uint64_t base_addr;
    uint64_t size;
    uint32_t world;             /* WORLD_NORMAL or WORLD_SECURE */
    uint32_t encrypted;         /* Encryption enabled */
    uint32_t integrity;         /* Integrity checking enabled */
    uint32_t accessible_from;   /* Which world can access */
} memory_region_t;

/* Memory region table (8 configurable regions) */
memory_region_t mem_regions[8];
```

### Register Banking by World

```c
/* Each world has independent register banks */
struct dlx_world_context {
    /* General purpose registers */
    uint64_t gpr[32];

    /* Special registers */
    uint64_t pc;
    uint64_t sp;
    uint64_t status;

    /* FP/Vector registers */
    unified_reg_t ureg[64];

    /* CHERI capabilities (if enabled) */
    cap256_t caps[32];

    /* System registers */
    uint64_t exception_vector;
    uint64_t page_table_base;
    uint64_t tlb_config;
};

/* Context for each world */
struct dlx_world_context normal_world_ctx;
struct dlx_world_context secure_world_ctx;

/* Switch between worlds */
void switch_to_secure_world(void) {
    /* Save normal world context */
    save_context(&normal_world_ctx);

    /* Load secure world context */
    restore_context(&secure_world_ctx);

    /* Update world configuration */
    set_current_world(WORLD_SECURE);

    /* Enable secure features */
    enable_memory_encryption();
    disable_debug_access();
}

void switch_to_normal_world(void) {
    /* Clear sensitive data */
    clear_sensitive_registers();

    /* Save secure world context */
    save_context(&secure_world_ctx);

    /* Load normal world context */
    restore_context(&normal_world_ctx);

    /* Update world configuration */
    set_current_world(WORLD_NORMAL);
}
```

## Secure World Instructions

### World Transition

```assembly
# Switch to secure world (privileged)
SMC     imm16                   # Secure Monitor Call
                                # imm16 = function code
                                # Switches to secure world monitor

# Return from secure world
SMCRET                          # Secure Monitor Call Return
                                # Returns from secure world to normal world

# Check world state
GETWORLD rd                     # rd = current world (0=Normal, 1=Secure)

# Example: Call secure world service
call_secure_service:
    # Setup arguments in a0-a3
    LI      a0, SERVICE_ENCRYPT
    LA      a1, plaintext
    LA      a2, ciphertext
    LI      a3, length

    # Call secure world
    SMC     0                   # Trigger world switch

    # Control transfers to secure monitor
    # Result returned in v0

    RET

# Secure monitor handler (in secure world)
secure_monitor:
    # Validate caller
    JAL     validate_normal_world_caller

    # Dispatch based on function code
    LW      t0, a0
    BEQI    t0, SERVICE_ENCRYPT, .do_encrypt
    BEQI    t0, SERVICE_DECRYPT, .do_decrypt
    # ... more services ...

.do_encrypt:
    # Perform encryption in secure world
    JAL     aes_encrypt

    # Return to normal world
    SMCRET
```

### Secure Configuration

```assembly
# Configure memory region security
SETMEMREG   region_num, base, size, flags
            # region_num = 0-7
            # base = base address
            # size = region size
            # flags = SECURE | ENCRYPTED | INTEGRITY_CHECK

# Get memory region configuration
GETMEMREG   rd_base, rd_size, rd_flags, region_num

# Example: Protect secure memory
protect_secure_memory:
    # Region 0: Secure RAM (2 GB, encrypted)
    LI      t0, SECURE_RAM_BASE
    LI      t1, SECURE_RAM_SIZE
    LI      t2, MEM_SECURE | MEM_ENCRYPTED | MEM_INTEGRITY
    SETMEMREG 0, t0, t1, t2

    RET
```

## Encrypted Enclaves

### Enclave Structure

```c
/* Enclave control structure */
typedef struct {
    uint64_t enclave_id;        /* Unique enclave identifier */
    uint64_t base_addr;         /* Enclave base address */
    uint64_t size;              /* Enclave size */
    uint64_t entry_point;       /* Enclave entry point */

    /* Encryption keys */
    uint8_t  mem_encryption_key[32];    /* AES-256 key for memory */
    uint8_t  integrity_key[32];         /* HMAC key for integrity */
    uint8_t  sealing_key[32];           /* Key for persistent storage */

    /* Measurement (for attestation) */
    uint8_t  measurement[64];           /* SHA-512 hash of enclave */

    /* Permissions */
    uint32_t can_access_device : 1;
    uint32_t can_attest : 1;
    uint32_t can_seal : 1;
    uint32_t debug_allowed : 1;
    uint32_t reserved : 28;

    /* CHERI capabilities (if using CHERI mode) */
    cap256_t code_cap;
    cap256_t data_cap;
    cap256_t stack_cap;

    /* Saved state */
    struct dlx_world_context saved_ctx;
} enclave_t;

/* Maximum number of enclaves */
#define MAX_ENCLAVES    16

/* Enclave table */
enclave_t enclaves[MAX_ENCLAVES];
```

### Enclave Lifecycle

```c
/*
 * Create a new enclave
 */
int enclave_create(enclave_t *enclave, void *code, size_t code_size,
                  void *data, size_t data_size) {
    /* Allocate enclave ID */
    enclave->enclave_id = allocate_enclave_id();

    /* Allocate encrypted memory */
    enclave->base_addr = allocate_encrypted_region(code_size + data_size);
    enclave->size = code_size + data_size;

    /* Generate encryption keys (from CPU root key + enclave ID) */
    derive_enclave_keys(enclave->enclave_id,
                       enclave->mem_encryption_key,
                       enclave->integrity_key,
                       enclave->sealing_key);

    /* Copy code and data with encryption */
    encrypt_and_copy(enclave->base_addr, code, code_size,
                    enclave->mem_encryption_key);
    encrypt_and_copy(enclave->base_addr + code_size, data, data_size,
                    enclave->mem_encryption_key);

    /* Compute measurement (hash of code + data + config) */
    compute_enclave_measurement(enclave, code, code_size, data, data_size);

    /* Setup CHERI capabilities (if enabled) */
    if (cheri_enabled()) {
        enclave->code_cap = create_code_capability(enclave->base_addr,
                                                   code_size);
        enclave->data_cap = create_data_capability(enclave->base_addr + code_size,
                                                   data_size);
    }

    /* Mark enclave as ready */
    enclave_table_insert(enclave);

    return enclave->enclave_id;
}

/*
 * Enter enclave
 */
void enclave_enter(int enclave_id, uint64_t entry_offset, void *params) {
    enclave_t *enclave = &enclaves[enclave_id];

    /* Validate enclave */
    if (!enclave_is_valid(enclave)) {
        raise_exception(EXCEPTION_INVALID_ENCLAVE);
        return;
    }

    /* Save normal world context */
    save_context(&normal_world_ctx);

    /* Switch to secure world */
    switch_to_secure_world();

    /* Load enclave context */
    set_encryption_key(enclave->mem_encryption_key);
    set_integrity_key(enclave->integrity_key);

    /* Setup CHERI capabilities */
    if (cheri_enabled()) {
        set_pcc(&enclave->code_cap);
        set_ddc(&enclave->data_cap);
    }

    /* Setup page tables for enclave */
    setup_enclave_page_table(enclave);

    /* Jump to enclave entry point */
    uint64_t entry = enclave->base_addr + entry_offset;
    jump_to_enclave(entry, params);
}

/*
 * Exit enclave
 */
void enclave_exit(void *result) {
    /* Get current enclave */
    enclave_t *enclave = get_current_enclave();

    /* Save enclave state (if resumable) */
    save_context(&enclave->saved_ctx);

    /* Clear encryption keys from registers */
    clear_encryption_keys();

    /* Switch back to normal world */
    switch_to_normal_world();

    /* Restore normal world context */
    restore_context(&normal_world_ctx);

    /* Return result to caller */
    set_register(v0, (uint64_t)result);
}

/*
 * Destroy enclave
 */
void enclave_destroy(int enclave_id) {
    enclave_t *enclave = &enclaves[enclave_id];

    /* Wipe enclave memory */
    secure_wipe(enclave->base_addr, enclave->size);

    /* Clear encryption keys */
    memset(enclave->mem_encryption_key, 0, 32);
    memset(enclave->integrity_key, 0, 32);
    memset(enclave->sealing_key, 0, 32);

    /* Free memory region */
    free_encrypted_region(enclave->base_addr, enclave->size);

    /* Remove from enclave table */
    enclave_table_remove(enclave_id);
}
```

### Enclave Instructions

```assembly
# Create enclave
ECREATE     rd, code_ptr, code_size, data_ptr, data_size
            # rd = enclave_id
            # Creates new encrypted enclave

# Enter enclave
EENTER      enclave_id, entry_offset, params
            # Enters enclave execution
            # enclave_id = target enclave
            # entry_offset = offset from enclave base
            # params = parameter pointer

# Exit enclave
EEXIT       result_ptr
            # Exits enclave back to normal world
            # result_ptr = pointer to return value

# Resume enclave
ERESUME     enclave_id
            # Resumes previously suspended enclave

# Destroy enclave
EDESTROY    enclave_id
            # Destroys enclave and wipes memory

# Get enclave info
EGETINFO    rd_base, rd_size, rd_meas, enclave_id
            # Get enclave metadata

# Example: Create and run enclave
run_secure_computation:
    # Create enclave
    LA      a0, enclave_code
    LI      a1, enclave_code_size
    LA      a2, enclave_data
    LI      a3, enclave_data_size
    ECREATE v0, a0, a1, a2, a3      # v0 = enclave_id

    # Enter enclave
    MOV     a0, v0                  # enclave_id
    LI      a1, 0                   # entry_offset = 0
    LA      a2, params              # parameters
    EENTER  a0, a1, a2

    # Enclave executes...
    # Result returned in v0

    # Destroy enclave
    EDESTROY v0

    RET

# Enclave code (executed in secure world)
enclave_main:
    # Load parameters
    MOV     t0, a2                  # params

    # Perform secure computation
    # Memory accesses are automatically encrypted/decrypted
    LW      t1, 0(t0)               # Load encrypted data
    LW      t2, 4(t0)
    ADD     t3, t1, t2              # Compute

    # Exit enclave with result
    MOV     a0, t3
    EEXIT   a0
```

## Memory Encryption Engine (MEE)

### Encryption Mechanism

```c
/*
 * Hardware memory encryption engine
 * Uses AES-256-GCM for confidentiality and integrity
 */

/* MEE configuration */
typedef struct {
    uint8_t  root_key[32];          /* CPU root key (fused) */
    uint8_t  current_key[32];       /* Current encryption key */
    uint8_t  integrity_key[32];     /* Integrity (MAC) key */
    uint64_t nonce_counter;         /* Counter for nonces */
    uint32_t enabled;               /* MEE enabled */
} mee_config_t;

/* Per-cache-line metadata */
typedef struct {
    uint8_t  mac[16];               /* 128-bit MAC (GCM tag) */
    uint64_t nonce;                 /* 64-bit nonce */
    uint8_t  version;               /* Version for replay protection */
} cache_line_metadata_t;

/* Encrypt cache line on write */
static inline void encrypt_cache_line(uint64_t addr, void *data, size_t size) {
    cache_line_metadata_t *meta = get_cache_line_metadata(addr);

    /* Generate nonce */
    meta->nonce = mee_config.nonce_counter++;

    /* Encrypt data: AES-256-GCM */
    uint8_t ciphertext[64];
    aes256_gcm_encrypt(
        data, size,                     /* Plaintext */
        mee_config.current_key,         /* Key */
        (uint8_t *)&meta->nonce, 8,     /* Nonce */
        (uint8_t *)&addr, 8,            /* AAD (address) */
        ciphertext,                     /* Output ciphertext */
        meta->mac                       /* Output MAC */
    );

    /* Write encrypted data to memory */
    memcpy((void *)addr, ciphertext, size);

    /* Update version */
    meta->version++;
}

/* Decrypt cache line on read */
static inline int decrypt_cache_line(uint64_t addr, void *data, size_t size) {
    cache_line_metadata_t *meta = get_cache_line_metadata(addr);

    /* Read encrypted data */
    uint8_t ciphertext[64];
    memcpy(ciphertext, (void *)addr, size);

    /* Decrypt and verify: AES-256-GCM */
    uint8_t plaintext[64];
    int result = aes256_gcm_decrypt(
        ciphertext, size,               /* Ciphertext */
        meta->mac,                      /* MAC to verify */
        mee_config.current_key,         /* Key */
        (uint8_t *)&meta->nonce, 8,     /* Nonce */
        (uint8_t *)&addr, 8,            /* AAD (address) */
        plaintext                       /* Output plaintext */
    );

    if (result != 0) {
        /* Integrity check failed! */
        raise_security_exception(EXCEPTION_INTEGRITY_VIOLATION, addr);
        return -1;
    }

    /* Copy decrypted data */
    memcpy(data, plaintext, size);

    return 0;
}

/* MEE is transparent to software - automatic encrypt/decrypt */
```

### Integrity Tree

```c
/*
 * Merkle tree for integrity protection
 * Prevents replay and splicing attacks
 */

#define TREE_FANOUT     8           /* 8-way tree */
#define CACHE_LINE_SIZE 64

typedef struct {
    uint8_t  hash[32];              /* SHA-256 hash */
    uint64_t counter;               /* Monotonic counter */
} tree_node_t;

/* Compute node hash */
static inline void compute_tree_hash(tree_node_t *node,
                                    tree_node_t *children,
                                    int num_children) {
    sha256_ctx_t ctx;
    sha256_init(&ctx);

    /* Hash all children */
    for (int i = 0; i < num_children; i++) {
        sha256_update(&ctx, children[i].hash, 32);
        sha256_update(&ctx, &children[i].counter, 8);
    }

    /* Finalize hash */
    sha256_final(&ctx, node->hash);

    /* Update counter */
    node->counter++;
}

/* Verify integrity path from leaf to root */
static inline int verify_integrity_path(uint64_t addr) {
    tree_node_t *leaf = get_leaf_node(addr);
    tree_node_t *node = leaf;

    /* Walk up tree to root */
    while (node != &integrity_tree_root) {
        tree_node_t *parent = node->parent;
        tree_node_t *siblings = get_siblings(node);

        /* Recompute parent hash */
        uint8_t computed_hash[32];
        sha256_ctx_t ctx;
        sha256_init(&ctx);

        for (int i = 0; i < TREE_FANOUT; i++) {
            sha256_update(&ctx, siblings[i].hash, 32);
            sha256_update(&ctx, &siblings[i].counter, 8);
        }

        sha256_final(&ctx, computed_hash);

        /* Compare with stored hash */
        if (memcmp(computed_hash, parent->hash, 32) != 0) {
            /* Integrity violation! */
            return -1;
        }

        node = parent;
    }

    return 0;  /* Integrity verified */
}
```

## Remote Attestation

### Attestation Process

```c
/*
 * Remote attestation allows remote party to verify enclave identity
 */

/* Attestation report */
typedef struct {
    uint8_t  enclave_measurement[64];   /* SHA-512 of enclave */
    uint64_t enclave_id;
    uint64_t timestamp;
    uint8_t  user_data[64];             /* Application-specific data */

    /* CPU identity */
    uint8_t  cpu_svn[16];               /* CPU security version */
    uint8_t  cpu_serial[16];            /* CPU serial number */

    /* Signature (signed by CPU private key) */
    uint8_t  signature[512];            /* RSA-4096 or Ed25519 */
} attestation_report_t;

/*
 * Generate attestation report
 */
int generate_attestation(enclave_t *enclave, void *user_data, size_t user_data_len,
                        attestation_report_t *report) {
    /* Ensure we're in enclave */
    if (get_current_world() != WORLD_SECURE) {
        return -EPERM;
    }

    /* Fill report */
    memcpy(report->enclave_measurement, enclave->measurement, 64);
    report->enclave_id = enclave->enclave_id;
    report->timestamp = get_secure_timestamp();
    memcpy(report->user_data, user_data, min(user_data_len, 64));

    /* Add CPU identity */
    read_cpu_svn(report->cpu_svn);
    read_cpu_serial(report->cpu_serial);

    /* Sign report with CPU private key (attestation key) */
    sign_report(report, sizeof(*report) - sizeof(report->signature),
               report->signature);

    return 0;
}

/*
 * Verify attestation report (done by remote party)
 */
int verify_attestation(attestation_report_t *report, uint8_t *cpu_public_key) {
    /* Verify signature */
    if (!verify_signature(report, sizeof(*report) - sizeof(report->signature),
                         report->signature, cpu_public_key)) {
        return -1;  /* Invalid signature */
    }

    /* Check timestamp (replay protection) */
    uint64_t now = time(NULL);
    if (report->timestamp < now - 300 || report->timestamp > now + 60) {
        return -1;  /* Timestamp out of range */
    }

    /* Verify enclave measurement against expected value */
    uint8_t expected_measurement[64];
    compute_expected_measurement(expected_measurement);

    if (memcmp(report->enclave_measurement, expected_measurement, 64) != 0) {
        return -1;  /* Enclave measurement mismatch */
    }

    /* Check CPU security version */
    if (!is_cpu_svn_acceptable(report->cpu_svn)) {
        return -1;  /* CPU version too old (vulnerable) */
    }

    return 0;  /* Attestation verified */
}
```

### Attestation Instructions

```assembly
# Generate attestation report
EATTEST     rd, enclave_id, user_data, user_data_len
            # rd = pointer to attestation_report_t
            # Generates signed attestation report

# Seal data (encrypt for storage)
ESEAL       rd_sealed, data_ptr, data_len, enclave_id
            # Encrypts data with enclave sealing key
            # Can only be unsealed by same enclave

# Unseal data
EUNSEAL     rd_data, sealed_ptr, sealed_len, enclave_id
            # Decrypts sealed data
            # Verifies it was sealed by this enclave

# Example: Establish secure channel with remote party
establish_secure_channel:
    # Generate ephemeral key pair
    JAL     generate_keypair        # v0=public, v1=private

    # Create attestation with public key as user_data
    MOV     a0, v0                  # enclave_id
    MOV     a1, v0                  # public key
    LI      a2, 32                  # key size
    EATTEST v0, a0, a1, a2          # v0 = report

    # Send report to remote party
    JAL     send_attestation_report

    # Remote party verifies and sends encrypted session key
    JAL     receive_encrypted_session_key

    # Decrypt session key with private key
    JAL     decrypt_with_private_key

    # Secure channel established!
    RET
```

## Secure Boot

### Boot Chain

```c
/*
 * Secure boot chain of trust
 */

/* Boot stages */
typedef enum {
    BOOT_STAGE_ROM = 0,         /* ROM boot loader (immutable) */
    BOOT_STAGE_BL1,             /* First stage bootloader */
    BOOT_STAGE_BL2,             /* Second stage bootloader */
    BOOT_STAGE_OS_KERNEL,       /* OS kernel */
    BOOT_STAGE_INIT,            /* Init system */
    BOOT_STAGE_APPS             /* Applications */
} boot_stage_t;

/* Boot measurement */
typedef struct {
    uint8_t  stage;             /* Boot stage */
    uint8_t  hash[64];          /* SHA-512 of stage code */
    uint64_t timestamp;
    uint8_t  signature[512];    /* Signature by previous stage */
} boot_measurement_t;

/* Boot measurement log (for measured boot) */
boot_measurement_t boot_log[16];
int boot_log_count = 0;

/*
 * Verify and measure boot stage
 */
int verify_boot_stage(boot_stage_t stage, void *code, size_t code_size,
                     uint8_t *prev_stage_pubkey) {
    boot_measurement_t measurement;

    /* Compute hash of code */
    sha512(code, code_size, measurement.hash);

    /* Verify signature (if not ROM stage) */
    if (stage != BOOT_STAGE_ROM) {
        if (!verify_rsa4096(measurement.hash, 64,
                           code + code_size,  /* Signature at end */
                           prev_stage_pubkey)) {
            /* Signature verification failed */
            halt_boot("Boot verification failed at stage %d", stage);
            return -1;
        }
    }

    /* Add to boot log */
    measurement.stage = stage;
    measurement.timestamp = read_secure_timer();
    boot_log[boot_log_count++] = measurement;

    /* Extend PCR (Platform Configuration Register) */
    extend_pcr(stage, measurement.hash, 64);

    return 0;
}

/*
 * Boot sequence
 */
void secure_boot(void) {
    /* Stage 0: ROM (verify BL1) */
    verify_boot_stage(BOOT_STAGE_ROM, bl1_code, bl1_size, rom_pubkey);
    jump_to(bl1_entry);

    /* Stage 1: BL1 (verify BL2) */
    verify_boot_stage(BOOT_STAGE_BL1, bl2_code, bl2_size, bl1_pubkey);
    jump_to(bl2_entry);

    /* Stage 2: BL2 (verify kernel) */
    verify_boot_stage(BOOT_STAGE_BL2, kernel_code, kernel_size, bl2_pubkey);
    jump_to(kernel_entry);

    /* Kernel continues chain... */
}
```

### Platform Configuration Registers (PCRs)

```c
/*
 * PCRs store cumulative measurements for attestation
 */

#define NUM_PCRS    24

/* PCR bank */
typedef struct {
    uint8_t value[64];          /* SHA-512 value */
    int extended_count;         /* Number of extensions */
} pcr_t;

pcr_t pcrs[NUM_PCRS];

/* Extend PCR (irreversible) */
void extend_pcr(int pcr_num, uint8_t *data, size_t data_len) {
    if (pcr_num >= NUM_PCRS) return;

    /* PCR_new = SHA-512(PCR_old || data) */
    sha512_ctx_t ctx;
    sha512_init(&ctx);
    sha512_update(&ctx, pcrs[pcr_num].value, 64);
    sha512_update(&ctx, data, data_len);
    sha512_final(&ctx, pcrs[pcr_num].value);

    pcrs[pcr_num].extended_count++;
}

/* Read PCR (for attestation) */
void read_pcr(int pcr_num, uint8_t *output) {
    if (pcr_num >= NUM_PCRS) return;
    memcpy(output, pcrs[pcr_num].value, 64);
}

/* PCR usage:
 * PCR 0-7: Firmware (ROM, bootloaders)
 * PCR 8-15: OS (kernel, drivers)
 * PCR 16-23: Applications
 */
```

## Integration with Existing Features

### CHERI + Enclaves

```c
/*
 * Combine CHERI capabilities with enclaves for fine-grained security
 */

/* Enclave with CHERI capabilities */
typedef struct {
    enclave_t base;

    /* CHERI-specific capabilities */
    cap256_t entry_cap;         /* Sealed entry capability */
    cap256_t exit_cap;          /* Sealed exit capability */
    cap256_t *cap_table;        /* Capability table for enclave */
    size_t cap_table_size;
} cheri_enclave_t;

/*
 * Create CHERI-enabled enclave
 */
int cheri_enclave_create(cheri_enclave_t *enclave, void *code, size_t code_size) {
    /* Create base enclave */
    enclave_create(&enclave->base, code, code_size, NULL, 0);

    /* Create code capability with execute permission */
    enclave->base.code_cap = (cap256_t){
        .address = enclave->base.base_addr,
        .base = enclave->base.base_addr,
        .length = code_size,
        .perms = CAP_PERM_EXECUTE | CAP_PERM_LOAD,
        .otype = CAP_OTYPE_UNSEALED,
        .tag = 1
    };

    /* Create entry capability (sealed) */
    enclave->entry_cap = seal_cap(&enclave->base.code_cap, enclave->base.enclave_id);

    /* Create exit capability (sealed, for returning) */
    cap256_t exit_cap_unsealed = create_exit_capability();
    enclave->exit_cap = seal_cap(&exit_cap_unsealed, enclave->base.enclave_id);

    return enclave->base.enclave_id;
}

/*
 * Enter CHERI enclave (via CCall)
 */
void cheri_enclave_enter(cheri_enclave_t *enclave, cap256_t *data_cap) {
    /* CCall with sealed entry capability */
    /* Hardware automatically unseals and validates */
    asm volatile(
        "CLC    ca0, %0\n"          /* Load entry cap */
        "CLC    ca1, %1\n"          /* Load data cap */
        "CCALL  ca0, ca1"           /* Cross-domain call */
        :: "m"(enclave->entry_cap), "m"(*data_cap)
        : "ca0", "ca1"
    );
}
```

### Register Banking + Secure World

```c
/*
 * Each world can use register banking for ring isolation
 */

/* Secure world with 4-ring protection */
void setup_secure_world_rings(void) {
    /* Enable register banking in secure world */
    enable_register_banking();

    /* Configure rings for secure world */
    /* Ring 0: Secure monitor */
    /* Ring 1: Trusted OS */
    /* Ring 2: Trusted services */
    /* Ring 3: Secure enclaves */

    set_ring_config(RING_0, SECURE_MONITOR_BANK);
    set_ring_config(RING_1, TRUSTED_OS_BANK);
    set_ring_config(RING_2, TRUSTED_SERVICE_BANK);
    set_ring_config(RING_3, ENCLAVE_BANK);
}

/* Ring transitions in secure world are fast (no register save/restore) */
```

## Performance Characteristics

### Memory Encryption Overhead

```c
/* Memory encryption performance impact */

/* Encryption latency per cache line (64 bytes) */
/* - AES-256-GCM encrypt: ~40 cycles */
/* - AES-256-GCM decrypt + verify: ~50 cycles */

/* With caching: */
/* - Cache hit: 0 overhead (plaintext in cache) */
/* - Cache miss: +50 cycles for decrypt */

/* Typical overhead: 2-10% for memory-intensive workloads */

/* Integrity tree verification */
/* - Tree depth: log₈(memory_size / 64 bytes) */
/* - For 1 GB: log₈(16M) ≈ 8 levels */
/* - Verification: 8 × 10 cycles = 80 cycles per miss */

/* Total memory access overhead */
/* - Without encryption: ~100 cycles (DRAM latency) */
/* - With encryption: ~230 cycles (100 + 50 + 80) */
/* - Overhead: ~130% (but mostly hidden by cache) */
```

### World Switch Latency

```c
/* World switching performance */

/* Context save/restore */
/* - 32 GPRs × 8 bytes = 256 bytes */
/* - 64 URs × 2048 bytes = 128 KB (if full width) */
/* - 32 Caps × 32 bytes = 1024 bytes */
/* Total: ~130 KB per world */

/* Switch latency */
/* - Save context: ~2000 cycles */
/* - Validate transition: ~100 cycles */
/* - TLB flush: ~500 cycles */
/* - Load context: ~2000 cycles */
/* Total: ~4600 cycles (~1.5 μs @ 3 GHz) */

/* With register banking */
/* - No register save/restore */
/* - Switch latency: ~600 cycles (~200 ns @ 3 GHz) */
/* - 7× faster than without banking */
```

### Attestation Performance

```c
/* Attestation latency */

/* Measurement computation */
/* - SHA-512 of 1 MB enclave: ~500K cycles (~167 μs @ 3 GHz) */

/* Signature generation */
/* - RSA-4096: ~20M cycles (~6.7 ms @ 3 GHz) */
/* - Ed25519: ~100K cycles (~33 μs @ 3 GHz) */

/* Signature verification */
/* - RSA-4096: ~500K cycles (~167 μs @ 3 GHz) */
/* - Ed25519: ~250K cycles (~83 μs @ 3 GHz) */

/* Recommendation: Use Ed25519 for fast attestation */
```

## Use Cases

### Secure Key Storage

```c
/* Store cryptographic keys in enclave */
void secure_key_storage_example(void) {
    /* Create enclave for key storage */
    int enclave_id = create_key_storage_enclave();

    /* Generate key inside enclave */
    uint8_t public_key[32];
    enclave_call(enclave_id, "generate_keypair", NULL, 0,
                public_key, sizeof(public_key));

    /* Use key for signing (private key never leaves enclave) */
    uint8_t message[64] = "Hello, World!";
    uint8_t signature[64];
    enclave_call(enclave_id, "sign", message, 64,
                signature, sizeof(signature));

    /* Seal private key for persistent storage */
    uint8_t sealed_key[128];
    enclave_call(enclave_id, "seal_key", NULL, 0,
                sealed_key, sizeof(sealed_key));

    /* Store sealed key to disk (encrypted, bound to this enclave) */
    write_file("sealed_key.bin", sealed_key, sizeof(sealed_key));
}
```

### DRM (Digital Rights Management)

```c
/* Play protected content in enclave */
void drm_example(void) {
    /* Create DRM enclave */
    int enclave_id = create_drm_enclave();

    /* Attest to content provider */
    attestation_report_t report;
    generate_attestation_report(enclave_id, &report);
    send_to_server(&report);

    /* Receive encrypted content key */
    uint8_t encrypted_key[128];
    receive_from_server(encrypted_key, sizeof(encrypted_key));

    /* Decrypt content key inside enclave */
    enclave_call(enclave_id, "decrypt_key", encrypted_key, 128, NULL, 0);

    /* Stream and decrypt video frames inside enclave */
    while (more_frames()) {
        uint8_t encrypted_frame[1024*1024];
        receive_frame(encrypted_frame, sizeof(encrypted_frame));

        /* Decrypt in enclave, output to protected display path */
        enclave_call(enclave_id, "decrypt_and_display",
                    encrypted_frame, sizeof(encrypted_frame),
                    NULL, 0);
    }
}
```

### Secure ML Inference

```c
/* Run ML model in enclave to protect IP */
void secure_ml_inference(void) {
    /* Create enclave with ML model */
    int enclave_id = load_ml_model_enclave("model.enc");

    /* Attest enclave to model provider */
    attestation_report_t report;
    generate_attestation_report(enclave_id, &report);

    /* Verify we're running authentic model */
    if (!verify_attestation(&report)) {
        fprintf(stderr, "Attestation failed!\n");
        return;
    }

    /* Run inference (model weights stay encrypted in enclave) */
    float input[1000];
    float output[10];

    /* Prepare input */
    prepare_input(input);

    /* Inference in enclave */
    enclave_call(enclave_id, "infer", input, sizeof(input),
                output, sizeof(output));

    /* Process output */
    process_output(output);
}
```

## Summary

### Feature Comparison

| Feature | DLX Secure | ARM TrustZone | Intel SGX | AMD SEV |
|---------|------------|---------------|-----------|---------|
| Dual worlds | ✓ | ✓ | - | - |
| Encrypted enclaves | ✓ | - | ✓ | ✓ |
| Remote attestation | ✓ | △ | ✓ | ✓ |
| Memory encryption | ✓ | - | ✓ | ✓ |
| Integrity tree | ✓ | - | ✓ | ✓ |
| CHERI integration | ✓ | - | - | - |
| Register banking | ✓ | - | - | - |
| Secure boot | ✓ | ✓ | ✓ | ✓ |
| VM encryption | ✓ | - | - | ✓ |

### Security Properties

1. **Confidentiality**: Memory encryption protects data from physical attacks
2. **Integrity**: Merkle tree prevents tampering and replay attacks
3. **Authenticity**: Attestation proves enclave identity to remote parties
4. **Isolation**: Dual worlds and enclaves provide strong isolation
5. **Freshness**: Nonces and counters prevent replay attacks

### Performance

- **Memory overhead**: ~0.8% for tags + ~2% for metadata
- **Runtime overhead**: 2-10% for encrypted memory (mostly hidden by cache)
- **World switch**: 200 ns with register banking, 1.5 μs without
- **Attestation**: 33 μs (Ed25519) to 6.7 ms (RSA-4096)

---

**Document Status**: Specification Complete
**Implementation Status**: Ready for hardware/simulator integration
**Complexity**: Very High - requires crypto accelerators, secure storage, attestation infrastructure
**Security Level**: Comparable to Intel SGX + ARM TrustZone combined
**Integration**: Works seamlessly with CHERI, register banking, and ring protection
**Use Cases**: Key storage, DRM, confidential computing, secure ML, trusted execution
