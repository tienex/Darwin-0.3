# Chapter 9: Bit Manipulation Extension (B)

```assembly
# Count operations
clz         rd, rs              # Count leading zeros
ctz         rd, rs              # Count trailing zeros
pcnt        rd, rs              # Population count (number of 1 bits)

# Rotate operations
rol         rd, rs1, rs2        # Rotate left
ror         rd, rs1, rs2        # Rotate right
rori        rd, rs, imm         # Rotate right immediate

# Byte operations
rev8        rd, rs              # Reverse bytes (64-bit → byte swap)
rev16       rd, rs              # Reverse halfwords
orc.b       rd, rs              # OR combine bytes
rev.b       rd, rs              # Reverse bits in each byte

# Single-bit operations
bset        rd, rs1, rs2        # Set bit: rd ← rs1 | (1 << rs2)
bclr        rd, rs1, rs2        # Clear bit: rd ← rs1 & ~(1 << rs2)
binv        rd, rs1, rs2        # Invert bit: rd ← rs1 ^ (1 << rs2)
bext        rd, rs1, rs2        # Extract bit: rd ← (rs1 >> rs2) & 1

# Logic with complement
andn        rd, rs1, rs2        # AND NOT: rd ← rs1 & ~rs2
orn         rd, rs1, rs2        # OR NOT: rd ← rs1 | ~rs2
xnor        rd, rs1, rs2        # XOR NOT: rd ← ~(rs1 ^ rs2)

# Min/max
min         rd, rs1, rs2        # Minimum (signed)
minu        rd, rs1, rs2        # Minimum (unsigned)
max         rd, rs1, rs2        # Maximum (signed)
maxu        rd, rs1, rs2        # Maximum (unsigned)

# Sign/zero extension
sext.b      rd, rs              # Sign-extend byte
sext.h      rd, rs              # Sign-extend halfword
zext.b      rd, rs              # Zero-extend byte
zext.h      rd, rs              # Zero-extend halfword
```
