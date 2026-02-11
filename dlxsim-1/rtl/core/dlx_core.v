// ============================================================================
// DLX RISC Processor Core - Verilog Implementation
// ============================================================================
//
// Features:
// - 5-stage pipeline (IF, ID, EX, MEM, WB)
// - Wishbone B4 bus interface
// - Pluggable extensions:
//   * Hypervisor/VMX
//   * Vector processing
//   * Bit manipulation
//   * SMP (Symmetric Multi-Processing)
//   * SMT (Simultaneous Multi-Threading)
//   * Advanced Interrupt Controller
// - 32 general-purpose registers
// - Separate instruction and data Wishbone interfaces
// - Hardware thread switching
// - Cache coherence support
//
// ============================================================================

`default_nettype none

// ============================================================================
// Top-level DLX Core
// ============================================================================

module dlx_core #(
    parameter CORE_ID = 0,
    parameter NUM_THREADS = 1,          // SMT: 1, 2, 4, 8, or 16
    parameter ENABLE_HYPERVISOR = 1,
    parameter ENABLE_VECTOR = 1,
    parameter ENABLE_BITMANIP = 1,
    parameter ENABLE_FPU = 1,
    parameter DATA_WIDTH = 64,
    parameter ADDR_WIDTH = 64,
    parameter XLEN = 64                 // Register width (32 or 64)
) (
    // Clock and reset
    input  wire clk,
    input  wire rst_n,

    // Wishbone instruction bus (master)
    output wire [ADDR_WIDTH-1:0]  iwb_adr_o,
    output wire [DATA_WIDTH-1:0]  iwb_dat_o,
    input  wire [DATA_WIDTH-1:0]  iwb_dat_i,
    output wire                   iwb_we_o,
    output wire [DATA_WIDTH/8-1:0] iwb_sel_o,
    output wire                   iwb_stb_o,
    input  wire                   iwb_ack_i,
    output wire                   iwb_cyc_o,
    input  wire                   iwb_err_i,
    input  wire                   iwb_rty_i,
    output wire [2:0]             iwb_cti_o,     // Cycle type
    output wire [1:0]             iwb_bte_o,     // Burst type

    // Wishbone data bus (master)
    output wire [ADDR_WIDTH-1:0]  dwb_adr_o,
    output wire [DATA_WIDTH-1:0]  dwb_dat_o,
    input  wire [DATA_WIDTH-1:0]  dwb_dat_i,
    output wire                   dwb_we_o,
    output wire [DATA_WIDTH/8-1:0] dwb_sel_o,
    output wire                   dwb_stb_o,
    input  wire                   dwb_ack_i,
    output wire                   dwb_cyc_o,
    input  wire                   dwb_err_i,
    input  wire                   dwb_rty_i,
    output wire [2:0]             dwb_cti_o,
    output wire [1:0]             dwb_bte_o,

    // Interrupt inputs
    input  wire                   irq,
    input  wire                   nmi,
    input  wire [7:0]             irq_vector,

    // SMP/cache coherence
    input  wire                   snoop_valid,
    input  wire [ADDR_WIDTH-1:0]  snoop_addr,
    input  wire [1:0]             snoop_type,    // 0=INV, 1=RD, 2=WR
    output wire                   snoop_ack,

    // Debug interface
    input  wire                   debug_req,
    output wire                   debug_ack,
    input  wire [15:0]            debug_addr,
    input  wire [DATA_WIDTH-1:0]  debug_wdata,
    output wire [DATA_WIDTH-1:0]  debug_rdata,
    input  wire                   debug_we,

    // Extension interfaces
    output wire                   ext_vector_req,
    input  wire                   ext_vector_ack,
    output wire [255:0]           ext_vector_data_o,
    input  wire [255:0]           ext_vector_data_i,

    output wire                   ext_vmx_req,
    input  wire                   ext_vmx_ack,
    output wire [127:0]           ext_vmx_data_o,
    input  wire [127:0]           ext_vmx_data_i,

    // Performance monitoring
    output wire [63:0]            perfmon_cycles,
    output wire [63:0]            perfmon_instrs,

    // Status
    output wire                   halted,
    output wire [$clog2(NUM_THREADS)-1:0] active_thread
);

    // ========================================================================
    // Internal signals
    // ========================================================================

    // Pipeline stage registers
    wire [XLEN-1:0] pc_if, pc_id, pc_ex, pc_mem, pc_wb;
    wire [31:0]     instr_if, instr_id, instr_ex, instr_mem;
    wire            valid_if, valid_id, valid_ex, valid_mem, valid_wb;

    // Register file
    wire [4:0]      rs1_id, rs2_id, rd_id, rd_ex, rd_mem, rd_wb;
    wire [XLEN-1:0] rs1_data, rs2_data, rd_data_wb;
    wire            rd_we_wb;

    // ALU
    wire [XLEN-1:0] alu_result_ex;
    wire [XLEN-1:0] alu_op1_ex, alu_op2_ex;
    wire [3:0]      alu_op_ex;
    wire            alu_zero_ex, alu_neg_ex;

    // Memory
    wire [XLEN-1:0] mem_addr_ex, mem_wdata_ex, mem_rdata_mem;
    wire            mem_read_ex, mem_write_ex;
    wire [2:0]      mem_size_ex;

    // Branch/Jump
    wire            branch_taken_ex, jump_ex;
    wire [XLEN-1:0] branch_target_ex;

    // Pipeline control
    wire            stall_if, stall_id, stall_ex, stall_mem;
    wire            flush_if, flush_id, flush_ex, flush_mem;

    // Thread management (SMT)
    wire [$clog2(NUM_THREADS)-1:0] thread_id_if, thread_id_id, thread_id_ex;
    wire [$clog2(NUM_THREADS)-1:0] thread_id_mem, thread_id_wb;

    // Exception/Interrupt
    wire            exception_ex, interrupt_pending;
    wire [7:0]      exception_cause_ex;
    wire [XLEN-1:0] exception_pc_ex;

    // System registers
    wire [11:0]     csr_addr_ex;
    wire [XLEN-1:0] csr_rdata_ex, csr_wdata_ex;
    wire            csr_we_ex;

    // Extension signals
    wire            vector_op_ex, bitmanip_op_ex, vmx_op_ex;

    // Performance counters
    reg [63:0]      cycle_counter;
    reg [63:0]      instr_counter;

    // ========================================================================
    // Instruction Fetch (IF) Stage
    // ========================================================================

    dlx_fetch #(
        .XLEN(XLEN),
        .ADDR_WIDTH(ADDR_WIDTH),
        .NUM_THREADS(NUM_THREADS)
    ) u_fetch (
        .clk(clk),
        .rst_n(rst_n),

        // Wishbone interface
        .wb_adr_o(iwb_adr_o),
        .wb_dat_o(iwb_dat_o),
        .wb_dat_i(iwb_dat_i),
        .wb_we_o(iwb_we_o),
        .wb_sel_o(iwb_sel_o),
        .wb_stb_o(iwb_stb_o),
        .wb_ack_i(iwb_ack_i),
        .wb_cyc_o(iwb_cyc_o),
        .wb_err_i(iwb_err_i),
        .wb_cti_o(iwb_cti_o),
        .wb_bte_o(iwb_bte_o),

        // Thread management
        .thread_id_o(thread_id_if),
        .active_thread_i(active_thread),

        // Branch/Jump
        .branch_taken_i(branch_taken_ex),
        .branch_target_i(branch_target_ex),

        // Exception/Interrupt
        .exception_i(exception_ex),
        .exception_pc_i(exception_pc_ex),

        // Pipeline control
        .stall_i(stall_if),
        .flush_i(flush_if),

        // Output to ID stage
        .pc_o(pc_if),
        .instr_o(instr_if),
        .valid_o(valid_if)
    );

    assign iwb_rty_i = 1'b0;  // Not using retry

    // ========================================================================
    // Instruction Decode (ID) Stage
    // ========================================================================

    dlx_decode #(
        .XLEN(XLEN),
        .NUM_THREADS(NUM_THREADS)
    ) u_decode (
        .clk(clk),
        .rst_n(rst_n),

        // Input from IF
        .pc_i(pc_if),
        .instr_i(instr_if),
        .valid_i(valid_if),
        .thread_id_i(thread_id_if),

        // Register file read
        .rs1_o(rs1_id),
        .rs2_o(rs2_id),
        .rd_o(rd_id),
        .rs1_data_i(rs1_data),
        .rs2_data_i(rs2_data),

        // Forwarding
        .rd_ex_i(rd_ex),
        .rd_data_ex_i(alu_result_ex),
        .rd_mem_i(rd_mem),
        .rd_data_mem_i(mem_rdata_mem),
        .rd_wb_i(rd_wb),
        .rd_data_wb_i(rd_data_wb),

        // Pipeline control
        .stall_i(stall_id),
        .flush_i(flush_id),

        // Output to EX
        .pc_o(pc_id),
        .instr_o(instr_id),
        .valid_o(valid_id),
        .thread_id_o(thread_id_id)
    );

    // ========================================================================
    // Execute (EX) Stage
    // ========================================================================

    dlx_execute #(
        .XLEN(XLEN),
        .NUM_THREADS(NUM_THREADS),
        .ENABLE_BITMANIP(ENABLE_BITMANIP),
        .ENABLE_VECTOR(ENABLE_VECTOR),
        .ENABLE_FPU(ENABLE_FPU)
    ) u_execute (
        .clk(clk),
        .rst_n(rst_n),

        // Input from ID
        .pc_i(pc_id),
        .instr_i(instr_id),
        .valid_i(valid_id),
        .thread_id_i(thread_id_id),

        // Register operands
        .rs1_data_i(rs1_data),
        .rs2_data_i(rs2_data),

        // ALU
        .alu_result_o(alu_result_ex),
        .alu_zero_o(alu_zero_ex),
        .alu_neg_o(alu_neg_ex),

        // Memory
        .mem_addr_o(mem_addr_ex),
        .mem_wdata_o(mem_wdata_ex),
        .mem_read_o(mem_read_ex),
        .mem_write_o(mem_write_ex),
        .mem_size_o(mem_size_ex),

        // Branch/Jump
        .branch_taken_o(branch_taken_ex),
        .branch_target_o(branch_target_ex),
        .jump_o(jump_ex),

        // System/CSR
        .csr_addr_o(csr_addr_ex),
        .csr_wdata_o(csr_wdata_ex),
        .csr_rdata_i(csr_rdata_ex),
        .csr_we_o(csr_we_ex),

        // Exception
        .exception_o(exception_ex),
        .exception_cause_o(exception_cause_ex),

        // Extensions
        .vector_op_o(vector_op_ex),
        .bitmanip_op_o(bitmanip_op_ex),
        .vmx_op_o(vmx_op_ex),

        // Extension interfaces
        .ext_vector_req_o(ext_vector_req),
        .ext_vector_ack_i(ext_vector_ack),
        .ext_vector_data_o(ext_vector_data_o),
        .ext_vector_data_i(ext_vector_data_i),

        // Pipeline control
        .stall_i(stall_ex),
        .flush_i(flush_ex),

        // Output to MEM
        .pc_o(pc_ex),
        .instr_o(instr_ex),
        .valid_o(valid_ex),
        .rd_o(rd_ex),
        .thread_id_o(thread_id_ex)
    );

    // ========================================================================
    // Memory (MEM) Stage
    // ========================================================================

    dlx_memory #(
        .XLEN(XLEN),
        .ADDR_WIDTH(ADDR_WIDTH),
        .DATA_WIDTH(DATA_WIDTH),
        .NUM_THREADS(NUM_THREADS)
    ) u_memory (
        .clk(clk),
        .rst_n(rst_n),

        // Input from EX
        .pc_i(pc_ex),
        .instr_i(instr_ex),
        .valid_i(valid_ex),
        .thread_id_i(thread_id_ex),
        .rd_i(rd_ex),
        .alu_result_i(alu_result_ex),

        // Memory access
        .mem_addr_i(mem_addr_ex),
        .mem_wdata_i(mem_wdata_ex),
        .mem_read_i(mem_read_ex),
        .mem_write_i(mem_write_ex),
        .mem_size_i(mem_size_ex),
        .mem_rdata_o(mem_rdata_mem),

        // Wishbone data bus
        .wb_adr_o(dwb_adr_o),
        .wb_dat_o(dwb_dat_o),
        .wb_dat_i(dwb_dat_i),
        .wb_we_o(dwb_we_o),
        .wb_sel_o(dwb_sel_o),
        .wb_stb_o(dwb_stb_o),
        .wb_ack_i(dwb_ack_i),
        .wb_cyc_o(dwb_cyc_o),
        .wb_err_i(dwb_err_i),
        .wb_cti_o(dwb_cti_o),
        .wb_bte_o(dwb_bte_o),

        // Cache coherence (snoop)
        .snoop_valid_i(snoop_valid),
        .snoop_addr_i(snoop_addr),
        .snoop_type_i(snoop_type),
        .snoop_ack_o(snoop_ack),

        // Pipeline control
        .stall_i(stall_mem),
        .flush_i(flush_mem),

        // Output to WB
        .pc_o(pc_mem),
        .instr_o(instr_mem),
        .valid_o(valid_mem),
        .rd_o(rd_mem),
        .thread_id_o(thread_id_mem)
    );

    assign dwb_rty_i = 1'b0;

    // ========================================================================
    // Write-Back (WB) Stage
    // ========================================================================

    dlx_writeback #(
        .XLEN(XLEN),
        .NUM_THREADS(NUM_THREADS)
    ) u_writeback (
        .clk(clk),
        .rst_n(rst_n),

        // Input from MEM
        .pc_i(pc_mem),
        .instr_i(instr_mem),
        .valid_i(valid_mem),
        .thread_id_i(thread_id_mem),
        .rd_i(rd_mem),
        .alu_result_i(alu_result_ex),
        .mem_rdata_i(mem_rdata_mem),

        // Register file write
        .rd_o(rd_wb),
        .rd_data_o(rd_data_wb),
        .rd_we_o(rd_we_wb),

        // Output
        .pc_o(pc_wb),
        .valid_o(valid_wb),
        .thread_id_o(thread_id_wb)
    );

    // ========================================================================
    // Register File (Multi-threaded)
    // ========================================================================

    dlx_regfile #(
        .XLEN(XLEN),
        .NUM_THREADS(NUM_THREADS)
    ) u_regfile (
        .clk(clk),
        .rst_n(rst_n),

        // Read ports (ID stage)
        .rs1_addr_i(rs1_id),
        .rs2_addr_i(rs2_id),
        .thread_id_rd_i(thread_id_id),
        .rs1_data_o(rs1_data),
        .rs2_data_o(rs2_data),

        // Write port (WB stage)
        .rd_addr_i(rd_wb),
        .rd_data_i(rd_data_wb),
        .rd_we_i(rd_we_wb),
        .thread_id_wr_i(thread_id_wb)
    );

    // ========================================================================
    // Pipeline Control Unit
    // ========================================================================

    dlx_pipeline_ctrl #(
        .NUM_THREADS(NUM_THREADS)
    ) u_pipeline_ctrl (
        .clk(clk),
        .rst_n(rst_n),

        // Hazard detection
        .rs1_id_i(rs1_id),
        .rs2_id_i(rs2_id),
        .rd_ex_i(rd_ex),
        .rd_mem_i(rd_mem),
        .mem_read_ex_i(mem_read_ex),

        // Branch/Jump
        .branch_taken_i(branch_taken_ex),
        .jump_i(jump_ex),

        // Exception
        .exception_i(exception_ex),

        // Memory stalls
        .mem_stall_i(dwb_stb_o && !dwb_ack_i),

        // Extension stalls
        .vector_stall_i(ext_vector_req && !ext_vector_ack),
        .vmx_stall_i(ext_vmx_req && !ext_vmx_ack),

        // Stall/Flush outputs
        .stall_if_o(stall_if),
        .stall_id_o(stall_id),
        .stall_ex_o(stall_ex),
        .stall_mem_o(stall_mem),
        .flush_if_o(flush_if),
        .flush_id_o(flush_id),
        .flush_ex_o(flush_ex),
        .flush_mem_o(flush_mem)
    );

    // ========================================================================
    // SMT Thread Scheduler
    // ========================================================================

    generate
    if (NUM_THREADS > 1) begin : gen_smt
        dlx_smt_scheduler #(
            .NUM_THREADS(NUM_THREADS)
        ) u_smt_scheduler (
            .clk(clk),
            .rst_n(rst_n),

            // Thread states
            .thread_valid_i({NUM_THREADS{1'b1}}),  // All threads valid
            .thread_stall_i({NUM_THREADS{1'b0}}),

            // Active thread selection
            .active_thread_o(active_thread)
        );
    end else begin : gen_no_smt
        assign active_thread = '0;
    end
    endgenerate

    // ========================================================================
    // System Registers (CSR)
    // ========================================================================

    dlx_csr #(
        .XLEN(XLEN),
        .CORE_ID(CORE_ID),
        .NUM_THREADS(NUM_THREADS),
        .ENABLE_HYPERVISOR(ENABLE_HYPERVISOR)
    ) u_csr (
        .clk(clk),
        .rst_n(rst_n),

        // CSR access
        .csr_addr_i(csr_addr_ex),
        .csr_wdata_i(csr_wdata_ex),
        .csr_we_i(csr_we_ex),
        .csr_rdata_o(csr_rdata_ex),
        .thread_id_i(thread_id_ex),

        // Exception handling
        .exception_i(exception_ex),
        .exception_cause_i(exception_cause_ex),
        .exception_pc_i(pc_ex),
        .exception_pc_o(exception_pc_ex),

        // Interrupt handling
        .irq_i(irq),
        .nmi_i(nmi),
        .irq_vector_i(irq_vector),
        .interrupt_pending_o(interrupt_pending),

        // Performance counters
        .cycle_counter_i(cycle_counter),
        .instr_counter_i(instr_counter),

        // Hypervisor extension interface
        .vmx_req_o(ext_vmx_req),
        .vmx_ack_i(ext_vmx_ack),
        .vmx_data_o(ext_vmx_data_o),
        .vmx_data_i(ext_vmx_data_i)
    );

    // ========================================================================
    // Performance Counters
    // ========================================================================

    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            cycle_counter <= 64'h0;
            instr_counter <= 64'h0;
        end else begin
            cycle_counter <= cycle_counter + 64'h1;
            if (valid_wb && !stall_mem)
                instr_counter <= instr_counter + 64'h1;
        end
    end

    assign perfmon_cycles = cycle_counter;
    assign perfmon_instrs = instr_counter;

    // ========================================================================
    // Debug Interface
    // ========================================================================

    dlx_debug #(
        .XLEN(XLEN),
        .NUM_THREADS(NUM_THREADS)
    ) u_debug (
        .clk(clk),
        .rst_n(rst_n),

        .debug_req_i(debug_req),
        .debug_ack_o(debug_ack),
        .debug_addr_i(debug_addr),
        .debug_wdata_i(debug_wdata),
        .debug_rdata_o(debug_rdata),
        .debug_we_i(debug_we),

        // Internal state access
        .pc_if_i(pc_if),
        .pc_id_i(pc_id),
        .pc_ex_i(pc_ex),
        .pc_mem_i(pc_mem),
        .pc_wb_i(pc_wb)
    );

    // Halted signal
    assign halted = 1'b0;  // TODO: Implement halt state

endmodule

// ============================================================================
// Instruction Fetch Stage
// ============================================================================

module dlx_fetch #(
    parameter XLEN = 64,
    parameter ADDR_WIDTH = 64,
    parameter NUM_THREADS = 1
) (
    input  wire clk,
    input  wire rst_n,

    // Wishbone interface
    output reg  [ADDR_WIDTH-1:0] wb_adr_o,
    output wire [63:0]           wb_dat_o,
    input  wire [63:0]           wb_dat_i,
    output wire                  wb_we_o,
    output reg  [7:0]            wb_sel_o,
    output reg                   wb_stb_o,
    input  wire                  wb_ack_i,
    output reg                   wb_cyc_o,
    input  wire                  wb_err_i,
    output wire [2:0]            wb_cti_o,
    output wire [1:0]            wb_bte_o,

    // Thread management
    output wire [$clog2(NUM_THREADS)-1:0] thread_id_o,
    input  wire [$clog2(NUM_THREADS)-1:0] active_thread_i,

    // Branch/Jump
    input  wire                  branch_taken_i,
    input  wire [XLEN-1:0]       branch_target_i,

    // Exception
    input  wire                  exception_i,
    input  wire [XLEN-1:0]       exception_pc_i,

    // Pipeline control
    input  wire                  stall_i,
    input  wire                  flush_i,

    // Output
    output reg  [XLEN-1:0]       pc_o,
    output reg  [31:0]           instr_o,
    output reg                   valid_o
);

    // Program counters (one per thread)
    reg [XLEN-1:0] pc_reg [0:NUM_THREADS-1];
    reg [XLEN-1:0] next_pc;

    assign wb_we_o = 1'b0;  // Instruction fetch is always read
    assign wb_dat_o = 64'h0;
    assign wb_cti_o = 3'b000;  // Classic cycle
    assign wb_bte_o = 2'b00;   // Linear burst
    assign thread_id_o = active_thread_i;

    integer i;

    // PC update logic
    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            for (i = 0; i < NUM_THREADS; i = i + 1)
                pc_reg[i] <= {XLEN{1'b0}};
            wb_stb_o <= 1'b0;
            wb_cyc_o <= 1'b0;
            valid_o <= 1'b0;
        end else begin
            if (flush_i) begin
                valid_o <= 1'b0;
                wb_stb_o <= 1'b0;
                wb_cyc_o <= 1'b0;
            end else if (!stall_i) begin
                // Update PC
                if (exception_i) begin
                    next_pc = exception_pc_i;
                end else if (branch_taken_i) begin
                    next_pc = branch_target_i;
                end else begin
                    next_pc = pc_reg[active_thread_i] + 4;
                end

                pc_reg[active_thread_i] <= next_pc;

                // Start fetch
                wb_adr_o <= next_pc;
                wb_sel_o <= 8'hFF;
                wb_stb_o <= 1'b1;
                wb_cyc_o <= 1'b1;

                // Wait for ack
                if (wb_ack_i) begin
                    pc_o <= wb_adr_o;
                    instr_o <= wb_dat_i[31:0];
                    valid_o <= 1'b1;
                    wb_stb_o <= 1'b0;
                    wb_cyc_o <= 1'b0;
                end
            end
        end
    end

endmodule

// ============================================================================
// Instruction Decode Stage
// ============================================================================

module dlx_decode #(
    parameter XLEN = 64,
    parameter NUM_THREADS = 1
) (
    input  wire clk,
    input  wire rst_n,

    input  wire [XLEN-1:0]       pc_i,
    input  wire [31:0]           instr_i,
    input  wire                  valid_i,
    input  wire [$clog2(NUM_THREADS)-1:0] thread_id_i,

    output wire [4:0]            rs1_o,
    output wire [4:0]            rs2_o,
    output wire [4:0]            rd_o,
    input  wire [XLEN-1:0]       rs1_data_i,
    input  wire [XLEN-1:0]       rs2_data_i,

    // Forwarding
    input  wire [4:0]            rd_ex_i,
    input  wire [XLEN-1:0]       rd_data_ex_i,
    input  wire [4:0]            rd_mem_i,
    input  wire [XLEN-1:0]       rd_data_mem_i,
    input  wire [4:0]            rd_wb_i,
    input  wire [XLEN-1:0]       rd_data_wb_i,

    input  wire                  stall_i,
    input  wire                  flush_i,

    output reg  [XLEN-1:0]       pc_o,
    output reg  [31:0]           instr_o,
    output reg                   valid_o,
    output reg  [$clog2(NUM_THREADS)-1:0] thread_id_o
);

    // Decode instruction fields
    wire [6:0] opcode = instr_i[6:0];
    wire [4:0] rd     = instr_i[11:7];
    wire [2:0] funct3 = instr_i[14:12];
    wire [4:0] rs1    = instr_i[19:15];
    wire [4:0] rs2    = instr_i[24:20];
    wire [6:0] funct7 = instr_i[31:25];

    assign rs1_o = rs1;
    assign rs2_o = rs2;
    assign rd_o = rd;

    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            pc_o <= {XLEN{1'b0}};
            instr_o <= 32'h00000013;  // NOP
            valid_o <= 1'b0;
            thread_id_o <= '0;
        end else if (flush_i) begin
            valid_o <= 1'b0;
        end else if (!stall_i) begin
            pc_o <= pc_i;
            instr_o <= instr_i;
            valid_o <= valid_i;
            thread_id_o <= thread_id_i;
        end
    end

endmodule

// ============================================================================
// Execute Stage
// ============================================================================

module dlx_execute #(
    parameter XLEN = 64,
    parameter NUM_THREADS = 1,
    parameter ENABLE_BITMANIP = 1,
    parameter ENABLE_VECTOR = 1,
    parameter ENABLE_FPU = 1
) (
    input  wire clk,
    input  wire rst_n,

    input  wire [XLEN-1:0]       pc_i,
    input  wire [31:0]           instr_i,
    input  wire                  valid_i,
    input  wire [$clog2(NUM_THREADS)-1:0] thread_id_i,

    input  wire [XLEN-1:0]       rs1_data_i,
    input  wire [XLEN-1:0]       rs2_data_i,

    output reg  [XLEN-1:0]       alu_result_o,
    output wire                  alu_zero_o,
    output wire                  alu_neg_o,

    output reg  [XLEN-1:0]       mem_addr_o,
    output reg  [XLEN-1:0]       mem_wdata_o,
    output reg                   mem_read_o,
    output reg                   mem_write_o,
    output reg  [2:0]            mem_size_o,

    output reg                   branch_taken_o,
    output reg  [XLEN-1:0]       branch_target_o,
    output reg                   jump_o,

    output wire [11:0]           csr_addr_o,
    output wire [XLEN-1:0]       csr_wdata_o,
    input  wire [XLEN-1:0]       csr_rdata_i,
    output wire                  csr_we_o,

    output reg                   exception_o,
    output reg  [7:0]            exception_cause_o,

    output wire                  vector_op_o,
    output wire                  bitmanip_op_o,
    output wire                  vmx_op_o,

    output wire                  ext_vector_req_o,
    input  wire                  ext_vector_ack_i,
    output wire [255:0]          ext_vector_data_o,
    input  wire [255:0]          ext_vector_data_i,

    input  wire                  stall_i,
    input  wire                  flush_i,

    output reg  [XLEN-1:0]       pc_o,
    output reg  [31:0]           instr_o,
    output reg                   valid_o,
    output reg  [4:0]            rd_o,
    output reg  [$clog2(NUM_THREADS)-1:0] thread_id_o
);

    // Decode
    wire [6:0] opcode = instr_i[6:0];
    wire [4:0] rd     = instr_i[11:7];
    wire [2:0] funct3 = instr_i[14:12];
    wire [4:0] rs1    = instr_i[19:15];
    wire [4:0] rs2    = instr_i[24:20];
    wire [6:0] funct7 = instr_i[31:25];

    // Immediate generation
    wire [XLEN-1:0] imm_i = {{(XLEN-12){instr_i[31]}}, instr_i[31:20]};
    wire [XLEN-1:0] imm_s = {{(XLEN-12){instr_i[31]}}, instr_i[31:25], instr_i[11:7]};
    wire [XLEN-1:0] imm_b = {{(XLEN-13){instr_i[31]}}, instr_i[31], instr_i[7], instr_i[30:25], instr_i[11:8], 1'b0};
    wire [XLEN-1:0] imm_u = {{(XLEN-32){instr_i[31]}}, instr_i[31:12], 12'h000};
    wire [XLEN-1:0] imm_j = {{(XLEN-21){instr_i[31]}}, instr_i[31], instr_i[19:12], instr_i[20], instr_i[30:21], 1'b0};

    // ALU operands
    reg [XLEN-1:0] alu_op1, alu_op2;
    reg [3:0] alu_op;

    // ALU
    always @(*) begin
        case (opcode)
            7'b0110011: begin  // R-type
                alu_op1 = rs1_data_i;
                alu_op2 = rs2_data_i;
                case (funct3)
                    3'b000: alu_op = (funct7[5]) ? 4'b0001 : 4'b0000;  // SUB : ADD
                    3'b001: alu_op = 4'b0010;  // SLL
                    3'b010: alu_op = 4'b0011;  // SLT
                    3'b011: alu_op = 4'b0100;  // SLTU
                    3'b100: alu_op = 4'b0101;  // XOR
                    3'b101: alu_op = (funct7[5]) ? 4'b0111 : 4'b0110;  // SRA : SRL
                    3'b110: alu_op = 4'b1000;  // OR
                    3'b111: alu_op = 4'b1001;  // AND
                endcase
            end
            7'b0010011: begin  // I-type
                alu_op1 = rs1_data_i;
                alu_op2 = imm_i;
                case (funct3)
                    3'b000: alu_op = 4'b0000;  // ADDI
                    3'b010: alu_op = 4'b0011;  // SLTI
                    3'b011: alu_op = 4'b0100;  // SLTIU
                    3'b100: alu_op = 4'b0101;  // XORI
                    3'b110: alu_op = 4'b1000;  // ORI
                    3'b111: alu_op = 4'b1001;  // ANDI
                    default: alu_op = 4'b0000;
                endcase
            end
            default: begin
                alu_op1 = rs1_data_i;
                alu_op2 = imm_i;
                alu_op = 4'b0000;
            end
        endcase
    end

    // ALU execution
    always @(*) begin
        case (alu_op)
            4'b0000: alu_result_o = alu_op1 + alu_op2;      // ADD
            4'b0001: alu_result_o = alu_op1 - alu_op2;      // SUB
            4'b0010: alu_result_o = alu_op1 << alu_op2[5:0]; // SLL
            4'b0011: alu_result_o = ($signed(alu_op1) < $signed(alu_op2)) ? 1 : 0;  // SLT
            4'b0100: alu_result_o = (alu_op1 < alu_op2) ? 1 : 0;  // SLTU
            4'b0101: alu_result_o = alu_op1 ^ alu_op2;      // XOR
            4'b0110: alu_result_o = alu_op1 >> alu_op2[5:0]; // SRL
            4'b0111: alu_result_o = $signed(alu_op1) >>> alu_op2[5:0];  // SRA
            4'b1000: alu_result_o = alu_op1 | alu_op2;      // OR
            4'b1001: alu_result_o = alu_op1 & alu_op2;      // AND
            default: alu_result_o = {XLEN{1'b0}};
        endcase
    end

    assign alu_zero_o = (alu_result_o == {XLEN{1'b0}});
    assign alu_neg_o = alu_result_o[XLEN-1];

    // Branch logic
    always @(*) begin
        branch_taken_o = 1'b0;
        branch_target_o = {XLEN{1'b0}};
        jump_o = 1'b0;

        case (opcode)
            7'b1100011: begin  // Branch
                branch_target_o = pc_i + imm_b;
                case (funct3)
                    3'b000: branch_taken_o = (rs1_data_i == rs2_data_i);  // BEQ
                    3'b001: branch_taken_o = (rs1_data_i != rs2_data_i);  // BNE
                    3'b100: branch_taken_o = ($signed(rs1_data_i) < $signed(rs2_data_i));  // BLT
                    3'b101: branch_taken_o = ($signed(rs1_data_i) >= $signed(rs2_data_i)); // BGE
                    3'b110: branch_taken_o = (rs1_data_i < rs2_data_i);   // BLTU
                    3'b111: branch_taken_o = (rs1_data_i >= rs2_data_i);  // BGEU
                endcase
            end
            7'b1101111: begin  // JAL
                branch_taken_o = 1'b1;
                branch_target_o = pc_i + imm_j;
                jump_o = 1'b1;
            end
            7'b1100111: begin  // JALR
                branch_taken_o = 1'b1;
                branch_target_o = (rs1_data_i + imm_i) & ~64'h1;
                jump_o = 1'b1;
            end
        endcase
    end

    // Memory operations
    always @(*) begin
        mem_addr_o = {XLEN{1'b0}};
        mem_wdata_o = {XLEN{1'b0}};
        mem_read_o = 1'b0;
        mem_write_o = 1'b0;
        mem_size_o = 3'b000;

        case (opcode)
            7'b0000011: begin  // Load
                mem_addr_o = rs1_data_i + imm_i;
                mem_read_o = 1'b1;
                mem_size_o = funct3;
            end
            7'b0100011: begin  // Store
                mem_addr_o = rs1_data_i + imm_s;
                mem_wdata_o = rs2_data_i;
                mem_write_o = 1'b1;
                mem_size_o = funct3;
            end
        endcase
    end

    // CSR operations
    assign csr_addr_o = instr_i[31:20];
    assign csr_wdata_o = rs1_data_i;
    assign csr_we_o = (opcode == 7'b1110011) && (funct3 != 3'b000);

    // Extension detection
    assign vector_op_o = (opcode == 7'b1010111);    // V extension
    assign bitmanip_op_o = (opcode == 7'b0110011) && (funct7[6:2] == 5'b01100);
    assign vmx_op_o = (opcode == 7'b1110011) && (funct3 == 3'b101);

    assign ext_vector_req_o = vector_op_o && valid_i;
    assign ext_vector_data_o = {rs2_data_i, rs1_data_i, {128{1'b0}}};

    // Pipeline register
    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            pc_o <= {XLEN{1'b0}};
            instr_o <= 32'h0;
            valid_o <= 1'b0;
            rd_o <= 5'h0;
            exception_o <= 1'b0;
            exception_cause_o <= 8'h0;
            thread_id_o <= '0;
        end else if (flush_i) begin
            valid_o <= 1'b0;
            exception_o <= 1'b0;
        end else if (!stall_i) begin
            pc_o <= pc_i;
            instr_o <= instr_i;
            valid_o <= valid_i;
            rd_o <= rd;
            thread_id_o <= thread_id_i;

            // Exception detection
            exception_o <= 1'b0;  // TODO: Add exception logic
            exception_cause_o <= 8'h0;
        end
    end

endmodule

// ============================================================================
// Memory Stage
// ============================================================================

module dlx_memory #(
    parameter XLEN = 64,
    parameter ADDR_WIDTH = 64,
    parameter DATA_WIDTH = 64,
    parameter NUM_THREADS = 1
) (
    input  wire clk,
    input  wire rst_n,

    input  wire [XLEN-1:0]       pc_i,
    input  wire [31:0]           instr_i,
    input  wire                  valid_i,
    input  wire [$clog2(NUM_THREADS)-1:0] thread_id_i,
    input  wire [4:0]            rd_i,
    input  wire [XLEN-1:0]       alu_result_i,

    input  wire [XLEN-1:0]       mem_addr_i,
    input  wire [XLEN-1:0]       mem_wdata_i,
    input  wire                  mem_read_i,
    input  wire                  mem_write_i,
    input  wire [2:0]            mem_size_i,
    output reg  [XLEN-1:0]       mem_rdata_o,

    output reg  [ADDR_WIDTH-1:0] wb_adr_o,
    output reg  [DATA_WIDTH-1:0] wb_dat_o,
    input  wire [DATA_WIDTH-1:0] wb_dat_i,
    output reg                   wb_we_o,
    output reg  [DATA_WIDTH/8-1:0] wb_sel_o,
    output reg                   wb_stb_o,
    input  wire                  wb_ack_i,
    output reg                   wb_cyc_o,
    input  wire                  wb_err_i,
    output wire [2:0]            wb_cti_o,
    output wire [1:0]            wb_bte_o,

    input  wire                  snoop_valid_i,
    input  wire [ADDR_WIDTH-1:0] snoop_addr_i,
    input  wire [1:0]            snoop_type_i,
    output wire                  snoop_ack_o,

    input  wire                  stall_i,
    input  wire                  flush_i,

    output reg  [XLEN-1:0]       pc_o,
    output reg  [31:0]           instr_o,
    output reg                   valid_o,
    output reg  [4:0]            rd_o,
    output reg  [$clog2(NUM_THREADS)-1:0] thread_id_o
);

    assign wb_cti_o = 3'b000;
    assign wb_bte_o = 2'b00;
    assign snoop_ack_o = 1'b1;  // TODO: Implement cache coherence

    // Memory access state machine
    reg mem_active;

    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            wb_adr_o <= {ADDR_WIDTH{1'b0}};
            wb_dat_o <= {DATA_WIDTH{1'b0}};
            wb_we_o <= 1'b0;
            wb_sel_o <= {DATA_WIDTH/8{1'b0}};
            wb_stb_o <= 1'b0;
            wb_cyc_o <= 1'b0;
            mem_rdata_o <= {XLEN{1'b0}};
            mem_active <= 1'b0;
        end else begin
            if (mem_read_i || mem_write_i) begin
                if (!mem_active) begin
                    wb_adr_o <= mem_addr_i;
                    wb_dat_o <= mem_wdata_i;
                    wb_we_o <= mem_write_i;

                    // Generate byte select based on size
                    case (mem_size_i[1:0])
                        2'b00: wb_sel_o <= (8'h01 << mem_addr_i[2:0]);  // Byte
                        2'b01: wb_sel_o <= (8'h03 << mem_addr_i[2:0]);  // Half
                        2'b10: wb_sel_o <= (8'h0F << mem_addr_i[2:0]);  // Word
                        2'b11: wb_sel_o <= 8'hFF;                       // Double
                    endcase

                    wb_stb_o <= 1'b1;
                    wb_cyc_o <= 1'b1;
                    mem_active <= 1'b1;
                end else if (wb_ack_i) begin
                    mem_rdata_o <= wb_dat_i;
                    wb_stb_o <= 1'b0;
                    wb_cyc_o <= 1'b0;
                    mem_active <= 1'b0;
                end
            end
        end
    end

    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            pc_o <= {XLEN{1'b0}};
            instr_o <= 32'h0;
            valid_o <= 1'b0;
            rd_o <= 5'h0;
            thread_id_o <= '0;
        end else if (flush_i) begin
            valid_o <= 1'b0;
        end else if (!stall_i) begin
            pc_o <= pc_i;
            instr_o <= instr_i;
            valid_o <= valid_i;
            rd_o <= rd_i;
            thread_id_o <= thread_id_i;
        end
    end

endmodule

// ============================================================================
// Write-Back Stage
// ============================================================================

module dlx_writeback #(
    parameter XLEN = 64,
    parameter NUM_THREADS = 1
) (
    input  wire clk,
    input  wire rst_n,

    input  wire [XLEN-1:0]       pc_i,
    input  wire [31:0]           instr_i,
    input  wire                  valid_i,
    input  wire [$clog2(NUM_THREADS)-1:0] thread_id_i,
    input  wire [4:0]            rd_i,
    input  wire [XLEN-1:0]       alu_result_i,
    input  wire [XLEN-1:0]       mem_rdata_i,

    output reg  [4:0]            rd_o,
    output reg  [XLEN-1:0]       rd_data_o,
    output reg                   rd_we_o,

    output reg  [XLEN-1:0]       pc_o,
    output reg                   valid_o,
    output reg  [$clog2(NUM_THREADS)-1:0] thread_id_o
);

    wire [6:0] opcode = instr_i[6:0];
    wire is_load = (opcode == 7'b0000011);

    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            rd_o <= 5'h0;
            rd_data_o <= {XLEN{1'b0}};
            rd_we_o <= 1'b0;
            pc_o <= {XLEN{1'b0}};
            valid_o <= 1'b0;
            thread_id_o <= '0;
        end else begin
            pc_o <= pc_i;
            valid_o <= valid_i;
            thread_id_o <= thread_id_i;
            rd_o <= rd_i;

            // Select result source
            if (is_load)
                rd_data_o <= mem_rdata_i;
            else
                rd_data_o <= alu_result_i;

            // Enable write if rd != 0 and valid
            rd_we_o <= valid_i && (rd_i != 5'h0);
        end
    end

endmodule

// Additional modules follow in next file...
