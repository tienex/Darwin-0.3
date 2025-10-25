/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * Alpha Cache Management
 */

#include <mach/mach_types.h>
#include <architecture/alpha/cpu.h>
#include <architecture/alpha/pal.h>

extern alpha_cpu_info_t cpu_info;

/*
 * Flush instruction cache for a range of addresses
 */
void
alpha_icache_flush(vm_offset_t start, vm_offset_t end)
{
	/*
	 * Alpha requires IMB (I-stream memory barrier) after code modification
	 * This ensures instruction cache coherency
	 */
	alpha_imb();
}

/*
 * Flush data cache for a range of addresses
 */
void
alpha_dcache_flush(vm_offset_t start, vm_offset_t end)
{
	/*
	 * Alpha implements cache coherency in hardware
	 * Just need write memory barrier
	 */
	alpha_wmb();
}

/*
 * Flush entire data cache
 */
void
alpha_dcache_flush_all(void)
{
	unsigned long pfn;
	extern void pal_unix_cflush(unsigned long);

	/*
	 * Flush cache by page frame number
	 * This is expensive, only use when absolutely necessary
	 */
	for (pfn = 0; pfn < (cpu_info.dcache_size >> ALPHA_PGSHIFT); pfn++) {
		pal_unix_cflush(pfn);
	}

	alpha_mb();
}

/*
 * Flush entire instruction cache
 */
void
alpha_icache_flush_all(void)
{
	alpha_imb();
}

/*
 * Synchronize caches after DMA
 */
void
alpha_cache_sync_dma(vm_offset_t addr, vm_size_t len, int direction)
{
	/*
	 * Ensure all writes are visible before DMA
	 */
	if (direction == DMA_TO_DEVICE || direction == DMA_BIDIRECTIONAL) {
		alpha_wmb();
	}

	/*
	 * Ensure DMA reads see latest data
	 */
	if (direction == DMA_FROM_DEVICE || direction == DMA_BIDIRECTIONAL) {
		alpha_mb();
	}
}

/*
 * Invalidate cache lines
 */
void
alpha_cache_invalidate(vm_offset_t start, vm_offset_t end)
{
	/*
	 * Alpha hardware maintains cache coherency
	 * Just need memory barrier
	 */
	alpha_mb();
}

/*
 * Clean (write back) cache lines
 */
void
alpha_cache_clean(vm_offset_t start, vm_offset_t end)
{
	alpha_wmb();
}

/*
 * Get cache line size
 */
vm_size_t
alpha_cache_line_size(void)
{
	switch (cpu_info.implementation) {
	case ALPHA_IMPL_EV4:
	case ALPHA_IMPL_EV45:
		return ALPHA_EV4_DCACHE_LINE;

	case ALPHA_IMPL_EV5:
	case ALPHA_IMPL_EV56:
	case ALPHA_IMPL_PCA56:
		return ALPHA_EV5_DCACHE_LINE;

	case ALPHA_IMPL_EV6:
	case ALPHA_IMPL_EV67:
		return ALPHA_EV6_DCACHE_LINE;

	default:
		return 32;  /* Conservative default */
	}
}

/*
 * Prefetch cache line
 */
void
alpha_prefetch(const void *addr)
{
	/*
	 * Use load-locked to prefetch into cache
	 * The value is discarded but the line is cached
	 */
	__asm__ volatile (
		"ldq_l $31, 0(%0)"
		: : "r" (addr)
		: "memory"
	);
}

/*
 * Prefetch for write
 */
void
alpha_prefetch_w(void *addr)
{
	/*
	 * Use load-locked to bring line into cache with exclusive access
	 */
	__asm__ volatile (
		"ldq_l $31, 0(%0)\n\t"
		"wmb"
		: : "r" (addr)
		: "memory"
	);
}

/* DMA direction constants */
#define DMA_TO_DEVICE		1
#define DMA_FROM_DEVICE		2
#define DMA_BIDIRECTIONAL	3
