# Chapter 9: Performance, Testing, and Integration

## 9.1 Performance

On a modern CPU:
- ~10-50 million DLX instructions per second
- Suitable for kernel development and testing
- Cycle-accurate timing

## 9.2 Testing

### Basic Test

Create a simple test program:

```c
/* test.c - Compile with DLX GCC */
int main() {
    return 42;  // Exit code in r1
}
```

Compile to binary:
```bash
dlx-apple-darwin-gcc -nostdlib -e main -Ttext=0 test.c -o test.elf
dlx-apple-darwin-objcopy -O binary test.elf test.bin
```

Run:
```bash
./dlxsim -v -b test.bin
```

### Extended Test Suite

```bash
# Test arithmetic
./dlxsim -t -b tests/arithmetic.bin

# Test memory
./dlxsim -t -b tests/memory.bin

# Test exceptions
./dlxsim -t -b tests/exceptions.bin
```

## 9.3 Integration with Darwin

DLXSIM integrates with the Darwin-0.3 DLX port:

```
Source Code
    ↓
cc-791 (GCC DLX backend)
    ↓
Assembly Code
    ↓
as (DLX assembler)
    ↓
Object Code
    ↓
ld (DLX linker)
    ↓
Mach-O Executable
    ↓
dlxsim (This simulator) ← Executes the binary
```
