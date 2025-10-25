# DLX System Extensions (DLX-S)

## Overview

DLX-S adds advanced system-level features including power management, atomic operations, overflow/carry handling, thread pointers, and processor identification. Inspired by Intel (MWAIT), M88K (overflow/carry), and modern RISC architectures.

---

## 1. Power Management (Intel MWAIT-inspired)

### 1.1 Monitor and Wait Instructions

```asm
MONITOR  rs, extensions, hints  ; Set up linear address range to monitor
                                 ; rs: address to monitor
                                 ; extensions: reserved (must be 0)
                                 ; hints: optimization hints

MWAIT    rcx, extensions        ; Wait until monitored address is written
                                 ; rcx: C-state hints
                                 ; extensions: reserved (must be 0)
                                 ; Privilege: Kernel only
```

**C-State Hints** (rcx register):
```
Bits 3-0: Target C-state
  0x0: C0 (running)
  0x1: C1 (halt)
  0x2: C2 (stop clock)
  0x3: C3 (deep sleep)
  0x4: C4 (deeper sleep)

Bit 4: Break on interrupt even if masked
Bits 31-5: Reserved
```

**Example - Spinlock with Power Saving**:
```asm
; Efficient spinlock using MONITOR/MWAIT
spin_wait:
  monitor lock_addr, r0, r0    ; Monitor lock address
  lw   r1, (lock_addr)         ; Check if lock is free
  beqz r1, got_lock            ; If free, try to acquire

  li   r2, 0x1                 ; C1 state (halt until write)
  mwait r2, r0                 ; Wait for lock write event
  j    spin_wait               ; Retry

got_lock:
  ; Attempt atomic acquisition
  ...
```

### 1.2 Halt and Pause

```asm
HALT                            ; Halt processor until interrupt
                                ; Privilege: Kernel only
                                ; Power savings: ~90%

PAUSE                           ; Pause for spinlock (hint to pipeline)
                                ; No privilege required
                                ; Improves spinlock performance

WFI                             ; Wait for interrupt
                                ; Privilege: Kernel only
                                ; Lower power than HALT

WFE                             ; Wait for event
                                ; Privilege: Kernel only
                                ; Wake on any event signal
```

**Example - Idle Loop**:
```asm
idle_loop:
  ; Disable interrupts
  di

  ; Check if work available
  lw   r1, work_queue
  bnez r1, process_work

  ; Enter low power mode
  wfi                           ; Wait for interrupt

  ; Re-enable and check
  ei
  j    idle_loop
```

---

## 2. Advanced Atomic Operations

### 2.1 Load-Linked / Store-Conditional (LL/SC)

```asm
LL   rd, (rs)                   ; Load-linked word
                                ; rd = mem[rs]
                                ; Set link register to rs

SC   rd, rt, (rs)               ; Store-conditional word
                                ; if (link_valid && link_addr == rs)
                                ;   mem[rs] = rt; rd = 1 (success)
                                ; else
                                ;   rd = 0 (failure)

LLD  rd, (rs)                   ; Load-linked doubleword (DLX64)
SCD  rd, rt, (rs)               ; Store-conditional doubleword (DLX64)
```

**Example - Atomic Increment**:
```asm
atomic_inc:
  ll   r1, (r10)                ; Load-linked
  addi r1, r1, 1                ; Increment
  sc   r2, r1, (r10)            ; Store-conditional
  beqz r2, atomic_inc           ; Retry if failed
  ; r1 now contains incremented value
```

### 2.2 Compare-and-Swap (CAS)

```asm
CAS   rd, rs1, rs2, (rt)        ; Compare-and-swap word
                                ; if (mem[rt] == rs1)
                                ;   mem[rt] = rs2; rd = 1
                                ; else
                                ;   rd = mem[rt]; rd = 0

CASD  rd, rs1, rs2, (rt)        ; Compare-and-swap doubleword (DLX64)

CASB  rd, rs1, rs2, (rt)        ; Compare-and-swap byte
CASH  rd, rs1, rs2, (rt)        ; Compare-and-swap halfword
```

**Example - Lock-Free Stack Push**:
```asm
; Push r1 onto lock-free stack at r10
push_retry:
  lw   r2, (r10)                ; Load current top
  sw   r1, NEXT_OFFSET(r1)      ; new->next = top
  cas  r3, r2, r1, (r10)        ; CAS(top, old, new)
  beqz r3, push_retry           ; Retry if failed
```

### 2.3 Fetch-and-Op Atomic Operations

```asm
FETCHADD  rd, rs, (rt)          ; Fetch-and-add
                                ; rd = mem[rt]; mem[rt] += rs

FETCHAND  rd, rs, (rt)          ; Fetch-and-AND
FETCHOR   rd, rs, (rt)          ; Fetch-and-OR
FETCHXOR  rd, rs, (rt)          ; Fetch-and-XOR

FETCHMAX  rd, rs, (rt)          ; Fetch-and-max (signed)
FETCHMIN  rd, rs, (rt)          ; Fetch-and-min (signed)
FETCHUMAX rd, rs, (rt)          ; Fetch-and-max (unsigned)
FETCHUMIN rd, rs, (rt)          ; Fetch-and-min (unsigned)

SWAP      rd, rs, (rt)          ; Atomic swap
                                ; rd = mem[rt]; mem[rt] = rs
```

**Example - Atomic Counter**:
```asm
; Atomically increment counter and get old value
fetchadd r1, r2, (r3)           ; r1 = old value, mem[r3] += r2
```

### 2.4 Test-and-Set / Test-and-Clear

```asm
TAS   rd, (rs)                  ; Test-and-set
                                ; rd = mem[rs]; mem[rs] = 1

TAC   rd, (rs)                  ; Test-and-clear
                                ; rd = mem[rs]; mem[rs] = 0

BTS   rd, bit, (rs)             ; Bit test-and-set
                                ; rd = bit_test(mem[rs], bit)
                                ; mem[rs] |= (1 << bit)

BTR   rd, bit, (rs)             ; Bit test-and-reset
BTC   rd, bit, (rs)             ; Bit test-and-complement
```

**Example - Simple Spinlock**:
```asm
acquire_lock:
  tas  r1, (lock_addr)          ; Test-and-set
  bnez r1, acquire_lock         ; Spin if already locked
  ; Lock acquired

release_lock:
  tac  r0, (lock_addr)          ; Test-and-clear (release)
```

---

## 3. Overflow and Carry Flags (M88K-inspired)

### 3.1 Status Register Extensions

**PSR (Processor Status Register) Additional Bits**:
```
Bit 31: C (Carry flag)
Bit 30: V (Overflow flag)
Bit 29: N (Negative flag)
Bit 28: Z (Zero flag)
```

### 3.2 Arithmetic with Carry

```asm
ADDC   rd, rs1, rs2             ; Add with carry
                                ; rd = rs1 + rs2 + C
                                ; Updates C, V, N, Z

SUBC   rd, rs1, rs2             ; Subtract with carry (borrow)
                                ; rd = rs1 - rs2 - C
                                ; Updates C, V, N, Z

ADDCO  rd, rs1, rs2             ; Add with carry, trap on overflow
SUBCO  rd, rs1, rs2             ; Subtract with carry, trap on overflow
```

### 3.3 Arithmetic with Overflow Detection

```asm
ADDO   rd, rs1, rs2             ; Add, trap on overflow
                                ; rd = rs1 + rs2
                                ; If overflow: trap to exception handler

SUBO   rd, rs1, rs2             ; Subtract, trap on overflow
MULO   rd, rs1, rs2             ; Multiply, trap on overflow

ADDV   rd, rs1, rs2             ; Add, set V flag
SUBV   rd, rs1, rs2             ; Subtract, set V flag
MULV   rd, rs1, rs2             ; Multiply, set V flag
```

### 3.4 Condition Code Operations

```asm
SETC                            ; Set carry flag
CLRC                            ; Clear carry flag
CPLC                            ; Complement carry flag

SETV                            ; Set overflow flag
CLRV                            ; Clear overflow flag

MOVCC  rd, cc                   ; Move condition codes to register
                                ; rd = {C, V, N, Z, ...}

MOVCCFROM rs                    ; Move register to condition codes
```

**Example - Multi-Precision Addition (128-bit)**:
```asm
; Add two 128-bit numbers (4 x 32-bit words)
; r1:r2:r3:r4 = r5:r6:r7:r8 + r9:r10:r11:r12

  add  r4, r8, r12              ; Low word (no carry in)
  addc r3, r7, r11              ; Word 1 (with carry)
  addc r2, r6, r10              ; Word 2 (with carry)
  addc r1, r5, r9               ; High word (with carry)
  ; C flag now indicates final carry out
```

---

## 4. Thread Pointer Registers

### 4.1 Thread Pointer Special Registers

**New CP0 Registers**:
```
$tpuser   (CP0 reg 29, sel 0)   ; User thread pointer
$tpkernel (CP0 reg 29, sel 1)   ; Kernel thread pointer
```

**Access Instructions**:
```asm
MFTP  rd, user/kernel           ; Move from thread pointer
                                ; rd = (user_mode ? $tpuser : $tpkernel)
                                ; User mode can only read $tpuser

MTTP  rs, user/kernel           ; Move to thread pointer
                                ; (user_mode ? $tpuser : $tpkernel) = rs
                                ; User mode can only write $tpuser
                                ; Privilege: user write allowed only to $tpuser
```

**Fast Access (No Privilege Check)**:
```asm
RDTP  rd                        ; Read current thread pointer
                                ; rd = current mode's thread pointer
                                ; No privilege check, very fast
```

**Example - Thread-Local Storage Access**:
```asm
; Access thread-local variable at offset 0x100
rdtp r1                         ; Get thread pointer (fast)
lw   r2, 0x100(r1)              ; Load from TLS
```

### 4.2 Thread ID Registers

```
$tid (CP0 reg 30)               ; Thread ID (read-only in user mode)
```

```asm
MFTID rd                        ; Move from thread ID
                                ; rd = $tid
```

---

## 5. Processor Identification

### 5.1 Implementation Registers

**New CP0 Registers**:
```
$implname  (CP0 reg 15, sel 0)  ; Implementation name (64-bit ID)
$vendor    (CP0 reg 15, sel 1)  ; Vendor identifier
$revision  (CP0 reg 15, sel 2)  ; Revision number
$extflags0 (CP0 reg 15, sel 3)  ; Extension flags 0
$extflags1 (CP0 reg 15, sel 4)  ; Extension flags 1
$extflags2 (CP0 reg 15, sel 5)  ; Extension flags 2
$extflags3 (CP0 reg 15, sel 6)  ; Extension flags 3
```

### 5.2 Implementation Name ($implname)

64-bit register encoding implementation string:
```
Bits 63-56: Architecture version major
Bits 55-48: Architecture version minor
Bits 47-0:  Implementation-specific ID
```

**Example Values**:
```
"DLXSIM01" - DLX Simulator version 1
"DLXV200A" - DLX VLSI version 2.00 rev A
"FPGADLX1" - FPGA DLX implementation 1
```

### 5.3 Vendor Identifier ($vendor)

32-bit vendor code:
```
0x00000000: Generic/Unknown
0x41505043: "APPC" - Apple Computer
0x4D495053: "MIPS" - MIPS Technologies
0x41524D20: "ARM " - ARM Holdings
0x494E5443: "INTC" - Intel Corporation
0x414D4420: "AMD " - AMD
0x52495356: "RISV" - RISC-V Foundation
```

### 5.4 Revision Number ($revision)

```
Bits 31-24: Major revision
Bits 23-16: Minor revision
Bits 15-8:  Patch level
Bits 7-0:   Build number
```

**Example**: `0x01020304` = version 1.2.3 build 4

### 5.5 Extension Flags ($extflags0-3)

**$extflags0** (Base Extensions):
```
Bit 0:  DLX64 (64-bit architecture)
Bit 1:  DLX-C (Compressed instructions)
Bit 2:  DLX-H (Hypervisor extensions)
Bit 3:  DLX-V (Vector extensions)
Bit 4:  DLX-B (Bit manipulation)
Bit 5:  DLX-AI (AI/ML extensions)
Bit 6:  DLX-S (System extensions)
Bit 7:  FPU (Floating-point unit)
Bit 8:  MMU (Memory management unit)
Bit 9:  TLB (Translation lookaside buffer)
Bit 10: BE (Configurable endianness)
Bit 11: CARRY (Carry/overflow flags)
Bit 12: ATOMIC (Advanced atomics)
Bit 13: MWAIT (Monitor/wait power management)
Bit 14: CLMUL (Carry-less multiply)
Bit 15: THREADPTR (Thread pointer registers)
Bits 31-16: Reserved
```

**$extflags1** (Vector Capabilities):
```
Bits 3-0:   Max vector width (0=128, 1=256, 2=512, 3=1024, 4=2048 bits)
Bit 4:      Predicate registers
Bit 5:      Tile/matrix registers
Bit 6:      Gather/scatter
Bit 7:      First-fault loads
Bits 15-8:  Reserved
Bits 31-16: Vector element types supported
```

**$extflags2** (AI/ML Capabilities):
```
Bit 0:  Matrix multiply
Bit 1:  Convolution
Bit 2:  Attention
Bit 3:  Activation functions
Bit 4:  Normalization
Bit 5:  Pooling
Bit 6:  Quantization (INT8)
Bit 7:  Mixed precision (FP16/BF16)
Bits 31-8: Reserved
```

**$extflags3** (Reserved for future extensions)

### 5.6 Reading Identification

```asm
CPUID  rd, selector              ; Read CPU ID register
                                 ; rd = CPx[15, selector]
                                 ; No privilege required
```

**Example - Feature Detection**:
```asm
; Check if DLX-V vector extensions are available
cpuid r1, 3                     ; Read $extflags0
andi  r2, r1, 0x0008            ; Test bit 3 (DLX-V)
beqz  r2, no_vectors

; Vector code
...

no_vectors:
; Scalar fallback
...
```

---

## 6. Privilege Checking

### 6.1 Privileged Instructions

**Kernel-Only Instructions** (trap in user mode):
```
Power Management: MWAIT, HALT, WFI, WFE
System Control:   MOVI2S (write Status), RFE, ERET
TLB:             TLBR, TLBWI, TLBWR, TLBP, TLBINV
Cache:           CACHE (all variants)
Hypervisor:      HYPCALL, MFGC0, MTGC0, TLBG*
Kernel TP Write: MTTP kernel
```

**User-Allowed Instructions**:
```
Atomics:         All atomic operations (LL/SC, CAS, FETCH*)
Bit Test:        TAS, TAC, BTS, BTR, BTC
Thread Pointer:  RDTP, MFTP user, MTTP user, MFTID
Identification:  CPUID, MFIMPL, MFVENDOR, etc.
Pause:           PAUSE (power hint)
```

### 6.2 Privilege Violation Exception

When user mode executes privileged instruction:
```
Exception Code: 11 (EXC_CPU - Coprocessor Unusable)
EPC:           Address of privileged instruction
Cause:         0x0000002C (privileged instruction attempt)

Handler should:
1. Log violation
2. Terminate process or deliver SIGILL
3. Never emulate privileged operation
```

**Example - Privilege Check in Instruction Decode**:
```c
void execute_instruction(cpu_t *cpu, uint32_t instr) {
    int opcode = OPCODE(instr);
    int user_mode = !(cpu->status & STATUS_KUC);

    switch (opcode) {
    case OP_MWAIT:
        if (user_mode) {
            dlx_exception(cpu, EXC_CPU);
            return;
        }
        // Execute MWAIT
        break;

    case OP_RDTP:
        // Allowed in user mode
        cpu->regs[RD(instr)] = user_mode ?
            cpu->tpuser : cpu->tpkernel;
        break;
    }
}
```

---

## 7. Carry-Less Multiplication (Enhanced)

### 7.1 Polynomial Multiplication

Already defined in DLX-B, but enhanced here:

```asm
CLMUL   rd, rs1, rs2            ; Carry-less multiply (low)
                                ; Polynomial multiply (GF(2))
                                ; rd = (rs1 * rs2)[31:0]

CLMULH  rd, rs1, rs2            ; Carry-less multiply (high)
                                ; rd = (rs1 * rs2)[63:32]

CLMULR  rd, rs1, rs2            ; Carry-less multiply (reversed)
                                ; rd = (rs1 * rs2)[63-i:32-i] for optimal CRC
```

### 7.2 CRC and Hash Acceleration

```asm
CRC32   rd, rs, poly            ; CRC-32 with polynomial
CRC32C  rd, rs                  ; CRC-32C (Castagnoli, 0x1EDC6F41)
CRC32K  rd, rs                  ; CRC-32K (Koopman, 0x741B8CD7)
```

**Example - CRC-32C Calculation**:
```asm
; Calculate CRC-32C of buffer
li    r1, 0xFFFFFFFF            ; Initial CRC value

loop:
  lbu   r2, (r10)               ; Load byte
  xor   r1, r1, r2              ; XOR with CRC
  crc32c r1, r1                 ; Update CRC
  addi  r10, r10, 1             ; Next byte
  subi  r11, r11, 1             ; Decrement count
  bnez  r11, loop

xori  r1, r1, 0xFFFFFFFF        ; Final XOR
; r1 now contains CRC-32C
```

---

## 8. Memory Ordering and Barriers

### 8.1 Memory Barriers

```asm
SYNC                            ; Full memory barrier
                                ; All loads/stores before complete
                                ; before loads/stores after

LWSYNC                          ; Light-weight sync
                                ; Orders loads and stores, but allows
                                ; store-load reordering

DMB    domain, type             ; Data memory barrier
                                ; domain: SY(system), ISH(inner shareable)
                                ; type: LD, ST, LDST

DSB    domain, type             ; Data synchronization barrier

ISB                             ; Instruction synchronization barrier
```

**Example - Lock Release with Barrier**:
```asm
; Release lock with proper ordering
sw    r1, shared_data(r0)       ; Update shared data
sync                            ; Ensure visible before lock release
sw    r0, lock(r0)              ; Release lock
```

---

## 9. Complete System Register Map

### CP0 Register Summary

| Reg | Sel | Name | Access | Description |
|-----|-----|------|--------|-------------|
| 0 | 0 | Index | R/W | TLB entry index |
| 1 | 0 | Random | R | TLB random index |
| 2 | 0 | EntryLo0 | R/W | TLB entry low 0 |
| 3 | 0 | EntryLo1 | R/W | TLB entry low 1 |
| 4 | 0 | Context | R/W | TLB context |
| 5 | 0 | PageMask | R/W | TLB page mask |
| 6 | 0 | Wired | R/W | TLB wired entries |
| 8 | 0 | BadVAddr | R | Bad virtual address |
| 9 | 0 | Count | R/W | Timer count |
| 10 | 0 | EntryHi | R/W | TLB entry high |
| 11 | 0 | Compare | R/W | Timer compare |
| 12 | 0 | Status | R/W | Processor status |
| 13 | 0 | Cause | R | Exception cause |
| 14 | 0 | EPC | R/W | Exception PC |
| 15 | 0 | ImplName | R | Implementation name |
| 15 | 1 | Vendor | R | Vendor ID |
| 15 | 2 | Revision | R | Revision number |
| 15 | 3 | ExtFlags0 | R | Extension flags 0 |
| 15 | 4 | ExtFlags1 | R | Extension flags 1 |
| 15 | 5 | ExtFlags2 | R | Extension flags 2 |
| 15 | 6 | ExtFlags3 | R | Extension flags 3 |
| 16 | 0 | Config | R/W | Configuration |
| 29 | 0 | TPUser | R/W(U) | User thread pointer |
| 29 | 1 | TPKernel | R/W(K) | Kernel thread pointer |
| 30 | 0 | TID | R | Thread ID |

**Access**: R=Read, W=Write, K=Kernel only, U=User allowed

---

## 10. Code Examples

### Example 1: Atomic Queue Implementation

```asm
; Lock-free MPSC queue using CAS

; Producer: Enqueue item in r1
enqueue:
  lw   r2, tail_ptr             ; Load current tail
  sw   r2, NEXT(r1)             ; new->next = tail

enqueue_retry:
  cas  r3, r2, r1, tail_ptr     ; CAS(tail, old, new)
  beqz r3, enqueue_retry        ; Retry if failed
  ; Item enqueued

; Consumer: Dequeue to r1
dequeue:
  ll   r1, head_ptr             ; Load-linked head
  beqz r1, queue_empty          ; Check if empty
  lw   r2, NEXT(r1)             ; Get next
  sc   r3, r2, head_ptr         ; Store-conditional
  beqz r3, dequeue              ; Retry if failed
  ; r1 contains dequeued item

queue_empty:
  li   r1, 0
```

### Example 2: Power-Efficient Event Wait

```asm
; Wait for event with power management

wait_for_event:
  monitor event_flag, r0, r0    ; Monitor event flag

check_event:
  lw   r1, event_flag           ; Check flag
  bnez r1, event_occurred       ; Handle if set

  li   r2, 0x3                  ; C3 sleep state
  mwait r2, r0                  ; Sleep until write
  j    check_event              ; Check again

event_occurred:
  ; Process event
  sw   r0, event_flag           ; Clear flag
```

### Example 3: Multi-Precision Arithmetic

```asm
; 256-bit addition with overflow detection
; r1-r8 = r9-r16 + r17-r24

  clrc                          ; Clear carry

  add  r8, r16, r24             ; Word 0
  addc r7, r15, r23             ; Word 1 + carry
  addc r6, r14, r22             ; Word 2 + carry
  addc r5, r13, r21             ; Word 3 + carry
  addc r4, r12, r20             ; Word 4 + carry
  addc r3, r11, r19             ; Word 5 + carry
  addc r2, r10, r18             ; Word 6 + carry
  addc r1, r9, r17              ; Word 7 + carry

  ; Check final carry
  bcs  overflow_occurred
```

---

## 11. Summary

DLX-S adds **50+ system-level instructions**:

1. **Power Management**: MONITOR, MWAIT, HALT, PAUSE, WFI, WFE (6 ops)
2. **Atomics**: LL/SC, CAS, FETCH*, SWAP, TAS/TAC, BTS/BTR/BTC (15 ops)
3. **Carry/Overflow**: ADDC, SUBC, ADDO, ADDV, flag operations (10 ops)
4. **Thread Pointers**: RDTP, MFTP, MTTP, MFTID (4 ops)
5. **Identification**: CPUID (1 op, 7 registers)
6. **CRC**: CRC32, CRC32C, CRC32K, CLMUL variants (6 ops)
7. **Barriers**: SYNC, LWSYNC, DMB, DSB, ISB (5 ops)

**Total DLX Complete**: 350+ instructions across all extensions

**Key Features**:
✅ Intel-style power management
✅ Comprehensive atomic operations
✅ M88K-style carry/overflow handling
✅ Thread-local storage support
✅ Full processor identification
✅ Privilege checking enforcement
✅ Lock-free algorithm support

**Use Cases**:
- Operating systems (power management, synchronization)
- Multi-threaded applications (atomics, thread pointers)
- Lock-free data structures (LL/SC, CAS)
- High-reliability systems (overflow detection)
- Runtime introspection (CPUID)

---

**Document Version**: 1.0
**Last Updated**: October 2024
**Status**: Architecture Specification Complete
