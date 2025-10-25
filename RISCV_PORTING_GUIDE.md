# RISC-V Hardware Porting Guide for Darwin-0.3

This guide provides detailed instructions for porting Darwin-0.3 to RISC-V hardware platforms.

## Table of Contents

1. [Prerequisites](#prerequisites)
2. [Hardware Requirements](#hardware-requirements)
3. [Memory Map Configuration](#memory-map-configuration)
4. [Device Tree Integration](#device-tree-integration)
5. [Platform-Specific Customization](#platform-specific-customization)
6. [Bootloader Integration](#bootloader-integration)
7. [Testing and Debugging](#testing-and-debugging)

## Prerequisites

### Required Tools

```bash
# RISC-V GCC Toolchain
riscv64-unknown-elf-gcc --version
riscv64-unknown-elf-ld --version
riscv64-unknown-elf-objcopy --version
riscv64-unknown-elf-objdump --version

# Optional but recommended
riscv64-unknown-elf-gdb --version
spike --version  # RISC-V ISA simulator
qemu-system-riscv64 --version
```

### Building the Toolchain

If you need to build the RISC-V toolchain:

```bash
git clone https://github.com/riscv/riscv-gnu-toolchain
cd riscv-gnu-toolchain
./configure --prefix=/opt/riscv --with-arch=rv64g --with-abi=lp64d
make -j$(nproc)
export PATH=/opt/riscv/bin:$PATH
```

## Hardware Requirements

### Minimum Requirements

- **ISA**: RV64G (RV64IMAFD) or RV32G (RV32IMAFD)
- **Privilege Modes**: Machine (M) and Supervisor (S) modes
- **Memory**: 64MB RAM minimum, 256MB recommended
- **Timer**: Machine timer (mtime/mtimecmp)
- **Serial**: UART for console (16550 compatible recommended)

### Recommended Features

- SBI (Supervisor Binary Interface) support
- PLIC (Platform-Level Interrupt Controller)
- CLINT (Core-Local Interruptor)
- MMU with Sv39 (RV64) or Sv32 (RV32) page tables

### Tested Platforms

This port has been designed with the following platforms in mind:

1. **SiFive HiFive Unleashed** (FU540)
   - RV64GC, 8GB RAM
   - SiFive UART
   - SiFive PLIC

2. **SiFive HiFive Unmatched** (FU740)
   - RV64GC, 16GB RAM
   - Similar to Unleashed

3. **QEMU virt machine**
   - Configurable RAM
   - VirtIO devices
   - Good for testing

## Memory Map Configuration

### Default Memory Layout (kernel-7/mach/riscv/vm_param.h)

**RV64 Layout:**
```
0x0000000000000000 - 0x0000004000000000  User space (256GB)
0xFFFFFFE000000000 - 0xFFFFFFFF80000000  Kernel space
```

**RV32 Layout:**
```
0x00000000 - 0xC0000000  User space (3GB)
0xC0000000 - 0xFFFFFFFF  Kernel space (1GB)
```

### Customizing for Your Platform

Edit `kernel-7/machdep/riscv/riscv_init.c`:

```c
void _riscv_init(void)
{
    /* Your platform initialization */

    /* Set memory size based on device tree or hardware probing */
    machine_info.memory_size = detect_memory_size();

    /* Initialize platform-specific devices */
    platform_early_init();

    /* Bootstrap VM with your load address */
    pmap_bootstrap(KERNEL_LOAD_ADDRESS);

    /* Continue to generic startup */
    machine_startup();
}
```

### Platform Memory Detection

Create `kernel-7/machdep/riscv/platform_memory.c`:

```c
#include <sys/param.h>

/* Example for SiFive platforms */
#define DRAM_BASE    0x80000000
#define DRAM_SIZE    (2UL * 1024 * 1024 * 1024)  /* 2GB */

vm_size_t detect_memory_size(void)
{
    /* Option 1: Read from device tree */
    /* Option 2: Hardware registers */
    /* Option 3: SBI firmware call */

    return DRAM_SIZE;
}
```

## Device Tree Integration

### Reading Device Tree

The bootloader (OpenSBI, U-Boot) passes device tree in `a1` register.

Modify `kernel-7/machdep/riscv/start.s`:

```asm
_start:
    /* a0 = hartid, a1 = device tree pointer */
    mv	s0, a0		/* Save hartid */
    mv	s1, a1		/* Save device tree pointer */

    /* Set up stack */
    la	sp, _stack_top

    /* Pass device tree to init */
    mv	a0, s1
    call	_riscv_init_with_dtb

    /* ... */
```

### Parsing Device Tree

Add FDT (Flattened Device Tree) support:

```c
#include <libfdt.h>

void _riscv_init_with_dtb(void *fdt)
{
    int node, len;
    const void *prop;

    /* Validate device tree */
    if (fdt_check_header(fdt) != 0) {
        panic("Invalid device tree");
    }

    /* Find memory node */
    node = fdt_path_offset(fdt, "/memory");
    if (node >= 0) {
        prop = fdt_getprop(fdt, node, "reg", &len);
        /* Parse memory regions */
    }

    /* Find UART */
    node = fdt_path_offset(fdt, "/soc/serial@10000000");
    if (node >= 0) {
        /* Initialize UART with base address from DT */
    }

    /* Continue normal initialization */
    _riscv_init();
}
```

## Platform-Specific Customization

### Creating Platform Directory

```bash
mkdir -p kernel-7/machdep/riscv/platforms/sifive-fu540
```

### Platform Configuration Header

`kernel-7/machdep/riscv/platforms/sifive-fu540/platform.h`:

```c
#ifndef _PLATFORM_SIFIVE_FU540_H_
#define _PLATFORM_SIFIVE_FU540_H_

/* Memory */
#define PLATFORM_DRAM_BASE      0x80000000
#define PLATFORM_DRAM_SIZE      0x200000000  /* 8GB */

/* UART */
#define PLATFORM_UART_BASE      0x10010000
#define PLATFORM_UART_CLOCK     (32 * 1000 * 1000)  /* 32MHz */

/* PLIC */
#define PLATFORM_PLIC_BASE      0x0C000000
#define PLATFORM_PLIC_SIZE      0x04000000

/* CLINT */
#define PLATFORM_CLINT_BASE     0x02000000
#define PLATFORM_CLINT_SIZE     0x00010000

/* Number of CPUs */
#define PLATFORM_MAX_CPUS       5

#endif
```

### UART Driver Implementation

`kernel-7/machdep/riscv/platforms/sifive-fu540/uart.c`:

```c
#include <sys/param.h>
#include "platform.h"

#define UART_REG(offset)  (*(volatile uint32_t *)(PLATFORM_UART_BASE + offset))

#define UART_TXDATA       0x00
#define UART_RXDATA       0x04
#define UART_TXCTRL       0x08
#define UART_RXCTRL       0x0C
#define UART_IE           0x10
#define UART_IP           0x14
#define UART_DIV          0x18

void sifive_uart_init(void)
{
    /* Set baud rate divisor for 115200 */
    uint32_t divisor = PLATFORM_UART_CLOCK / 115200;
    UART_REG(UART_DIV) = divisor;

    /* Enable TX and RX */
    UART_REG(UART_TXCTRL) = 0x1;
    UART_REG(UART_RXCTRL) = 0x1;
}

void sifive_uart_putc(char c)
{
    /* Wait for TX FIFO to be ready */
    while (UART_REG(UART_TXDATA) & 0x80000000)
        ;

    UART_REG(UART_TXDATA) = c;
}

int sifive_uart_getc(void)
{
    uint32_t data = UART_REG(UART_RXDATA);

    if (data & 0x80000000)
        return -1;  /* No data available */

    return (int)(data & 0xFF);
}
```

### Interrupt Controller (PLIC)

`kernel-7/machdep/riscv/platforms/sifive-fu540/plic.c`:

```c
#include "platform.h"

#define PLIC_PRIORITY(n)      (PLATFORM_PLIC_BASE + 4 * (n))
#define PLIC_ENABLE(hart)     (PLATFORM_PLIC_BASE + 0x2000 + 0x80 * (hart))
#define PLIC_THRESHOLD(hart)  (PLATFORM_PLIC_BASE + 0x200000 + 0x1000 * (hart))
#define PLIC_CLAIM(hart)      (PLATFORM_PLIC_BASE + 0x200004 + 0x1000 * (hart))

void plic_init(void)
{
    int i;

    /* Set all priorities to 1 */
    for (i = 1; i <= 53; i++) {
        *(volatile uint32_t *)PLIC_PRIORITY(i) = 1;
    }

    /* Set threshold to 0 (accept all) for hart 0 */
    *(volatile uint32_t *)PLIC_THRESHOLD(0) = 0;
}

void plic_enable_irq(int irq, int hart)
{
    uint32_t *enable = (uint32_t *)PLIC_ENABLE(hart);
    enable[irq / 32] |= (1 << (irq % 32));
}

int plic_claim(int hart)
{
    return *(volatile uint32_t *)PLIC_CLAIM(hart);
}

void plic_complete(int hart, int irq)
{
    *(volatile uint32_t *)PLIC_CLAIM(hart) = irq;
}
```

### Timer Support (CLINT)

`kernel-7/machdep/riscv/platforms/sifive-fu540/clint.c`:

```c
#include "platform.h"

#define CLINT_MTIMECMP(hart)  (PLATFORM_CLINT_BASE + 0x4000 + 8 * (hart))
#define CLINT_MTIME           (PLATFORM_CLINT_BASE + 0xBFF8)

#define TIMER_FREQ            1000000  /* 1MHz */

void clint_set_timer(int hart, uint64_t time)
{
    *(volatile uint64_t *)CLINT_MTIMECMP(hart) = time;
}

uint64_t clint_get_time(void)
{
    return *(volatile uint64_t *)CLINT_MTIME;
}

void clint_start_timer(int hart, uint64_t interval_us)
{
    uint64_t now = clint_get_time();
    uint64_t then = now + (interval_us * TIMER_FREQ / 1000000);
    clint_set_timer(hart, then);
}
```

## Bootloader Integration

### OpenSBI Integration

Darwin expects to be loaded in S-mode by OpenSBI or similar firmware.

**Required OpenSBI Configuration:**

```c
/* platform/generic/config.mk or platform/sifive/fu540/config.mk */
PLATFORM_RISCV_XLEN = 64
PLATFORM_RISCV_ABI = lp64d
PLATFORM_RISCV_ISA = rv64imafd
```

**Boot Flow:**
```
ROM → OpenSBI (M-mode) → Darwin Kernel (S-mode)
```

### U-Boot Integration

Alternatively, use U-Boot:

```bash
# U-Boot commands to boot Darwin
setenv bootargs "console=uart8250,mmio,0x10000000,115200n8"
load mmc 0:1 0x80200000 mach_kernel
go 0x80200000
```

### Creating Bootable Image

```bash
# Build kernel
cd kernel-7/conf
make ARCH=RISCV TYPE=RELEASE mach_kernel.kernel

# Create bootable image
riscv64-unknown-elf-objcopy -O binary \
    RELEASE_RISCV/mach_kernel.kernel \
    mach_kernel.bin

# Combine with OpenSBI (if needed)
cat fw_jump.bin mach_kernel.bin > boot.bin
```

### Kernel Linker Script

Create `kernel-7/machdep/riscv/riscv.ld`:

```ld
OUTPUT_ARCH(riscv)
ENTRY(_start)

SECTIONS
{
    . = 0x80000000;  /* Kernel load address */

    .text : {
        *(.text.start)
        *(.text*)
    }

    .rodata : {
        *(.rodata*)
    }

    .data : {
        *(.data*)
    }

    .bss : {
        _bss_start = .;
        *(.bss*)
        *(COMMON)
        _bss_end = .;
    }

    _kernel_end = .;
}
```

## Testing and Debugging

### QEMU Testing

```bash
# Build QEMU if needed
git clone https://github.com/qemu/qemu
cd qemu
./configure --target-list=riscv64-softmmu
make -j$(nproc)

# Run Darwin kernel in QEMU
qemu-system-riscv64 \
    -machine virt \
    -cpu rv64 \
    -m 2G \
    -bios none \
    -kernel mach_kernel.bin \
    -serial stdio \
    -nographic
```

### Debugging with GDB

```bash
# Terminal 1: Start QEMU with GDB server
qemu-system-riscv64 \
    -machine virt \
    -cpu rv64 \
    -m 2G \
    -kernel mach_kernel.bin \
    -serial stdio \
    -nographic \
    -s -S

# Terminal 2: Connect GDB
riscv64-unknown-elf-gdb mach_kernel.kernel
(gdb) target remote :1234
(gdb) break _start
(gdb) continue
```

### Adding Debug Output

Modify `kernel-7/machdep/riscv/riscv_init.c`:

```c
void _riscv_init(void)
{
    /* Early UART initialization */
    platform_uart_init();

    printf("Darwin/RISC-V booting...\n");
    printf("Hart ID: %lu\n", read_csr(mhartid));
    printf("ISA: 0x%lx\n", read_csr(misa));

    /* Print memory info */
    printf("Memory: base=0x%lx size=%luMB\n",
           DRAM_BASE, DRAM_SIZE / (1024*1024));

    /* ... rest of initialization ... */
}
```

### Common Issues and Solutions

**Issue: Kernel doesn't boot**
- Check that OpenSBI is loading kernel at correct address
- Verify linker script matches load address
- Ensure UART is initialized for debug output

**Issue: Page faults immediately**
- Verify memory map configuration
- Check that VM_MIN_KERNEL_ADDRESS is correct
- Ensure BSS is cleared before C code runs

**Issue: No serial output**
- Verify UART base address matches hardware
- Check baud rate divisor calculation
- Ensure UART clock frequency is correct

**Issue: Timer interrupts not working**
- Check SIE (Supervisor Interrupt Enable) bit
- Verify CLINT/PLIC configuration
- Ensure timer compare register is set correctly

## Platform Checklist

Before deploying to new hardware:

- [ ] Memory map configured correctly
- [ ] UART driver tested and working
- [ ] Interrupt controller initialized
- [ ] Timer/clock support implemented
- [ ] Device tree parsing (if applicable)
- [ ] Boot loader integration tested
- [ ] SMP support (for multi-core platforms)
- [ ] Platform-specific drivers added
- [ ] Performance tuning completed

## Example Configurations

### Configuration for QEMU virt

`kernel-7/conf/RELEASE.RISCV.QEMU`:
```
include MASTER.riscv
ident QEMU_VIRT
options PLATFORM_QEMU
```

### Configuration for SiFive HiFive Unleashed

`kernel-7/conf/RELEASE.RISCV.FU540`:
```
include MASTER.riscv
ident SIFIVE_FU540
options PLATFORM_SIFIVE_FU540
options SMP
```

## Performance Optimization

### Cache Configuration

```c
/* In riscv_init.c */
void configure_caches(void)
{
    /* Enable I-cache and D-cache if available */
    /* Platform-specific implementation */
}
```

### TLB Tuning

```c
/* In pmap.c */
void pmap_tlb_flush_range(vm_offset_t start, vm_offset_t end)
{
    /* Use sfence.vma with address range for efficiency */
    for (vm_offset_t addr = start; addr < end; addr += PAGE_SIZE) {
        __asm__ volatile("sfence.vma %0, zero" : : "r"(addr));
    }
}
```

## Resources

- **RISC-V Specifications**: https://riscv.org/technical/specifications/
- **OpenSBI Documentation**: https://github.com/riscv/opensbi/blob/master/docs/
- **SiFive Documentation**: https://www.sifive.com/documentation
- **QEMU RISC-V**: https://www.qemu.org/docs/master/system/target-riscv.html

## Support

For questions about this port:
- Review RISCV_SUPPORT.md in the repository root
- Check the Darwin kernel documentation
- Consult RISC-V privilege specification

---

**Last Updated**: Based on Darwin-0.3 RISC-V implementation commits 6e70eb00-eca7df84
