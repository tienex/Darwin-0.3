/* memory.c - DLX Memory Subsystem
 * Copyright (C) 1999 Apple Computer, Inc.
 */

#include "dlx.h"

/* Check if address is memory-mapped I/O */
static int is_mmio(uint32_t addr) {
    return (addr >= 0xFFF00000);
}

/* Read word from memory */
uint32_t dlx_mem_read_word(dlx_sim_t *sim, uint32_t addr) {
    uint32_t value;

    /* Check for memory-mapped I/O */
    if (is_mmio(addr)) {
        return dlx_device_read(sim, addr);
    }

    /* Check alignment */
    if (addr & 0x3) {
        fprintf(stderr, "Unaligned word read at 0x%08x\n", addr);
        dlx_exception(sim, EXC_ADEL);
        return 0;
    }

    /* Check bounds */
    if (addr >= sim->memory.size) {
        fprintf(stderr, "Read out of bounds: 0x%08x\n", addr);
        dlx_exception(sim, EXC_DBE);
        return 0;
    }

    /* Read big-endian */
    value = ((uint32_t)sim->memory.mem[addr] << 24) |
            ((uint32_t)sim->memory.mem[addr + 1] << 16) |
            ((uint32_t)sim->memory.mem[addr + 2] << 8) |
            ((uint32_t)sim->memory.mem[addr + 3]);

    return value;
}

/* Read halfword from memory */
uint16_t dlx_mem_read_half(dlx_sim_t *sim, uint32_t addr) {
    uint16_t value;

    /* Check for memory-mapped I/O */
    if (is_mmio(addr)) {
        return (uint16_t)dlx_device_read(sim, addr);
    }

    /* Check alignment */
    if (addr & 0x1) {
        fprintf(stderr, "Unaligned halfword read at 0x%08x\n", addr);
        dlx_exception(sim, EXC_ADEL);
        return 0;
    }

    /* Check bounds */
    if (addr >= sim->memory.size) {
        fprintf(stderr, "Read out of bounds: 0x%08x\n", addr);
        dlx_exception(sim, EXC_DBE);
        return 0;
    }

    /* Read big-endian */
    value = ((uint16_t)sim->memory.mem[addr] << 8) |
            ((uint16_t)sim->memory.mem[addr + 1]);

    return value;
}

/* Read byte from memory */
uint8_t dlx_mem_read_byte(dlx_sim_t *sim, uint32_t addr) {
    /* Check for memory-mapped I/O */
    if (is_mmio(addr)) {
        return (uint8_t)dlx_device_read(sim, addr);
    }

    /* Check bounds */
    if (addr >= sim->memory.size) {
        fprintf(stderr, "Read out of bounds: 0x%08x\n", addr);
        dlx_exception(sim, EXC_DBE);
        return 0;
    }

    return sim->memory.mem[addr];
}

/* Write word to memory */
void dlx_mem_write_word(dlx_sim_t *sim, uint32_t addr, uint32_t value) {
    /* Check for memory-mapped I/O */
    if (is_mmio(addr)) {
        dlx_device_write(sim, addr, value);
        return;
    }

    /* Check alignment */
    if (addr & 0x3) {
        fprintf(stderr, "Unaligned word write at 0x%08x\n", addr);
        dlx_exception(sim, EXC_ADES);
        return;
    }

    /* Check bounds */
    if (addr >= sim->memory.size) {
        fprintf(stderr, "Write out of bounds: 0x%08x\n", addr);
        dlx_exception(sim, EXC_DBE);
        return;
    }

    /* Write big-endian */
    sim->memory.mem[addr] = (value >> 24) & 0xFF;
    sim->memory.mem[addr + 1] = (value >> 16) & 0xFF;
    sim->memory.mem[addr + 2] = (value >> 8) & 0xFF;
    sim->memory.mem[addr + 3] = value & 0xFF;
}

/* Write halfword to memory */
void dlx_mem_write_half(dlx_sim_t *sim, uint32_t addr, uint16_t value) {
    /* Check for memory-mapped I/O */
    if (is_mmio(addr)) {
        dlx_device_write(sim, addr, value);
        return;
    }

    /* Check alignment */
    if (addr & 0x1) {
        fprintf(stderr, "Unaligned halfword write at 0x%08x\n", addr);
        dlx_exception(sim, EXC_ADES);
        return;
    }

    /* Check bounds */
    if (addr >= sim->memory.size) {
        fprintf(stderr, "Write out of bounds: 0x%08x\n", addr);
        dlx_exception(sim, EXC_DBE);
        return;
    }

    /* Write big-endian */
    sim->memory.mem[addr] = (value >> 8) & 0xFF;
    sim->memory.mem[addr + 1] = value & 0xFF;
}

/* Write byte to memory */
void dlx_mem_write_byte(dlx_sim_t *sim, uint32_t addr, uint8_t value) {
    /* Check for memory-mapped I/O */
    if (is_mmio(addr)) {
        dlx_device_write(sim, addr, value);
        return;
    }

    /* Check bounds */
    if (addr >= sim->memory.size) {
        fprintf(stderr, "Write out of bounds: 0x%08x\n", addr);
        dlx_exception(sim, EXC_DBE);
        return;
    }

    sim->memory.mem[addr] = value;
}

/* Load binary file into memory */
int dlx_load_binary(dlx_sim_t *sim, const char *filename, uint32_t addr) {
    FILE *f;
    size_t len;

    f = fopen(filename, "rb");
    if (!f) {
        perror("fopen");
        return -1;
    }

    /* Get file size */
    fseek(f, 0, SEEK_END);
    len = ftell(f);
    fseek(f, 0, SEEK_SET);

    /* Check if it fits */
    if (addr + len > sim->memory.size) {
        fprintf(stderr, "Binary too large: %zu bytes at 0x%x\n", len, addr);
        fclose(f);
        return -1;
    }

    /* Read directly into memory */
    if (fread(&sim->memory.mem[addr], 1, len, f) != len) {
        perror("fread");
        fclose(f);
        return -1;
    }

    fclose(f);
    return len;
}

/* MMU address translation */
uint32_t dlx_mmu_translate(dlx_sim_t *sim, uint32_t vaddr, int write) {
    /* For now, use identity mapping (no translation) */
    /* Real implementation would check TLB and page tables */
    return vaddr;
}

/* Exception handling */
void dlx_exception(dlx_sim_t *sim, int code) {
    dlx_cpu_t *cpu = &sim->cpu;

    /* Set exception code */
    cpu->cause = (code << 2);

    /* Save PC */
    cpu->epc = cpu->pc - 4;

    /* Update status register (shift stack) */
    cpu->status = (cpu->status & ~0x3F) | ((cpu->status << 2) & 0x3C);

    /* Jump to exception vector */
    cpu->pc = 0x00000180;  /* Exception vector address */

    if (sim->verbose) {
        fprintf(stderr, "Exception %d at PC=0x%08x\n", code, cpu->epc);
    }

    /* For some exceptions, stop simulation */
    if (code == EXC_RI || code == EXC_DBE || code == EXC_IBE) {
        cpu->running = 0;
    }
}
