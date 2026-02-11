/* dlx.h - DLX Architecture Definitions
 * Copyright (C) 1999 Apple Computer, Inc.
 *
 * DLX RISC processor simulator for Darwin
 */

#ifndef DLX_H
#define DLX_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* DLX Architecture Constants */
#define DLX_NUM_REGS      32
#define DLX_NUM_FREGS     32
#define DLX_MEMORY_SIZE   (16 * 1024 * 1024)  /* 16 MB */
#define DLX_PAGE_SIZE     8192
#define DLX_TLB_ENTRIES   64

/* Register Names */
#define REG_ZERO    0
#define REG_SP      29
#define REG_FP      30
#define REG_RA      31

/* Instruction Formats */
#define OPCODE(i)   (((i) >> 26) & 0x3F)
#define RS1(i)      (((i) >> 21) & 0x1F)
#define RS2(i)      (((i) >> 16) & 0x1F)
#define RD(i)       (((i) >> 11) & 0x1F)
#define FUNC(i)     ((i) & 0x7FF)
#define IMM16(i)    ((int16_t)((i) & 0xFFFF))
#define UIMM16(i)   ((i) & 0xFFFF)
#define IMM26(i)    ((i) & 0x03FFFFFF)

/* Sign extension for immediates */
#define SEXT16(x)   ((int32_t)(int16_t)(x))
#define SEXT26(x)   (((int32_t)((x) << 6)) >> 6)

/* Opcodes */
#define OP_SPECIAL  0x00
#define OP_FPARITH  0x01
#define OP_J        0x02
#define OP_JAL      0x03
#define OP_BEQZ     0x04
#define OP_BNEZ     0x05
#define OP_BFPT     0x06
#define OP_BFPF     0x07
#define OP_ADDI     0x08
#define OP_ADDUI    0x09
#define OP_SUBI     0x0A
#define OP_SUBUI    0x0B
#define OP_ANDI     0x0C
#define OP_ORI      0x0D
#define OP_XORI     0x0E
#define OP_LHI      0x0F
#define OP_RFE      0x10
#define OP_TRAP     0x11
#define OP_JR       0x12
#define OP_JALR     0x13
#define OP_SLLI     0x14
#define OP_SRLI     0x16
#define OP_SRAI     0x17
#define OP_SEQI     0x18
#define OP_SNEI     0x19
#define OP_SLTI     0x1A
#define OP_SGTI     0x1B
#define OP_SLEI     0x1C
#define OP_SGEI     0x1D
#define OP_LB       0x20
#define OP_LH       0x21
#define OP_LW       0x23
#define OP_LBU      0x24
#define OP_LHU      0x25
#define OP_LF       0x26
#define OP_LD       0x27
#define OP_SB       0x28
#define OP_SH       0x29
#define OP_SW       0x2B
#define OP_SF       0x2E
#define OP_SD       0x2F
#define OP_SEQUI    0x30
#define OP_SNEUI    0x31
#define OP_SLTUI    0x32
#define OP_SGTUI    0x33
#define OP_SLEUI    0x34
#define OP_SGEUI    0x35

/* Function codes for OP_SPECIAL */
#define FUNC_NOP    0x000
#define FUNC_SLL    0x004
#define FUNC_SRL    0x006
#define FUNC_SRA    0x007
#define FUNC_ADD    0x020
#define FUNC_ADDU   0x021
#define FUNC_SUB    0x022
#define FUNC_SUBU   0x023
#define FUNC_AND    0x024
#define FUNC_OR     0x025
#define FUNC_XOR    0x026
#define FUNC_SEQ    0x028
#define FUNC_SNE    0x029
#define FUNC_SLT    0x02A
#define FUNC_SGT    0x02B
#define FUNC_SLE    0x02C
#define FUNC_SGE    0x02D
#define FUNC_MOVI2S 0x030
#define FUNC_MOVS2I 0x040
#define FUNC_MOVF   0x050
#define FUNC_MOVD   0x051
#define FUNC_MOVFP2I 0x052
#define FUNC_MOVI2FP 0x053
#define FUNC_MULT   0x060
#define FUNC_MULTU  0x061
#define FUNC_DIV    0x062
#define FUNC_DIVU   0x063

/* Function codes for OP_FPARITH */
#define FUNC_ADDF   0x000
#define FUNC_SUBF   0x001
#define FUNC_MULTF  0x002
#define FUNC_DIVF   0x003
#define FUNC_ADDD   0x004
#define FUNC_SUBD   0x005
#define FUNC_MULTD  0x006
#define FUNC_DIVD   0x007
#define FUNC_CVTF2D 0x008
#define FUNC_CVTF2I 0x009
#define FUNC_CVTD2F 0x00A
#define FUNC_CVTD2I 0x00B
#define FUNC_CVTI2F 0x00C
#define FUNC_CVTI2D 0x00D
#define FUNC_EQF    0x010
#define FUNC_NEF    0x011
#define FUNC_LTF    0x012
#define FUNC_GTF    0x013
#define FUNC_LEF    0x014
#define FUNC_GEF    0x015
#define FUNC_EQD    0x018
#define FUNC_NED    0x019
#define FUNC_LTD    0x01A
#define FUNC_GTD    0x01B
#define FUNC_LED    0x01C
#define FUNC_GED    0x01D

/* Status Register Bits */
#define STATUS_IE       0x00000001  /* Interrupt Enable */
#define STATUS_KUC      0x00000002  /* Kernel/User Current */
#define STATUS_IEC      0x00000004  /* IE Current */
#define STATUS_KUP      0x00000008  /* Kernel/User Previous */
#define STATUS_IEP      0x00000010  /* IE Previous */
#define STATUS_KUO      0x00000020  /* Kernel/User Old */
#define STATUS_IEO      0x00000040  /* IE Old */
#define STATUS_INTRMASK 0x0000FF00  /* Interrupt Mask */
#define STATUS_PAGE_TABLE 0x00010000  /* Page Table Mode */
#define STATUS_TLB      0x00020000  /* TLB Mode */
#define STATUS_SYS_BE   0x00040000  /* System Mode Big-Endian (1=BE, 0=LE) */
#define STATUS_USR_BE   0x00080000  /* User Mode Big-Endian (1=BE, 0=LE) */

/* Cause Register Bits */
#define CAUSE_EXCCODE   0x0000003C  /* Exception Code */
#define CAUSE_BD        0x80000000  /* Branch Delay */

/* Exception Codes */
#define EXC_INT         0   /* Interrupt */
#define EXC_TLBL        1   /* TLB Miss (Load) */
#define EXC_TLBS        2   /* TLB Miss (Store) */
#define EXC_ADEL        4   /* Address Error (Load) */
#define EXC_ADES        5   /* Address Error (Store) */
#define EXC_IBE         6   /* Bus Error (Instruction) */
#define EXC_DBE         7   /* Bus Error (Data) */
#define EXC_SYS         8   /* Syscall */
#define EXC_BP          9   /* Breakpoint */
#define EXC_RI          10  /* Reserved Instruction */
#define EXC_CPU         11  /* Coprocessor Unusable */
#define EXC_OVF         12  /* Arithmetic Overflow */

/* Memory-Mapped I/O Addresses */
#define TIMER_ADDR      0xFFF00010  /* Timer register */
#define KBD_DATA_ADDR   0xFFF00100  /* Keyboard data */
#define KBD_STATUS_ADDR 0xFFF00104  /* Keyboard status */

/* Helper macros for endianness */
#define IS_KERNEL_MODE(status) ((status) & STATUS_KUC)
#define IS_BIG_ENDIAN(status) \
    (IS_KERNEL_MODE(status) ? ((status) & STATUS_SYS_BE) : ((status) & STATUS_USR_BE))

/* DLX CPU State */
typedef struct {
    uint32_t regs[DLX_NUM_REGS];    /* General-purpose registers */
    uint32_t fregs[DLX_NUM_FREGS];  /* Floating-point registers */
    uint32_t pc;                     /* Program counter */
    uint32_t status;                 /* Status register */
    uint32_t cause;                  /* Cause register */
    uint32_t epc;                    /* Exception PC */
    uint32_t hi, lo;                 /* Multiply/divide results */
    int running;                     /* Simulator running flag */
    uint64_t cycles;                 /* Cycle counter */
} dlx_cpu_t;

/* Memory System */
typedef struct {
    uint8_t *mem;                    /* Main memory */
    uint32_t size;                   /* Memory size */
} dlx_memory_t;

/* TLB Entry */
typedef struct {
    uint32_t vpn;                    /* Virtual page number */
    uint32_t pfn;                    /* Physical frame number */
    uint32_t valid;                  /* Valid bit */
    uint32_t dirty;                  /* Dirty bit */
} dlx_tlb_entry_t;

/* MMU State */
typedef struct {
    dlx_tlb_entry_t entries[DLX_TLB_ENTRIES];
    uint32_t *page_table;            /* Page table pointer */
} dlx_mmu_t;

/* I/O Devices */
typedef struct {
    uint32_t timer;                  /* Timer register */
    char kbd_buffer[256];            /* Keyboard buffer */
    int kbd_head, kbd_tail;          /* Keyboard buffer pointers */
} dlx_devices_t;

/* Executable Format Types */
typedef enum {
    FORMAT_RAW,      /* Raw binary */
    FORMAT_MACHO,    /* Mach-O executable */
    FORMAT_PECOFF    /* PE/COFF executable */
} dlx_format_t;

/* Complete DLX Simulator State */
typedef struct {
    dlx_cpu_t cpu;
    dlx_memory_t memory;
    dlx_mmu_t mmu;
    dlx_devices_t devices;
    int verbose;                     /* Verbose output flag */
    int trace;                       /* Instruction trace flag */
} dlx_sim_t;

/* Function Prototypes */

/* Simulator lifecycle */
dlx_sim_t *dlx_create(void);
void dlx_destroy(dlx_sim_t *sim);
void dlx_reset(dlx_sim_t *sim);

/* Execution */
void dlx_run(dlx_sim_t *sim);
void dlx_step(dlx_sim_t *sim);
void dlx_execute_instruction(dlx_sim_t *sim, uint32_t instr);

/* Memory access */
uint32_t dlx_mem_read_word(dlx_sim_t *sim, uint32_t addr);
uint16_t dlx_mem_read_half(dlx_sim_t *sim, uint32_t addr);
uint8_t dlx_mem_read_byte(dlx_sim_t *sim, uint32_t addr);
void dlx_mem_write_word(dlx_sim_t *sim, uint32_t addr, uint32_t value);
void dlx_mem_write_half(dlx_sim_t *sim, uint32_t addr, uint16_t value);
void dlx_mem_write_byte(dlx_sim_t *sim, uint32_t addr, uint8_t value);

/* Memory loading */
int dlx_load_binary(dlx_sim_t *sim, const char *filename, uint32_t addr);
int dlx_load_macho(dlx_sim_t *sim, const char *filename);
int dlx_load_pecoff(dlx_sim_t *sim, const char *filename);
dlx_format_t dlx_detect_format(const char *filename);

/* MMU */
uint32_t dlx_mmu_translate(dlx_sim_t *sim, uint32_t vaddr, int write);

/* Exceptions */
void dlx_exception(dlx_sim_t *sim, int code);

/* Devices */
void dlx_devices_init(dlx_sim_t *sim);
uint32_t dlx_device_read(dlx_sim_t *sim, uint32_t addr);
void dlx_device_write(dlx_sim_t *sim, uint32_t addr, uint32_t value);

/* Debugging */
void dlx_dump_regs(dlx_sim_t *sim);
void dlx_disassemble(uint32_t instr, uint32_t pc);

/* Utilities */
const char *dlx_reg_name(int reg);
const char *dlx_opcode_name(int opcode);

#endif /* DLX_H */
