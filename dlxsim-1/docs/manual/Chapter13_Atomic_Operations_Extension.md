# Chapter 13: Atomic Operations Extension (A)

## 13.1 Load-Reserved / Store-Conditional

```assembly
# Load-reserved word
lr.w        rd, (rs1)           # rd ← mem[rs1], set reservation

# Store-conditional word
sc.w        rd, rs2, (rs1)      # if reservation valid:
                                #   mem[rs1] ← rs2, rd ← 0
                                # else:
                                #   rd ← 1

# 64-bit variants
lr.d        rd, (rs1)           # Load-reserved doubleword
sc.d        rd, rs2, (rs1)      # Store-conditional doubleword

# Example: Atomic increment
retry:
    lr.w    t0, (a0)            # Load current value
    addi    t0, t0, 1           # Increment
    sc.w    t1, t0, (a0)        # Try to store
    bnez    t1, retry           # Retry if failed
```

## 13.2 Atomic Memory Operations (AMO)

### Atomic Fetch-and-Op

```assembly
# Atomic swap
amoswap.w   rd, rs2, (rs1)      # rd ← mem[rs1]
                                # mem[rs1] ← rs2

# Atomic add
amoadd.w    rd, rs2, (rs1)      # rd ← mem[rs1]
                                # mem[rs1] ← mem[rs1] + rs2

# Atomic XOR
amoxor.w    rd, rs2, (rs1)      # rd ← mem[rs1]
                                # mem[rs1] ← mem[rs1] ⊕ rs2

# Atomic AND
amoand.w    rd, rs2, (rs1)      # rd ← mem[rs1]
                                # mem[rs1] ← mem[rs1] & rs2

# Atomic OR
amoor.w     rd, rs2, (rs1)      # rd ← mem[rs1]
                                # mem[rs1] ← mem[rs1] | rs2

# Atomic min (signed)
amomin.w    rd, rs2, (rs1)      # rd ← mem[rs1]
                                # mem[rs1] ← min(mem[rs1], rs2)

# Atomic max (signed)
amomax.w    rd, rs2, (rs1)      # rd ← mem[rs1]
                                # mem[rs1] ← max(mem[rs1], rs2)

# Atomic min (unsigned)
amominu.w   rd, rs2, (rs1)      # rd ← mem[rs1]
                                # mem[rs1] ← minu(mem[rs1], rs2)

# Atomic max (unsigned)
amomaxu.w   rd, rs2, (rs1)      # rd ← mem[rs1]
                                # mem[rs1] ← maxu(mem[rs1], rs2)
```

### Memory Ordering

All AMO instructions support memory ordering suffixes:

```assembly
# Acquire ordering (load-acquire)
amoadd.w.aq rd, rs2, (rs1)

# Release ordering (store-release)
amoadd.w.rl rd, rs2, (rs1)

# Sequentially consistent (acquire + release)
amoadd.w.aqrl rd, rs2, (rs1)
```

## 13.3 Compare-and-Swap

```assembly
# Compare-and-swap word
cas.w       rd, rs2, rs3, (rs1) # if mem[rs1] == rs2:
                                #   rd ← 1, mem[rs1] ← rs3
                                # else:
                                #   rd ← 0

# 64-bit variant
cas.d       rd, rs2, rs3, (rs1) # CAS doubleword

# Double-width CAS (128-bit on DLX64)
casd.q      rd, rs2, rs3, (rs1) # CAS quadword

# Example: Lock-free stack push
push:
    ld      t0, stack_top       # Load current top
retry:
    sd      t0, 0(a0)           # node->next = top
    cas.d   t1, t0, a0, stack_top
    beqz    t1, retry           # Retry if CAS failed
```

## 13.4 Memory Barriers

```assembly
fence                           # Full memory barrier
fence.r                         # Load fence
fence.w                         # Store fence
fence.rw                        # Read-write fence
fence.i                         # Instruction fence (I-cache sync)
```

Fence ordering modes:
```assembly
fence   predecessor, successor

# Examples:
fence   rw, rw                  # Full barrier
fence   w, rw                   # Store-release barrier
fence   rw, r                   # Load-acquire barrier
fence   w, w                    # Store-store barrier
```

## 13.5 Synchronization Primitives

### Spinlock

```assembly
# Acquire spinlock
acquire_spinlock:
    li      t0, 1
retry:
    lr.w    t1, (a0)            # Load lock
    bnez    t1, retry           # Spin if locked
    sc.w    t2, t0, (a0)        # Try to acquire
    bnez    t2, retry           # Retry if failed
    fence   rw, rw              # Memory barrier
    ret

# Release spinlock
release_spinlock:
    fence   rw, w               # Memory barrier
    sw      zero, (a0)          # Release lock
    ret
```

### Semaphore

```assembly
# Decrement semaphore (P operation)
sem_wait:
retry:
    lr.w    t0, (a0)            # Load count
    beqz    t0, retry           # Wait if zero
    addi    t0, t0, -1          # Decrement
    sc.w    t1, t0, (a0)        # Try to store
    bnez    t1, retry           # Retry if failed
    ret

# Increment semaphore (V operation)
sem_post:
    li      t0, 1
    amoadd.w.rl t1, t0, (a0)    # Atomic increment
    ret
```

## 13.6 Lock-Free Data Structures

### Lock-Free Queue (Michael-Scott)

```assembly
# Enqueue
enqueue:
    ld      t0, tail            # Load tail pointer
retry:
    ld      t1, 0(t0)           # next = tail->next
    bnez    t1, update_tail     # Tail not at end
    cas.d   t2, zero, a0, 0(t0) # Try to link new node
    beqz    t2, retry
    cas.d   t3, t0, a0, tail    # Update tail
    ret
update_tail:
    cas.d   t3, t0, t1, tail    # Help update tail
    j       retry
```

## 13.7 Atomic 128-bit Operations (DLX64)

```assembly
# Atomic 128-bit compare-and-swap
casdq       rd, rs2_pair, rs3_pair, (rs1)
            # Compare and swap 128 bits atomically

# Atomic 128-bit load/store
lr.q        rd_pair, (rs1)      # Load-reserved 128-bit
sc.q        rd, rs2_pair, (rs1) # Store-conditional 128-bit
```

## 13.8 Performance Considerations

- **Contention**: LR/SC may fail under high contention, use exponential backoff
- **Cache coherence**: AMO operations invalidate remote caches
- **Memory ordering**: Relaxed ordering (.aq, .rl) can improve performance
- **False sharing**: Avoid atomics on same cache line from different threads
