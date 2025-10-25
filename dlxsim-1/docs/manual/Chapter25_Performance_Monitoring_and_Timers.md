# Chapter 25: Performance Monitoring and Timers

## 25.1 Time Stamp Counter (TSC)

64-bit counter incrementing at constant rate:

```assembly
# Read TSC
rdtsc   rd                      # rd ← TSC (64-bit)

# Read TSC frequency
csrr    rd, tsc_freq            # rd ← TSC frequency in Hz

# Example: Measure cycles
    rdtsc   t0                  # Start time
    call    function            # Function to measure
    rdtsc   t1                  # End time
    sub     a0, t1, t0          # Cycles elapsed
```

### TSC Properties
- **Constant rate**: Independent of CPU frequency scaling
- **Monotonic**: Never decreases
- **Synchronized**: Consistent across cores
- **Resolution**: Nanosecond-level on most systems

## 25.2 Real-Time Clock (RTC)

High-precision wall-clock time:

```assembly
# Read RTC (nanoseconds since epoch)
csrr    rd, rtc                 # rd ← nanoseconds

# Example: Get current time
    csrr    a0, rtc
    li      t0, 1000000000      # 1 billion ns/s
    div     a1, a0, t0          # Seconds
    rem     a2, a0, t0          # Nanoseconds
```

## 25.3 Performance Monitoring Counters (PMC)

8 programmable 64-bit performance counters:

```assembly
# Read PMC
csrr    rd, pmc0                # Read counter 0
csrr    rd, pmc1                # Read counter 1
# ... pmc0-pmc7 (8 counters)

# Configure PMC event
csrw    pmcsel0, rs             # Select event for PMC0

# Enable/disable counters
csrw    pmu_enable, rs          # Bitmask of enabled counters

# Reset counter
csrw    pmc0, zero              # Reset PMC0
```

## 25.4 Performance Events

Over 100 hardware events available:

### Cycle Counting
```
EVENT_CYCLES            0x00    /* CPU cycles */
EVENT_REF_CYCLES        0x01    /* Reference cycles (constant rate) */
EVENT_STALL_CYCLES      0x02    /* Stalled cycles */
EVENT_FRONTEND_STALL    0x03    /* Frontend stalls */
EVENT_BACKEND_STALL     0x04    /* Backend stalls */
```

### Instruction Counting
```
EVENT_INSTRUCTIONS      0x10    /* Instructions retired */
EVENT_BRANCHES          0x11    /* Branch instructions */
EVENT_BRANCH_MISS       0x12    /* Branch mispredictions */
EVENT_LOADS             0x13    /* Load instructions */
EVENT_STORES            0x14    /* Store instructions */
```

### Cache Events
```
EVENT_L1I_ACCESS        0x20    /* L1 I-cache accesses */
EVENT_L1I_MISS          0x21    /* L1 I-cache misses */
EVENT_L1D_ACCESS        0x22    /* L1 D-cache accesses */
EVENT_L1D_MISS          0x23    /* L1 D-cache misses */
EVENT_L2_ACCESS         0x24    /* L2 cache accesses */
EVENT_L2_MISS           0x25    /* L2 cache misses */
EVENT_LLC_ACCESS        0x26    /* Last-level cache accesses */
EVENT_LLC_MISS          0x27    /* Last-level cache misses */
```

### TLB Events
```
EVENT_ITLB_ACCESS       0x30    /* ITLB accesses */
EVENT_ITLB_MISS         0x31    /* ITLB misses */
EVENT_DTLB_ACCESS       0x32    /* DTLB accesses */
EVENT_DTLB_MISS         0x33    /* DTLB misses */
EVENT_TLB_FLUSH         0x34    /* TLB flushes */
```

## 25.5 Statistical Profiling

Sample-based profiling with call stacks:

```c
/* Profiling sample */
struct prof_sample {
    uint64_t pc;                /* Program counter */
    uint64_t timestamp;         /* Sample time */
    uint32_t event_count;       /* Event count */
    uint32_t pid;               /* Process ID */
    uint64_t callchain[16];     /* Call stack */
};

/* Enable profiling */
struct prof_config {
    uint32_t enable      : 1;
    uint32_t event_sel   : 8;   /* Event to sample */
    uint32_t sample_freq : 16;  /* Samples per second */
    uint32_t stack_depth : 7;   /* Call stack depth */
};
```

## 25.6 Watchpoint Timers

Programmable interval timers:

```assembly
# Set timer interval
csrw    timer_period, rs        # Period in cycles

# Enable timer interrupt
csrw    timer_ctrl, rs          # Control register

# Read timer value
csrr    rd, timer_value         # Current timer value
```

## 25.7 Deadline Timers

Absolute-time timers for real-time tasks:

```assembly
# Set deadline
csrw    deadline_timer, rs      # Absolute TSC value

# Example: Sleep until time
    rdtsc   t0
    li      t1, 1000000         # 1M cycles
    add     t0, t0, t1          # Deadline = now + 1M
    csrw    deadline_timer, t0
    wfi                         # Wait for interrupt
```

## 25.8 Performance Monitoring API

```c
/* Configure PMC */
int pmc_config(int counter, int event) {
    asm volatile("csrw pmc_sel%0, %1" :: "i"(counter), "r"(event));
    return 0;
}

/* Read PMC */
uint64_t pmc_read(int counter) {
    uint64_t value;
    asm volatile("csrr %0, pmc%1" : "=r"(value) : "i"(counter));
    return value;
}

/* Enable counters */
void pmc_enable(uint8_t mask) {
    asm volatile("csrw pmu_enable, %0" :: "r"(mask));
}

/* Example: Measure cache misses */
void measure_cache_misses(void) {
    pmc_config(0, EVENT_L1D_MISS);
    pmc_config(1, EVENT_LLC_MISS);
    pmc_enable(0x03);           /* Enable PMC0 and PMC1 */

    uint64_t start_l1 = pmc_read(0);
    uint64_t start_llc = pmc_read(1);

    /* Code to measure */
    run_workload();

    uint64_t end_l1 = pmc_read(0);
    uint64_t end_llc = pmc_read(1);

    printf("L1D misses: %lu\n", end_l1 - start_l1);
    printf("LLC misses: %lu\n", end_llc - start_llc);
}
```

## 25.9 Overhead Measurement

```c
/* Calibrate timer overhead */
uint64_t calibrate_overhead(void) {
    uint64_t start, end;
    uint64_t min = UINT64_MAX;

    for (int i = 0; i < 1000; i++) {
        asm volatile("rdtsc %0" : "=r"(start));
        asm volatile("rdtsc %0" : "=r"(end));
        uint64_t delta = end - start;
        if (delta < min) min = delta;
    }
    return min;
}
```

## 25.10 Performance Analysis Tools

Integration with standard tools:
- **perf** (Linux)
- **dtrace** (Solaris/BSD/macOS)
- **Intel VTune**
- **AMD uProf**
- **Custom profilers**

See DLX_TIMERS_PROFILING.md for complete documentation.
