# DLX High-Precision Timers and Performance Profiling

## Overview

The DLX architecture includes comprehensive support for high-precision timing and hardware performance monitoring, enabling accurate profiling, benchmarking, and performance optimization. This document describes the timer architecture, performance monitoring counters (PMCs), and profiling capabilities.

## Table of Contents

1. [High-Precision Timer Architecture](#high-precision-timer-architecture)
2. [Time Stamp Counter (TSC)](#time-stamp-counter-tsc)
3. [Performance Monitoring Unit (PMU)](#performance-monitoring-unit-pmu)
4. [Performance Events](#performance-events)
5. [Statistical Profiling](#statistical-profiling)
6. [Debug and Trace](#debug-and-trace)
7. [Timer Interrupts](#timer-interrupts)
8. [Instructions](#instructions)
9. [Programming Examples](#programming-examples)

---

## 1. High-Precision Timer Architecture

### Timer Hierarchy

```
┌─────────────────────────────────────────────────────────┐
│                     System Timers                        │
├─────────────────────────────────────────────────────────┤
│  ┌───────────────┐  ┌───────────────┐  ┌─────────────┐ │
│  │  Time Stamp   │  │  Performance  │  │   Real-Time │ │
│  │   Counter     │  │   Monitoring  │  │    Clock    │ │
│  │    (TSC)      │  │     Unit      │  │    (RTC)    │ │
│  └───────────────┘  └───────────────┘  └─────────────┘ │
│         │                    │                  │        │
│    ┌────┴────┐          ┌────┴────┐        ┌────┴────┐ │
│    │ Cycle   │          │  PMC0   │        │ Wall    │ │
│    │ Counter │          │  PMC1   │        │ Clock   │ │
│    │ 64-bit  │          │  PMC2   │        │ ns      │ │
│    └─────────┘          │  PMC3   │        └─────────┘ │
│                         │  PMC4   │                     │
│                         │  PMC5   │                     │
│                         │  PMC6   │                     │
│                         │  PMC7   │                     │
│                         └─────────┘                     │
└─────────────────────────────────────────────────────────┘
```

### Timer Registers (System Registers)

```c
/* Timer control and status registers */
#define SR_TSC           0x200      /* Time Stamp Counter (64-bit) */
#define SR_TSC_FREQ      0x201      /* TSC frequency in Hz */
#define SR_RTC           0x202      /* Real-Time Clock (64-bit ns) */
#define SR_RTC_SECONDS   0x203      /* RTC seconds since epoch */
#define SR_TIMER_CTRL    0x204      /* Timer control register */

/* Performance monitoring registers */
#define SR_PMU_CTRL      0x210      /* PMU control register */
#define SR_PMU_ENABLE    0x211      /* PMU counter enable mask */
#define SR_PMU_OVERFLOW  0x212      /* PMU overflow status */
#define SR_PMU_FILTER    0x213      /* PMU event filter */

/* Performance counter registers (8 counters) */
#define SR_PMC0          0x220      /* Performance counter 0 */
#define SR_PMC1          0x221      /* Performance counter 1 */
#define SR_PMC2          0x222      /* Performance counter 2 */
#define SR_PMC3          0x223      /* Performance counter 3 */
#define SR_PMC4          0x224      /* Performance counter 4 */
#define SR_PMC5          0x225      /* Performance counter 5 */
#define SR_PMC6          0x226      /* Performance counter 6 */
#define SR_PMC7          0x227      /* Performance counter 7 */

/* Performance event select registers */
#define SR_PMCSEL0       0x230      /* Counter 0 event select */
#define SR_PMCSEL1       0x231      /* Counter 1 event select */
#define SR_PMCSEL2       0x232      /* Counter 2 event select */
#define SR_PMCSEL3       0x233      /* Counter 3 event select */
#define SR_PMCSEL4       0x234      /* Counter 4 event select */
#define SR_PMCSEL5       0x235      /* Counter 5 event select */
#define SR_PMCSEL6       0x236      /* Counter 6 event select */
#define SR_PMCSEL7       0x237      /* Counter 7 event select */

/* Statistical profiling */
#define SR_PROF_CTRL     0x240      /* Profiling control */
#define SR_PROF_PC       0x241      /* Sampled PC */
#define SR_PROF_DATA     0x242      /* Profiling data */
#define SR_PROF_BUFFER   0x243      /* Profiling buffer base */
#define SR_PROF_COUNT    0x244      /* Profile sample count */
```

---

## 2. Time Stamp Counter (TSC)

### TSC Architecture

The Time Stamp Counter is a 64-bit counter that increments at a constant rate, independent of CPU frequency scaling or sleep states.

```c
typedef struct {
    uint64_t counter;           /* 64-bit cycle counter */
    uint32_t frequency;         /* Counter frequency in Hz */
    uint32_t flags;             /* TSC flags */
} tsc_state_t;

/* TSC flags */
#define TSC_FLAG_INVARIANT      0x00000001  /* Constant rate TSC */
#define TSC_FLAG_NONSTOP        0x00000002  /* Runs in sleep states */
#define TSC_FLAG_SYNCHRONIZED   0x00000004  /* Synchronized across cores */
```

### TSC Operations

```c
/* Read time stamp counter */
static inline uint64_t rdtsc(void) {
    uint64_t tsc;
    asm volatile("mfsr %0, $TSC" : "=r"(tsc));
    return tsc;
}

/* Read TSC with serialization (ensures all prior instructions complete) */
static inline uint64_t rdtscp(uint32_t *aux) {
    uint64_t tsc;
    asm volatile(
        "fence.i\n"             /* Serialize instruction stream */
        "mfsr %0, $TSC\n"
        "mfsr %1, $CORE_ID"     /* Core ID for aux */
        : "=r"(tsc), "=r"(*aux)
        :
        : "memory"
    );
    return tsc;
}

/* Convert TSC to nanoseconds */
static inline uint64_t tsc_to_ns(uint64_t tsc) {
    uint64_t freq;
    asm volatile("mfsr %0, $TSC_FREQ" : "=r"(freq));
    return (tsc * 1000000000ULL) / freq;
}

/* Get wall-clock time in nanoseconds */
static inline uint64_t get_wall_time_ns(void) {
    uint64_t rtc;
    asm volatile("mfsr %0, $RTC" : "=r"(rtc));
    return rtc;
}
```

### TSC Synchronization

For multi-core systems, TSC synchronization ensures consistent timestamps:

```c
/* TSC synchronization protocol */
typedef struct {
    volatile uint64_t master_tsc;
    volatile uint64_t slave_tsc;
    volatile int ready;
    volatile int done;
} tsc_sync_t;

void synchronize_tsc(int core_id, tsc_sync_t *sync) {
    if (core_id == 0) {
        /* Master core */
        __sync_synchronize();
        sync->master_tsc = rdtsc();
        sync->ready = 1;
        while (!sync->done);

        int64_t offset = sync->slave_tsc - sync->master_tsc;
        adjust_tsc_offset(offset);
    } else {
        /* Slave cores */
        while (!sync->ready);
        sync->slave_tsc = rdtsc();
        __sync_synchronize();
        sync->done = 1;
    }
}
```

---

## 3. Performance Monitoring Unit (PMU)

### PMU Architecture

The DLX PMU provides 8 programmable 64-bit performance counters that can monitor various hardware events.

```c
typedef struct {
    uint64_t counter;           /* 64-bit counter value */
    uint32_t event_select;      /* Event to count */
    uint32_t flags;             /* Counter flags */
    uint64_t overflow_count;    /* Overflow count */
} pmc_state_t;

/* PMU control register */
typedef struct {
    uint32_t enable      : 1;   /* Global PMU enable */
    uint32_t freeze      : 1;   /* Freeze all counters */
    uint32_t overflow_en : 1;   /* Enable overflow interrupts */
    uint32_t user_access : 1;   /* Allow user-mode access */
    uint32_t kernel_only : 1;   /* Count kernel events only */
    uint32_t user_only   : 1;   /* Count user events only */
    uint32_t reserved    : 26;
} pmu_ctrl_t;

/* Event select register */
typedef struct {
    uint32_t event       : 16;  /* Event number */
    uint32_t umask       : 8;   /* Unit mask */
    uint32_t threshold   : 4;   /* Threshold */
    uint32_t invert      : 1;   /* Invert counter mask */
    uint32_t enable      : 1;   /* Enable this counter */
    uint32_t kernel      : 1;   /* Count in kernel mode */
    uint32_t user        : 1;   /* Count in user mode */
} pmc_select_t;
```

### PMU Setup

```c
/* Initialize performance counter */
void pmu_init_counter(int counter, uint32_t event, uint32_t flags) {
    uint32_t select = 0;
    select |= (event & 0xFFFF);             /* Event number */
    select |= (flags & 0xFF) << 16;         /* Unit mask */
    select |= (1 << 30);                    /* Enable counter */
    select |= (1 << 31) | (1 << 29);        /* Count kernel + user */

    /* Write event select register */
    asm volatile("mtsr $PMCSEL0 + %0, %1" : : "r"(counter), "r"(select));

    /* Reset counter to zero */
    asm volatile("mtsr $PMC0 + %0, $zero" : : "r"(counter));
}

/* Enable PMU globally */
void pmu_enable(void) {
    uint32_t ctrl = 0;
    ctrl |= (1 << 0);   /* Enable PMU */
    ctrl |= (1 << 3);   /* Allow user access */

    asm volatile("mtsr $PMU_CTRL, %0" : : "r"(ctrl));

    /* Enable all 8 counters */
    asm volatile("mtsr $PMU_ENABLE, %0" : : "r"(0xFF));
}

/* Read performance counter */
uint64_t pmu_read_counter(int counter) {
    uint64_t value;
    asm volatile("mfsr %0, $PMC0 + %1" : "=r"(value) : "r"(counter));
    return value;
}

/* Reset performance counter */
void pmu_reset_counter(int counter) {
    asm volatile("mtsr $PMC0 + %0, $zero" : : "r"(counter));
}
```

---

## 4. Performance Events

### Event Categories

```c
/* CPU Core Events (0x0000 - 0x00FF) */
#define PMU_EVENT_CYCLES                0x0000  /* CPU cycles */
#define PMU_EVENT_INSTRUCTIONS          0x0001  /* Instructions retired */
#define PMU_EVENT_BRANCHES              0x0002  /* Branch instructions */
#define PMU_EVENT_BRANCH_MISS           0x0003  /* Branch mispredictions */
#define PMU_EVENT_LOADS                 0x0004  /* Load instructions */
#define PMU_EVENT_STORES                0x0005  /* Store instructions */
#define PMU_EVENT_NOPS                  0x0006  /* NOP instructions */
#define PMU_EVENT_EXCEPTIONS            0x0007  /* Exceptions taken */
#define PMU_EVENT_INTERRUPTS            0x0008  /* Interrupts taken */
#define PMU_EVENT_CONTEXT_SWITCHES      0x0009  /* Context switches */

/* Pipeline Events (0x0100 - 0x01FF) */
#define PMU_EVENT_STALL_CYCLES          0x0100  /* Pipeline stall cycles */
#define PMU_EVENT_STALL_FRONTEND        0x0101  /* Frontend stalls */
#define PMU_EVENT_STALL_BACKEND         0x0102  /* Backend stalls */
#define PMU_EVENT_STALL_LOAD            0x0103  /* Load use stalls */
#define PMU_EVENT_STALL_STORE           0x0104  /* Store buffer full */
#define PMU_EVENT_RESOURCE_STALL        0x0105  /* Resource conflicts */
#define PMU_EVENT_DECODE_STALL          0x0106  /* Decode bottleneck */
#define PMU_EVENT_RENAME_STALL          0x0107  /* Register rename stall */

/* Cache Events (0x0200 - 0x02FF) */
#define PMU_EVENT_L1I_ACCESS            0x0200  /* L1 I-cache accesses */
#define PMU_EVENT_L1I_MISS              0x0201  /* L1 I-cache misses */
#define PMU_EVENT_L1D_ACCESS            0x0202  /* L1 D-cache accesses */
#define PMU_EVENT_L1D_MISS              0x0203  /* L1 D-cache misses */
#define PMU_EVENT_L1D_WRITEBACK         0x0204  /* L1 D-cache writebacks */
#define PMU_EVENT_L2_ACCESS             0x0210  /* L2 cache accesses */
#define PMU_EVENT_L2_MISS               0x0211  /* L2 cache misses */
#define PMU_EVENT_L2_WRITEBACK          0x0212  /* L2 writebacks */
#define PMU_EVENT_L3_ACCESS             0x0220  /* L3 cache accesses */
#define PMU_EVENT_L3_MISS               0x0221  /* L3 cache misses */
#define PMU_EVENT_LLC_ACCESS            0x0230  /* Last-level cache access */
#define PMU_EVENT_LLC_MISS              0x0231  /* Last-level cache miss */

/* TLB Events (0x0300 - 0x03FF) */
#define PMU_EVENT_ITLB_ACCESS           0x0300  /* ITLB accesses */
#define PMU_EVENT_ITLB_MISS             0x0301  /* ITLB misses */
#define PMU_EVENT_DTLB_ACCESS           0x0302  /* DTLB accesses */
#define PMU_EVENT_DTLB_MISS             0x0303  /* DTLB misses */
#define PMU_EVENT_TLB_FLUSH             0x0304  /* TLB flushes */
#define PMU_EVENT_PAGE_WALK             0x0305  /* Page table walks */
#define PMU_EVENT_PAGE_WALK_CYCLES      0x0306  /* Cycles in page walk */

/* Memory Events (0x0400 - 0x04FF) */
#define PMU_EVENT_MEM_READ              0x0400  /* Memory reads */
#define PMU_EVENT_MEM_WRITE             0x0401  /* Memory writes */
#define PMU_EVENT_MEM_READ_BYTES        0x0402  /* Bytes read */
#define PMU_EVENT_MEM_WRITE_BYTES       0x0403  /* Bytes written */
#define PMU_EVENT_MEM_STALL_CYCLES      0x0404  /* Memory stall cycles */
#define PMU_EVENT_BUS_ACCESS            0x0410  /* Bus accesses */
#define PMU_EVENT_BUS_CYCLES            0x0411  /* Bus busy cycles */

/* Floating-Point Events (0x0500 - 0x05FF) */
#define PMU_EVENT_FP_INSTRUCTIONS       0x0500  /* FP instructions */
#define PMU_EVENT_FP_OPERATIONS         0x0501  /* FP operations */
#define PMU_EVENT_FP_SINGLE             0x0502  /* Single-precision FP */
#define PMU_EVENT_FP_DOUBLE             0x0503  /* Double-precision FP */
#define PMU_EVENT_FP_QUAD               0x0504  /* Quad-precision FP */
#define PMU_EVENT_FP_DIV                0x0510  /* FP divide */
#define PMU_EVENT_FP_SQRT               0x0511  /* FP sqrt */
#define PMU_EVENT_FP_FMA                0x0512  /* FP fused multiply-add */

/* Vector Events (0x0600 - 0x06FF) */
#define PMU_EVENT_VECTOR_INSTRUCTIONS   0x0600  /* Vector instructions */
#define PMU_EVENT_VECTOR_OPERATIONS     0x0601  /* Vector operations */
#define PMU_EVENT_VECTOR_INT            0x0602  /* Vector integer ops */
#define PMU_EVENT_VECTOR_FP             0x0603  /* Vector FP ops */
#define PMU_EVENT_VECTOR_LOAD           0x0604  /* Vector loads */
#define PMU_EVENT_VECTOR_STORE          0x0605  /* Vector stores */
#define PMU_EVENT_VECTOR_REDUCTION      0x0606  /* Vector reductions */

/* Matrix Events (0x0700 - 0x07FF) */
#define PMU_EVENT_MATRIX_INSTRUCTIONS   0x0700  /* Matrix instructions */
#define PMU_EVENT_MATRIX_OPERATIONS     0x0701  /* Matrix operations */
#define PMU_EVENT_MATRIX_MULTIPLY       0x0702  /* Matrix multiply ops */
#define PMU_EVENT_MATRIX_LOAD           0x0703  /* Matrix loads */
#define PMU_EVENT_MATRIX_STORE          0x0704  /* Matrix stores */

/* Neural Network Events (0x0800 - 0x08FF) */
#define PMU_EVENT_NN_INSTRUCTIONS       0x0800  /* NN instructions */
#define PMU_EVENT_NN_OPERATIONS         0x0801  /* NN operations */
#define PMU_EVENT_NN_FORWARD            0x0802  /* Forward pass ops */
#define PMU_EVENT_NN_BACKWARD           0x0803  /* Backward pass ops */
#define PMU_EVENT_NN_CONV               0x0804  /* Convolution ops */
#define PMU_EVENT_NN_POOL               0x0805  /* Pooling ops */
#define PMU_EVENT_NN_ACTIVATION         0x0806  /* Activation ops */

/* Cryptography Events (0x0900 - 0x09FF) */
#define PMU_EVENT_CRYPTO_INSTRUCTIONS   0x0900  /* Crypto instructions */
#define PMU_EVENT_AES_OPERATIONS        0x0901  /* AES operations */
#define PMU_EVENT_SHA_OPERATIONS        0x0902  /* SHA operations */
#define PMU_EVENT_RSA_OPERATIONS        0x0903  /* RSA operations */
#define PMU_EVENT_ECC_OPERATIONS        0x0904  /* ECC operations */

/* Security Events (0x0A00 - 0x0AFF) */
#define PMU_EVENT_CAPABILITY_CHECK      0x0A00  /* CHERI cap checks */
#define PMU_EVENT_CAPABILITY_FAULT      0x0A01  /* CHERI cap faults */
#define PMU_EVENT_ENCLAVE_ENTER         0x0A02  /* Enclave entries */
#define PMU_EVENT_ENCLAVE_EXIT          0x0A03  /* Enclave exits */
#define PMU_EVENT_WORLD_SWITCH          0x0A04  /* World switches */
#define PMU_EVENT_PRIVILEGE_CHANGE      0x0A05  /* Privilege changes */

/* Power Events (0x0B00 - 0x0BFF) */
#define PMU_EVENT_FREQUENCY_CHANGES     0x0B00  /* Frequency changes */
#define PMU_EVENT_SLEEP_CYCLES          0x0B01  /* Sleep state cycles */
#define PMU_EVENT_C1_CYCLES             0x0B02  /* C1 state cycles */
#define PMU_EVENT_C2_CYCLES             0x0B03  /* C2 state cycles */
#define PMU_EVENT_C3_CYCLES             0x0B04  /* C3 state cycles */
```

### Derived Metrics

```c
/* Calculate instructions per cycle (IPC) */
double calculate_ipc(void) {
    uint64_t cycles = pmu_read_counter(0);
    uint64_t instructions = pmu_read_counter(1);
    return (double)instructions / (double)cycles;
}

/* Calculate cache miss rate */
double calculate_cache_miss_rate(void) {
    uint64_t accesses = pmu_read_counter(2);
    uint64_t misses = pmu_read_counter(3);
    return (double)misses / (double)accesses;
}

/* Calculate branch misprediction rate */
double calculate_branch_miss_rate(void) {
    uint64_t branches = pmu_read_counter(4);
    uint64_t mispredicts = pmu_read_counter(5);
    return (double)mispredicts / (double)branches;
}

/* Calculate FLOPS (floating-point operations per second) */
double calculate_flops(uint64_t time_ns) {
    uint64_t fp_ops = pmu_read_counter(6);
    return (double)fp_ops / ((double)time_ns / 1e9);
}
```

---

## 5. Statistical Profiling

### Sampling-Based Profiling

Statistical profiling periodically samples the program counter and other state to build a performance profile.

```c
/* Profiling control register */
typedef struct {
    uint32_t enable         : 1;    /* Enable profiling */
    uint32_t mode           : 2;    /* Profiling mode */
    uint32_t sample_event   : 16;   /* Event to trigger sample */
    uint32_t sample_period  : 12;   /* Sample every N events */
    uint32_t overflow_int   : 1;    /* Interrupt on buffer full */
} prof_ctrl_t;

/* Profiling modes */
#define PROF_MODE_PC_ONLY       0   /* Sample PC only */
#define PROF_MODE_PC_LR         1   /* Sample PC + return address */
#define PROF_MODE_FULL_CONTEXT  2   /* Sample full context */
#define PROF_MODE_BRANCH_TRACE  3   /* Branch trace buffer */

/* Profile sample record */
typedef struct {
    uint64_t timestamp;             /* TSC timestamp */
    uint64_t pc;                    /* Program counter */
    uint64_t lr;                    /* Link register */
    uint32_t event_type;            /* Triggering event */
    uint32_t core_id;               /* Core ID */
    uint64_t cycles;                /* Cycle count */
    uint64_t instructions;          /* Instruction count */
} profile_sample_t;

/* Profile buffer */
typedef struct {
    profile_sample_t *samples;      /* Sample array */
    uint32_t capacity;              /* Buffer capacity */
    volatile uint32_t head;         /* Write head */
    volatile uint32_t tail;         /* Read tail */
    volatile uint32_t overflows;    /* Overflow count */
} profile_buffer_t;
```

### Profiling Setup

```c
/* Initialize statistical profiling */
void profiling_init(profile_buffer_t *buffer, uint32_t capacity) {
    buffer->samples = malloc(capacity * sizeof(profile_sample_t));
    buffer->capacity = capacity;
    buffer->head = 0;
    buffer->tail = 0;
    buffer->overflows = 0;

    /* Set buffer base address */
    asm volatile("mtsr $PROF_BUFFER, %0" : : "r"(buffer->samples));

    /* Configure profiling: sample every 10000 cycles */
    uint32_t ctrl = 0;
    ctrl |= (1 << 0);                       /* Enable */
    ctrl |= (PROF_MODE_PC_LR << 1);         /* PC + LR mode */
    ctrl |= (PMU_EVENT_CYCLES << 3);        /* Sample on cycles */
    ctrl |= (10000 << 19);                  /* Period = 10000 */
    ctrl |= (1 << 31);                      /* Interrupt on full */

    asm volatile("mtsr $PROF_CTRL, %0" : : "r"(ctrl));
}

/* Profiling interrupt handler */
void profiling_interrupt_handler(void) {
    uint64_t pc, lr, tsc;

    /* Read sampled state */
    asm volatile("mfsr %0, $PROF_PC" : "=r"(pc));
    asm volatile("mfsr %0, $LR" : "=r"(lr));
    asm volatile("mfsr %0, $TSC" : "=r"(tsc));

    /* Store sample in buffer */
    profile_buffer_t *buffer = get_profile_buffer();
    uint32_t head = buffer->head;

    if (((head + 1) % buffer->capacity) != buffer->tail) {
        buffer->samples[head].timestamp = tsc;
        buffer->samples[head].pc = pc;
        buffer->samples[head].lr = lr;
        buffer->samples[head].event_type = PMU_EVENT_CYCLES;
        buffer->samples[head].core_id = get_core_id();

        buffer->head = (head + 1) % buffer->capacity;
    } else {
        buffer->overflows++;
    }
}

/* Retrieve profiling samples */
int profiling_get_samples(profile_buffer_t *buffer,
                         profile_sample_t *out, uint32_t max_samples) {
    uint32_t count = 0;

    while (buffer->tail != buffer->head && count < max_samples) {
        out[count++] = buffer->samples[buffer->tail];
        buffer->tail = (buffer->tail + 1) % buffer->capacity;
    }

    return count;
}
```

### Call Stack Sampling

```c
/* Sample with call stack unwinding */
typedef struct {
    uint64_t timestamp;
    uint64_t pc;
    uint64_t stack_trace[32];       /* Up to 32 stack frames */
    uint32_t stack_depth;
    uint32_t core_id;
} stack_sample_t;

/* Unwind call stack */
uint32_t unwind_stack(uint64_t fp, uint64_t *trace, uint32_t max_depth) {
    uint32_t depth = 0;

    while (fp != 0 && depth < max_depth) {
        /* Read return address from stack frame */
        uint64_t ra = *(uint64_t *)(fp - 8);
        trace[depth++] = ra;

        /* Follow frame pointer chain */
        fp = *(uint64_t *)(fp - 16);

        /* Sanity check */
        if (fp < 0x1000 || fp > 0x7FFFFFFFFFFF)
            break;
    }

    return depth;
}
```

---

## 6. Debug and Trace

### Program Trace Buffer

```c
/* Trace buffer for instruction execution */
typedef struct {
    uint64_t pc;                    /* Instruction address */
    uint32_t instruction;           /* Instruction encoding */
    uint64_t operands[3];           /* Operand values */
    uint64_t result;                /* Result value */
    uint64_t timestamp;             /* TSC timestamp */
    uint32_t flags;                 /* Execution flags */
} trace_entry_t;

#define TRACE_FLAG_BRANCH_TAKEN     0x00000001
#define TRACE_FLAG_EXCEPTION        0x00000002
#define TRACE_FLAG_INTERRUPT        0x00000004
#define TRACE_FLAG_SYSCALL          0x00000008
#define TRACE_FLAG_CACHE_MISS       0x00000010

/* Trace control register */
typedef struct {
    uint32_t enable         : 1;    /* Enable tracing */
    uint32_t trace_mode     : 3;    /* Trace mode */
    uint32_t filter_enable  : 1;    /* Enable filtering */
    uint32_t wrap_mode      : 1;    /* Wrap on buffer full */
    uint32_t timestamp_en   : 1;    /* Include timestamps */
} trace_ctrl_t;

/* Trace modes */
#define TRACE_MODE_ALL          0   /* Trace all instructions */
#define TRACE_MODE_BRANCHES     1   /* Trace branches only */
#define TRACE_MODE_LOADS_STORES 2   /* Trace memory accesses */
#define TRACE_MODE_EXCEPTIONS   3   /* Trace exceptions/interrupts */
#define TRACE_MODE_FILTERED     4   /* Use address filter */
```

### Branch Trace

```c
/* Last Branch Record (LBR) buffer */
typedef struct {
    uint64_t from_pc;               /* Source address */
    uint64_t to_pc;                 /* Target address */
    uint64_t timestamp;             /* TSC timestamp */
    uint32_t mispredicted : 1;      /* Misprediction flag */
    uint32_t taken : 1;             /* Branch taken */
    uint32_t reserved : 30;
} lbr_entry_t;

#define LBR_DEPTH   32              /* 32 LBR entries */

/* LBR registers */
#define SR_LBR_CTRL     0x250       /* LBR control */
#define SR_LBR_TOS      0x251       /* LBR top-of-stack */
#define SR_LBR_FROM_0   0x260       /* LBR from address 0 */
#define SR_LBR_TO_0     0x280       /* LBR to address 0 */

/* Read LBR stack */
void read_lbr_stack(lbr_entry_t *entries, uint32_t *count) {
    uint32_t tos;
    asm volatile("mfsr %0, $LBR_TOS" : "=r"(tos));

    *count = (tos < LBR_DEPTH) ? tos : LBR_DEPTH;

    for (uint32_t i = 0; i < *count; i++) {
        uint64_t from, to;
        asm volatile("mfsr %0, $LBR_FROM_0 + %1" : "=r"(from) : "r"(i));
        asm volatile("mfsr %0, $LBR_TO_0 + %1" : "=r"(to) : "r"(i));

        entries[i].from_pc = from;
        entries[i].to_pc = to;
    }
}
```

---

## 7. Timer Interrupts

### Programmable Timer

```c
/* Timer interrupt registers */
#define SR_TIMER_VALUE      0x208   /* Current timer value */
#define SR_TIMER_COMPARE    0x209   /* Compare value */
#define SR_TIMER_INT_CTRL   0x20A   /* Interrupt control */

/* Timer control flags */
#define TIMER_INT_ENABLE    0x00000001  /* Enable timer interrupt */
#define TIMER_PERIODIC      0x00000002  /* Periodic mode */
#define TIMER_ONE_SHOT      0x00000000  /* One-shot mode */

/* Configure periodic timer interrupt */
void timer_setup_periodic(uint64_t period_ns) {
    uint64_t tsc_freq, ticks;

    asm volatile("mfsr %0, $TSC_FREQ" : "=r"(tsc_freq));
    ticks = (period_ns * tsc_freq) / 1000000000ULL;

    /* Set compare value */
    asm volatile("mtsr $TIMER_COMPARE, %0" : : "r"(ticks));

    /* Enable periodic timer interrupt */
    uint32_t ctrl = TIMER_INT_ENABLE | TIMER_PERIODIC;
    asm volatile("mtsr $TIMER_INT_CTRL, %0" : : "r"(ctrl));
}

/* Timer interrupt handler */
void timer_interrupt_handler(void) {
    /* Clear interrupt */
    uint32_t ctrl = TIMER_INT_ENABLE | TIMER_PERIODIC;
    asm volatile("mtsr $TIMER_INT_CTRL, %0" : : "r"(ctrl));

    /* Handle timer tick */
    handle_timer_tick();
}
```

### Watchdog Timer

```c
/* Watchdog timer registers */
#define SR_WATCHDOG_VALUE   0x20B   /* Watchdog counter */
#define SR_WATCHDOG_CTRL    0x20C   /* Watchdog control */

/* Watchdog control flags */
#define WATCHDOG_ENABLE     0x00000001
#define WATCHDOG_RESET_EN   0x00000002  /* Reset on timeout */
#define WATCHDOG_INT_EN     0x00000004  /* Interrupt before reset */

/* Setup watchdog timer */
void watchdog_init(uint64_t timeout_ns) {
    uint64_t tsc_freq, ticks;

    asm volatile("mfsr %0, $TSC_FREQ" : "=r"(tsc_freq));
    ticks = (timeout_ns * tsc_freq) / 1000000000ULL;

    /* Set watchdog timeout */
    asm volatile("mtsr $WATCHDOG_VALUE, %0" : : "r"(ticks));

    /* Enable watchdog with reset */
    uint32_t ctrl = WATCHDOG_ENABLE | WATCHDOG_RESET_EN | WATCHDOG_INT_EN;
    asm volatile("mtsr $WATCHDOG_CTRL, %0" : : "r"(ctrl));
}

/* Pet the watchdog */
static inline void watchdog_reset(void) {
    uint64_t ticks;
    asm volatile("mfsr %0, $WATCHDOG_VALUE" : "=r"(ticks));
    asm volatile("mtsr $WATCHDOG_VALUE, %0" : : "r"(ticks));
}
```

---

## 8. Instructions

### Timer Instructions (New ISA Mnemonics)

```assembly
# Time Stamp Counter
rdtsc       rd                      # Read TSC to rd
rdtscp      rd, aux                 # Read TSC + serialize + aux data

# Performance Monitoring
pmcread     rd, pmc_num             # Read PMC to rd
pmcwrite    pmc_num, rs             # Write rs to PMC
pmcreset    pmc_num                 # Reset PMC to zero
pmcfg       pmc_num, event, flags   # Configure PMC

# RTC (Real-Time Clock)
rdrtc       rd                      # Read RTC (nanoseconds)
rdtime      rd                      # Read wall-clock time

# Profiling
profstart                           # Start profiling
profstop                            # Stop profiling
profsample  rd                      # Take manual sample

# Timer control
timerset    value                   # Set timer compare value
timerget    rd                      # Get current timer value
```

### Legacy Instruction Encoding

For compatibility, legacy encodings are also supported:

```assembly
# System register access (generic)
mfsr        rd, sr_num              # Move from system register
mtsr        sr_num, rs              # Move to system register

# Examples:
mfsr        t0, $TSC                # Read TSC
mfsr        t1, $PMC0               # Read PMC0
mtsr        $PMCSEL0, t2            # Configure PMC0
```

---

## 9. Programming Examples

### Example 1: Measure Function Performance

```c
#include <stdint.h>
#include <stdio.h>

/* Measure cycles and instructions for a function */
typedef struct {
    uint64_t cycles;
    uint64_t instructions;
    uint64_t l1d_misses;
    uint64_t branches;
    uint64_t branch_misses;
    double ipc;
    double miss_rate;
    double branch_miss_rate;
} perf_stats_t;

void benchmark_function(void (*func)(void), perf_stats_t *stats) {
    /* Configure performance counters */
    pmu_init_counter(0, PMU_EVENT_CYCLES, 0);
    pmu_init_counter(1, PMU_EVENT_INSTRUCTIONS, 0);
    pmu_init_counter(2, PMU_EVENT_L1D_MISS, 0);
    pmu_init_counter(3, PMU_EVENT_BRANCHES, 0);
    pmu_init_counter(4, PMU_EVENT_BRANCH_MISS, 0);

    /* Reset all counters */
    for (int i = 0; i < 5; i++)
        pmu_reset_counter(i);

    /* Enable PMU */
    pmu_enable();

    /* Run function */
    func();

    /* Read counters */
    stats->cycles = pmu_read_counter(0);
    stats->instructions = pmu_read_counter(1);
    stats->l1d_misses = pmu_read_counter(2);
    stats->branches = pmu_read_counter(3);
    stats->branch_misses = pmu_read_counter(4);

    /* Calculate derived metrics */
    stats->ipc = (double)stats->instructions / (double)stats->cycles;
    stats->miss_rate = (double)stats->l1d_misses / (double)stats->instructions;
    stats->branch_miss_rate = (double)stats->branch_misses / (double)stats->branches;
}

/* Example usage */
void my_computation(void) {
    /* Some computation */
    for (int i = 0; i < 1000000; i++) {
        volatile int x = i * i;
    }
}

int main(void) {
    perf_stats_t stats;

    benchmark_function(my_computation, &stats);

    printf("Performance Statistics:\n");
    printf("  Cycles:            %llu\n", stats.cycles);
    printf("  Instructions:      %llu\n", stats.instructions);
    printf("  IPC:               %.3f\n", stats.ipc);
    printf("  L1D misses:        %llu\n", stats.l1d_misses);
    printf("  Cache miss rate:   %.4f%%\n", stats.miss_rate * 100.0);
    printf("  Branch misses:     %llu\n", stats.branch_misses);
    printf("  Branch miss rate:  %.2f%%\n", stats.branch_miss_rate * 100.0);

    return 0;
}
```

### Example 2: High-Resolution Timing

```c
/* Measure elapsed time with nanosecond precision */
typedef struct {
    uint64_t start_tsc;
    uint64_t start_rtc;
} timer_t;

void timer_start(timer_t *timer) {
    timer->start_rtc = get_wall_time_ns();
    timer->start_tsc = rdtsc();
}

uint64_t timer_elapsed_ns(timer_t *timer) {
    uint64_t end_tsc = rdtsc();
    return tsc_to_ns(end_tsc - timer->start_tsc);
}

uint64_t timer_elapsed_cycles(timer_t *timer) {
    return rdtsc() - timer->start_tsc;
}

/* Example: Benchmark memory copy */
void benchmark_memcpy(void) {
    timer_t timer;
    char src[1024 * 1024];
    char dst[1024 * 1024];

    timer_start(&timer);

    memcpy(dst, src, sizeof(src));

    uint64_t elapsed_ns = timer_elapsed_ns(&timer);
    uint64_t elapsed_cycles = timer_elapsed_cycles(&timer);

    printf("memcpy(1MB):\n");
    printf("  Time: %llu ns (%.3f us)\n", elapsed_ns, elapsed_ns / 1000.0);
    printf("  Cycles: %llu\n", elapsed_cycles);
    printf("  Bandwidth: %.2f GB/s\n",
           (1024.0 * 1024.0) / (elapsed_ns / 1e9) / 1e9);
}
```

### Example 3: System-Wide Profiling

```c
/* Profile entire system with call stacks */
typedef struct {
    uint64_t pc;
    uint64_t count;
    uint64_t stack_trace[32];
    uint32_t stack_depth;
} profile_entry_t;

#define MAX_PROFILE_ENTRIES 10000

profile_entry_t profile_data[MAX_PROFILE_ENTRIES];
uint32_t profile_count = 0;

void profile_system(uint32_t duration_ms) {
    profile_buffer_t buffer;
    profiling_init(&buffer, 100000);

    /* Wait for duration */
    uint64_t start = get_wall_time_ns();
    while ((get_wall_time_ns() - start) < (duration_ms * 1000000ULL)) {
        /* Let profiling happen */
    }

    /* Retrieve samples */
    profile_sample_t samples[1000];
    int count;

    while ((count = profiling_get_samples(&buffer, samples, 1000)) > 0) {
        for (int i = 0; i < count; i++) {
            /* Aggregate by PC */
            int found = 0;
            for (uint32_t j = 0; j < profile_count; j++) {
                if (profile_data[j].pc == samples[i].pc) {
                    profile_data[j].count++;
                    found = 1;
                    break;
                }
            }

            if (!found && profile_count < MAX_PROFILE_ENTRIES) {
                profile_data[profile_count].pc = samples[i].pc;
                profile_data[profile_count].count = 1;
                profile_count++;
            }
        }
    }

    /* Sort by count (hottest first) */
    qsort(profile_data, profile_count, sizeof(profile_entry_t),
          compare_profile_entries);

    /* Print top 10 hot spots */
    printf("Top 10 hot spots:\n");
    for (uint32_t i = 0; i < 10 && i < profile_count; i++) {
        printf("  0x%llx: %llu samples (%.2f%%)\n",
               profile_data[i].pc,
               profile_data[i].count,
               (double)profile_data[i].count * 100.0 / buffer.head);
    }
}
```

### Example 4: Cache Profiling

```c
/* Analyze cache behavior */
typedef struct {
    uint64_t l1i_accesses;
    uint64_t l1i_misses;
    uint64_t l1d_accesses;
    uint64_t l1d_misses;
    uint64_t l2_accesses;
    uint64_t l2_misses;
    uint64_t l3_accesses;
    uint64_t l3_misses;

    double l1i_miss_rate;
    double l1d_miss_rate;
    double l2_miss_rate;
    double l3_miss_rate;
} cache_stats_t;

void profile_cache_behavior(void (*func)(void), cache_stats_t *stats) {
    /* Configure counters for cache events */
    pmu_init_counter(0, PMU_EVENT_L1I_ACCESS, 0);
    pmu_init_counter(1, PMU_EVENT_L1I_MISS, 0);
    pmu_init_counter(2, PMU_EVENT_L1D_ACCESS, 0);
    pmu_init_counter(3, PMU_EVENT_L1D_MISS, 0);
    pmu_init_counter(4, PMU_EVENT_L2_ACCESS, 0);
    pmu_init_counter(5, PMU_EVENT_L2_MISS, 0);
    pmu_init_counter(6, PMU_EVENT_L3_ACCESS, 0);
    pmu_init_counter(7, PMU_EVENT_L3_MISS, 0);

    /* Reset and enable */
    for (int i = 0; i < 8; i++)
        pmu_reset_counter(i);
    pmu_enable();

    /* Run function */
    func();

    /* Read results */
    stats->l1i_accesses = pmu_read_counter(0);
    stats->l1i_misses = pmu_read_counter(1);
    stats->l1d_accesses = pmu_read_counter(2);
    stats->l1d_misses = pmu_read_counter(3);
    stats->l2_accesses = pmu_read_counter(4);
    stats->l2_misses = pmu_read_counter(5);
    stats->l3_accesses = pmu_read_counter(6);
    stats->l3_misses = pmu_read_counter(7);

    /* Calculate miss rates */
    stats->l1i_miss_rate = (double)stats->l1i_misses / stats->l1i_accesses;
    stats->l1d_miss_rate = (double)stats->l1d_misses / stats->l1d_accesses;
    stats->l2_miss_rate = (double)stats->l2_misses / stats->l2_accesses;
    stats->l3_miss_rate = (double)stats->l3_misses / stats->l3_accesses;
}
```

### Example 5: Multi-Core Profiling

```c
/* Per-core statistics */
typedef struct {
    uint32_t core_id;
    uint64_t cycles;
    uint64_t instructions;
    uint64_t cache_misses;
    double ipc;
    double utilization;
} core_stats_t;

/* Collect stats from all cores */
void profile_all_cores(core_stats_t *stats, uint32_t num_cores,
                       uint64_t duration_ms) {
    /* Start profiling on all cores */
    for (uint32_t i = 0; i < num_cores; i++) {
        /* Set affinity to core i */
        set_cpu_affinity(i);

        /* Reset counters */
        pmu_reset_counter(0);  /* Cycles */
        pmu_reset_counter(1);  /* Instructions */
        pmu_reset_counter(2);  /* Cache misses */
        pmu_enable();
    }

    /* Wait for duration */
    usleep(duration_ms * 1000);

    /* Collect results */
    for (uint32_t i = 0; i < num_cores; i++) {
        set_cpu_affinity(i);

        stats[i].core_id = i;
        stats[i].cycles = pmu_read_counter(0);
        stats[i].instructions = pmu_read_counter(1);
        stats[i].cache_misses = pmu_read_counter(2);
        stats[i].ipc = (double)stats[i].instructions / stats[i].cycles;

        /* Calculate utilization (assuming 3 GHz CPU) */
        uint64_t max_cycles = duration_ms * 3000000ULL;
        stats[i].utilization = (double)stats[i].cycles / max_cycles;
    }

    /* Print summary */
    printf("Core    Cycles       Instructions   IPC    Util%%\n");
    printf("----    ----------   ------------   ----   -----\n");
    for (uint32_t i = 0; i < num_cores; i++) {
        printf("%-4u    %-12llu %-14llu %.2f   %.1f%%\n",
               stats[i].core_id,
               stats[i].cycles,
               stats[i].instructions,
               stats[i].ipc,
               stats[i].utilization * 100.0);
    }
}
```

---

## Summary

The DLX timer and profiling architecture provides:

1. **High-precision timing**: 64-bit TSC with nanosecond resolution
2. **Performance monitoring**: 8 programmable counters tracking 100+ events
3. **Statistical profiling**: Sampling-based profiling with call stacks
4. **Cache analysis**: Detailed cache hierarchy profiling
5. **Branch profiling**: Last Branch Record (LBR) buffer
6. **Timer interrupts**: Programmable periodic and watchdog timers
7. **Multi-core support**: Per-core statistics and synchronization
8. **Low overhead**: Hardware-based monitoring with minimal performance impact

These features enable comprehensive performance analysis, optimization, and debugging for all types of workloads on the DLX architecture.
