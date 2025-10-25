# Simple test program for DLXSIM
# Prints "Hello!" to console and exits

# This is hand-assembled DLX code in hexadecimal
# Each line is one 32-bit instruction

# Start at address 0x0000

# r1 = 'H' (0x48)
# addi r1, r0, #72
20010048

# Write to console (sw r1, 0xFFF00000(r0))
# sw r1, -4096(r0)  - Using negative offset
2B01F000

# r1 = 'e' (0x65)
# addi r1, r0, #101
20010065

# Write to console
2B01F000

# r1 = 'l' (0x6C)
# addi r1, r0, #108
2001006C

# Write to console
2B01F000

# Write again (double 'l')
2B01F000

# r1 = 'o' (0x6F)
# addi r1, r0, #111
2001006F

# Write to console
2B01F000

# r1 = '!' (0x21)
# addi r1, r0, #33
20010021

# Write to console
2B01F000

# r1 = '\n' (0x0A)
# addi r1, r0, #10
2001000A

# Write to console
2B01F000

# Halt: sw r0, 0xFFF00004(r0)
2B00F004

# trap #0 (alternative halt)
44000000
