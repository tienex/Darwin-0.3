/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * Alpha Multi-Processor Support
 */

#include <mach/mach_types.h>
#include <kern/cpu_data.h>
#include <kern/processor.h>
#include <architecture/alpha/cpu.h>
#include <architecture/alpha/pal.h>

/*
 * Per-CPU data structures
 */
struct alpha_cpu_data {
	unsigned long	cpu_id;			/* CPU number */
	unsigned long	pcb_phys;		/* Physical address of PCB */
	unsigned long	stack_phys;		/* Physical address of stack */
	unsigned long	idle_thread;		/* Idle thread pointer */
	unsigned long	active_thread;		/* Currently running thread */
	unsigned long	interrupt_stack;	/* Interrupt stack */
	unsigned long	mces;			/* Machine check error summary */
	unsigned long	tlb_flush_count;	/* TLB flush statistics */
	unsigned long	context_switches;	/* Context switch count */
	unsigned long	interrupts;		/* Interrupt count */
};

#define MAX_CPUS	64

static struct alpha_cpu_data cpu_data[MAX_CPUS];
static volatile unsigned long cpu_online_map = 0;
static volatile unsigned long cpu_started_map = 0;

/*
 * Initialize SMP support
 */
void
alpha_smp_init(void)
{
	int i;

	/*
	 * Clear all CPU data structures
	 */
	for (i = 0; i < MAX_CPUS; i++) {
		cpu_data[i].cpu_id = i;
		cpu_data[i].pcb_phys = 0;
		cpu_data[i].stack_phys = 0;
		cpu_data[i].idle_thread = 0;
		cpu_data[i].active_thread = 0;
		cpu_data[i].interrupt_stack = 0;
		cpu_data[i].mces = 0;
		cpu_data[i].tlb_flush_count = 0;
		cpu_data[i].context_switches = 0;
		cpu_data[i].interrupts = 0;
	}

	/*
	 * Mark boot CPU as online
	 */
	cpu_online_map = 1;
	cpu_started_map = 1;
}

/*
 * Start a secondary CPU
 */
kern_return_t
alpha_start_cpu(int cpu_id)
{
	extern void start_secondary(void);
	unsigned long entry;
	unsigned long stack;

	if (cpu_id < 0 || cpu_id >= MAX_CPUS)
		return KERN_INVALID_ARGUMENT;

	if (cpu_online_map & (1UL << cpu_id))
		return KERN_SUCCESS;  /* Already running */

	/*
	 * Allocate stack for this CPU
	 */
	stack = (unsigned long)kalloc(16384);  /* 16KB stack */
	if (stack == 0)
		return KERN_RESOURCE_SHORTAGE;

	cpu_data[cpu_id].stack_phys = kvtophys(stack + 16384);

	/*
	 * Set entry point for secondary CPU
	 */
	entry = (unsigned long)start_secondary;

	/*
	 * Send IPI to start the CPU
	 * This is platform-specific - would typically write to
	 * a mailbox or use firmware call
	 */
	extern void pal_unix_wripir(unsigned long);
	pal_unix_wripir(cpu_id);

	/*
	 * Wait for CPU to come online (with timeout)
	 */
	int timeout = 1000000;
	while (!(cpu_started_map & (1UL << cpu_id)) && timeout-- > 0) {
		/* Busy wait */
	}

	if (timeout <= 0)
		return KERN_FAILURE;

	/*
	 * Mark CPU as online
	 */
	cpu_online_map |= (1UL << cpu_id);

	return KERN_SUCCESS;
}

/*
 * Secondary CPU entry point (called from start.s)
 */
void
slave_main(int cpu_id)
{
	/*
	 * Mark this CPU as started
	 */
	cpu_started_map |= (1UL << cpu_id);

	/*
	 * Initialize this CPU
	 */
	alpha_cpu_init_slave(cpu_id);

	/*
	 * Enable interrupts at low priority
	 */
	extern unsigned long pal_unix_swpipl(unsigned long);
	pal_unix_swpipl(0);

	/*
	 * Enter the scheduler
	 */
	/* cpu_launch_first_thread(current_thread()); */

	/*
	 * Should not return
	 */
	for (;;) {
		/* Halt if we somehow get here */
		extern void pal_unix_halt(void);
		pal_unix_halt();
	}
}

/*
 * Initialize secondary CPU
 */
void
alpha_cpu_init_slave(int cpu_id)
{
	extern void alpha_pmap_bootstrap(unsigned long);

	/*
	 * Initialize virtual memory for this CPU
	 * (share page tables with boot CPU)
	 */
	alpha_pmap_bootstrap(0);

	/*
	 * Initialize FPU
	 */
	extern void fpu_init(void);
	fpu_init();

	/*
	 * Initialize per-CPU data
	 */
	cpu_data[cpu_id].active_thread = 0;
}

/*
 * Send IPI (inter-processor interrupt) to specific CPU
 */
void
alpha_send_ipi(int cpu_id, int ipi_type)
{
	extern void pal_unix_wripir(unsigned long);

	if (cpu_id < 0 || cpu_id >= MAX_CPUS)
		return;

	if (!(cpu_online_map & (1UL << cpu_id)))
		return;

	/*
	 * For UNIX PALcode, use WRIPIR
	 * The IPI type would be encoded in platform-specific way
	 */
	pal_unix_wripir(cpu_id);
}

/*
 * Broadcast IPI to all CPUs
 */
void
alpha_broadcast_ipi(int ipi_type)
{
	int i;
	int my_cpu = cpu_number();

	for (i = 0; i < MAX_CPUS; i++) {
		if (i != my_cpu && (cpu_online_map & (1UL << i))) {
			alpha_send_ipi(i, ipi_type);
		}
	}
}

/*
 * TLB shootdown for SMP
 */
void
alpha_tlb_shootdown(vm_offset_t va)
{
	extern void pal_unix_tbi(unsigned long, unsigned long);

	/*
	 * Invalidate on local CPU
	 */
	pal_unix_tbi(2, va);  /* Type 2: invalidate single VA */

	/*
	 * Send IPI to other CPUs to invalidate their TLBs
	 */
#if NCPUS > 1
	alpha_broadcast_ipi(IPI_TLB_FLUSH);
#endif
}

/*
 * Get CPU data for specific CPU
 */
struct alpha_cpu_data *
alpha_get_cpu_data(int cpu_id)
{
	if (cpu_id < 0 || cpu_id >= MAX_CPUS)
		return NULL;

	return &cpu_data[cpu_id];
}

/*
 * Get number of online CPUs
 */
int
alpha_cpu_count(void)
{
	int count = 0;
	int i;

	for (i = 0; i < MAX_CPUS; i++) {
		if (cpu_online_map & (1UL << i))
			count++;
	}

	return count;
}

/*
 * Check if CPU is online
 */
boolean_t
alpha_cpu_is_online(int cpu_id)
{
	if (cpu_id < 0 || cpu_id >= MAX_CPUS)
		return FALSE;

	return (cpu_online_map & (1UL << cpu_id)) != 0;
}

/* IPI types */
#define IPI_TLB_FLUSH		1
#define IPI_RESCHEDULE		2
#define IPI_CALL_FUNCTION	3
#define IPI_STOP		4

/* Stub functions for linking */
unsigned long kvtophys(unsigned long va) { return va; }
void *kalloc(vm_size_t size) { return NULL; }
