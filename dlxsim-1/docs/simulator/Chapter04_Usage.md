# Chapter 4: Usage

## 4.1 Basic Execution

```bash
# Run with a binary
./dlxsim -b program.bin

# Load at specific address
./dlxsim -b program.bin -a 0x1000

# Verbose output
./dlxsim -v -b program.bin

# Instruction tracing
./dlxsim -t -b program.bin
```

## 4.2 Command-Line Options

- `-v` - Verbose output (show cycles, final state)
- `-t` - Trace instructions (disassemble and show each instruction)
- `-b <file>` - Load binary file into memory
- `-a <addr>` - Load address (hexadecimal, default 0x0)
- `-h` - Show help message

## 4.3 Example Session

```bash
$ ./dlxsim -v -b test.bin
Starting DLX simulator
Memory: 16777216 bytes
Loaded test.bin at 0x00000000

Simulation stopped after 1234 cycles

DLX Registers:
  r0=00000000  r1=0000002A  r2=00000000  r3=00000000
  r4=00000000  r5=00000000  r6=00000000  r7=00000000
  ...
  PC  =00000100  Status=00000003  Cause=00000000  EPC=00000000
  HI  =00000000  LO    =00000000
```

## 4.4 Exit Codes

The simulator returns the following exit codes:
- `0` - Normal exit
- `1` - Error loading binary
- `2` - Invalid command-line arguments
- `3` - Simulation error
