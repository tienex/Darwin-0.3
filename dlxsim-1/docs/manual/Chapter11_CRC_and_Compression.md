# Chapter 11: CRC and Compression (K Extension)

## 11.1 CRC Instructions

```assembly
# CRC-32
crc32.b     rd, rs              # CRC-32 byte
crc32.h     rd, rs              # CRC-32 halfword
crc32.w     rd, rs              # CRC-32 word
crc32.d     rd, rs              # CRC-32 doubleword

# CRC-32C (Castagnoli)
crc32c.b    rd, rs              # CRC-32C byte
crc32c.w    rd, rs              # CRC-32C word
crc32c.d    rd, rs              # CRC-32C doubleword

# Custom CRC
crc.custom  rd, rs, poly, init  # CRC with custom polynomial
```

## 11.2 Compression Instructions

```assembly
# LZ77-style compression
lz77.compress   vd, vs, dict    # LZ77 compress vector
lz77.decompress vd, vs, dict    # LZ77 decompress vector

# Huffman encoding
huff.encode     vd, vs, table   # Huffman encode
huff.decode     vd, vs, table   # Huffman decode

# Dictionary compression
dict.compress   vd, vs, dict    # Dictionary-based compress
dict.decompress vd, vs, dict    # Dictionary-based decompress
```
