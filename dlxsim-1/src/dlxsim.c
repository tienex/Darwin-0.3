/* dlxsim.c - DLX Simulator Main Implementation
 * Copyright (C) 1999 Apple Computer, Inc.
 */

#include "dlx.h"
#include <unistd.h>
#include <getopt.h>

/* Create new simulator instance */
dlx_sim_t *dlx_create(void) {
    dlx_sim_t *sim = calloc(1, sizeof(dlx_sim_t));
    if (!sim) {
        fprintf(stderr, "Failed to allocate simulator\n");
        return NULL;
    }

    sim->memory.size = DLX_MEMORY_SIZE;
    sim->memory.mem = calloc(1, sim->memory.size);
    if (!sim->memory.mem) {
        fprintf(stderr, "Failed to allocate memory\n");
        free(sim);
        return NULL;
    }

    dlx_reset(sim);
    dlx_devices_init(sim);

    return sim;
}

/* Destroy simulator instance */
void dlx_destroy(dlx_sim_t *sim) {
    if (sim) {
        if (sim->memory.mem) free(sim->memory.mem);
        free(sim);
    }
}

/* Reset simulator to initial state */
void dlx_reset(dlx_sim_t *sim) {
    memset(&sim->cpu, 0, sizeof(sim->cpu));
    sim->cpu.regs[REG_ZERO] = 0;  /* R0 is always 0 */
    sim->cpu.pc = 0x00000000;
    sim->cpu.running = 1;
    sim->cpu.cycles = 0;
    sim->cpu.status = STATUS_IE | STATUS_KUC;  /* Interrupts enabled, kernel mode */
}

/* Execute single instruction */
void dlx_execute_instruction(dlx_sim_t *sim, uint32_t instr) {
    dlx_cpu_t *cpu = &sim->cpu;
    int op = OPCODE(instr);
    int rs1 = RS1(instr);
    int rs2 = RS2(instr);
    int rd = RD(instr);
    int func = FUNC(instr);
    int16_t imm = IMM16(instr);
    uint16_t uimm = UIMM16(instr);
    int32_t simm = SEXT16(imm);
    uint32_t addr;
    int32_t tmp;

    /* Enforce R0 = 0 */
    cpu->regs[REG_ZERO] = 0;

    if (sim->trace) {
        printf("PC=%08x: ", cpu->pc - 4);
        dlx_disassemble(instr, cpu->pc - 4);
    }

    switch (op) {
    case OP_SPECIAL:
        switch (func) {
        case FUNC_NOP:
            break;
        case FUNC_ADD:
            cpu->regs[rd] = cpu->regs[rs1] + cpu->regs[rs2];
            break;
        case FUNC_ADDU:
            cpu->regs[rd] = cpu->regs[rs1] + cpu->regs[rs2];
            break;
        case FUNC_SUB:
            cpu->regs[rd] = cpu->regs[rs1] - cpu->regs[rs2];
            break;
        case FUNC_SUBU:
            cpu->regs[rd] = cpu->regs[rs1] - cpu->regs[rs2];
            break;
        case FUNC_AND:
            cpu->regs[rd] = cpu->regs[rs1] & cpu->regs[rs2];
            break;
        case FUNC_OR:
            cpu->regs[rd] = cpu->regs[rs1] | cpu->regs[rs2];
            break;
        case FUNC_XOR:
            cpu->regs[rd] = cpu->regs[rs1] ^ cpu->regs[rs2];
            break;
        case FUNC_SLL:
            cpu->regs[rd] = cpu->regs[rs1] << cpu->regs[rs2];
            break;
        case FUNC_SRL:
            cpu->regs[rd] = cpu->regs[rs1] >> cpu->regs[rs2];
            break;
        case FUNC_SRA:
            cpu->regs[rd] = (int32_t)cpu->regs[rs1] >> cpu->regs[rs2];
            break;
        case FUNC_SEQ:
            cpu->regs[rd] = (cpu->regs[rs1] == cpu->regs[rs2]) ? 1 : 0;
            break;
        case FUNC_SNE:
            cpu->regs[rd] = (cpu->regs[rs1] != cpu->regs[rs2]) ? 1 : 0;
            break;
        case FUNC_SLT:
            cpu->regs[rd] = ((int32_t)cpu->regs[rs1] < (int32_t)cpu->regs[rs2]) ? 1 : 0;
            break;
        case FUNC_SGT:
            cpu->regs[rd] = ((int32_t)cpu->regs[rs1] > (int32_t)cpu->regs[rs2]) ? 1 : 0;
            break;
        case FUNC_SLE:
            cpu->regs[rd] = ((int32_t)cpu->regs[rs1] <= (int32_t)cpu->regs[rs2]) ? 1 : 0;
            break;
        case FUNC_SGE:
            cpu->regs[rd] = ((int32_t)cpu->regs[rs1] >= (int32_t)cpu->regs[rs2]) ? 1 : 0;
            break;
        case FUNC_MULT:
            {
                int64_t result = (int64_t)(int32_t)cpu->regs[rs1] * (int64_t)(int32_t)cpu->regs[rs2];
                cpu->lo = (uint32_t)result;
                cpu->hi = (uint32_t)(result >> 32);
                cpu->regs[rd] = cpu->lo;
            }
            break;
        case FUNC_MULTU:
            {
                uint64_t result = (uint64_t)cpu->regs[rs1] * (uint64_t)cpu->regs[rs2];
                cpu->lo = (uint32_t)result;
                cpu->hi = (uint32_t)(result >> 32);
                cpu->regs[rd] = cpu->lo;
            }
            break;
        case FUNC_DIV:
            if (cpu->regs[rs2] != 0) {
                cpu->lo = (int32_t)cpu->regs[rs1] / (int32_t)cpu->regs[rs2];
                cpu->hi = (int32_t)cpu->regs[rs1] % (int32_t)cpu->regs[rs2];
                cpu->regs[rd] = cpu->lo;
            }
            break;
        case FUNC_DIVU:
            if (cpu->regs[rs2] != 0) {
                cpu->lo = cpu->regs[rs1] / cpu->regs[rs2];
                cpu->hi = cpu->regs[rs1] % cpu->regs[rs2];
                cpu->regs[rd] = cpu->lo;
            }
            break;
        case FUNC_MOVI2S:
            cpu->status = cpu->regs[rs1];
            break;
        case FUNC_MOVS2I:
            cpu->regs[rd] = cpu->status;
            break;
        default:
            fprintf(stderr, "Unknown SPECIAL function: 0x%x\n", func);
            dlx_exception(sim, EXC_RI);
            break;
        }
        break;

    case OP_ADDI:
        cpu->regs[rd] = cpu->regs[rs1] + simm;
        break;
    case OP_ADDUI:
        cpu->regs[rd] = cpu->regs[rs1] + simm;
        break;
    case OP_SUBI:
        cpu->regs[rd] = cpu->regs[rs1] - simm;
        break;
    case OP_SUBUI:
        cpu->regs[rd] = cpu->regs[rs1] - simm;
        break;
    case OP_ANDI:
        cpu->regs[rd] = cpu->regs[rs1] & uimm;
        break;
    case OP_ORI:
        cpu->regs[rd] = cpu->regs[rs1] | uimm;
        break;
    case OP_XORI:
        cpu->regs[rd] = cpu->regs[rs1] ^ uimm;
        break;
    case OP_LHI:
        cpu->regs[rd] = (uimm << 16);
        break;
    case OP_SLLI:
        cpu->regs[rd] = cpu->regs[rs1] << uimm;
        break;
    case OP_SRLI:
        cpu->regs[rd] = cpu->regs[rs1] >> uimm;
        break;
    case OP_SRAI:
        cpu->regs[rd] = (int32_t)cpu->regs[rs1] >> uimm;
        break;
    case OP_SEQI:
        cpu->regs[rd] = (cpu->regs[rs1] == (uint32_t)simm) ? 1 : 0;
        break;
    case OP_SNEI:
        cpu->regs[rd] = (cpu->regs[rs1] != (uint32_t)simm) ? 1 : 0;
        break;
    case OP_SLTI:
        cpu->regs[rd] = ((int32_t)cpu->regs[rs1] < simm) ? 1 : 0;
        break;
    case OP_SGTI:
        cpu->regs[rd] = ((int32_t)cpu->regs[rs1] > simm) ? 1 : 0;
        break;
    case OP_SLEI:
        cpu->regs[rd] = ((int32_t)cpu->regs[rs1] <= simm) ? 1 : 0;
        break;
    case OP_SGEI:
        cpu->regs[rd] = ((int32_t)cpu->regs[rs1] >= simm) ? 1 : 0;
        break;

    /* Load instructions */
    case OP_LW:
        addr = cpu->regs[rs1] + simm;
        cpu->regs[rd] = dlx_mem_read_word(sim, addr);
        break;
    case OP_LH:
        addr = cpu->regs[rs1] + simm;
        tmp = (int16_t)dlx_mem_read_half(sim, addr);
        cpu->regs[rd] = tmp;
        break;
    case OP_LHU:
        addr = cpu->regs[rs1] + simm;
        cpu->regs[rd] = dlx_mem_read_half(sim, addr);
        break;
    case OP_LB:
        addr = cpu->regs[rs1] + simm;
        tmp = (int8_t)dlx_mem_read_byte(sim, addr);
        cpu->regs[rd] = tmp;
        break;
    case OP_LBU:
        addr = cpu->regs[rs1] + simm;
        cpu->regs[rd] = dlx_mem_read_byte(sim, addr);
        break;

    /* Store instructions */
    case OP_SW:
        addr = cpu->regs[rs1] + simm;
        dlx_mem_write_word(sim, addr, cpu->regs[rd]);
        break;
    case OP_SH:
        addr = cpu->regs[rs1] + simm;
        dlx_mem_write_half(sim, addr, (uint16_t)cpu->regs[rd]);
        break;
    case OP_SB:
        addr = cpu->regs[rs1] + simm;
        dlx_mem_write_byte(sim, addr, (uint8_t)cpu->regs[rd]);
        break;

    /* Branch instructions */
    case OP_BEQZ:
        if (cpu->regs[rs1] == 0) {
            cpu->pc += (simm << 2);
        }
        break;
    case OP_BNEZ:
        if (cpu->regs[rs1] != 0) {
            cpu->pc += (simm << 2);
        }
        break;

    /* Jump instructions */
    case OP_J:
        cpu->pc = (cpu->pc & 0xF0000000) | (IMM26(instr) << 2);
        break;
    case OP_JAL:
        cpu->regs[REG_RA] = cpu->pc;
        cpu->pc = (cpu->pc & 0xF0000000) | (IMM26(instr) << 2);
        break;
    case OP_JR:
        cpu->pc = cpu->regs[rs1];
        break;
    case OP_JALR:
        tmp = cpu->pc;
        cpu->pc = cpu->regs[rs1];
        cpu->regs[REG_RA] = tmp;
        break;

    case OP_TRAP:
        dlx_exception(sim, EXC_SYS);
        break;

    case OP_RFE:
        /* Return from exception: restore status */
        cpu->status = (cpu->status & ~0x3F) | ((cpu->status >> 2) & 0x3F);
        break;

    default:
        fprintf(stderr, "Unknown opcode: 0x%x at PC=0x%x\n", op, cpu->pc - 4);
        dlx_exception(sim, EXC_RI);
        break;
    }

    /* Enforce R0 = 0 again */
    cpu->regs[REG_ZERO] = 0;
}

/* Single step execution */
void dlx_step(dlx_sim_t *sim) {
    uint32_t instr;

    if (!sim->cpu.running) return;

    /* Fetch instruction */
    instr = dlx_mem_read_word(sim, sim->cpu.pc);
    sim->cpu.pc += 4;
    sim->cpu.cycles++;

    /* Execute */
    dlx_execute_instruction(sim, instr);
}

/* Run simulator */
void dlx_run(dlx_sim_t *sim) {
    while (sim->cpu.running) {
        dlx_step(sim);

        /* Check for CTRL-C or limits */
        if (sim->cpu.cycles > 10000000) {  /* 10M cycle limit for safety */
            fprintf(stderr, "\nCycle limit reached\n");
            break;
        }
    }

    if (sim->verbose) {
        printf("\nSimulation stopped after %llu cycles\n",
               (unsigned long long)sim->cpu.cycles);
        dlx_dump_regs(sim);
    }
}

/* Register names */
const char *dlx_reg_name(int reg) {
    static char *names[] = {
        "r0",  "r1",  "r2",  "r3",  "r4",  "r5",  "r6",  "r7",
        "r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15",
        "r16", "r17", "r18", "r19", "r20", "r21", "r22", "r23",
        "r24", "r25", "r26", "r27", "r28", "r29", "r30", "r31"
    };
    if (reg >= 0 && reg < 32) return names[reg];
    return "??";
}

/* Dump registers */
void dlx_dump_regs(dlx_sim_t *sim) {
    int i;
    printf("\nDLX Registers:\n");
    for (i = 0; i < 32; i += 4) {
        printf("  %s=%08x  %s=%08x  %s=%08x  %s=%08x\n",
               dlx_reg_name(i), sim->cpu.regs[i],
               dlx_reg_name(i+1), sim->cpu.regs[i+1],
               dlx_reg_name(i+2), sim->cpu.regs[i+2],
               dlx_reg_name(i+3), sim->cpu.regs[i+3]);
    }
    printf("  PC  =%08x  Status=%08x  Cause=%08x  EPC=%08x\n",
           sim->cpu.pc, sim->cpu.status, sim->cpu.cause, sim->cpu.epc);
    printf("  HI  =%08x  LO    =%08x\n", sim->cpu.hi, sim->cpu.lo);
}

/* Simple disassembler */
void dlx_disassemble(uint32_t instr, uint32_t pc) {
    int op = OPCODE(instr);
    int rs1 = RS1(instr);
    int rs2 = RS2(instr);
    int rd = RD(instr);
    int func = FUNC(instr);
    int16_t imm = IMM16(instr);

    switch (op) {
    case OP_SPECIAL:
        switch (func) {
        case FUNC_NOP:   printf("nop\n"); break;
        case FUNC_ADD:   printf("add %s,%s,%s\n", dlx_reg_name(rd), dlx_reg_name(rs1), dlx_reg_name(rs2)); break;
        case FUNC_SUB:   printf("sub %s,%s,%s\n", dlx_reg_name(rd), dlx_reg_name(rs1), dlx_reg_name(rs2)); break;
        case FUNC_AND:   printf("and %s,%s,%s\n", dlx_reg_name(rd), dlx_reg_name(rs1), dlx_reg_name(rs2)); break;
        case FUNC_OR:    printf("or %s,%s,%s\n", dlx_reg_name(rd), dlx_reg_name(rs1), dlx_reg_name(rs2)); break;
        case FUNC_XOR:   printf("xor %s,%s,%s\n", dlx_reg_name(rd), dlx_reg_name(rs1), dlx_reg_name(rs2)); break;
        case FUNC_SLL:   printf("sll %s,%s,%s\n", dlx_reg_name(rd), dlx_reg_name(rs1), dlx_reg_name(rs2)); break;
        case FUNC_SRL:   printf("srl %s,%s,%s\n", dlx_reg_name(rd), dlx_reg_name(rs1), dlx_reg_name(rs2)); break;
        case FUNC_SRA:   printf("sra %s,%s,%s\n", dlx_reg_name(rd), dlx_reg_name(rs1), dlx_reg_name(rs2)); break;
        case FUNC_MULT:  printf("mult %s,%s,%s\n", dlx_reg_name(rd), dlx_reg_name(rs1), dlx_reg_name(rs2)); break;
        case FUNC_DIV:   printf("div %s,%s,%s\n", dlx_reg_name(rd), dlx_reg_name(rs1), dlx_reg_name(rs2)); break;
        default:         printf("special(0x%x)\n", func); break;
        }
        break;
    case OP_ADDI:  printf("addi %s,%s,#%d\n", dlx_reg_name(rd), dlx_reg_name(rs1), imm); break;
    case OP_SUBI:  printf("subi %s,%s,#%d\n", dlx_reg_name(rd), dlx_reg_name(rs1), imm); break;
    case OP_ANDI:  printf("andi %s,%s,#%d\n", dlx_reg_name(rd), dlx_reg_name(rs1), imm); break;
    case OP_ORI:   printf("ori %s,%s,#%d\n", dlx_reg_name(rd), dlx_reg_name(rs1), imm); break;
    case OP_XORI:  printf("xori %s,%s,#%d\n", dlx_reg_name(rd), dlx_reg_name(rs1), imm); break;
    case OP_LHI:   printf("lhi %s,#%d\n", dlx_reg_name(rd), imm); break;
    case OP_SLLI:  printf("slli %s,%s,#%d\n", dlx_reg_name(rd), dlx_reg_name(rs1), imm); break;
    case OP_SRLI:  printf("srli %s,%s,#%d\n", dlx_reg_name(rd), dlx_reg_name(rs1), imm); break;
    case OP_SRAI:  printf("srai %s,%s,#%d\n", dlx_reg_name(rd), dlx_reg_name(rs1), imm); break;
    case OP_LW:    printf("lw %s,%d(%s)\n", dlx_reg_name(rd), imm, dlx_reg_name(rs1)); break;
    case OP_LH:    printf("lh %s,%d(%s)\n", dlx_reg_name(rd), imm, dlx_reg_name(rs1)); break;
    case OP_LB:    printf("lb %s,%d(%s)\n", dlx_reg_name(rd), imm, dlx_reg_name(rs1)); break;
    case OP_SW:    printf("sw %s,%d(%s)\n", dlx_reg_name(rd), imm, dlx_reg_name(rs1)); break;
    case OP_SH:    printf("sh %s,%d(%s)\n", dlx_reg_name(rd), imm, dlx_reg_name(rs1)); break;
    case OP_SB:    printf("sb %s,%d(%s)\n", dlx_reg_name(rd), imm, dlx_reg_name(rs1)); break;
    case OP_BEQZ:  printf("beqz %s,0x%x\n", dlx_reg_name(rs1), pc + (imm << 2)); break;
    case OP_BNEZ:  printf("bnez %s,0x%x\n", dlx_reg_name(rs1), pc + (imm << 2)); break;
    case OP_J:     printf("j 0x%x\n", (pc & 0xF0000000) | (IMM26(instr) << 2)); break;
    case OP_JAL:   printf("jal 0x%x\n", (pc & 0xF0000000) | (IMM26(instr) << 2)); break;
    case OP_JR:    printf("jr %s\n", dlx_reg_name(rs1)); break;
    case OP_JALR:  printf("jalr %s\n", dlx_reg_name(rs1)); break;
    case OP_TRAP:  printf("trap #%d\n", IMM26(instr)); break;
    default:       printf("unknown(0x%x)\n", op); break;
    }
}

/* Main program */
int main(int argc, char **argv) {
    dlx_sim_t *sim;
    char *binary = NULL;
    uint32_t load_addr = 0;
    int c;

    sim = dlx_create();
    if (!sim) return 1;

    /* Parse command line */
    while ((c = getopt(argc, argv, "vb:a:th")) != -1) {
        switch (c) {
        case 'v':
            sim->verbose = 1;
            break;
        case 't':
            sim->trace = 1;
            break;
        case 'b':
            binary = optarg;
            break;
        case 'a':
            load_addr = strtoul(optarg, NULL, 0);
            break;
        case 'h':
        default:
            printf("Usage: %s [options]\n", argv[0]);
            printf("Options:\n");
            printf("  -v        Verbose output\n");
            printf("  -t        Trace instructions\n");
            printf("  -b <file> Load binary file\n");
            printf("  -a <addr> Load address (default 0)\n");
            printf("  -h        Show this help\n");
            return 0;
        }
    }

    /* Load binary if specified */
    if (binary) {
        if (dlx_load_binary(sim, binary, load_addr) < 0) {
            fprintf(stderr, "Failed to load binary: %s\n", binary);
            dlx_destroy(sim);
            return 1;
        }
        if (sim->verbose) {
            printf("Loaded %s at 0x%08x\n", binary, load_addr);
        }
    }

    /* Run simulation */
    if (sim->verbose) {
        printf("Starting DLX simulator\n");
        printf("Memory: %d bytes\n", sim->memory.size);
    }

    dlx_run(sim);

    dlx_destroy(sim);
    return 0;
}
