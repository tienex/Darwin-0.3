# Chapter 19: Matrix Operations (M Extension)

## 19.1 Matrix Configuration

```assembly
msetcfg     rows, cols, elem_width    # Configure matrix tile
```

## 19.2 Matrix Arithmetic

```assembly
# Matrix-Matrix Operations
mmadd       md, ms1, ms2        # Matrix add: md ← ms1 + ms2
mmsub       md, ms1, ms2        # Matrix subtract
mmmul       md, ms1, ms2        # Matrix multiply: md ← ms1 × ms2
mmtrans     md, ms              # Matrix transpose

# Matrix-Vector Operations
mmvadd      vd, ms, vs          # Matrix-vector add
mmvmul      vd, ms, vs          # Matrix-vector multiply: vd ← ms × vs

# Elementwise Operations
mmhadamard  md, ms1, ms2        # Hadamard product (element-wise multiply)
```

## 19.3 Matrix Load/Store

```assembly
mmld        md, (rs), stride    # Load matrix from memory
mmst        ms, (rd), stride    # Store matrix to memory
```
