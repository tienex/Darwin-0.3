/* mmix.c -- Assembler for the MMIX architecture
   Copyright (c) 1999-2025 Apple Computer, Inc.
   
   MMIX assembler implementation for Darwin cctools
   Based on Donald Knuth's MMIX specification
*/

#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>
#include "as.h"
#include "md.h"
#include "obstack.h"
#include "symbols.h"
#include "messages.h"
#include "sections.h"
#include "fixes.h"
#include "frags.h"
#include "read.h"
#include "libc.h"
#include "mmix-opcode.h"
#include <mach-o/mmix/reloc.h>

/*
 * These are the default cputype and cpusubtype for the MMIX architecture.
 */
const cpu_type_t md_cputype = CPU_TYPE_MMIX;
cpu_subtype_t md_cpusubtype = CPU_SUBTYPE_MMIX_ALL;

/* This is the byte sex for the MMIX architecture (big-endian) */
const enum byte_sex md_target_byte_sex = BIG_ENDIAN_BYTE_SEX;

/* These characters start a comment anywhere on the line */
const char md_comment_chars[] = ";%";

/* These characters only start a comment at the beginning of a line */
const char md_line_comment_chars[] = "#";

/* Characters that can separate mantissa from exponent in floating point */
const char md_EXP_CHARS[] = "eE";

/* Characters after 0 that indicate floating point constant */
const char md_FLT_CHARS[] = "dDfF";

/* MMIX has fixed instruction size, so no relaxation table needed */
const relax_typeS md_relax_table[] = { {0} };

/* Hash table for MMIX opcodes */
static struct hash_control *mmix_op_hash = NULL;

/* Current instruction being assembled */
struct mmix_insn {
    char *error;                    /* Error message or NULL */
    unsigned long opcode;           /* Assembled instruction */
    struct nlist *nlistp;           /* Symbol pointer */
    expressionS exp;                /* Expression */
    int pcrel;                      /* PC-relative flag */
    enum reloc_type_mmix reloc;     /* Relocation type */
};

static struct mmix_insn the_insn;

/* Function prototypes */
static void mmix_assemble_insn(char *str);
static int parse_register(char **str, unsigned char *reg);
static int parse_expression(char *str, expressionS *exp);

/* Pseudo-op table */
const pseudo_typeS md_pseudo_table[] = {
    {"greg", s_ignore, 0},    /* Global register allocation */
    {"loc", s_ignore, 0},     /* Location counter */
    {"byte", cons, 1},        /* Byte constant */
    {"wyde", cons, 2},        /* Wyde (16-bit) constant */
    {"tetra", cons, 4},       /* Tetra (32-bit) constant */
    {"octa", cons, 8},        /* Octa (64-bit) constant */
    {NULL, NULL, 0}
};

/*
 * The opcode table - declare all MMIX instructions
 */
static const struct mmix_opcode mmix_opcodes[] = {
    /* Arithmetic operations */
    {"add",     OP_ADD,     "XYZ", 0, mmix_base},
    {"addi",    OP_ADDI,    "XYI", MMIX_F_IMM, mmix_base},
    {"addu",    OP_ADDU,    "XYZ", 0, mmix_base},
    {"addui",   OP_ADDUI,   "XYI", MMIX_F_IMM, mmix_base},
    {"sub",     OP_SUB,     "XYZ", 0, mmix_base},
    {"subi",    OP_SUBI,    "XYI", MMIX_F_IMM, mmix_base},
    {"subu",    OP_SUBU,    "XYZ", 0, mmix_base},
    {"subui",   OP_SUBUI,   "XYI", MMIX_F_IMM, mmix_base},
    
    /* Multiplication and division */
    {"mul",     OP_MUL,     "XYZ", 0, mmix_base},
    {"muli",    OP_MULI,    "XYI", MMIX_F_IMM, mmix_base},
    {"mulu",    OP_MULU,    "XYZ", 0, mmix_base},
    {"mului",   OP_MULUI,   "XYI", MMIX_F_IMM, mmix_base},
    {"div",     OP_DIV,     "XYZ", 0, mmix_base},
    {"divi",    OP_DIVI,    "XYI", MMIX_F_IMM, mmix_base},
    {"divu",    OP_DIVU,    "XYZ", 0, mmix_base},
    {"divui",   OP_DIVUI,   "XYI", MMIX_F_IMM, mmix_base},
    
    /* Compare operations */
    {"cmp",     OP_CMP,     "XYZ", 0, mmix_base},
    {"cmpi",    OP_CMPI,    "XYI", MMIX_F_IMM, mmix_base},
    {"cmpu",    OP_CMPU,    "XYZ", 0, mmix_base},
    {"cmpui",   OP_CMPUI,   "XYI", MMIX_F_IMM, mmix_base},
    
    /* Bitwise operations */
    {"and",     OP_AND,     "XYZ", 0, mmix_base},
    {"andi",    OP_ANDI,    "XYI", MMIX_F_IMM, mmix_base},
    {"or",      OP_OR,      "XYZ", 0, mmix_base},
    {"ori",     OP_ORI,     "XYI", MMIX_F_IMM, mmix_base},
    {"xor",     OP_XOR,     "XYZ", 0, mmix_base},
    {"xori",    OP_XORI,    "XYI", MMIX_F_IMM, mmix_base},
    {"andn",    OP_ANDN,    "XYZ", 0, mmix_base},
    {"andni",   OP_ANDNI,   "XYI", MMIX_F_IMM, mmix_base},
    {"orn",     OP_ORN,     "XYZ", 0, mmix_base},
    {"orni",    OP_ORNI,    "XYI", MMIX_F_IMM, mmix_base},
    
    /* Shift operations */
    {"sl",      OP_SL,      "XYZ", 0, mmix_base},
    {"sli",     OP_SLI,     "XYI", MMIX_F_IMM, mmix_base},
    {"sr",      OP_SR,      "XYZ", 0, mmix_base},
    {"sri",     OP_SRI,     "XYI", MMIX_F_IMM, mmix_base},
    {"sru",     OP_SRU,     "XYZ", 0, mmix_base},
    {"srui",    OP_SRUI,    "XYI", MMIX_F_IMM, mmix_base},
    
    /* Load operations */
    {"ldb",     OP_LDB,     "XYZ", MMIX_F_LOAD, mmix_base},
    {"ldbi",    OP_LDBI,    "XYI", MMIX_F_LOAD | MMIX_F_IMM, mmix_base},
    {"ldbu",    OP_LDBU,    "XYZ", MMIX_F_LOAD, mmix_base},
    {"ldbui",   OP_LDBUI,   "XYI", MMIX_F_LOAD | MMIX_F_IMM, mmix_base},
    {"ldw",     OP_LDW,     "XYZ", MMIX_F_LOAD, mmix_base},
    {"ldwi",    OP_LDWI,    "XYI", MMIX_F_LOAD | MMIX_F_IMM, mmix_base},
    {"ldwu",    OP_LDWU,    "XYZ", MMIX_F_LOAD, mmix_base},
    {"ldwui",   OP_LDWUI,   "XYI", MMIX_F_LOAD | MMIX_F_IMM, mmix_base},
    {"ldt",     OP_LDT,     "XYZ", MMIX_F_LOAD, mmix_base},
    {"ldti",    OP_LDTI,    "XYI", MMIX_F_LOAD | MMIX_F_IMM, mmix_base},
    {"ldtu",    OP_LDTU,    "XYZ", MMIX_F_LOAD, mmix_base},
    {"ldtui",   OP_LDTUI,   "XYI", MMIX_F_LOAD | MMIX_F_IMM, mmix_base},
    {"ldo",     OP_LDO,     "XYZ", MMIX_F_LOAD, mmix_base},
    {"ldoi",    OP_LDOI,    "XYI", MMIX_F_LOAD | MMIX_F_IMM, mmix_base},
    {"ldou",    OP_LDOU,    "XYZ", MMIX_F_LOAD, mmix_base},
    {"ldoui",   OP_LDOUI,   "XYI", MMIX_F_LOAD | MMIX_F_IMM, mmix_base},
    
    /* Store operations */
    {"stb",     OP_STB,     "XYZ", MMIX_F_STORE, mmix_base},
    {"stbi",    OP_STBI,    "XYI", MMIX_F_STORE | MMIX_F_IMM, mmix_base},
    {"stbu",    OP_STBU,    "XYZ", MMIX_F_STORE, mmix_base},
    {"stbui",   OP_STBUI,   "XYI", MMIX_F_STORE | MMIX_F_IMM, mmix_base},
    {"stw",     OP_STW,     "XYZ", MMIX_F_STORE, mmix_base},
    {"stwi",    OP_STWI,    "XYI", MMIX_F_STORE | MMIX_F_IMM, mmix_base},
    {"stwu",    OP_STWU,    "XYZ", MMIX_F_STORE, mmix_base},
    {"stwui",   OP_STWUI,   "XYI", MMIX_F_STORE | MMIX_F_IMM, mmix_base},
    {"stt",     OP_STT,     "XYZ", MMIX_F_STORE, mmix_base},
    {"stti",    OP_STTI,    "XYI", MMIX_F_STORE | MMIX_F_IMM, mmix_base},
    {"sttu",    OP_STTU,    "XYZ", MMIX_F_STORE, mmix_base},
    {"sttui",   OP_STTUI,   "XYI", MMIX_F_STORE | MMIX_F_IMM, mmix_base},
    {"sto",     OP_STO,     "XYZ", MMIX_F_STORE, mmix_base},
    {"stoi",    OP_STOI,    "XYI", MMIX_F_STORE | MMIX_F_IMM, mmix_base},
    {"stou",    OP_STOU,    "XYZ", MMIX_F_STORE, mmix_base},
    {"stoui",   OP_STOUI,   "XYI", MMIX_F_STORE | MMIX_F_IMM, mmix_base},
    
    /* Branch operations */
    {"bn",      OP_BN,      "XB", MMIX_F_BRANCH | MMIX_F_COND_BR, mmix_base},
    {"bnb",     OP_BNB,     "XB", MMIX_F_BRANCH | MMIX_F_COND_BR, mmix_base},
    {"bz",      OP_BZ,      "XB", MMIX_F_BRANCH | MMIX_F_COND_BR, mmix_base},
    {"bzb",     OP_BZB,     "XB", MMIX_F_BRANCH | MMIX_F_COND_BR, mmix_base},
    {"bp",      OP_BP,      "XB", MMIX_F_BRANCH | MMIX_F_COND_BR, mmix_base},
    {"bpb",     OP_BPB,     "XB", MMIX_F_BRANCH | MMIX_F_COND_BR, mmix_base},
    {"bnn",     OP_BNN,     "XB", MMIX_F_BRANCH | MMIX_F_COND_BR, mmix_base},
    {"bnnb",    OP_BNNB,    "XB", MMIX_F_BRANCH | MMIX_F_COND_BR, mmix_base},
    {"bnz",     OP_BNZ,     "XB", MMIX_F_BRANCH | MMIX_F_COND_BR, mmix_base},
    {"bnzb",    OP_BNZB,    "XB", MMIX_F_BRANCH | MMIX_F_COND_BR, mmix_base},
    
    /* Jump operations */
    {"jmp",     OP_JMP,     "B", MMIX_F_JUMP, mmix_base},
    {"jmpb",    OP_JMPB,    "B", MMIX_F_JUMP, mmix_base},
    {"pushj",   OP_PUSHJ,   "XB", MMIX_F_JUMP, mmix_base},
    {"pushjb",  OP_PUSHJB,  "XB", MMIX_F_JUMP, mmix_base},
    {"geta",    OP_GETA,    "XB", 0, mmix_base},
    {"getab",   OP_GETAB,   "XB", 0, mmix_base},
    {"go",      OP_GO,      "XYZ", MMIX_F_JUMP, mmix_base},
    {"goi",     OP_GOI,     "XYI", MMIX_F_JUMP | MMIX_F_IMM, mmix_base},
    {"pushgo",  OP_PUSHGO,  "XYZ", MMIX_F_JUMP, mmix_base},
    {"pushgoi", OP_PUSHGOI, "XYI", MMIX_F_JUMP | MMIX_F_IMM, mmix_base},
    
    /* Set operations */
    {"seth",    OP_SETH,    "XI", 0, mmix_base},
    {"setmh",   OP_SETMH,   "XI", 0, mmix_base},
    {"setml",   OP_SETML,   "XI", 0, mmix_base},
    {"setl",    OP_SETL,    "XI", 0, mmix_base},
    
    /* Special operations */
    {"get",     OP_GET,     "X@", 0, mmix_base},
    {"put",     OP_PUT,     "@Y", 0, mmix_base},
    {"puti",    OP_PUTI,    "@I", MMIX_F_IMM, mmix_base},
    {"pop",     OP_POP,     "XI", 0, mmix_base},
    {"save",    OP_SAVE,    "X", 0, mmix_base},
    {"unsave",  OP_UNSAVE,  "YZ", 0, mmix_base},
    {"sync",    OP_SYNC,    "", 0, mmix_base},
    {"swym",    OP_SWYM,    "", 0, mmix_base},
    {"trap",    OP_TRAP,    "XYZ", 0, mmix_base},
    
    {NULL, 0, NULL, 0, mmix_base}
};

const int mmix_num_opcodes = (sizeof(mmix_opcodes) / sizeof(mmix_opcodes[0])) - 1;

/*
 * md_parse_option() - Parse machine-dependent command line options
 */
int
md_parse_option(
    char **argP,
    int *cntP,
    char ***vecP)
{
    return 0;  /* No MMIX-specific options yet */
}

/*
 * md_begin() - Called before assembly begins
 */
void
md_begin(void)
{
    const struct mmix_opcode *op;
    
    /* Initialize the opcode hash table */
    mmix_op_hash = hash_new();
    if (mmix_op_hash == NULL)
        as_fatal("Could not initialize MMIX opcode hash table");
    
    /* Add all opcodes to the hash table */
    for (op = mmix_opcodes; op->name != NULL; op++) {
        hash_insert(mmix_op_hash, op->name, (char *)op);
    }
}

/*
 * md_end() - Called after assembly ends
 */
void
md_end(void)
{
    /* Nothing to do */
}

/*
 * md_assemble() - Assemble a machine instruction
 */
void
md_assemble(char *str)
{
    mmix_assemble_insn(str);
}

/*
 * Parse a register specification ($0-$255 or special register names)
 */
static int
parse_register(char **str, unsigned char *reg)
{
    char *s = *str;
    
    if (*s != '$')
        return 0;
    
    s++;
    if (isdigit(*s)) {
        int val = 0;
        while (isdigit(*s)) {
            val = val * 10 + (*s - '0');
            s++;
        }
        if (val > 255) {
            as_warn("Register number %d out of range (0-255)", val);
            return 0;
        }
        *reg = (unsigned char)val;
        *str = s;
        return 1;
    }
    
    /* Could add special register name parsing here */
    return 0;
}

/*
 * Parse an expression
 */
static int
parse_expression(char *str, expressionS *exp)
{
    char *save_input_line_pointer;
    segT segment;
    
    save_input_line_pointer = input_line_pointer;
    input_line_pointer = str;
    
    segment = expression(exp);
    
    input_line_pointer = save_input_line_pointer;
    
    if (segment == SEG_NONE || segment == SEG_UNKNOWN)
        return 0;
    
    return 1;
}

/*
 * Assemble an MMIX instruction
 */
static void
mmix_assemble_insn(char *str)
{
    char *s;
    const struct mmix_opcode *opcode;
    char mnemonic[256];
    int i;
    unsigned char x_reg = 0, y_reg = 0, z_reg = 0;
    unsigned int imm16 = 0;
    char *frag_ptr;
    
    /* Skip leading whitespace */
    while (*str == ' ' || *str == '\t')
        str++;
    
    /* Extract mnemonic */
    s = str;
    i = 0;
    while (*s && *s != ' ' && *s != '\t' && i < 255) {
        mnemonic[i++] = tolower(*s++);
    }
    mnemonic[i] = '\0';
    
    /* Look up opcode */
    opcode = (const struct mmix_opcode *)hash_find(mmix_op_hash, mnemonic);
    if (opcode == NULL) {
        as_bad("Unknown instruction: %s", mnemonic);
        return;
    }
    
    /* Skip whitespace after mnemonic */
    while (*s == ' ' || *s == '\t')
        s++;
    
    /* Parse operands based on format string */
    the_insn.opcode = opcode->opcode << 24;
    the_insn.pcrel = 0;
    the_insn.reloc = MMIX_RELOC_VANILLA;
    
    /* Simple operand parsing - this would need to be expanded */
    if (strchr(opcode->args, 'X')) {
        if (!parse_register(&s, &x_reg)) {
            as_bad("Expected register for X operand");
            return;
        }
        the_insn.opcode |= (x_reg << 16);
        
        while (*s == ' ' || *s == '\t' || *s == ',')
            s++;
    }
    
    if (strchr(opcode->args, 'Y')) {
        if (!parse_register(&s, &y_reg)) {
            as_bad("Expected register for Y operand");
            return;
        }
        the_insn.opcode |= (y_reg << 8);
        
        while (*s == ' ' || *s == '\t' || *s == ',')
            s++;
    }
    
    if (strchr(opcode->args, 'Z')) {
        if (!parse_register(&s, &z_reg)) {
            as_bad("Expected register for Z operand");
            return;
        }
        the_insn.opcode |= z_reg;
    } else if (strchr(opcode->args, 'I')) {
        /* Immediate value - simplified parsing */
        expressionS exp;
        if (parse_expression(s, &exp)) {
            if (exp.X_op == O_constant) {
                imm16 = exp.X_add_number & 0xFFFF;
                the_insn.opcode |= imm16;
            } else {
                /* Need relocation */
                the_insn.exp = exp;
                the_insn.reloc = MMIX_RELOC_LOW16;
            }
        }
    }
    
    /* Emit the instruction */
    frag_ptr = frag_more(4);  /* MMIX instructions are 4 bytes */
    md_number_to_chars(frag_ptr, the_insn.opcode, 4);
    
    /* Add fixup if needed */
    if (the_insn.reloc != MMIX_RELOC_VANILLA && the_insn.exp.X_op != O_constant) {
        fix_new_exp(frag_now,
                    frag_ptr - frag_now->fr_literal,
                    4,
                    &the_insn.exp,
                    the_insn.pcrel,
                    the_insn.reloc);
    }
}

/*
 * md_atof() - Parse floating point numbers
 */
char *
md_atof(int type, char *litP, int *sizeP)
{
    return atof_ieee(litP, type, sizeP);
}

/*
 * md_number_to_chars() - Put out a value in target byte order
 */
void
md_number_to_chars(char *buf, long val, int n)
{
    /* MMIX is big-endian */
    switch (n) {
    case 8:
        *buf++ = (val >> 56) & 0xff;
        *buf++ = (val >> 48) & 0xff;
        *buf++ = (val >> 40) & 0xff;
        *buf++ = (val >> 32) & 0xff;
        /* fall through */
    case 4:
        *buf++ = (val >> 24) & 0xff;
        *buf++ = (val >> 16) & 0xff;
        /* fall through */
    case 2:
        *buf++ = (val >> 8) & 0xff;
        /* fall through */
    case 1:
        *buf = val & 0xff;
        break;
    default:
        as_fatal("Internal error: md_number_to_chars called with n=%d", n);
    }
}

/*
 * md_number_to_imm() - Put out a value for relocation
 */
void
md_number_to_imm(
    unsigned char *buf,
    long val,
    int n,
    fixS *fixP,
    int nsect)
{
    /* For MMIX, relocations are handled similarly to normal values */
    md_number_to_chars((char *)buf, val, n);
}

/*
 * md_estimate_size_before_relax() - Not needed for RISC
 */
int
md_estimate_size_before_relax(
    fragS *fragP,
    int nsect)
{
    as_fatal("Internal error: MMIX relaxation should not be called");
    return 0;
}

/*
 * md_convert_frag() - Not needed for RISC
 */
void
md_convert_frag(fragS *fragP)
{
    as_fatal("Internal error: MMIX frag conversion should not be called");
}
