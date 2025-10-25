/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * Alpha Performance Monitoring
 */

#include <mach/mach_types.h>
#include <architecture/alpha/cpu.h>
#include <architecture/alpha/pal.h>

/*
 * Performance counter data
 */
struct alpha_perf_counter {
	unsigned long	cycle_count;		/* CPU cycles */
	unsigned long	instruction_count;	/* Instructions executed */
	unsigned long	cache_misses;		/* Cache misses */
	unsigned long	tlb_misses;		/* TLB misses */
	unsigned long	branch_mispredicts;	/* Branch mispredictions */
	unsigned long	last_sample_time;	/* Last sample timestamp */
};

static struct alpha_perf_counter perf_counters[MAX_CPUS];

/*
 * Read CPU cycle counter
 *
 * The RPCC instruction returns:
 *   Bits 0-31: Cycle counter (wraps every ~4 billion cycles)
 *   Bits 32-63: Implementation-specific offset
 */
unsigned long
alpha_read_cycles(void)
{
	unsigned long cycles;

	__asm__ volatile ("rpcc %0" : "=r" (cycles));

	return cycles & 0xFFFFFFFFUL;
}

/*
 * Read high-resolution timestamp
 */
unsigned long long
alpha_read_timestamp(void)
{
	unsigned long cycles;

	__asm__ volatile ("rpcc %0" : "=r" (cycles));

	return cycles;
}

/*
 * Initialize performance monitoring
 */
void
alpha_perf_init(void)
{
	int i;

	for (i = 0; i < MAX_CPUS; i++) {
		perf_counters[i].cycle_count = 0;
		perf_counters[i].instruction_count = 0;
		perf_counters[i].cache_misses = 0;
		perf_counters[i].tlb_misses = 0;
		perf_counters[i].branch_mispredicts = 0;
		perf_counters[i].last_sample_time = alpha_read_timestamp();
	}
}

/*
 * Start performance counter
 */
void
alpha_perf_start(int counter_type)
{
	int cpu = cpu_number();

	switch (counter_type) {
	case PERF_CYCLES:
		perf_counters[cpu].cycle_count = alpha_read_cycles();
		break;

	case PERF_INSTRUCTIONS:
		/* Would configure hardware performance counters if available */
		break;

	default:
		break;
	}
}

/*
 * Stop and read performance counter
 */
unsigned long
alpha_perf_stop(int counter_type)
{
	int cpu = cpu_number();
	unsigned long delta = 0;

	switch (counter_type) {
	case PERF_CYCLES:
		delta = alpha_read_cycles() - perf_counters[cpu].cycle_count;
		break;

	case PERF_INSTRUCTIONS:
		/* Would read hardware performance counters if available */
		break;

	default:
		break;
	}

	return delta;
}

/*
 * Sample all performance counters
 */
void
alpha_perf_sample(void)
{
	int cpu = cpu_number();
	unsigned long long now = alpha_read_timestamp();
	unsigned long long delta = now - perf_counters[cpu].last_sample_time;

	perf_counters[cpu].last_sample_time = now;

	/*
	 * Additional performance counter sampling would go here
	 * using PALcode performance monitoring functions
	 */
}

/*
 * Get performance counter value
 */
unsigned long
alpha_perf_get(int counter_type)
{
	int cpu = cpu_number();

	switch (counter_type) {
	case PERF_CYCLES:
		return perf_counters[cpu].cycle_count;

	case PERF_INSTRUCTIONS:
		return perf_counters[cpu].instruction_count;

	case PERF_CACHE_MISSES:
		return perf_counters[cpu].cache_misses;

	case PERF_TLB_MISSES:
		return perf_counters[cpu].tlb_misses;

	case PERF_BRANCH_MISPREDICTS:
		return perf_counters[cpu].branch_mispredicts;

	default:
		return 0;
	}
}

/*
 * Reset performance counters
 */
void
alpha_perf_reset(void)
{
	int cpu = cpu_number();

	perf_counters[cpu].cycle_count = 0;
	perf_counters[cpu].instruction_count = 0;
	perf_counters[cpu].cache_misses = 0;
	perf_counters[cpu].tlb_misses = 0;
	perf_counters[cpu].branch_mispredicts = 0;
	perf_counters[cpu].last_sample_time = alpha_read_timestamp();
}

/*
 * Print performance statistics
 */
void
alpha_perf_print(void)
{
	int cpu = cpu_number();

	printf("\nPerformance Statistics (CPU %d):\n", cpu);
	printf("  Cycles:             %lu\n", perf_counters[cpu].cycle_count);
	printf("  Instructions:       %lu\n", perf_counters[cpu].instruction_count);
	printf("  Cache misses:       %lu\n", perf_counters[cpu].cache_misses);
	printf("  TLB misses:         %lu\n", perf_counters[cpu].tlb_misses);
	printf("  Branch mispredicts: %lu\n", perf_counters[cpu].branch_mispredicts);
}

/*
 * Measure function execution time
 */
unsigned long
alpha_perf_time_function(void (*func)(void))
{
	unsigned long start, end;

	start = alpha_read_cycles();
	func();
	end = alpha_read_cycles();

	return end - start;
}

/*
 * Calculate CPU frequency from cycle counter
 *
 * This requires a known time reference (e.g., RTC)
 */
unsigned long
alpha_calculate_cpu_frequency(void)
{
	unsigned long start_cycles, end_cycles;
	unsigned long start_time, end_time;
	unsigned long cycles, milliseconds;

	/*
	 * Sample over a known time period (e.g., 100ms)
	 */
	start_cycles = alpha_read_cycles();
	start_time = 0;  /* Would read from RTC */

	/* Wait 100ms */
	/* delay(100); */

	end_cycles = alpha_read_cycles();
	end_time = 100;  /* Would read from RTC */

	cycles = end_cycles - start_cycles;
	milliseconds = end_time - start_time;

	/*
	 * Calculate frequency in MHz
	 */
	if (milliseconds > 0)
		return (cycles / milliseconds) / 1000;

	return 0;
}

/* Performance counter types */
#define PERF_CYCLES			0
#define PERF_INSTRUCTIONS		1
#define PERF_CACHE_MISSES		2
#define PERF_TLB_MISSES			3
#define PERF_BRANCH_MISPREDICTS		4

#define MAX_CPUS 64
