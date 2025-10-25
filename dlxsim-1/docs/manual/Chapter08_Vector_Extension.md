# VOLUME II: STANDARD EXTENSIONS

# Chapter 8: Vector Extension (V)

## 8.1 Vector Configuration

```assembly
vsetvl   rd, rs1, vtypei        # Set vector length: rd ← VL
vsetvli  rd, rs, vtypei         # Set vector length immediate
```

## 8.2 Vector Arithmetic

```assembly
# Integer arithmetic
vadd.vv     vd, vs1, vs2, vm    # Vector add (vector-vector)
vadd.vx     vd, vs1, rs, vm     # Vector add (vector-scalar)
vadd.vi     vd, vs1, imm, vm    # Vector add (vector-immediate)
vsub.vv     vd, vs1, vs2, vm    # Vector subtract
vmul.vv     vd, vs1, vs2, vm    # Vector multiply
vdiv.vv     vd, vs1, vs2, vm    # Vector divide

# Widening operations
vwadd.vv    vd, vs1, vs2, vm    # Widening add (2×SEW result)
vwmul.vv    vd, vs1, vs2, vm    # Widening multiply

# Reduction operations
vredsum.vs  vd, vs1, vs2, vm    # Vector reduce sum
vredmax.vs  vd, vs1, vs2, vm    # Vector reduce maximum
vredmin.vs  vd, vs1, vs2, vm    # Vector reduce minimum
vredand.vs  vd, vs1, vs2, vm    # Vector reduce AND
vredor.vs   vd, vs1, vs2, vm    # Vector reduce OR
vredxor.vs  vd, vs1, vs2, vm    # Vector reduce XOR
```

## 8.3 Vector Load/Store

```assembly
vle.v       vd, (rs), vm        # Vector load (unit stride)
vse.v       vs, (rd), vm        # Vector store (unit stride)
vlse.v      vd, (rs), rs2, vm   # Vector load (strided)
vsse.v      vs, (rd), rs2, vm   # Vector store (strided)
vlxe.v      vd, (rs), vs2, vm   # Vector load (indexed)
vsxe.v      vs, (rd), vs2, vm   # Vector store (indexed)
```
