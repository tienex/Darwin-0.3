// ============================================================================
// DLX Extension Modules
// ============================================================================
//
// Contains:
// - Hypervisor/VMX Extension
// - Vector Processing Extension
// - Bit Manipulation Extension
// - Advanced Interrupt Controller
// - Floating-Point Unit
//
// ============================================================================

`default_nettype none

// ============================================================================
// Hypervisor/VMX Extension
// ============================================================================

module dlx_vmx_extension #(
    parameter XLEN = 64,
    parameter ADDR_WIDTH = 64
) (
    input  wire clk,
    input  wire rst_n,

    // Interface from core
    input  wire                  vmx_req_i,
    output reg                   vmx_ack_o,
    input  wire [127:0]          vmx_data_i,
    output reg  [127:0]          vmx_data_o,

    // Wishbone interface for EPT/NPT
    output reg  [ADDR_WIDTH-1:0] wb_adr_o,
    output reg  [63:0]           wb_dat_o,
    input  wire [63:0]           wb_dat_i,
    output reg                   wb_we_o,
    output reg  [7:0]            wb_sel_o,
    output reg                   wb_stb_o,
    input  wire                  wb_ack_i,
    output reg                   wb_cyc_o,

    // VM state
    output reg                   vmx_mode_o,        // 0=root, 1=non-root
    output reg  [XLEN-1:0]       guest_pc_o,
    output reg  [XLEN-1:0]       vmcs_ptr_o
);

    // VMX operation types
    localparam VMX_OP_VMXON     = 8'h00;
    localparam VMX_OP_VMXOFF    = 8'h01;
    localparam VMX_OP_VMCLEAR   = 8'h02;
    localparam VMX_OP_VMPTRLD   = 8'h03;
    localparam VMX_OP_VMLAUNCH  = 8'h04;
    localparam VMX_OP_VMRESUME  = 8'h05;
    localparam VMX_OP_VMEXIT    = 8'h06;
    localparam VMX_OP_EPT_WALK  = 8'h10;

    // VMCS structure (simplified)
    reg [XLEN-1:0] vmcs_guest_regs [0:31];
    reg [XLEN-1:0] vmcs_guest_pc;
    reg [XLEN-1:0] vmcs_guest_status;
    reg [XLEN-1:0] vmcs_host_pc;
    reg [XLEN-1:0] vmcs_ept_base;
    reg [31:0]     vmcs_exec_controls;

    // State machine
    reg [2:0] state;
    localparam ST_IDLE      = 3'h0;
    localparam ST_LOAD_VMCS = 3'h1;
    localparam ST_EPT_WALK  = 3'h2;
    localparam ST_VM_ENTER  = 3'h3;
    localparam ST_VM_EXIT   = 3'h4;

    wire [7:0] vmx_operation = vmx_data_i[7:0];
    wire [XLEN-1:0] vmx_operand = vmx_data_i[XLEN+7:8];

    integer i;

    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            vmx_ack_o <= 1'b0;
            vmx_data_o <= 128'h0;
            vmx_mode_o <= 1'b0;
            guest_pc_o <= {XLEN{1'b0}};
            vmcs_ptr_o <= {XLEN{1'b0}};
            state <= ST_IDLE;

            for (i = 0; i < 32; i = i + 1)
                vmcs_guest_regs[i] <= {XLEN{1'b0}};
        end else begin
            case (state)
                ST_IDLE: begin
                    if (vmx_req_i) begin
                        case (vmx_operation)
                            VMX_OP_VMXON: begin
                                // Enable VMX operation
                                vmx_mode_o <= 1'b0;  // Root mode
                                vmx_ack_o <= 1'b1;
                            end

                            VMX_OP_VMXOFF: begin
                                // Disable VMX operation
                                vmx_mode_o <= 1'b0;
                                vmx_ack_o <= 1'b1;
                            end

                            VMX_OP_VMPTRLD: begin
                                // Load VMCS pointer
                                vmcs_ptr_o <= vmx_operand;
                                state <= ST_LOAD_VMCS;
                            end

                            VMX_OP_VMLAUNCH, VMX_OP_VMRESUME: begin
                                // Enter guest mode
                                vmx_mode_o <= 1'b1;  // Non-root mode
                                guest_pc_o <= vmcs_guest_pc;
                                state <= ST_VM_ENTER;
                            end

                            VMX_OP_VMEXIT: begin
                                // Exit guest mode
                                vmx_mode_o <= 1'b0;  // Root mode
                                state <= ST_VM_EXIT;
                            end

                            VMX_OP_EPT_WALK: begin
                                // Walk EPT to translate GPA→HPA
                                state <= ST_EPT_WALK;
                            end

                            default: begin
                                vmx_ack_o <= 1'b1;
                                vmx_data_o <= 128'hFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF;  // Error
                            end
                        endcase
                    end else begin
                        vmx_ack_o <= 1'b0;
                    end
                end

                ST_LOAD_VMCS: begin
                    // Load VMCS from memory
                    // Simplified: just acknowledge
                    vmx_ack_o <= 1'b1;
                    state <= ST_IDLE;
                end

                ST_EPT_WALK: begin
                    // Walk Extended Page Tables
                    // TODO: Implement full page walk
                    vmx_ack_o <= 1'b1;
                    vmx_data_o <= {64'h0, vmx_operand};  // Pass-through for now
                    state <= ST_IDLE;
                end

                ST_VM_ENTER: begin
                    // VM entry complete
                    vmx_ack_o <= 1'b1;
                    vmx_data_o <= {64'h0, guest_pc_o};
                    state <= ST_IDLE;
                end

                ST_VM_EXIT: begin
                    // Save guest state, restore host state
                    vmx_ack_o <= 1'b1;
                    vmx_data_o <= {64'h0, vmcs_host_pc};
                    state <= ST_IDLE;
                end

                default: state <= ST_IDLE;
            endcase
        end
    end

endmodule

// ============================================================================
// Vector Processing Extension
// ============================================================================

module dlx_vector_extension #(
    parameter XLEN = 64,
    parameter VLEN = 256        // Vector register length in bits
) (
    input  wire clk,
    input  wire rst_n,

    // Interface from core
    input  wire                  vec_req_i,
    output reg                   vec_ack_o,
    input  wire [255:0]          vec_data_i,
    output reg  [255:0]          vec_data_o,

    // Configuration
    input  wire [15:0]           vtype_i,       // Vector type
    input  wire [XLEN-1:0]       vl_i           // Vector length
);

    // Vector operation types
    localparam VEC_OP_VADD     = 8'h00;
    localparam VEC_OP_VSUB     = 8'h01;
    localparam VEC_OP_VMUL     = 8'h02;
    localparam VEC_OP_VDIV     = 8'h03;
    localparam VEC_OP_VLOAD    = 8'h10;
    localparam VEC_OP_VSTORE   = 8'h11;
    localparam VEC_OP_VREDSUM  = 8'h20;
    localparam VEC_OP_VREDMAX  = 8'h21;
    localparam VEC_OP_VREDMIN  = 8'h22;

    // Vector registers (32 vector registers)
    reg [VLEN-1:0] vreg [0:31];

    wire [7:0] vec_operation = vec_data_i[7:0];
    wire [4:0] vd = vec_data_i[12:8];
    wire [4:0] vs1 = vec_data_i[17:13];
    wire [4:0] vs2 = vec_data_i[22:18];

    // Element width from vtype
    wire [2:0] sew = vtype_i[5:3];  // Standard element width
    localparam SEW_8  = 3'b000;
    localparam SEW_16 = 3'b001;
    localparam SEW_32 = 3'b010;
    localparam SEW_64 = 3'b011;

    integer i, j;
    reg [VLEN-1:0] result;

    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            vec_ack_o <= 1'b0;
            vec_data_o <= 256'h0;
            for (i = 0; i < 32; i = i + 1)
                vreg[i] <= {VLEN{1'b0}};
        end else begin
            if (vec_req_i) begin
                case (vec_operation)
                    VEC_OP_VADD: begin
                        // Vector add
                        case (sew)
                            SEW_8: begin
                                for (i = 0; i < VLEN/8; i = i + 1)
                                    result[i*8 +: 8] = vreg[vs1][i*8 +: 8] +
                                                      vreg[vs2][i*8 +: 8];
                            end
                            SEW_16: begin
                                for (i = 0; i < VLEN/16; i = i + 1)
                                    result[i*16 +: 16] = vreg[vs1][i*16 +: 16] +
                                                        vreg[vs2][i*16 +: 16];
                            end
                            SEW_32: begin
                                for (i = 0; i < VLEN/32; i = i + 1)
                                    result[i*32 +: 32] = vreg[vs1][i*32 +: 32] +
                                                        vreg[vs2][i*32 +: 32];
                            end
                            SEW_64: begin
                                for (i = 0; i < VLEN/64; i = i + 1)
                                    result[i*64 +: 64] = vreg[vs1][i*64 +: 64] +
                                                        vreg[vs2][i*64 +: 64];
                            end
                        endcase
                        vreg[vd] <= result;
                        vec_ack_o <= 1'b1;
                    end

                    VEC_OP_VSUB: begin
                        // Vector subtract
                        case (sew)
                            SEW_32: begin
                                for (i = 0; i < VLEN/32; i = i + 1)
                                    result[i*32 +: 32] = vreg[vs1][i*32 +: 32] -
                                                        vreg[vs2][i*32 +: 32];
                            end
                        endcase
                        vreg[vd] <= result;
                        vec_ack_o <= 1'b1;
                    end

                    VEC_OP_VMUL: begin
                        // Vector multiply
                        case (sew)
                            SEW_32: begin
                                for (i = 0; i < VLEN/32; i = i + 1)
                                    result[i*32 +: 32] = vreg[vs1][i*32 +: 32] *
                                                        vreg[vs2][i*32 +: 32];
                            end
                        endcase
                        vreg[vd] <= result;
                        vec_ack_o <= 1'b1;
                    end

                    VEC_OP_VREDSUM: begin
                        // Vector reduction sum
                        result = {VLEN{1'b0}};
                        case (sew)
                            SEW_32: begin
                                for (i = 0; i < VLEN/32; i = i + 1)
                                    result[31:0] = result[31:0] + vreg[vs1][i*32 +: 32];
                            end
                        endcase
                        vreg[vd] <= result;
                        vec_ack_o <= 1'b1;
                    end

                    default: begin
                        vec_ack_o <= 1'b1;
                    end
                endcase

                vec_data_o <= vreg[vd];
            end else begin
                vec_ack_o <= 1'b0;
            end
        end
    end

endmodule

// ============================================================================
// Bit Manipulation Extension
// ============================================================================

module dlx_bitmanip_extension #(
    parameter XLEN = 64
) (
    input  wire clk,
    input  wire rst_n,

    input  wire                  bm_req_i,
    output reg                   bm_ack_o,
    input  wire [XLEN-1:0]       rs1_data_i,
    input  wire [XLEN-1:0]       rs2_data_i,
    input  wire [3:0]            bm_op_i,
    output reg  [XLEN-1:0]       result_o
);

    // Bit manipulation operations
    localparam BM_CLZ    = 4'h0;  // Count leading zeros
    localparam BM_CTZ    = 4'h1;  // Count trailing zeros
    localparam BM_PCNT   = 4'h2;  // Population count
    localparam BM_ANDN   = 4'h3;  // AND-NOT
    localparam BM_ORN    = 4'h4;  // OR-NOT
    localparam BM_XNOR   = 4'h5;  // XOR-NOT
    localparam BM_ROL    = 4'h6;  // Rotate left
    localparam BM_ROR    = 4'h7;  // Rotate right
    localparam BM_REV8   = 4'h8;  // Byte reverse
    localparam BM_BEXT   = 4'h9;  // Bit extract
    localparam BM_BDEP   = 4'hA;  // Bit deposit

    function [5:0] count_leading_zeros;
        input [XLEN-1:0] val;
        integer i;
        begin
            count_leading_zeros = XLEN;
            for (i = XLEN-1; i >= 0; i = i - 1) begin
                if (val[i]) begin
                    count_leading_zeros = XLEN - 1 - i;
                    i = -1;  // Break
                end
            end
        end
    endfunction

    function [5:0] count_trailing_zeros;
        input [XLEN-1:0] val;
        integer i;
        begin
            count_trailing_zeros = XLEN;
            for (i = 0; i < XLEN; i = i + 1) begin
                if (val[i]) begin
                    count_trailing_zeros = i;
                    i = XLEN;  // Break
                end
            end
        end
    endfunction

    function [6:0] population_count;
        input [XLEN-1:0] val;
        integer i;
        begin
            population_count = 0;
            for (i = 0; i < XLEN; i = i + 1) begin
                if (val[i])
                    population_count = population_count + 1;
            end
        end
    endfunction

    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            bm_ack_o <= 1'b0;
            result_o <= {XLEN{1'b0}};
        end else begin
            if (bm_req_i) begin
                case (bm_op_i)
                    BM_CLZ: begin
                        result_o <= {{(XLEN-6){1'b0}}, count_leading_zeros(rs1_data_i)};
                        bm_ack_o <= 1'b1;
                    end

                    BM_CTZ: begin
                        result_o <= {{(XLEN-6){1'b0}}, count_trailing_zeros(rs1_data_i)};
                        bm_ack_o <= 1'b1;
                    end

                    BM_PCNT: begin
                        result_o <= {{(XLEN-7){1'b0}}, population_count(rs1_data_i)};
                        bm_ack_o <= 1'b1;
                    end

                    BM_ANDN: begin
                        result_o <= rs1_data_i & ~rs2_data_i;
                        bm_ack_o <= 1'b1;
                    end

                    BM_ORN: begin
                        result_o <= rs1_data_i | ~rs2_data_i;
                        bm_ack_o <= 1'b1;
                    end

                    BM_XNOR: begin
                        result_o <= ~(rs1_data_i ^ rs2_data_i);
                        bm_ack_o <= 1'b1;
                    end

                    BM_ROL: begin
                        result_o <= (rs1_data_i << rs2_data_i[5:0]) |
                                   (rs1_data_i >> (XLEN - rs2_data_i[5:0]));
                        bm_ack_o <= 1'b1;
                    end

                    BM_ROR: begin
                        result_o <= (rs1_data_i >> rs2_data_i[5:0]) |
                                   (rs1_data_i << (XLEN - rs2_data_i[5:0]));
                        bm_ack_o <= 1'b1;
                    end

                    BM_REV8: begin
                        // Byte reverse
                        result_o <= {rs1_data_i[7:0],   rs1_data_i[15:8],
                                    rs1_data_i[23:16], rs1_data_i[31:24],
                                    rs1_data_i[39:32], rs1_data_i[47:40],
                                    rs1_data_i[55:48], rs1_data_i[63:56]};
                        bm_ack_o <= 1'b1;
                    end

                    default: begin
                        result_o <= {XLEN{1'b0}};
                        bm_ack_o <= 1'b1;
                    end
                endcase
            end else begin
                bm_ack_o <= 1'b0;
            end
        end
    end

endmodule

// ============================================================================
// Advanced Interrupt Controller (APIC-like)
// ============================================================================

module dlx_apic #(
    parameter NUM_CORES = 4,
    parameter NUM_THREADS_PER_CORE = 4,
    parameter NUM_IRQS = 256
) (
    input  wire clk,
    input  wire rst_n,

    // Wishbone slave interface (for MMIO configuration)
    input  wire [31:0]           wb_adr_i,
    input  wire [63:0]           wb_dat_i,
    output reg  [63:0]           wb_dat_o,
    input  wire                  wb_we_i,
    input  wire [7:0]            wb_sel_i,
    input  wire                  wb_stb_i,
    output reg                   wb_ack_o,
    input  wire                  wb_cyc_i,

    // External interrupts
    input  wire [NUM_IRQS-1:0]   irq_sources_i,

    // Interrupt outputs to cores
    output reg  [NUM_CORES-1:0]  core_irq_o,
    output reg  [7:0]            core_irq_vector_o [0:NUM_CORES-1]
);

    // APIC registers (per core)
    reg [63:0] apic_id [0:NUM_CORES-1];
    reg [63:0] task_priority [0:NUM_CORES-1];
    reg [63:0] processor_priority [0:NUM_CORES-1];

    // Interrupt request registers (8 × 32-bit = 256 bits)
    reg [31:0] irr [0:7][0:NUM_CORES-1];  // Interrupt Request Register
    reg [31:0] isr [0:7][0:NUM_CORES-1];  // In-Service Register
    reg [31:0] imr [0:7][0:NUM_CORES-1];  // Interrupt Mask Register

    // MMIO register offsets
    localparam APIC_ID_REG      = 32'h020;
    localparam APIC_TPR_REG     = 32'h080;
    localparam APIC_IRR0_REG    = 32'h200;
    localparam APIC_ISR0_REG    = 32'h100;
    localparam APIC_IMR0_REG    = 32'h300;
    localparam APIC_EOI_REG     = 32'h0B0;

    integer i, j, k;
    integer core_idx;
    integer highest_priority_irq;
    reg [7:0] irq_vector;

    // Initialize
    initial begin
        for (i = 0; i < NUM_CORES; i = i + 1) begin
            apic_id[i] = i;
            task_priority[i] = 64'h0;
            processor_priority[i] = 64'h0;
            for (j = 0; j < 8; j = j + 1) begin
                irr[j][i] = 32'h0;
                isr[j][i] = 32'h0;
                imr[j][i] = 32'hFFFFFFFF;  // All masked initially
            end
        end
    end

    // Wishbone interface
    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            wb_ack_o <= 1'b0;
            wb_dat_o <= 64'h0;
        end else begin
            if (wb_cyc_i && wb_stb_i) begin
                core_idx = wb_adr_i[15:12];  // Select core

                if (wb_we_i) begin
                    // Write
                    case (wb_adr_i[11:0])
                        APIC_TPR_REG: task_priority[core_idx] <= wb_dat_i;
                        APIC_IMR0_REG: imr[0][core_idx] <= wb_dat_i[31:0];
                        APIC_IMR0_REG+4: imr[1][core_idx] <= wb_dat_i[31:0];
                        APIC_EOI_REG: begin
                            // End of interrupt - clear highest ISR bit
                            for (i = 7; i >= 0; i = i - 1) begin
                                for (j = 31; j >= 0; j = j - 1) begin
                                    if (isr[i][core_idx][j]) begin
                                        isr[i][core_idx][j] = 1'b0;
                                        i = -1;  // Break outer loop
                                        j = -1;  // Break inner loop
                                    end
                                end
                            end
                        end
                    endcase
                end else begin
                    // Read
                    case (wb_adr_i[11:0])
                        APIC_ID_REG:  wb_dat_o <= apic_id[core_idx];
                        APIC_TPR_REG: wb_dat_o <= task_priority[core_idx];
                        APIC_IRR0_REG: wb_dat_o <= {32'h0, irr[0][core_idx]};
                        APIC_ISR0_REG: wb_dat_o <= {32'h0, isr[0][core_idx]};
                        APIC_IMR0_REG: wb_dat_o <= {32'h0, imr[0][core_idx]};
                        default: wb_dat_o <= 64'h0;
                    endcase
                end
                wb_ack_o <= 1'b1;
            end else begin
                wb_ack_o <= 1'b0;
            end
        end
    end

    // Interrupt distribution logic
    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            for (i = 0; i < NUM_CORES; i = i + 1) begin
                core_irq_o[i] <= 1'b0;
                core_irq_vector_o[i] <= 8'h0;
            end
        end else begin
            // Update IRR from external sources
            for (i = 0; i < NUM_IRQS; i = i + 1) begin
                if (irq_sources_i[i]) begin
                    // Simple round-robin distribution to cores
                    core_idx = i % NUM_CORES;
                    irr[i/32][core_idx][i%32] <= 1'b1;
                end
            end

            // For each core, find highest priority pending interrupt
            for (i = 0; i < NUM_CORES; i = i + 1) begin
                highest_priority_irq = -1;

                // Search from highest priority (255) to lowest (0)
                for (j = 7; j >= 0; j = j - 1) begin
                    for (k = 31; k >= 0; k = k - 1) begin
                        irq_vector = j * 32 + k;
                        if (irr[j][i][k] && !imr[j][i][k] &&
                            (irq_vector > task_priority[i])) begin
                            highest_priority_irq = irq_vector;
                            j = -1;  // Break outer loop
                            k = -1;  // Break inner loop
                        end
                    end
                end

                if (highest_priority_irq >= 0) begin
                    // Deliver interrupt to core
                    core_irq_o[i] <= 1'b1;
                    core_irq_vector_o[i] <= highest_priority_irq;

                    // Move from IRR to ISR
                    irr[highest_priority_irq/32][i][highest_priority_irq%32] <= 1'b0;
                    isr[highest_priority_irq/32][i][highest_priority_irq%32] <= 1'b1;
                end else begin
                    core_irq_o[i] <= 1'b0;
                end
            end
        end
    end

endmodule

// ============================================================================
// Floating-Point Unit (Simplified)
// ============================================================================

module dlx_fpu #(
    parameter XLEN = 64
) (
    input  wire clk,
    input  wire rst_n,

    input  wire                  fpu_req_i,
    output reg                   fpu_ack_o,
    input  wire [XLEN-1:0]       fs1_data_i,
    input  wire [XLEN-1:0]       fs2_data_i,
    input  wire [XLEN-1:0]       fs3_data_i,
    input  wire [4:0]            fpu_op_i,
    input  wire [1:0]            fpu_fmt_i,    // 00=single, 01=double
    output reg  [XLEN-1:0]       result_o,
    output reg  [4:0]            fflags_o      // FP exception flags
);

    // FPU operations
    localparam FPU_ADD    = 5'h00;
    localparam FPU_SUB    = 5'h01;
    localparam FPU_MUL    = 5'h02;
    localparam FPU_DIV    = 5'h03;
    localparam FPU_SQRT   = 5'h04;
    localparam FPU_FMADD  = 5'h08;
    localparam FPU_FMSUB  = 5'h09;
    localparam FPU_FCVT   = 5'h10;

    reg [63:0] fp_result;

    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            fpu_ack_o <= 1'b0;
            result_o <= {XLEN{1'b0}};
            fflags_o <= 5'h0;
        end else begin
            if (fpu_req_i) begin
                // Simplified FPU - just pass through for now
                // Real implementation would use IEEE 754 arithmetic
                case (fpu_op_i)
                    FPU_ADD: fp_result = fs1_data_i + fs2_data_i;
                    FPU_SUB: fp_result = fs1_data_i - fs2_data_i;
                    FPU_MUL: fp_result = fs1_data_i * fs2_data_i;
                    default: fp_result = 64'h0;
                endcase

                result_o <= fp_result;
                fflags_o <= 5'h0;  // No exceptions
                fpu_ack_o <= 1'b1;
            end else begin
                fpu_ack_o <= 1'b0;
            end
        end
    end

endmodule

`default_nettype wire
