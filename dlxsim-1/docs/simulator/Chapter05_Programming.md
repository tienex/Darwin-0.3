# Chapter 5: Programming for DLXSIM

## 5.1 Simple Assembly Program

```asm
        ; Hello World for DLX
        .org 0x0000
start:
        addi r1, r0, 'H'    ; Load 'H'
        sw   r1, 0(r0)      ; Write to console (0xFFF00000)
        addi r1, r0, 'i'    ; Load 'i'
        sw   r1, 0(r0)      ; Write to console
        trap #0             ; Exit
```

## 5.2 System Calls

### Write to Console

```asm
addi r1, r0, 65         ; ASCII 'A'
sw   r1, 0xFFF00000(r0) ; Write to console
```

### Halt Simulation

```asm
sw   r0, 0xFFF00004(r0) ; Write 0 to control register
```

### Read Timer

```asm
lw   r1, 0xFFF00010(r0) ; Read cycle count
```

## 5.3 Function Calling Convention

### Register Usage

- **Arguments**: r1-r8 (first 8 arguments)
- **Return value**: r1
- **Saved registers**: r9-r28 (callee must save)
- **Temporary registers**: r1-r8 (caller must save if needed)
- **Return address**: r31

### Example Function

```asm
factorial:
        subi r29, r29, 16   ; Allocate stack frame
        sw   r31, 12(r29)   ; Save return address
        sw   r9, 8(r29)     ; Save r9

        add  r9, r0, r1     ; Save n in r9
        slti r2, r1, 2      ; if (n < 2)
        beqz r2, recurse

        addi r1, r0, 1      ; return 1
        j    done

recurse:
        subi r1, r9, 1      ; n - 1
        jal  factorial      ; factorial(n-1)
        mult r1, r9, r1     ; n * factorial(n-1)

done:
        lw   r9, 8(r29)     ; Restore r9
        lw   r31, 12(r29)   ; Restore return address
        addi r29, r29, 16   ; Deallocate stack
        jr   r31            ; Return
```

## 5.4 Memory-Mapped I/O

### Console Output (0xFFF00000)

Writing a byte to this address outputs it to the console.

### Simulator Control (0xFFF00004)

Writing to this address controls the simulator:
- `0` - Halt simulation

### Timer (0xFFF00010)

Reading from this address returns the current cycle count.

### Keyboard (0xFFF00100-0xFFF00107)

- `0xFFF00100` - Keyboard data
- `0xFFF00104` - Keyboard status
