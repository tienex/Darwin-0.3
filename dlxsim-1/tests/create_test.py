#!/usr/bin/env python3
# Create test binaries for DLXSIM

import struct

def write_instruction(f, opcode, rs1, rd, imm):
    """Write an I-type instruction"""
    instr = (opcode << 26) | (rs1 << 21) | (rd << 16) | (imm & 0xFFFF)
    f.write(struct.pack('>I', instr))

def write_rtype(f, rs1, rs2, rd, func):
    """Write an R-type instruction"""
    instr = (0 << 26) | (rs1 << 21) | (rs2 << 16) | (rd << 11) | (func & 0x7FF)
    f.write(struct.pack('>I', instr))

# Test 1: Return 42
with open('tests/test42.bin', 'wb') as f:
    # addi r1, r0, #42
    write_instruction(f, 0x08, 0, 1, 42)
    # sw r0, 0xFFF00004(r0) - halt simulator
    write_instruction(f, 0x2B, 0, 0, 0xF004)

print("Created tests/test42.bin (8 bytes)")

# Test 2: Arithmetic
with open('tests/test_arith.bin', 'wb') as f:
    # addi r1, r0, #10
    write_instruction(f, 0x08, 0, 1, 10)
    # addi r2, r0, #5
    write_instruction(f, 0x08, 0, 2, 5)
    # add r3, r1, r2  (r3 = 15)
    write_rtype(f, 1, 2, 3, 0x020)
    # sub r4, r1, r2  (r4 = 5)
    write_rtype(f, 1, 2, 4, 0x022)
    # mult r5, r1, r2 (r5 = 50)
    write_rtype(f, 1, 2, 5, 0x060)
    # halt
    write_instruction(f, 0x2B, 0, 0, 0xF004)

print("Created tests/test_arith.bin (24 bytes)")

# Test 3: Memory load/store
with open('tests/test_mem.bin', 'wb') as f:
    # addi r1, r0, #100
    write_instruction(f, 0x08, 0, 1, 100)
    # addi r2, r0, #0x1000
    write_instruction(f, 0x08, 0, 2, 0x1000)
    # sw r1, 0(r2)  - Store 100 at 0x1000
    write_instruction(f, 0x2B, 2, 1, 0)
    # lw r3, 0(r2)  - Load back into r3
    write_instruction(f, 0x23, 2, 3, 0)
    # halt
    write_instruction(f, 0x2B, 0, 0, 0xF004)

print("Created tests/test_mem.bin (20 bytes)")

print("\nAll test binaries created successfully!")
print("\nRun tests with:")
print("  ./dlxsim -v -b tests/test42.bin")
print("  ./dlxsim -v -b tests/test_arith.bin")
print("  ./dlxsim -v -b tests/test_mem.bin")
