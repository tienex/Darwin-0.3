# Chapter 7: Architecture Details

## 7.1 Instruction Formats

### R-Type (Register)

```
 31    26 25   21 20   16 15   11 10      0
+--------+-------+-------+-------+----------+
| opcode |  rs1  |  rs2  |  rd   |   func   |
+--------+-------+-------+-------+----------+
    6       5       5       5        11
```

### I-Type (Immediate)

```
 31    26 25   21 20   16 15             0
+--------+-------+-------+----------------+
| opcode |  rs   |  rd   |   immediate    |
+--------+-------+-------+----------------+
    6       5       5          16
```

### J-Type (Jump)

```
 31    26 25                             0
+--------+---------------------------------+
| opcode |          target address         |
+--------+---------------------------------+
    6                 26
```

## 7.2 Exception Vectors

```
0x00000000  Reset vector
0x00000180  General exception vector
```

## 7.3 Status Register

```
Bit  31-8: Interrupt mask
Bit    17: TLB mode enabled
Bit    16: Page table mode enabled
Bits  6-0: Status stack (KUo, IEo, KUp, IEp, KUc, IEc, IE)
```

## 7.4 Cause Register

```
Bit    31: Branch delay slot
Bits  5-2: Exception code
Bits  1-0: Reserved
```

## 7.5 Endianness

The simulator uses **big-endian** byte order for memory storage.
