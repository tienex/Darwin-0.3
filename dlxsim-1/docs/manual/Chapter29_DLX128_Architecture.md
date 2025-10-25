# VOLUME V: FUTURE EXTENSIONS

# Chapter 29: DLX128 Architecture

## 29.1 128-bit Instructions

```assembly
# Load/Store 128-bit
lq      rd, offset(rs)          # Load quadword (128-bit)
sq      rt, offset(rs)          # Store quadword

# Arithmetic 128-bit
addq    rd, rs, rt              # Add quadword
subq    rd, rs, rt              # Subtract quadword
mulq    rd, rs, rt              # Multiply quadword
divq    rd, rs, rt              # Divide quadword

# Logical 128-bit
andq    rd, rs, rt              # AND quadword
orq     rd, rs, rt              # OR quadword
xorq    rd, rs, rt              # XOR quadword

# Shift 128-bit
sllq    rd, rs, rt              # Shift left logical quadword
srlq    rd, rs, rt              # Shift right logical quadword
sraq    rd, rs, rt              # Shift right arithmetic quadword

# Compare/Branch 128-bit
cmpq    rd, rs, rt              # Compare quadword
beqq    rs, rt, offset          # Branch if equal quadword
bneq    rs, rt, offset          # Branch if not equal quadword
```
