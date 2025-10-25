/*
 * Copyright (c) 1999-2025 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * MMIX Disassembler - Complete Implementation
 * 
 * Full disassembler for Donald Knuth's MMIX architecture with all 256 opcodes.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
#include <stdio.h>
#include <string.h>
#include <mach-o/loader.h>
#include <mach-o/nlist.h>
#include <mach-o/reloc.h>
#include <mach-o/mmix/reloc.h>
#include "stuff/bytesex.h"
#include "otool.h"
#include "ofile_print.h"

/* MMIX instruction field extraction macros */
#define MMIX_OP(insn)    (((insn) >> 24) & 0xFF)
#define MMIX_X(insn)     (((insn) >> 16) & 0xFF)
#define MMIX_Y(insn)     (((insn) >> 8) & 0xFF)
#define MMIX_Z(insn)     ((insn) & 0xFF)
#define MMIX_YZ(insn)    ((insn) & 0xFFFF)
#define MMIX_XYZ(insn)   ((insn) & 0xFFFFFF)

/* Instruction format types */
typedef enum {
    FMT_XYZ,      /* Three register operands: $X,$Y,$Z */
    FMT_XYI,      /* Register, register, immediate: $X,$Y,Z */
    FMT_XI,       /* Register, 16-bit immediate: $X,YZ */
    FMT_BRANCH,   /* Branch: $X,offset (signed YZ) */
    FMT_JUMP,     /* Jump: offset (signed XYZ) */
    FMT_JUMPX,    /* Jump with register: $X,offset (signed YZ) */
    FMT_SPECIAL,  /* Special formats (GET/PUT/etc) */
    FMT_NONE      /* No operands */
} mmix_fmt_t;

/* Opcode table entry */
struct mmix_opcode_entry {
    const char *name;
    mmix_fmt_t format;
};

/* Complete MMIX opcode table - all 256 instructions */
static const struct mmix_opcode_entry mmix_opcodes[256] = {
    /* 0x00-0x0F: Floating point and conversion */
    [0x00] = {"trap",    FMT_XYZ},     [0x01] = {"fcmp",    FMT_XYZ},
    [0x02] = {"fun",     FMT_XYZ},     [0x03] = {"feql",    FMT_XYZ},
    [0x04] = {"fadd",    FMT_XYZ},     [0x05] = {"fix",     FMT_XYZ},
    [0x06] = {"fsub",    FMT_XYZ},     [0x07] = {"fixu",    FMT_XYZ},
    [0x08] = {"flot",    FMT_XYZ},     [0x09] = {"floti",   FMT_XYI},
    [0x0A] = {"flotu",   FMT_XYZ},     [0x0B] = {"flotui",  FMT_XYI},
    [0x0C] = {"sflot",   FMT_XYZ},     [0x0D] = {"sfloti",  FMT_XYI},
    [0x0E] = {"sflotu",  FMT_XYZ},     [0x0F] = {"sflotui", FMT_XYI},
    
    /* 0x10-0x1F: Floating multiply/divide, integer multiply/divide */
    [0x10] = {"fmul",    FMT_XYZ},     [0x11] = {"fcmpe",   FMT_XYZ},
    [0x12] = {"fune",    FMT_XYZ},     [0x13] = {"feqle",   FMT_XYZ},
    [0x14] = {"fdiv",    FMT_XYZ},     [0x15] = {"fsqrt",   FMT_XYZ},
    [0x16] = {"frem",    FMT_XYZ},     [0x17] = {"fint",    FMT_XYZ},
    [0x18] = {"mul",     FMT_XYZ},     [0x19] = {"muli",    FMT_XYI},
    [0x1A] = {"mulu",    FMT_XYZ},     [0x1B] = {"mului",   FMT_XYI},
    [0x1C] = {"div",     FMT_XYZ},     [0x1D] = {"divi",    FMT_XYI},
    [0x1E] = {"divu",    FMT_XYZ},     [0x1F] = {"divui",   FMT_XYI},
    
    /* 0x20-0x2F: Add/subtract and scaled add */
    [0x20] = {"add",     FMT_XYZ},     [0x21] = {"addi",    FMT_XYI},
    [0x22] = {"addu",    FMT_XYZ},     [0x23] = {"addui",   FMT_XYI},
    [0x24] = {"sub",     FMT_XYZ},     [0x25] = {"subi",    FMT_XYI},
    [0x26] = {"subu",    FMT_XYZ},     [0x27] = {"subui",   FMT_XYI},
    [0x28] = {"2addu",   FMT_XYZ},     [0x29] = {"2addui",  FMT_XYI},
    [0x2A] = {"4addu",   FMT_XYZ},     [0x2B] = {"4addui",  FMT_XYI},
    [0x2C] = {"8addu",   FMT_XYZ},     [0x2D] = {"8addui",  FMT_XYI},
    [0x2E] = {"16addu",  FMT_XYZ},     [0x2F] = {"16addui", FMT_XYI},
    
    /* 0x30-0x3F: Compare, negate, shift */
    [0x30] = {"cmp",     FMT_XYZ},     [0x31] = {"cmpi",    FMT_XYI},
    [0x32] = {"cmpu",    FMT_XYZ},     [0x33] = {"cmpui",   FMT_XYI},
    [0x34] = {"neg",     FMT_XYZ},     [0x35] = {"negi",    FMT_XYI},
    [0x36] = {"negu",    FMT_XYZ},     [0x37] = {"negui",   FMT_XYI},
    [0x38] = {"sl",      FMT_XYZ},     [0x39] = {"sli",     FMT_XYI},
    [0x3A] = {"slu",     FMT_XYZ},     [0x3B] = {"slui",    FMT_XYI},
    [0x3C] = {"sr",      FMT_XYZ},     [0x3D] = {"sri",     FMT_XYI},
    [0x3E] = {"sru",     FMT_XYZ},     [0x3F] = {"srui",    FMT_XYI},
    
    /* 0x40-0x4F: Conditional branches (forward and backward) */
    [0x40] = {"bn",      FMT_BRANCH},  [0x41] = {"bnb",     FMT_BRANCH},
    [0x42] = {"bz",      FMT_BRANCH},  [0x43] = {"bzb",     FMT_BRANCH},
    [0x44] = {"bp",      FMT_BRANCH},  [0x45] = {"bpb",     FMT_BRANCH},
    [0x46] = {"bod",     FMT_BRANCH},  [0x47] = {"bodb",    FMT_BRANCH},
    [0x48] = {"bnn",     FMT_BRANCH},  [0x49] = {"bnnb",    FMT_BRANCH},
    [0x4A] = {"bnz",     FMT_BRANCH},  [0x4B] = {"bnzb",    FMT_BRANCH},
    [0x4C] = {"bnp",     FMT_BRANCH},  [0x4D] = {"bnpb",    FMT_BRANCH},
    [0x4E] = {"bev",     FMT_BRANCH},  [0x4F] = {"bevb",    FMT_BRANCH},
    
    /* 0x50-0x5F: Probable branches */
    [0x50] = {"pbn",     FMT_BRANCH},  [0x51] = {"pbnb",    FMT_BRANCH},
    [0x52] = {"pbz",     FMT_BRANCH},  [0x53] = {"pbzb",    FMT_BRANCH},
    [0x54] = {"pbp",     FMT_BRANCH},  [0x55] = {"pbpb",    FMT_BRANCH},
    [0x56] = {"pbod",    FMT_BRANCH},  [0x57] = {"pbodb",   FMT_BRANCH},
    [0x58] = {"pbnn",    FMT_BRANCH},  [0x59] = {"pbnnb",   FMT_BRANCH},
    [0x5A] = {"pbnz",    FMT_BRANCH},  [0x5B] = {"pbnzb",   FMT_BRANCH},
    [0x5C] = {"pbnp",    FMT_BRANCH},  [0x5D] = {"pbnpb",   FMT_BRANCH},
    [0x5E] = {"pbev",    FMT_BRANCH},  [0x5F] = {"pbevb",   FMT_BRANCH},
    
    /* 0x60-0x6F: Conditional set operations */
    [0x60] = {"csn",     FMT_XYZ},     [0x61] = {"csni",    FMT_XYI},
    [0x62] = {"csz",     FMT_XYZ},     [0x63] = {"cszi",    FMT_XYI},
    [0x64] = {"csp",     FMT_XYZ},     [0x65] = {"cspi",    FMT_XYI},
    [0x66] = {"csod",    FMT_XYZ},     [0x67] = {"csodi",   FMT_XYI},
    [0x68] = {"csnn",    FMT_XYZ},     [0x69] = {"csnni",   FMT_XYI},
    [0x6A] = {"csnz",    FMT_XYZ},     [0x6B] = {"csnzi",   FMT_XYI},
    [0x6C] = {"csnp",    FMT_XYZ},     [0x6D] = {"csnpi",   FMT_XYI},
    [0x6E] = {"csev",    FMT_XYZ},     [0x6F] = {"csevi",   FMT_XYI},
    
    /* 0x70-0x7F: Zero or set operations */
    [0x70] = {"zsn",     FMT_XYZ},     [0x71] = {"zsni",    FMT_XYI},
    [0x72] = {"zsz",     FMT_XYZ},     [0x73] = {"zszi",    FMT_XYI},
    [0x74] = {"zsp",     FMT_XYZ},     [0x75] = {"zspi",    FMT_XYI},
    [0x76] = {"zsod",    FMT_XYZ},     [0x77] = {"zsodi",   FMT_XYI},
    [0x78] = {"zsnn",    FMT_XYZ},     [0x79] = {"zsnni",   FMT_XYI},
    [0x7A] = {"zsnz",    FMT_XYZ},     [0x7B] = {"zsnzi",   FMT_XYI},
    [0x7C] = {"zsnp",    FMT_XYZ},     [0x7D] = {"zsnpi",   FMT_XYI},
    [0x7E] = {"zsev",    FMT_XYZ},     [0x7F] = {"zsevi",   FMT_XYI},
    
    /* 0x80-0x8F: Load byte, wyde, tetra, octa */
    [0x80] = {"ldb",     FMT_XYZ},     [0x81] = {"ldbi",    FMT_XYI},
    [0x82] = {"ldbu",    FMT_XYZ},     [0x83] = {"ldbui",   FMT_XYI},
    [0x84] = {"ldw",     FMT_XYZ},     [0x85] = {"ldwi",    FMT_XYI},
    [0x86] = {"ldwu",    FMT_XYZ},     [0x87] = {"ldwui",   FMT_XYI},
    [0x88] = {"ldt",     FMT_XYZ},     [0x89] = {"ldti",    FMT_XYI},
    [0x8A] = {"ldtu",    FMT_XYZ},     [0x8B] = {"ldtui",   FMT_XYI},
    [0x8C] = {"ldo",     FMT_XYZ},     [0x8D] = {"ldoi",    FMT_XYI},
    [0x8E] = {"ldou",    FMT_XYZ},     [0x8F] = {"ldoui",   FMT_XYI},
    
    /* 0x90-0x9F: Load special, prefetch, goto */
    [0x90] = {"ldsf",    FMT_XYZ},     [0x91] = {"ldsfi",   FMT_XYI},
    [0x92] = {"ldht",    FMT_XYZ},     [0x93] = {"ldhti",   FMT_XYI},
    [0x94] = {"cswap",   FMT_XYZ},     [0x95] = {"cswapi",  FMT_XYI},
    [0x96] = {"ldunc",   FMT_XYZ},     [0x97] = {"ldunci",  FMT_XYI},
    [0x98] = {"ldvts",   FMT_XYZ},     [0x99] = {"ldvtsi",  FMT_XYI},
    [0x9A] = {"preld",   FMT_XYZ},     [0x9B] = {"preldi",  FMT_XYI},
    [0x9C] = {"prego",   FMT_XYZ},     [0x9D] = {"pregoi",  FMT_XYI},
    [0x9E] = {"go",      FMT_XYZ},     [0x9F] = {"goi",     FMT_XYI},
    
    /* 0xA0-0xAF: Store byte, wyde, tetra, octa */
    [0xA0] = {"stb",     FMT_XYZ},     [0xA1] = {"stbi",    FMT_XYI},
    [0xA2] = {"stbu",    FMT_XYZ},     [0xA3] = {"stbui",   FMT_XYI},
    [0xA4] = {"stw",     FMT_XYZ},     [0xA5] = {"stwi",    FMT_XYI},
    [0xA6] = {"stwu",    FMT_XYZ},     [0xA7] = {"stwui",   FMT_XYI},
    [0xA8] = {"stt",     FMT_XYZ},     [0xA9] = {"stti",    FMT_XYI},
    [0xAA] = {"sttu",    FMT_XYZ},     [0xAB] = {"sttui",   FMT_XYI},
    [0xAC] = {"sto",     FMT_XYZ},     [0xAD] = {"stoi",    FMT_XYI},
    [0xAE] = {"stou",    FMT_XYZ},     [0xAF] = {"stoui",   FMT_XYI},
    
    /* 0xB0-0xBF: Store special, sync, prestore, pushgo */
    [0xB0] = {"stsf",    FMT_XYZ},     [0xB1] = {"stsfi",   FMT_XYI},
    [0xB2] = {"stht",    FMT_XYZ},     [0xB3] = {"sthti",   FMT_XYI},
    [0xB4] = {"stco",    FMT_XYZ},     [0xB5] = {"stcoi",   FMT_XYI},
    [0xB6] = {"stunc",   FMT_XYZ},     [0xB7] = {"stunci",  FMT_XYI},
    [0xB8] = {"syncd",   FMT_XYZ},     [0xB9] = {"syncdi",  FMT_XYI},
    [0xBA] = {"prest",   FMT_XYZ},     [0xBB] = {"presti",  FMT_XYI},
    [0xBC] = {"syncid",  FMT_XYZ},     [0xBD] = {"syncidi", FMT_XYI},
    [0xBE] = {"pushgo",  FMT_XYZ},     [0xBF] = {"pushgoi", FMT_XYI},
    
    /* 0xC0-0xCF: Bitwise OR/XOR/AND operations */
    [0xC0] = {"or",      FMT_XYZ},     [0xC1] = {"ori",     FMT_XYI},
    [0xC2] = {"orn",     FMT_XYZ},     [0xC3] = {"orni",    FMT_XYI},
    [0xC4] = {"nor",     FMT_XYZ},     [0xC5] = {"nori",    FMT_XYI},
    [0xC6] = {"xor",     FMT_XYZ},     [0xC7] = {"xori",    FMT_XYI},
    [0xC8] = {"and",     FMT_XYZ},     [0xC9] = {"andi",    FMT_XYI},
    [0xCA] = {"andn",    FMT_XYZ},     [0xCB] = {"andni",   FMT_XYI},
    [0xCC] = {"nand",    FMT_XYZ},     [0xCD] = {"nandi",   FMT_XYI},
    [0xCE] = {"nxor",    FMT_XYZ},     [0xCF] = {"nxori",   FMT_XYI},
    
    /* 0xD0-0xDF: Byte/wyde/tetra/octa difference, multiplex */
    [0xD0] = {"bdif",    FMT_XYZ},     [0xD1] = {"bdifi",   FMT_XYI},
    [0xD2] = {"wdif",    FMT_XYZ},     [0xD3] = {"wdifi",   FMT_XYI},
    [0xD4] = {"tdif",    FMT_XYZ},     [0xD5] = {"tdifi",   FMT_XYI},
    [0xD6] = {"odif",    FMT_XYZ},     [0xD7] = {"odifi",   FMT_XYI},
    [0xD8] = {"mux",     FMT_XYZ},     [0xD9] = {"muxi",    FMT_XYI},
    [0xDA] = {"sadd",    FMT_XYZ},     [0xDB] = {"saddi",   FMT_XYI},
    [0xDC] = {"mor",     FMT_XYZ},     [0xDD] = {"mori",    FMT_XYI},
    [0xDE] = {"mxor",    FMT_XYZ},     [0xDF] = {"mxori",   FMT_XYI},
    
    /* 0xE0-0xEF: SET/INC/OR/ANDN with high/medium/low wydes */
    [0xE0] = {"seth",    FMT_XI},      [0xE1] = {"setmh",   FMT_XI},
    [0xE2] = {"setml",   FMT_XI},      [0xE3] = {"setl",    FMT_XI},
    [0xE4] = {"inch",    FMT_XI},      [0xE5] = {"incmh",   FMT_XI},
    [0xE6] = {"incml",   FMT_XI},      [0xE7] = {"incl",    FMT_XI},
    [0xE8] = {"orh",     FMT_XI},      [0xE9] = {"ormh",    FMT_XI},
    [0xEA] = {"orml",    FMT_XI},      [0xEB] = {"orl",     FMT_XI},
    [0xEC] = {"andnh",   FMT_XI},      [0xED] = {"andnmh",  FMT_XI},
    [0xEE] = {"andnml",  FMT_XI},      [0xEF] = {"andnl",   FMT_XI},
    
    /* 0xF0-0xFF: Jump, special register, control */
    [0xF0] = {"jmp",     FMT_JUMP},    [0xF1] = {"jmpb",    FMT_JUMP},
    [0xF2] = {"pushj",   FMT_JUMPX},   [0xF3] = {"pushjb",  FMT_JUMPX},
    [0xF4] = {"geta",    FMT_JUMPX},   [0xF5] = {"getab",   FMT_JUMPX},
    [0xF6] = {"put",     FMT_SPECIAL}, [0xF7] = {"puti",    FMT_SPECIAL},
    [0xF8] = {"pop",     FMT_SPECIAL}, [0xF9] = {"resume",  FMT_NONE},
    [0xFA] = {"save",    FMT_SPECIAL}, [0xFB] = {"unsave",  FMT_SPECIAL},
    [0xFC] = {"sync",    FMT_NONE},    [0xFD] = {"swym",    FMT_NONE},
    [0xFE] = {"get",     FMT_SPECIAL}, [0xFF] = {"trip",    FMT_XYZ},
};

/* Special register names */
static const char *special_regs[32] = {
    "rA", "rB", "rC", "rD", "rE", "rF", "rG", "rH",
    "rI", "rJ", "rK", "rL", "rM", "rN", "rO", "rP",
    "rQ", "rR", "rS", "rT", "rU", "rV", "rW", "rX",
    "rY", "rZ", "rBB", "rTT", "rWW", "rXX", "rYY", "rZZ"
};

/*
 * mmix_disassemble - Complete MMIX instruction disassembler
 * 
 * Decodes and displays all 256 MMIX instructions with proper formatting.
 * Returns the number of bytes disassembled (always 4 for MMIX).
 */
unsigned long
mmix_disassemble(
    char *sect,
    unsigned long left,
    unsigned long addr,
    unsigned long sect_addr,
    enum byte_sex object_byte_sex,
    struct relocation_info *sorted_relocs,
    unsigned long nsorted_relocs,
    struct nlist *symbols,
    unsigned long nsymbols,
    struct nlist *sorted_symbols,
    unsigned long nsorted_symbols,
    char *strings,
    unsigned long strings_size,
    unsigned long *indirect_symbols,
    unsigned long nindirect_symbols,
    struct mach_header *mh,
    struct load_command *load_commands,
    enum bool verbose)
{
    unsigned long insn;
    unsigned char op, x, y, z;
    unsigned int yz, xyz;
    const struct mmix_opcode_entry *opentry;
    const char *mnemonic;
    int offset;
    
    /* MMIX instructions are always 4 bytes */
    if(left < 4){
        printf(".long\t0x%08x (truncated)\n", 0);
        return left;
    }
    
    /* Get instruction in big-endian format */
    memcpy(&insn, sect, 4);
    if(object_byte_sex != BIG_ENDIAN_BYTE_SEX)
        insn = SWAP_LONG(insn);
    
    /* Decode instruction fields */
    op = MMIX_OP(insn);
    x = MMIX_X(insn);
    y = MMIX_Y(insn);
    z = MMIX_Z(insn);
    yz = MMIX_YZ(insn);
    xyz = MMIX_XYZ(insn);
    
    /* Look up opcode */
    opentry = &mmix_opcodes[op];
    mnemonic = opentry->name;
    if(mnemonic == NULL)
        mnemonic = "???";
    
    /* Print instruction based on format and verbosity */
    if(verbose){
        printf("%s\t", mnemonic);
        
        /* Format operands based on instruction type */
        switch(opentry->format){
        case FMT_XYZ:
            /* Three register operands */
            printf("$%d,$%d,$%d", x, y, z);
            break;
            
        case FMT_XYI:
            /* Two registers and immediate byte */
            printf("$%d,$%d,%d", x, y, z);
            break;
            
        case FMT_XI:
            /* Register and 16-bit immediate */
            printf("$%d,0x%04x", x, yz);
            break;
            
        case FMT_BRANCH:
            /* Branch: register and signed 16-bit offset */
            offset = (short)yz;  /* Sign-extend */
            offset *= 4;         /* Offset is in instructions */
            printf("$%d,%+d", x, offset);
            /* Could add target address: (addr + 4 + offset) */
            break;
            
        case FMT_JUMP:
            /* Unconditional jump: signed 24-bit offset */
            offset = (int)(xyz << 8) >> 8;  /* Sign-extend 24-bit */
            offset *= 4;                     /* Offset is in instructions */
            printf("%+d", offset);
            /* Could add target address: (addr + 4 + offset) */
            break;
            
        case FMT_JUMPX:
            /* Jump with register: $X and signed 16-bit offset */
            offset = (short)yz;  /* Sign-extend */
            offset *= 4;         /* Offset is in instructions */
            printf("$%d,%+d", x, offset);
            break;
            
        case FMT_SPECIAL:
            /* Special register operations */
            if(op == 0xF6){  /* PUT $Z,$Y */
                if(z < 32)
                    printf("%s,$%d", special_regs[z], y);
                else
                    printf("$%d,$%d", z, y);
            }
            else if(op == 0xF7){  /* PUTI $Z,Y */
                if(z < 32)
                    printf("%s,%d", special_regs[z], y);
                else
                    printf("$%d,%d", z, y);
            }
            else if(op == 0xFE){  /* GET $X,$Z */
                printf("$%d,", x);
                if(z < 32)
                    printf("%s", special_regs[z]);
                else
                    printf("$%d", z);
            }
            else if(op == 0xF8){  /* POP X,YZ */
                printf("%d,%d", x, yz);
            }
            else if(op == 0xFA){  /* SAVE $X,0 */
                printf("$%d,0", x);
            }
            else if(op == 0xFB){  /* UNSAVE $Z,$Y */
                printf("$%d,$%d", z, y);
            }
            else{
                printf("$%d,$%d,$%d", x, y, z);
            }
            break;
            
        case FMT_NONE:
            /* No operands (SYNC, SWYM, RESUME) */
            if(op == 0xF9)  /* RESUME */
                printf("%d", xyz);
            else if(op == 0xFC)  /* SYNC */
                printf("%d", xyz);
            else if(op == 0xFD)  /* SWYM */
                printf("%d,%d,%d", x, y, z);
            break;
            
        default:
            /* Fallback */
            printf("$%d,$%d,$%d", x, y, z);
            break;
        }
    }
    else{
        /* Non-verbose mode: just print hex */
        printf("%02x%02x%02x%02x", op, x, y, z);
    }
    
    printf("\n");
    return 4;  /* MMIX instructions are always 4 bytes */
}
