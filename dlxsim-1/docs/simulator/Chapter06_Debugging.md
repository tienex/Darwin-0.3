# Chapter 6: Debugging

## 6.1 Instruction Tracing

Use the `-t` flag to see every instruction as it executes:

```bash
$ ./dlxsim -t -b test.bin
PC=00000000: addi r1,r0,#42
PC=00000004: sw r1,0(r0)
PC=00000008: trap #0
```

## 6.2 Register Inspection

With `-v`, registers are dumped at the end:

```
DLX Registers:
  r0=00000000  r1=0000002A  r2=00000000  r3=00000000
  ...
```

## 6.3 Memory Inspection

Modify `dlxsim.c` to add memory dump functionality:

```c
void dlx_dump_memory(dlx_sim_t *sim, uint32_t addr, uint32_t len) {
    for (uint32_t i = 0; i < len; i += 16) {
        printf("%08x: ", addr + i);
        for (int j = 0; j < 16 && i + j < len; j++) {
            printf("%02x ", sim->memory.mem[addr + i + j]);
        }
        printf("\n");
    }
}
```

## 6.4 Breakpoints

The simulator does not currently support interactive breakpoints, but you can:
1. Use instruction tracing (`-t`) to see all executed instructions
2. Modify the source code to add breakpoint checking
3. Use a debugger (gdb) on the simulator itself
