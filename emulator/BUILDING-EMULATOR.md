# Building an MMIX Emulator for Darwin

## Overview

This guide describes how to build or adapt an MMIX emulator to run Darwin-0.3.

## Option 1: Adapt Existing MMIX Simulator

### MMIXware (Reference Implementation)

Donald Knuth's MMIXware package includes `mmix-sim`, a complete MMIX simulator.

**Source**: http://mmix.cs.hm.edu/

**Modifications Needed**:

1. **Add Memory-Mapped Devices**:
   ```c
   /* Add to mmix-sim.c */

   /* Console device at 0xFFFFFFFF00000000 */
   if (addr >= 0xFFFFFFFF00000000ULL && addr < 0xFFFFFFFF00001000ULL) {
       return console_read(addr);
   }

   /* Disk device at 0xFFFFFFFF00001000 */
   if (addr >= 0xFFFFFFFF00001000ULL && addr < 0xFFFFFFFF00003000ULL) {
       return disk_read(addr);
   }
   ```

2. **Add Device Implementations**:
   ```c
   /* console.c */
   octa console_read(octa addr) {
       if (addr == CONSOLE_IN) {
           int c = getchar();
           return (c == EOF) ? 0 : c;
       }
       return 0;
   }

   void console_write(octa addr, octa value) {
       if (addr == CONSOLE_OUT) {
           putchar(value & 0xFF);
           fflush(stdout);
       }
   }

   /* disk.c */
   octa disk_read(octa addr) {
       /* Implement disk sector I/O */
   }
   ```

3. **Build**:
   ```bash
   cd mmixware
   # Apply patches
   patch -p1 < darwin-mmix.patch
   make
   ```

### QEMU (Advanced Option)

Add MMIX target to QEMU for full system emulation.

**Advantages**:
- Full device support
- Debugging tools
- Mature codebase

**Disadvantages**:
- Complex
- Requires significant development

**Steps**:
1. Study existing QEMU targets (e.g., target/riscv64)
2. Implement MMIX CPU in target/mmix/
3. Add MMIX machine type
4. Implement Darwin-specific devices

## Option 2: Build Custom Emulator

### Minimal Implementation

Create a focused emulator for Darwin boot testing.

**Structure**:
```
mmix-darwin-emu/
├── src/
│   ├── cpu.c/h          # CPU core
│   ├── memory.c/h       # Memory management
│   ├── devices/
│   │   ├── console.c/h
│   │   ├── disk.c/h
│   │   └── timer.c/h
│   ├── main.c           # Entry point
│   └── debug.c/h        # Debugging support
├── Makefile
└── README.md
```

**CPU Core** (cpu.c):
```c
typedef struct {
    uint64_t g[256];      /* General registers $0-$255 */
    uint64_t s[32];       /* Special registers */
    uint64_t pc;          /* Program counter */
    bool running;
} mmix_cpu;

void cpu_reset(mmix_cpu *cpu);
void cpu_step(mmix_cpu *cpu);
uint32_t fetch_instruction(mmix_cpu *cpu);
void execute_instruction(mmix_cpu *cpu, uint32_t inst);
```

**Memory System** (memory.c):
```c
typedef struct {
    uint8_t *ram;
    size_t ram_size;
    device_t *devices;
} memory_t;

uint64_t mem_read(memory_t *mem, uint64_t addr, int size);
void mem_write(memory_t *mem, uint64_t addr, uint64_t value, int size);
```

**Main Loop** (main.c):
```c
int main(int argc, char **argv) {
    mmix_cpu cpu;
    memory_t mem;

    /* Initialize */
    cpu_reset(&cpu);
    memory_init(&mem, 256 * 1024 * 1024);  /* 256 MB */
    load_bootloader(&mem, argv[1]);
    attach_devices(&mem);

    /* Set boot parameters */
    cpu.g[0] = mem.ram_size;
    cpu.g[255] = 0x4D4D4958;  /* 'MMIX' */
    cpu.pc = 0x1000;

    /* Run */
    cpu.running = true;
    while (cpu.running) {
        cpu_step(&cpu);
    }

    return 0;
}
```

**Build**:
```bash
gcc -O2 -Wall -o mmix-emu src/*.c src/devices/*.c
```

## Device Implementation Details

### Console Device

```c
typedef struct {
    uint8_t input_buffer[256];
    int input_head, input_tail;
    bool input_ready;
} console_device;

void console_init(console_device *dev);

uint64_t console_read(console_device *dev, uint64_t offset) {
    switch (offset) {
    case 0x00:  /* Output register - write-only */
        return 0;
    case 0x08:  /* Input register */
        if (dev->input_head != dev->input_tail) {
            uint8_t c = dev->input_buffer[dev->input_tail];
            dev->input_tail = (dev->input_tail + 1) % 256;
            return c;
        }
        return 0;
    case 0x10:  /* Status register */
        return dev->input_head != dev->input_tail ? 1 : 0;
    }
    return 0;
}

void console_write(console_device *dev, uint64_t offset, uint64_t value) {
    if (offset == 0x00) {
        putchar(value & 0xFF);
        fflush(stdout);
    }
}
```

### Disk Device

```c
typedef struct {
    FILE *image;
    uint64_t sector;
    uint8_t buffer[8192];
    int status;  /* 0=ready, 1=busy, 2=error */
} disk_device;

void disk_init(disk_device *dev, const char *image_path);

void disk_write(disk_device *dev, uint64_t offset, uint64_t value) {
    switch (offset) {
    case 0x0000:  /* Sector number */
        dev->sector = value;
        break;
    case 0x0008 ... 0x2007:  /* Buffer */
        dev->buffer[offset - 0x0008] = value & 0xFF;
        break;
    case 0x2008:  /* Command */
        if (value == 1) {  /* Read */
            fseek(dev->image, dev->sector * 8192, SEEK_SET);
            fread(dev->buffer, 1, 8192, dev->image);
            dev->status = 0;
        } else if (value == 2) {  /* Write */
            fseek(dev->image, dev->sector * 8192, SEEK_SET);
            fwrite(dev->buffer, 1, 8192, dev->image);
            fflush(dev->image);
            dev->status = 0;
        }
        break;
    }
}
```

## Testing

### Test Bootloader

Create minimal test bootloader:
```mmixal
        LOC #1000
Main    GETA $255,Msg
        LDB  $0,$255,0
Loop    BZ   $0,Done
        STCO $0,Console,0
        INCL $255,1
        LDB  $0,$255,0
        JMP  Loop
Done    TRAP 0,0,0

Msg     BYTE "Hello, Darwin!",#0a,0
Console IS   #FFFFFFFF00000000
```

Assemble and run:
```bash
mmixal -o test.mmo test.mms
mmix-emu test.mmo
```

Expected output:
```
Hello, Darwin!
```

### Test Disk I/O

```c
/* Write test pattern */
for (int i = 0; i < 8192; i++) {
    disk_write(&disk, 0x0008 + i, i & 0xFF);
}
disk_write(&disk, 0x0000, 0);  /* Sector 0 */
disk_write(&disk, 0x2008, 2);  /* Write command */

/* Read back */
disk_write(&disk, 0x0000, 0);  /* Sector 0 */
disk_write(&disk, 0x2008, 1);  /* Read command */
for (int i = 0; i < 8192; i++) {
    uint8_t b = disk_read(&disk, 0x0008 + i);
    assert(b == (i & 0xFF));
}
```

## Debugging Features

### Instruction Trace

```c
void cpu_step(mmix_cpu *cpu) {
    uint32_t inst = fetch_instruction(cpu);

    if (trace_enabled) {
        printf("[0x%016llx] ", cpu->pc);
        disassemble(inst);
        printf("\n");
    }

    execute_instruction(cpu, inst);
}
```

### GDB Remote Protocol

Implement basic GDB stub:
```c
void gdb_server(mmix_cpu *cpu, int port) {
    /* Listen on port */
    /* Handle GDB commands: */
    /* - 'g' (read registers) */
    /* - 'G' (write registers) */
    /* - 'm' (read memory) */
    /* - 'M' (write memory) */
    /* - 'c' (continue) */
    /* - 's' (single step) */
}
```

Connect with:
```bash
gdb
(gdb) target remote localhost:1234
(gdb) break *0x1000
(gdb) continue
```

## Performance Optimization

### JIT Compilation

For better performance, consider:
- LLVM-based JIT
- Threaded interpreter
- Block compilation

### Caching

- Implement TLB cache
- Cache decoded instructions
- Use mmap for large disk images

## Resources

- **MMIXware**: http://mmix.cs.hm.edu/
- **MMIX Documentation**: http://mmix.cs.hm.edu/doc/
- **QEMU**: https://www.qemu.org/
- **Darwin Source**: https://opensource.apple.com/

## Support

For questions about MMIX emulator development for Darwin:
- See MMIX-EMULATOR-SPEC.md
- Check existing MMIX simulators
- Consult MMIX architecture documentation
