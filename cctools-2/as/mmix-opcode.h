/* mmix-opcode.h -- MMIX opcode table
   Copyright (c) 1999-2025 Apple Computer, Inc.
   
   MMIX Architecture Opcode Definitions
   Based on Donald Knuth's MMIX specification
   
   All MMIX instructions are 32 bits wide with format:
   OP X Y Z (8 bits each)
   
   Where:
   OP = opcode (256 possible opcodes)
   X = destination/first operand register
   Y = second operand register
   Z = third operand register or immediate value
*/

#ifndef _MMIX_OPCODE_H_
#define _MMIX_OPCODE_H_

/* MMIX architecture versions */
enum mmix_architecture {
	mmix_base = 0
};

/* Opcode structure */
struct mmix_opcode {
	const char *name;        /* Instruction mnemonic */
	unsigned char opcode;    /* 8-bit opcode */
	const char *args;        /* Argument format string */
	unsigned char flags;     /* Instruction flags */
	enum mmix_architecture architecture;
};

/* Instruction flags */
#define MMIX_F_BRANCH     0x01  /* Branch instruction */
#define MMIX_F_COND_BR    0x02  /* Conditional branch */
#define MMIX_F_JUMP       0x04  /* Jump/call instruction */
#define MMIX_F_LOAD       0x08  /* Load instruction */
#define MMIX_F_STORE      0x10  /* Store instruction */
#define MMIX_F_IMM        0x20  /* Has immediate variant */

/* Argument format codes:
   X = destination register $X
   Y = source register $Y  
   Z = source register $Z
   I = immediate value (YZ combined into 16-bit)
   B = branch offset (XYZ combined into 24-bit)
   @ = special register
*/

/* MMIX Opcodes (alphabetical by mnemonic) */
#define OP_TRAP        0x00  /* TRAP X,Y,Z - System trap */
#define OP_FCMP        0x01  /* FCMP $X,$Y,$Z - Floating compare */
#define OP_FUN         0x02  /* FUN $X,$Y,$Z - Floating unordered */
#define OP_FEQL        0x03  /* FEQL $X,$Y,$Z - Floating equal */
#define OP_FADD        0x04  /* FADD $X,$Y,$Z - Floating add */
#define OP_FIX         0x05  /* FIX $X,$Y - Float to integer */
#define OP_FSUB        0x06  /* FSUB $X,$Y,$Z - Floating subtract */
#define OP_FIXU        0x07  /* FIXU $X,$Y - Float to unsigned */

#define OP_FLOT        0x08  /* FLOT $X,$Y,$Z - Integer to float */
#define OP_FLOTI       0x09  /* FLOTI $X,$Y,Z - Integer to float immed */
#define OP_FLOTU       0x0A  /* FLOTU $X,$Y,$Z - Unsigned to float */
#define OP_FLOTUI      0x0B  /* FLOTUI $X,$Y,Z - Unsigned to float immed */
#define OP_SFLOT       0x0C  /* SFLOT $X,$Y,$Z - Short float */
#define OP_SFLOTI      0x0D  /* SFLOTI $X,$Y,Z - Short float immed */
#define OP_SFLOTU      0x0E  /* SFLOTU $X,$Y,$Z - Short float unsigned */
#define OP_SFLOTUI     0x0F  /* SFLOTUI $X,$Y,Z - Short float unsigned immed */

#define OP_FMUL        0x10  /* FMUL $X,$Y,$Z - Floating multiply */
#define OP_FCMPE       0x11  /* FCMPE $X,$Y,$Z - Floating compare epsilon */
#define OP_FUNE        0x12  /* FUNE $X,$Y,$Z - Floating unordered epsilon */
#define OP_FEQLE       0x13  /* FEQLE $X,$Y,$Z - Floating equal epsilon */
#define OP_FDIV        0x14  /* FDIV $X,$Y,$Z - Floating divide */
#define OP_FSQRT       0x15  /* FSQRT $X,$Y,$Z - Floating square root */
#define OP_FREM        0x16  /* FREM $X,$Y,$Z - Floating remainder */
#define OP_FINT        0x17  /* FINT $X,$Y,$Z - Floating to integer */

#define OP_MUL         0x18  /* MUL $X,$Y,$Z - Multiply */
#define OP_MULI        0x19  /* MULI $X,$Y,Z - Multiply immediate */
#define OP_MULU        0x1A  /* MULU $X,$Y,$Z - Multiply unsigned */
#define OP_MULUI       0x1B  /* MULUI $X,$Y,Z - Multiply unsigned immediate */
#define OP_DIV         0x1C  /* DIV $X,$Y,$Z - Divide */
#define OP_DIVI        0x1D  /* DIVI $X,$Y,Z - Divide immediate */
#define OP_DIVU        0x1E  /* DIVU $X,$Y,$Z - Divide unsigned */
#define OP_DIVUI       0x1F  /* DIVUI $X,$Y,Z - Divide unsigned immediate */

#define OP_ADD         0x20  /* ADD $X,$Y,$Z - Add */
#define OP_ADDI        0x21  /* ADDI $X,$Y,Z - Add immediate */
#define OP_ADDU        0x22  /* ADDU $X,$Y,$Z - Add unsigned */
#define OP_ADDUI       0x23  /* ADDUI $X,$Y,Z - Add unsigned immediate */
#define OP_SUB         0x24  /* SUB $X,$Y,$Z - Subtract */
#define OP_SUBI        0x25  /* SUBI $X,$Y,Z - Subtract immediate */
#define OP_SUBU        0x26  /* SUBU $X,$Y,$Z - Subtract unsigned */
#define OP_SUBUI       0x27  /* SUBUI $X,$Y,Z - Subtract unsigned immediate */

#define OP_2ADDU       0x28  /* 2ADDU $X,$Y,$Z - 2*Y + Z */
#define OP_2ADDUI      0x29  /* 2ADDUI $X,$Y,Z - 2*Y + Z immediate */
#define OP_4ADDU       0x2A  /* 4ADDU $X,$Y,$Z - 4*Y + Z */
#define OP_4ADDUI      0x2B  /* 4ADDUI $X,$Y,Z - 4*Y + Z immediate */
#define OP_8ADDU       0x2C  /* 8ADDU $X,$Y,$Z - 8*Y + Z */
#define OP_8ADDUI      0x2D  /* 8ADDUI $X,$Y,Z - 8*Y + Z immediate */
#define OP_16ADDU      0x2E  /* 16ADDU $X,$Y,$Z - 16*Y + Z */
#define OP_16ADDUI     0x2F  /* 16ADDUI $X,$Y,Z - 16*Y + Z immediate */

#define OP_CMP         0x30  /* CMP $X,$Y,$Z - Compare */
#define OP_CMPI        0x31  /* CMPI $X,$Y,Z - Compare immediate */
#define OP_CMPU        0x32  /* CMPU $X,$Y,$Z - Compare unsigned */
#define OP_CMPUI       0x33  /* CMPUI $X,$Y,Z - Compare unsigned immediate */
#define OP_NEG         0x34  /* NEG $X,$Y,$Z - Negate */
#define OP_NEGI        0x35  /* NEGI $X,$Y,Z - Negate immediate */
#define OP_NEGU        0x36  /* NEGU $X,$Y,$Z - Negate unsigned */
#define OP_NEGUI       0x37  /* NEGUI $X,$Y,Z - Negate unsigned immediate */

#define OP_SL          0x38  /* SL $X,$Y,$Z - Shift left */
#define OP_SLI         0x39  /* SLI $X,$Y,Z - Shift left immediate */
#define OP_SLU         0x3A  /* SLU $X,$Y,$Z - Shift left unsigned */
#define OP_SLUI        0x3B  /* SLUI $X,$Y,Z - Shift left unsigned immediate */
#define OP_SR          0x3C  /* SR $X,$Y,$Z - Shift right */
#define OP_SRI         0x3D  /* SRI $X,$Y,Z - Shift right immediate */
#define OP_SRU         0x3E  /* SRU $X,$Y,$Z - Shift right unsigned */
#define OP_SRUI        0x3F  /* SRUI $X,$Y,Z - Shift right unsigned immediate */

/* Branch instructions */
#define OP_BN          0x40  /* BN $X,@+4*YZ - Branch if negative */
#define OP_BNB         0x41  /* BNB $X,@+4*YZ - Branch if negative (backward) */
#define OP_BZ          0x42  /* BZ $X,@+4*YZ - Branch if zero */
#define OP_BZB         0x43  /* BZB $X,@+4*YZ - Branch if zero (backward) */
#define OP_BP          0x44  /* BP $X,@+4*YZ - Branch if positive */
#define OP_BPB         0x45  /* BPB $X,@+4*YZ - Branch if positive (backward) */
#define OP_BOD         0x46  /* BOD $X,@+4*YZ - Branch if odd */
#define OP_BODB        0x47  /* BODB $X,@+4*YZ - Branch if odd (backward) */

#define OP_BNN         0x48  /* BNN $X,@+4*YZ - Branch if non-negative */
#define OP_BNNB        0x49  /* BNNB $X,@+4*YZ - Branch if non-negative (backward) */
#define OP_BNZ         0x4A  /* BNZ $X,@+4*YZ - Branch if non-zero */
#define OP_BNZB        0x4B  /* BNZB $X,@+4*YZ - Branch if non-zero (backward) */
#define OP_BNP         0x4C  /* BNP $X,@+4*YZ - Branch if non-positive */
#define OP_BNPB        0x4D  /* BNPB $X,@+4*YZ - Branch if non-positive (backward) */
#define OP_BEV         0x4E  /* BEV $X,@+4*YZ - Branch if even */
#define OP_BEVB        0x4F  /* BEVB $X,@+4*YZ - Branch if even (backward) */

/* Probable branch instructions */
#define OP_PBN         0x50  /* PBN $X,@+4*YZ - Probable branch if negative */
#define OP_PBNB        0x51  /* PBNB $X,@+4*YZ - Probable branch if negative (backward) */
#define OP_PBZ         0x52  /* PBZ $X,@+4*YZ - Probable branch if zero */
#define OP_PBZB        0x53  /* PBZB $X,@+4*YZ - Probable branch if zero (backward) */
#define OP_PBP         0x54  /* PBP $X,@+4*YZ - Probable branch if positive */
#define OP_PBPB        0x55  /* PBPB $X,@+4*YZ - Probable branch if positive (backward) */
#define OP_PBOD        0x56  /* PBOD $X,@+4*YZ - Probable branch if odd */
#define OP_PBODB       0x57  /* PBODB $X,@+4*YZ - Probable branch if odd (backward) */

#define OP_PBNN        0x58  /* PBNN $X,@+4*YZ - Probable branch if non-negative */
#define OP_PBNNB       0x59  /* PBNNB $X,@+4*YZ - Probable branch if non-negative (backward) */
#define OP_PBNZ        0x5A  /* PBNZ $X,@+4*YZ - Probable branch if non-zero */
#define OP_PBNZB       0x5B  /* PBNZB $X,@+4*YZ - Probable branch if non-zero (backward) */
#define OP_PBNP        0x5C  /* PBNP $X,@+4*YZ - Probable branch if non-positive */
#define OP_PBNPB       0x5D  /* PBNPB $X,@+4*YZ - Probable branch if non-positive (backward) */
#define OP_PBEV        0x5E  /* PBEV $X,@+4*YZ - Probable branch if even */
#define OP_PBEVB       0x5F  /* PBEVB $X,@+4*YZ - Probable branch if even (backward) */

/* Bitwise operations */
#define OP_CSN         0x60  /* CSN $X,$Y,$Z - Conditional set if negative */
#define OP_CSNI        0x61  /* CSNI $X,$Y,Z - Conditional set if negative immediate */
#define OP_CSZ         0x62  /* CSZ $X,$Y,$Z - Conditional set if zero */
#define OP_CSZI        0x63  /* CSZI $X,$Y,Z - Conditional set if zero immediate */
#define OP_CSP         0x64  /* CSP $X,$Y,$Z - Conditional set if positive */
#define OP_CSPI        0x65  /* CSPI $X,$Y,Z - Conditional set if positive immediate */
#define OP_CSOD        0x66  /* CSOD $X,$Y,$Z - Conditional set if odd */
#define OP_CSODI       0x67  /* CSODI $X,$Y,Z - Conditional set if odd immediate */

#define OP_CSNN        0x68  /* CSNN $X,$Y,$Z - Conditional set if non-negative */
#define OP_CSNNI       0x69  /* CSNNI $X,$Y,Z - Conditional set if non-negative immediate */
#define OP_CSNZ        0x6A  /* CSNZ $X,$Y,$Z - Conditional set if non-zero */
#define OP_CSNZI       0x6B  /* CSNZI $X,$Y,Z - Conditional set if non-zero immediate */
#define OP_CSNP        0x6C  /* CSNP $X,$Y,$Z - Conditional set if non-positive */
#define OP_CSNPI       0x6D  /* CSNPI $X,$Y,Z - Conditional set if non-positive immediate */
#define OP_CSEV        0x6E  /* CSEV $X,$Y,$Z - Conditional set if even */
#define OP_CSEVI       0x6F  /* CSEVI $X,$Y,Z - Conditional set if even immediate */

#define OP_ZSN         0x70  /* ZSN $X,$Y,$Z - Zero or set if negative */
#define OP_ZSNI        0x71  /* ZSNI $X,$Y,Z - Zero or set if negative immediate */
#define OP_ZSZ         0x72  /* ZSZ $X,$Y,$Z - Zero or set if zero */
#define OP_ZSZI        0x73  /* ZSZI $X,$Y,Z - Zero or set if zero immediate */
#define OP_ZSP         0x74  /* ZSP $X,$Y,$Z - Zero or set if positive */
#define OP_ZSPI        0x75  /* ZSPI $X,$Y,Z - Zero or set if positive immediate */
#define OP_ZSOD        0x76  /* ZSOD $X,$Y,$Z - Zero or set if odd */
#define OP_ZSODI       0x77  /* ZSODI $X,$Y,Z - Zero or set if odd immediate */

#define OP_ZSNN        0x78  /* ZSNN $X,$Y,$Z - Zero or set if non-negative */
#define OP_ZSNNI       0x79  /* ZSNNI $X,$Y,Z - Zero or set if non-negative immediate */
#define OP_ZSNZ        0x7A  /* ZSNZ $X,$Y,$Z - Zero or set if non-zero */
#define OP_ZSNZI       0x7B  /* ZSNZI $X,$Y,Z - Zero or set if non-zero immediate */
#define OP_ZSNP        0x7C  /* ZSNP $X,$Y,$Z - Zero or set if non-positive */
#define OP_ZSNPI       0x7D  /* ZSNPI $X,$Y,Z - Zero or set if non-positive immediate */
#define OP_ZSEV        0x7E  /* ZSEV $X,$Y,$Z - Zero or set if even */
#define OP_ZSEVI       0x7F  /* ZSEVI $X,$Y,Z - Zero or set if even immediate */

/* Load instructions */
#define OP_LDB         0x80  /* LDB $X,$Y,$Z - Load byte */
#define OP_LDBI        0x81  /* LDBI $X,$Y,Z - Load byte immediate */
#define OP_LDBU        0x82  /* LDBU $X,$Y,$Z - Load byte unsigned */
#define OP_LDBUI       0x83  /* LDBUI $X,$Y,Z - Load byte unsigned immediate */
#define OP_LDW         0x84  /* LDW $X,$Y,$Z - Load wyde (2 bytes) */
#define OP_LDWI        0x85  /* LDWI $X,$Y,Z - Load wyde immediate */
#define OP_LDWU        0x86  /* LDWU $X,$Y,$Z - Load wyde unsigned */
#define OP_LDWUI       0x87  /* LDWUI $X,$Y,Z - Load wyde unsigned immediate */

#define OP_LDT         0x88  /* LDT $X,$Y,$Z - Load tetra (4 bytes) */
#define OP_LDTI        0x89  /* LDTI $X,$Y,Z - Load tetra immediate */
#define OP_LDTU        0x8A  /* LDTU $X,$Y,$Z - Load tetra unsigned */
#define OP_LDTUI       0x8B  /* LDTUI $X,$Y,Z - Load tetra unsigned immediate */
#define OP_LDO         0x8C  /* LDO $X,$Y,$Z - Load octa (8 bytes) */
#define OP_LDOI        0x8D  /* LDOI $X,$Y,Z - Load octa immediate */
#define OP_LDOU        0x8E  /* LDOU $X,$Y,$Z - Load octa unsigned */
#define OP_LDOUI       0x8F  /* LDOUI $X,$Y,Z - Load octa unsigned immediate */

#define OP_LDSF        0x90  /* LDSF $X,$Y,$Z - Load short float */
#define OP_LDSFI       0x91  /* LDSFI $X,$Y,Z - Load short float immediate */
#define OP_LDHT        0x92  /* LDHT $X,$Y,$Z - Load high tetra */
#define OP_LDHTI       0x93  /* LDHTI $X,$Y,Z - Load high tetra immediate */
#define OP_CSWAP       0x94  /* CSWAP $X,$Y,$Z - Compare and swap */
#define OP_CSWAPI      0x95  /* CSWAPI $X,$Y,Z - Compare and swap immediate */
#define OP_LDUNC       0x96  /* LDUNC $X,$Y,$Z - Load uncached */
#define OP_LDUNCI      0x97  /* LDUNCI $X,$Y,Z - Load uncached immediate */

#define OP_LDVTS       0x98  /* LDVTS $X,$Y,$Z - Load virtual translation status */
#define OP_LDVTSI      0x99  /* LDVTSI $X,$Y,Z - Load virtual translation status immediate */
#define OP_PRELD       0x9A  /* PRELD X,$Y,$Z - Preload data */
#define OP_PRELDI      0x9B  /* PRELDI X,$Y,Z - Preload data immediate */
#define OP_PREGO       0x9C  /* PREGO X,$Y,$Z - Prefetch to go */
#define OP_PREGOI      0x9D  /* PREGOI X,$Y,Z - Prefetch to go immediate */
#define OP_GO          0x9E  /* GO $X,$Y,$Z - Goto */
#define OP_GOI         0x9F  /* GOI $X,$Y,Z - Goto immediate */

/* Store instructions */
#define OP_STB         0xA0  /* STB $X,$Y,$Z - Store byte */
#define OP_STBI        0xA1  /* STBI $X,$Y,Z - Store byte immediate */
#define OP_STBU        0xA2  /* STBU $X,$Y,$Z - Store byte unsigned */
#define OP_STBUI       0xA3  /* STBUI $X,$Y,Z - Store byte unsigned immediate */
#define OP_STW         0xA4  /* STW $X,$Y,$Z - Store wyde */
#define OP_STWI        0xA5  /* STWI $X,$Y,Z - Store wyde immediate */
#define OP_STWU        0xA6  /* STWU $X,$Y,$Z - Store wyde unsigned */
#define OP_STWUI       0xA7  /* STWUI $X,$Y,Z - Store wyde unsigned immediate */

#define OP_STT         0xA8  /* STT $X,$Y,$Z - Store tetra */
#define OP_STTI        0xA9  /* STTI $X,$Y,Z - Store tetra immediate */
#define OP_STTU        0xAA  /* STTU $X,$Y,$Z - Store tetra unsigned */
#define OP_STTUI       0xAB  /* STTUI $X,$Y,Z - Store tetra unsigned immediate */
#define OP_STO         0xAC  /* STO $X,$Y,$Z - Store octa */
#define OP_STOI        0xAD  /* STOI $X,$Y,Z - Store octa immediate */
#define OP_STOU        0xAE  /* STOU $X,$Y,$Z - Store octa unsigned */
#define OP_STOUI       0xAF  /* STOUI $X,$Y,Z - Store octa unsigned immediate */

#define OP_STSF        0xB0  /* STSF $X,$Y,$Z - Store short float */
#define OP_STSFI       0xB1  /* STSFI $X,$Y,Z - Store short float immediate */
#define OP_STHT        0xB2  /* STHT $X,$Y,$Z - Store high tetra */
#define OP_STHTI       0xB3  /* STHTI $X,$Y,Z - Store high tetra immediate */
#define OP_STCO        0xB4  /* STCO X,$Y,$Z - Store constant octa */
#define OP_STCOI       0xB5  /* STCOI X,$Y,Z - Store constant octa immediate */
#define OP_STUNC       0xB6  /* STUNC $X,$Y,$Z - Store uncached */
#define OP_STUNCI      0xB7  /* STUNCI $X,$Y,Z - Store uncached immediate */

#define OP_SYNCD       0xB8  /* SYNCD X,$Y,$Z - Synchronize data */
#define OP_SYNCDI      0xB9  /* SYNCDI X,$Y,Z - Synchronize data immediate */
#define OP_PREST       0xBA  /* PREST X,$Y,$Z - Prestore */
#define OP_PRESTI      0xBB  /* PRESTI X,$Y,Z - Prestore immediate */
#define OP_SYNCID      0xBC  /* SYNCID X,$Y,$Z - Synchronize instruction */
#define OP_SYNCIDI     0xBD  /* SYNCIDI X,$Y,Z - Synchronize instruction immediate */
#define OP_PUSHGO      0xBE  /* PUSHGO $X,$Y,$Z - Push registers and goto */
#define OP_PUSHGOI     0xBF  /* PUSHGOI $X,$Y,Z - Push registers and goto immediate */

/* Bitwise logical operations */
#define OP_OR          0xC0  /* OR $X,$Y,$Z - Bitwise OR */
#define OP_ORI         0xC1  /* ORI $X,$Y,Z - Bitwise OR immediate */
#define OP_ORN         0xC2  /* ORN $X,$Y,$Z - Bitwise OR-NOT */
#define OP_ORNI        0xC3  /* ORNI $X,$Y,Z - Bitwise OR-NOT immediate */
#define OP_NOR         0xC4  /* NOR $X,$Y,$Z - Bitwise NOR */
#define OP_NORI        0xC5  /* NORI $X,$Y,Z - Bitwise NOR immediate */
#define OP_XOR         0xC6  /* XOR $X,$Y,$Z - Bitwise XOR */
#define OP_XORI        0xC7  /* XORI $X,$Y,Z - Bitwise XOR immediate */

#define OP_AND         0xC8  /* AND $X,$Y,$Z - Bitwise AND */
#define OP_ANDI        0xC9  /* ANDI $X,$Y,Z - Bitwise AND immediate */
#define OP_ANDN        0xCA  /* ANDN $X,$Y,$Z - Bitwise AND-NOT */
#define OP_ANDNI       0xCB  /* ANDNI $X,$Y,Z - Bitwise AND-NOT immediate */
#define OP_NAND        0xCC  /* NAND $X,$Y,$Z - Bitwise NAND */
#define OP_NANDI       0xCD  /* NANDI $X,$Y,Z - Bitwise NAND immediate */
#define OP_NXOR        0xCE  /* NXOR $X,$Y,$Z - Bitwise NXOR */
#define OP_NXORI       0xCF  /* NXORI $X,$Y,Z - Bitwise NXOR immediate */

#define OP_BDIF        0xD0  /* BDIF $X,$Y,$Z - Byte difference */
#define OP_BDIFI       0xD1  /* BDIFI $X,$Y,Z - Byte difference immediate */
#define OP_WDIF        0xD2  /* WDIF $X,$Y,$Z - Wyde difference */
#define OP_WDIFI       0xD3  /* WDIFI $X,$Y,Z - Wyde difference immediate */
#define OP_TDIF        0xD4  /* TDIF $X,$Y,$Z - Tetra difference */
#define OP_TDIFI       0xD5  /* TDIFI $X,$Y,Z - Tetra difference immediate */
#define OP_ODIF        0xD6  /* ODIF $X,$Y,$Z - Octa difference */
#define OP_ODIFI       0xD7  /* ODIFI $X,$Y,Z - Octa difference immediate */

#define OP_MUX         0xD8  /* MUX $X,$Y,$Z - Multiplex */
#define OP_MUXI        0xD9  /* MUXI $X,$Y,Z - Multiplex immediate */
#define OP_SADD        0xDA  /* SADD $X,$Y,$Z - Sideways add */
#define OP_SADDI       0xDB  /* SADDI $X,$Y,Z - Sideways add immediate */
#define OP_MOR         0xDC  /* MOR $X,$Y,$Z - Multiple OR */
#define OP_MORI        0xDD  /* MORI $X,$Y,Z - Multiple OR immediate */
#define OP_MXOR        0xDE  /* MXOR $X,$Y,$Z - Multiple XOR */
#define OP_MXORI       0xDF  /* MXORI $X,$Y,Z - Multiple XOR immediate */

/* High multiplication and special operations */
#define OP_SETH        0xE0  /* SETH $X,YZ - Set high wyde */
#define OP_SETMH       0xE1  /* SETMH $X,YZ - Set medium high wyde */
#define OP_SETML       0xE2  /* SETML $X,YZ - Set medium low wyde */
#define OP_SETL        0xE3  /* SETL $X,YZ - Set low wyde */
#define OP_INCH        0xE4  /* INCH $X,YZ - Increase by high wyde */
#define OP_INCMH       0xE5  /* INCMH $X,YZ - Increase by medium high wyde */
#define OP_INCML       0xE6  /* INCML $X,YZ - Increase by medium low wyde */
#define OP_INCL        0xE7  /* INCL $X,YZ - Increase by low wyde */

#define OP_ORH         0xE8  /* ORH $X,YZ - OR with high wyde */
#define OP_ORMH        0xE9  /* ORMH $X,YZ - OR with medium high wyde */
#define OP_ORML        0xEA  /* ORML $X,YZ - OR with medium low wyde */
#define OP_ORL         0xEB  /* ORL $X,YZ - OR with low wyde */
#define OP_ANDNH       0xEC  /* ANDNH $X,YZ - AND-NOT with high wyde */
#define OP_ANDNMH      0xED  /* ANDNMH $X,YZ - AND-NOT with medium high wyde */
#define OP_ANDNML      0xEE  /* ANDNML $X,YZ - AND-NOT with medium low wyde */
#define OP_ANDNL       0xEF  /* ANDNL $X,YZ - AND-NOT with low wyde */

/* Jump and special register operations */
#define OP_JMP         0xF0  /* JMP @+4*XYZ - Unconditional jump */
#define OP_JMPB        0xF1  /* JMPB @+4*XYZ - Unconditional jump backward */
#define OP_PUSHJ       0xF2  /* PUSHJ $X,@+4*YZ - Push and jump */
#define OP_PUSHJB      0xF3  /* PUSHJB $X,@+4*YZ - Push and jump backward */
#define OP_GETA        0xF4  /* GETA $X,@+4*YZ - Get address */
#define OP_GETAB       0xF5  /* GETAB $X,@+4*YZ - Get address backward */
#define OP_PUT         0xF6  /* PUT $Z,$Y - Put to special register */
#define OP_PUTI        0xF7  /* PUTI $Z,Y - Put immediate to special register */

#define OP_POP         0xF8  /* POP X,YZ - Pop registers and return */
#define OP_RESUME      0xF9  /* RESUME XYZ - Resume after interrupt */
#define OP_SAVE        0xFA  /* SAVE $X,0 - Save process state */
#define OP_UNSAVE      0xFB  /* UNSAVE $Z,$Y - Restore process state */
#define OP_SYNC        0xFC  /* SYNC XYZ - Synchronize */
#define OP_SWYM        0xFD  /* SWYM X,Y,Z - Sympathize with your machinery (nop) */
#define OP_GET         0xFE  /* GET $X,$Z - Get from special register */
#define OP_TRIP        0xFF  /* TRIP X,Y,Z - Trip interrupt */

/* Opcode table */
extern const struct mmix_opcode mmix_opcodes[];
extern const int mmix_num_opcodes;

#endif /* _MMIX_OPCODE_H_ */
