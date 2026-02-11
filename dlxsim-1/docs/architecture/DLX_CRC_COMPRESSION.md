# DLX CRC and Compression Acceleration

## Overview

DLX provides hardware acceleration for CRC (Cyclic Redundancy Check) computations and data compression algorithms, with special support for distributed/parallel checksum computation.

## 1. Flexible CRC Computation

### Programmable CRC Engine

```c
/* CRC configuration register */
typedef struct {
    uint32_t polynomial;        /* CRC polynomial (1 to 64 bits) */
    uint32_t width:7;           /* CRC width (1-64 bits) */
    uint32_t init_value:1;      /* Initial value (0 or all 1s) */
    uint32_t final_xor:1;       /* Final XOR with all 1s */
    uint32_t reflect_in:1;      /* Reflect input bytes */
    uint32_t reflect_out:1;     /* Reflect output */
    uint32_t reserved:21;
} crc_config_t;

#define SR_CRC_CONFIG   0xA0
#define SR_CRC_POLY     0xA1
#define SR_CRC_INIT     0xA2
```

### CRC Instructions

```assembly
# Configure CRC parameters
CRC.CONFIG  poly, width, init, final_xor, reflect   # Configure CRC engine
CRC.INIT    value                                   # Initialize CRC state

# Compute CRC
CRC.UPDATE  rd, rs, rdata           # Update CRC with data
CRC.UPDATE.V vd, vs, vdata          # Vector CRC update (process 16 bytes)
CRC.FINAL   rd, rs                  # Finalize CRC (apply final XOR)

# Fast CRC for common polynomials
CRC32       rd, rs, rdata           # CRC-32 (Ethernet, ZIP, PNG)
CRC32C      rd, rs, rdata           # CRC-32C (Castagnoli, iSCSI)
CRC16       rd, rs, rdata           # CRC-16 (ANSI)
CRC16.CCITT rd, rs, rdata           # CRC-16 CCITT
CRC64       rd, rs, rdata           # CRC-64 (ECMA-182)

# Vectorized CRC (process multiple bytes)
VCRC32      vd, vs, vdata           # 16-byte CRC-32 update
VCRC32C     vd, vs, vdata           # 16-byte CRC-32C update
VCRC64      vd, vs, vdata           # 16-byte CRC-64 update

# Examples - CRC-32 (ZIP/PNG/Ethernet)
crc32_compute:
    LI      r_crc, 0xFFFFFFFF   # Init CRC-32 (inverted)
    LI      r_ptr, data_start
    LI      r_len, data_length

.crc_loop:
    LW      r_word, 0(r_ptr)
    CRC32   r_crc, r_crc, r_word

    ADDI    r_ptr, r_ptr, 4
    SUBI    r_len, r_len, 4
    BGTZ    r_len, .crc_loop

    XORI    r_crc, r_crc, 0xFFFFFFFF  # Final XOR
    SW      r_crc, crc_result
```

### Standard CRC Polynomials

```c
/* Predefined CRC configurations */

/* CRC-8 */
#define CRC8_POLY           0x07
#define CRC8_INIT           0x00

/* CRC-16 variants */
#define CRC16_ANSI_POLY     0x8005
#define CRC16_CCITT_POLY    0x1021
#define CRC16_IBM_POLY      0x8005
#define CRC16_MODBUS_POLY   0x8005

/* CRC-32 variants */
#define CRC32_POLY          0x04C11DB7  /* Ethernet, ZIP, PNG */
#define CRC32C_POLY         0x1EDC6F41  /* Castagnoli (iSCSI) */
#define CRC32K_POLY         0x741B8CD7  /* Koopman */

/* CRC-64 variants */
#define CRC64_ECMA_POLY     0x42F0E1EBA9EA3693ULL
#define CRC64_ISO_POLY      0x000000000000001BULL
```

### Distributed/Parallel CRC Computation

```assembly
# Split CRC computation across multiple cores/threads
CRC.SPLIT   rd_crc, rd_len, rtotal_len, rchunk_id   # Calculate offset for chunk
CRC.COMBINE rd, rcrc1, rlen1, rcrc2, rlen2, rpoly   # Combine two CRC results

# Example - Parallel CRC-32 (4 threads)
parallel_crc32:
    # Thread 0
    LI      r_chunk_id, 0
    CRC.SPLIT r_offset, r_chunk_len, r_total_len, r_chunk_id
    JAL     compute_crc_chunk   # Returns CRC in r1

    # Thread 1
    LI      r_chunk_id, 1
    CRC.SPLIT r_offset, r_chunk_len, r_total_len, r_chunk_id
    JAL     compute_crc_chunk

    # ... threads 2 and 3 ...

    # Combine results (on master thread)
combine_crcs:
    CRC.COMBINE r_crc01, r_crc0, r_len0, r_crc1, r_len1, CRC32_POLY
    CRC.COMBINE r_crc23, r_crc2, r_len2, r_crc3, r_len3, CRC32_POLY
    CRC.COMBINE r_final, r_crc01, r_len01, r_crc23, r_len23, CRC32_POLY

# CRC combining uses property: CRC(A || B) = CRC(A) ⊕ (CRC(B) ⊗ x^|A|)
```

### Vector CRC for High Throughput

```assembly
# Process 128 bytes per iteration using vectors
vectorized_crc32:
    LI      r_crc, 0xFFFFFFFF
    LI      r_ptr, data_start
    LI      r_len, data_length

.vec_loop:
    # Load 128 bytes (8×16-byte vectors)
    VLD.128 v0, 0(r_ptr)
    VLD.128 v1, 16(r_ptr)
    VLD.128 v2, 32(r_ptr)
    VLD.128 v3, 48(r_ptr)
    VLD.128 v4, 64(r_ptr)
    VLD.128 v5, 80(r_ptr)
    VLD.128 v6, 96(r_ptr)
    VLD.128 v7, 112(r_ptr)

    # Parallel CRC updates
    VCRC32  v_crc0, r_crc, v0
    VCRC32  v_crc1, v_crc0, v1
    VCRC32  v_crc2, v_crc1, v2
    VCRC32  v_crc3, v_crc2, v3
    VCRC32  v_crc4, v_crc3, v4
    VCRC32  v_crc5, v_crc4, v5
    VCRC32  v_crc6, v_crc5, v6
    VCRC32  r_crc, v_crc6, v7

    ADDI    r_ptr, r_ptr, 128
    SUBI    r_len, r_len, 128
    BGTZ    r_len, .vec_loop

    XORI    r_crc, r_crc, 0xFFFFFFFF
```

## 2. Compression Acceleration

### LZ77/LZ78/LZW/LZSS Compression

```assembly
# LZ77 match finding (sliding window)
LZ77.FIND_MATCH  rd_offset, rd_len, rsrc, rwindow_size, rmax_match_len
LZ77.ENCODE      rd_literal, rd_offset, rd_len, rsrc
LZ77.DECODE      rd_data, rliteral, roffset, rlen

# LZSS (Lempel-Ziv-Storer-Szymanski)
LZSS.FIND_MATCH  rd_offset, rd_len, rsrc, rdict
LZSS.ENCODE      rd_token, rsrc
LZSS.DECODE      rd_data, rtoken

# LZW (Lempel-Ziv-Welch)
LZW.ENCODE       rd_code, rdata, rdict
LZW.DECODE       rd_data, rcode, rdict
LZW.DICT.INIT    rdict
LZW.DICT.ADD     rdict, rcode, rdata

# Example - LZ77 compression (DEFLATE-style)
lz77_compress:
    LI      r_wsize, 32768      # 32KB window
    LI      r_maxmatch, 258     # Max match length
    LI      r_src, input_data
    LI      r_dst, output_data

.compress_loop:
    # Find longest match in sliding window
    LZ77.FIND_MATCH r_offset, r_len, r_src, r_wsize, r_maxmatch

    # Check if match is worthwhile (len >= 3)
    LI      r_min_match, 3
    BLT     r_len, r_min_match, .literal

    # Encode match (offset, length)
    LZ77.ENCODE r_token, r_offset, r_len, r_src
    SW      r_token, 0(r_dst)
    ADDI    r_dst, r_dst, 4
    ADD     r_src, r_src, r_len
    J       .next

.literal:
    # Encode literal byte
    LB      r_byte, 0(r_src)
    SB      r_byte, 0(r_dst)
    ADDI    r_dst, r_dst, 1
    ADDI    r_src, r_src, 1

.next:
    # Continue until end of input
    BLT     r_src, r_end, .compress_loop
```

### Huffman Coding

```assembly
# Huffman tree construction
HUFF.BUILD_TREE  rtree, rfreq_table, rnum_symbols   # Build Huffman tree
HUFF.GEN_CODES   rcodes, rtree                       # Generate Huffman codes

# Huffman encoding/decoding
HUFF.ENCODE      rd_bits, rd_bitlen, rsymbol, rcodes
HUFF.DECODE      rd_symbol, rbits, rtree

# Canonical Huffman (DEFLATE-style)
HUFF.CANONICAL   rcodes, rbit_lengths, rnum_symbols

# Example - Huffman encoding
huffman_encode:
    # Build frequency table
    JAL     count_frequencies   # → freq_table

    # Build Huffman tree
    LI      r_freq, freq_table
    LI      r_nsymbols, 256
    HUFF.BUILD_TREE r_tree, r_freq, r_nsymbols

    # Generate codes
    HUFF.GEN_CODES r_codes, r_tree

    # Encode data
    LI      r_src, input_data
    LI      r_dst_bits, output_bits
    LI      r_bitpos, 0

.encode_loop:
    LB      r_symbol, 0(r_src)
    HUFF.ENCODE r_bits, r_bitlen, r_symbol, r_codes

    # Write bits to output
    JAL     write_bits          # (r_bits, r_bitlen, r_dst_bits, r_bitpos)

    ADDI    r_src, r_src, 1
    BLT     r_src, r_end, .encode_loop
```

### Arithmetic Coding

```assembly
# Arithmetic coding state management
ARITH.INIT       rstate, rprob_model         # Initialize arithmetic coder
ARITH.ENCODE     rstate, rsymbol, rprob      # Encode symbol
ARITH.DECODE     rd_symbol, rstate, rprob    # Decode symbol
ARITH.FLUSH      rstate, rout                # Flush final bits

# Probability model update
ARITH.UPDATE     rprob, rsymbol              # Update probability model (adaptive)

# Example - Arithmetic encoding
arithmetic_encode:
    # Initialize
    ARITH.INIT r_state, r_prob_model

.encode_loop:
    LB      r_symbol, 0(r_src)

    # Get symbol probability
    ARITH.GET_PROB r_prob, r_prob_model, r_symbol

    # Encode symbol
    ARITH.ENCODE r_state, r_symbol, r_prob

    # Update model (adaptive)
    ARITH.UPDATE r_prob_model, r_symbol

    ADDI    r_src, r_src, 1
    BLT     r_src, r_end, .encode_loop

    # Flush final bits
    ARITH.FLUSH r_state, r_output
```

### Range Coding

```assembly
# Range coding (similar to arithmetic coding, but simpler)
RANGE.INIT       rstate                      # Initialize range coder
RANGE.ENCODE     rstate, rsymbol, rfreq, rtotal
RANGE.DECODE     rd_symbol, rstate, rfreq, rtotal
RANGE.RENORM     rstate                      # Renormalize range

# Example - Range encoding
range_encode:
    RANGE.INIT r_state

.encode_loop:
    LB      r_symbol, 0(r_src)

    # Get frequency and total
    LW      r_freq, freq_table(r_symbol)
    LW      r_total, total_freq

    # Encode
    RANGE.ENCODE r_state, r_symbol, r_freq, r_total

    # Renormalize if needed
    RANGE.RENORM r_state

    ADDI    r_src, r_src, 1
    BLT     r_src, r_end, .encode_loop
```

### Run-Length Encoding (RLE)

```assembly
# RLE encoding/decoding
RLE.FIND_RUN     rd_len, rsrc                # Find run length
RLE.ENCODE       rd_byte, rd_count, rsrc     # Encode run
RLE.DECODE       rd_data, rbyte, rcount      # Decode run

# Vectorized RLE for images/video
VRLE.FIND_RUN    rd_len, vsrc                # Find run in vector
VRLE.ENCODE      vd, vsrc                    # Encode runs

# Example - RLE compression
rle_compress:
    LI      r_src, input_data
    LI      r_dst, output_data

.rle_loop:
    # Find run of identical bytes
    RLE.FIND_RUN r_len, r_src

    # Get byte value
    LB      r_byte, 0(r_src)

    # Encode: [count][byte]
    SB      r_len, 0(r_dst)
    SB      r_byte, 1(r_dst)

    ADDI    r_dst, r_dst, 2
    ADD     r_src, r_src, r_len
    BLT     r_src, r_end, .rle_loop
```

### Markov Compression

```assembly
# Markov model (PPM - Prediction by Partial Matching)
MARKOV.INIT      rmodel, rorder              # Initialize Markov model
MARKOV.PREDICT   rd_pred, rmodel, rcontext   # Predict next symbol
MARKOV.UPDATE    rmodel, rcontext, rsymbol   # Update model
MARKOV.GET_PROB  rd_prob, rmodel, rcontext, rsymbol

# Example - PPM compression
ppm_compress:
    LI      r_order, 3          # Order-3 PPM
    MARKOV.INIT r_model, r_order

.ppm_loop:
    # Get context (last N symbols)
    JAL     get_context         # → r_context

    # Predict next symbol
    MARKOV.PREDICT r_pred, r_model, r_context

    # Encode using predicted probabilities
    LB      r_symbol, 0(r_src)
    MARKOV.GET_PROB r_prob, r_model, r_context, r_symbol
    ARITH.ENCODE r_state, r_symbol, r_prob

    # Update model
    MARKOV.UPDATE r_model, r_context, r_symbol

    ADDI    r_src, r_src, 1
    BLT     r_src, r_end, .ppm_loop
```

### Adaptive Compression (Dynamic Model)

```assembly
# Adaptive Huffman coding
ADAPT_HUFF.INIT  rtree                       # Initialize adaptive Huffman tree
ADAPT_HUFF.ENC   rd_bits, rtree, rsymbol     # Encode + update tree
ADAPT_HUFF.DEC   rd_symbol, rtree, rbits     # Decode + update tree

# Adaptive arithmetic coding
ADAPT_ARITH.INIT rstate, rmodel
ADAPT_ARITH.ENC  rstate, rmodel, rsymbol     # Encode + update probabilities
ADAPT_ARITH.DEC  rd_symbol, rstate, rmodel   # Decode + update probabilities

# Example - Adaptive Huffman compression
adaptive_huffman:
    ADAPT_HUFF.INIT r_tree

.adapt_loop:
    LB      r_symbol, 0(r_src)

    # Encode and update tree simultaneously
    ADAPT_HUFF.ENC r_bits, r_tree, r_symbol

    # Write bits
    JAL     write_bits

    ADDI    r_src, r_src, 1
    BLT     r_src, r_end, .adapt_loop
```

### Modern Compression Algorithms

```assembly
# Brotli compression primitives
BROTLI.DICT_MATCH rd_offset, rd_len, rsrc    # Dictionary match
BROTLI.CONTEXT    rd_ctx, rsrc, rmode        # Context modeling
BROTLI.ENCODE     rd_token, rsymbol, rctx    # Encode with context

# Zstandard (zstd) primitives
ZSTD.FSE_ENCODE   rd_bits, rsymbol, rtable   # FSE encoding
ZSTD.FSE_DECODE   rd_symbol, rbits, rtable   # FSE decoding
ZSTD.MATCH_FIND   rd_offset, rd_len, rsrc    # Fast match finding
ZSTD.DICT_LOAD    rdict, rdata, rsize        # Load dictionary

# LZ4 (ultra-fast compression)
LZ4.COMPRESS      rd_out_len, rsrc, rdst, rlen
LZ4.DECOMPRESS    rd_out_len, rsrc, rdst, rlen
LZ4.MATCH_FIND    rd_offset, rd_len, rsrc    # Fast match (hash table)

# Snappy (Google)
SNAPPY.COMPRESS   rd_out_len, rsrc, rdst, rlen
SNAPPY.DECOMPRESS rd_out_len, rsrc, rdst, rlen
```

## 3. Bit Stream Operations (for Compression)

```assembly
# Bit-level operations for compression
BITSTREAM.INIT   rbs, rbuffer                # Initialize bit stream
BITSTREAM.WRITE  rbs, rbits, rbitlen         # Write N bits
BITSTREAM.READ   rd_bits, rbs, rbitlen       # Read N bits
BITSTREAM.ALIGN  rbs                         # Align to byte boundary
BITSTREAM.POS    rd, rbs                     # Get bit position

# Bit reversal (for some codecs)
BITREV.W    rd, rs                           # Reverse bits in 32-bit word
BITREV.H    rd, rs                           # Reverse bits in 16-bit halfword
BITREV.B    rd, rs                           # Reverse bits in byte

# Examples - Write variable-length code
write_vlc_code:
    # Write 13 bits: code = 0x1A3F (bits 12-0)
    LI      r_code, 0x1A3F
    LI      r_bitlen, 13
    BITSTREAM.WRITE r_bs, r_code, r_bitlen
```

## 4. Hash Tables for Compression

```assembly
# Fast hash table for LZ compression
HASH.INIT       rhash, rsize                 # Initialize hash table
HASH.INSERT     rhash, rkey, rvalue          # Insert key-value pair
HASH.LOOKUP     rd_value, rhash, rkey        # Lookup value by key
HASH.UPDATE     rhash, rkey, rvalue          # Update existing entry
HASH.CLEAR      rhash                        # Clear hash table

# Rolling hash (for sliding window)
HASH.ROLLING    rd_hash, rdata, rwindow_size # Compute rolling hash
HASH.UPDATE_ROLL rd_hash, rd_hash, rold_byte, rnew_byte, rwindow_size

# Example - LZ77 with hash table
lz77_with_hash:
    HASH.INIT r_hash, 65536     # 64K hash table

.compress_loop:
    # Compute hash of next 3 bytes
    LW      r_triple, 0(r_src)
    ANDI    r_triple, r_triple, 0xFFFFFF
    HASH.ROLLING r_hash_val, r_triple, 3

    # Lookup previous occurrence
    HASH.LOOKUP r_prev_pos, r_hash, r_hash_val

    # Find match length
    JAL     compare_match       # → r_match_len

    # Insert current position
    HASH.INSERT r_hash, r_hash_val, r_src_pos

    # Encode match or literal
    # ...
```

## 5. Performance Characteristics

```c
/*
 * Compression/CRC Performance:
 *
 * Algorithm           Throughput      Compression Ratio
 * ───────────────────────────────────────────────────────
 * CRC-32              40 GB/s         N/A
 * CRC-32 (vectorized) 120 GB/s        N/A
 * CRC-64              35 GB/s         N/A
 * LZ4                 4 GB/s          2.0:1
 * Snappy              3.5 GB/s        1.5-2.0:1
 * LZ77 (DEFLATE)      800 MB/s        2.5-3.0:1
 * Brotli              200 MB/s        2.7-3.5:1
 * Zstandard           600 MB/s        2.8-3.2:1
 * Huffman only        2 GB/s          1.5-2.0:1
 * Arithmetic coding   400 MB/s        3.0-4.0:1
 * Range coding        500 MB/s        3.0-4.0:1
 * PPM                 100 MB/s        3.5-5.0:1
 *
 * Hardware acceleration provides 10-100x speedup vs software
 */
```

## Summary

**CRC Features**:
- Programmable CRC (1-64 bit polynomials)
- Standard CRC variants (CRC-8/16/32/32C/64)
- Vectorized CRC (process 16 bytes at once)
- Distributed CRC (combine results from multiple threads)
- Up to 120 GB/s throughput (vectorized)

**Compression Algorithms**:
- LZ77/LZSS/LZW (dictionary-based)
- Huffman coding (canonical and adaptive)
- Arithmetic coding
- Range coding
- Run-Length Encoding (RLE)
- Markov/PPM (context modeling)
- Adaptive compression
- Modern: Brotli, Zstandard, LZ4, Snappy

**Supporting Operations**:
- Fast match finding (hash tables, rolling hash)
- Bit stream I/O
- Bit reversal
- Probability modeling
- Dictionary management

**Use Cases**:
- Network protocols (CRC checksums)
- File compression (ZIP, gzip, 7-Zip)
- Database compression
- Network compression (HTTP/2, HTTP/3)
- Real-time video/audio compression
- Log file compression
- Distributed storage (erasure coding with CRC)

---

**Document Status**: Specification Complete
**Implementation Status**: Ready for hardware integration
**Performance**: 10-100x speedup vs software implementations
