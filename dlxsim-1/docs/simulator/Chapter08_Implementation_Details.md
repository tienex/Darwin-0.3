# Chapter 8: Implementation Details

## 8.1 Source Files

- **src/dlx.h** (365 lines) - Architecture definitions and data structures
- **src/dlxsim.c** (487 lines) - Main simulator and instruction execution
- **src/memory.c** (187 lines) - Memory subsystem and MMU
- **src/device.c** (68 lines) - I/O device simulation
- **Makefile** (65 lines) - Build system

Total: ~1,172 lines of C code

## 8.2 Key Data Structures

```c
typedef struct {
    uint32_t regs[32];      // General registers
    uint32_t fregs[32];     // FP registers
    uint32_t pc;            // Program counter
    uint32_t status;        // Status register
    uint32_t cause;         // Cause register
    uint32_t epc;           // Exception PC
    uint32_t hi, lo;        // Mult/div results
    int running;            // Running flag
    uint64_t cycles;        // Cycle count
} dlx_cpu_t;
```

## 8.3 Execution Loop

```c
void dlx_run(dlx_sim_t *sim) {
    while (sim->cpu.running) {
        uint32_t instr = dlx_mem_read_word(sim, sim->cpu.pc);
        sim->cpu.pc += 4;
        dlx_execute_instruction(sim, instr);
        sim->cpu.cycles++;
    }
}
```

## 8.4 Memory Subsystem

- **16MB simulated memory**
- **Big-endian byte order**
- **Memory-mapped I/O**
- **TLB simulation** (basic)

## 8.5 Instruction Decoder

The simulator uses a switch-case instruction decoder based on opcode:

```c
switch (opcode) {
    case OP_ADD:
        /* Execute ADD */
        break;
    case OP_LW:
        /* Execute LW */
        break;
    /* ... */
}
```
