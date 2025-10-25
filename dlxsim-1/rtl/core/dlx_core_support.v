// ============================================================================
// DLX Core Support Modules
// ============================================================================
//
// Contains:
// - Register File (multi-threaded)
// - Pipeline Control Unit
// - SMT Thread Scheduler
// - System Registers (CSR)
// - Debug Interface
//
// ============================================================================

`default_nettype none

// ============================================================================
// Multi-Threaded Register File
// ============================================================================

module dlx_regfile #(
    parameter XLEN = 64,
    parameter NUM_THREADS = 1
) (
    input  wire clk,
    input  wire rst_n,

    // Read ports
    input  wire [4:0]            rs1_addr_i,
    input  wire [4:0]            rs2_addr_i,
    input  wire [$clog2(NUM_THREADS)-1:0] thread_id_rd_i,
    output wire [XLEN-1:0]       rs1_data_o,
    output wire [XLEN-1:0]       rs2_data_o,

    // Write port
    input  wire [4:0]            rd_addr_i,
    input  wire [XLEN-1:0]       rd_data_i,
    input  wire                  rd_we_i,
    input  wire [$clog2(NUM_THREADS)-1:0] thread_id_wr_i
);

    // Register file: 32 registers × NUM_THREADS
    reg [XLEN-1:0] regs [0:31][0:NUM_THREADS-1];

    integer i, j;

    // Initialize registers
    initial begin
        for (i = 0; i < 32; i = i + 1) begin
            for (j = 0; j < NUM_THREADS; j = j + 1) begin
                regs[i][j] = {XLEN{1'b0}};
            end
        end
    end

    // Read ports (combinational)
    assign rs1_data_o = (rs1_addr_i == 5'h0) ? {XLEN{1'b0}} :
                        regs[rs1_addr_i][thread_id_rd_i];
    assign rs2_data_o = (rs2_addr_i == 5'h0) ? {XLEN{1'b0}} :
                        regs[rs2_addr_i][thread_id_rd_i];

    // Write port (synchronous)
    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            for (i = 0; i < 32; i = i + 1) begin
                for (j = 0; j < NUM_THREADS; j = j + 1) begin
                    regs[i][j] <= {XLEN{1'b0}};
                end
            end
        end else begin
            if (rd_we_i && (rd_addr_i != 5'h0)) begin
                regs[rd_addr_i][thread_id_wr_i] <= rd_data_i;
            end
        end
    end

endmodule

// ============================================================================
// Pipeline Control Unit
// ============================================================================

module dlx_pipeline_ctrl #(
    parameter NUM_THREADS = 1
) (
    input  wire clk,
    input  wire rst_n,

    // Hazard detection
    input  wire [4:0] rs1_id_i,
    input  wire [4:0] rs2_id_i,
    input  wire [4:0] rd_ex_i,
    input  wire [4:0] rd_mem_i,
    input  wire       mem_read_ex_i,

    // Branch/Jump
    input  wire       branch_taken_i,
    input  wire       jump_i,

    // Exception
    input  wire       exception_i,

    // Memory stalls
    input  wire       mem_stall_i,

    // Extension stalls
    input  wire       vector_stall_i,
    input  wire       vmx_stall_i,

    // Stall/Flush outputs
    output reg        stall_if_o,
    output reg        stall_id_o,
    output reg        stall_ex_o,
    output reg        stall_mem_o,
    output reg        flush_if_o,
    output reg        flush_id_o,
    output reg        flush_ex_o,
    output reg        flush_mem_o
);

    // Data hazard detection (load-use)
    wire load_use_hazard = mem_read_ex_i &&
                          ((rd_ex_i == rs1_id_i) || (rd_ex_i == rs2_id_i)) &&
                          (rd_ex_i != 5'h0);

    // Control hazard (branch/jump)
    wire control_hazard = branch_taken_i || jump_i;

    // Any stall condition
    wire any_stall = load_use_hazard || mem_stall_i ||
                    vector_stall_i || vmx_stall_i;

    always @(*) begin
        // Default: no stall or flush
        stall_if_o = 1'b0;
        stall_id_o = 1'b0;
        stall_ex_o = 1'b0;
        stall_mem_o = 1'b0;
        flush_if_o = 1'b0;
        flush_id_o = 1'b0;
        flush_ex_o = 1'b0;
        flush_mem_o = 1'b0;

        // Load-use hazard: stall IF and ID, insert bubble in EX
        if (load_use_hazard) begin
            stall_if_o = 1'b1;
            stall_id_o = 1'b1;
            flush_ex_o = 1'b1;
        end

        // Memory stall: stall all stages
        if (mem_stall_i) begin
            stall_if_o = 1'b1;
            stall_id_o = 1'b1;
            stall_ex_o = 1'b1;
            stall_mem_o = 1'b1;
        end

        // Extension stalls
        if (vector_stall_i || vmx_stall_i) begin
            stall_if_o = 1'b1;
            stall_id_o = 1'b1;
            stall_ex_o = 1'b1;
        end

        // Control hazard: flush IF and ID
        if (control_hazard) begin
            flush_if_o = 1'b1;
            flush_id_o = 1'b1;
        end

        // Exception: flush all stages
        if (exception_i) begin
            flush_if_o = 1'b1;
            flush_id_o = 1'b1;
            flush_ex_o = 1'b1;
            flush_mem_o = 1'b1;
        end
    end

endmodule

// ============================================================================
// SMT Thread Scheduler
// ============================================================================

module dlx_smt_scheduler #(
    parameter NUM_THREADS = 4
) (
    input  wire clk,
    input  wire rst_n,

    // Thread states
    input  wire [NUM_THREADS-1:0] thread_valid_i,
    input  wire [NUM_THREADS-1:0] thread_stall_i,

    // Active thread selection
    output reg  [$clog2(NUM_THREADS)-1:0] active_thread_o
);

    // Round-robin scheduler
    reg [$clog2(NUM_THREADS)-1:0] next_thread;
    reg [$clog2(NUM_THREADS)-1:0] rr_counter;

    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            active_thread_o <= '0;
            rr_counter <= '0;
        end else begin
            // Find next valid, non-stalled thread
            next_thread = (rr_counter + 1) % NUM_THREADS;

            // Simple round-robin for now
            // TODO: Add priority scheduling
            while (!thread_valid_i[next_thread] ||
                   thread_stall_i[next_thread]) begin
                next_thread = (next_thread + 1) % NUM_THREADS;

                // Prevent infinite loop
                if (next_thread == rr_counter)
                    break;
            end

            active_thread_o <= next_thread;
            rr_counter <= next_thread;
        end
    end

endmodule

// ============================================================================
// System Registers (CSR)
// ============================================================================

module dlx_csr #(
    parameter XLEN = 64,
    parameter CORE_ID = 0,
    parameter NUM_THREADS = 1,
    parameter ENABLE_HYPERVISOR = 1
) (
    input  wire clk,
    input  wire rst_n,

    // CSR access
    input  wire [11:0]           csr_addr_i,
    input  wire [XLEN-1:0]       csr_wdata_i,
    input  wire                  csr_we_i,
    output reg  [XLEN-1:0]       csr_rdata_o,
    input  wire [$clog2(NUM_THREADS)-1:0] thread_id_i,

    // Exception handling
    input  wire                  exception_i,
    input  wire [7:0]            exception_cause_i,
    input  wire [XLEN-1:0]       exception_pc_i,
    output reg  [XLEN-1:0]       exception_pc_o,

    // Interrupt handling
    input  wire                  irq_i,
    input  wire                  nmi_i,
    input  wire [7:0]            irq_vector_i,
    output wire                  interrupt_pending_o,

    // Performance counters
    input  wire [63:0]           cycle_counter_i,
    input  wire [63:0]           instr_counter_i,

    // Hypervisor extension
    output wire                  vmx_req_o,
    input  wire                  vmx_ack_i,
    output wire [127:0]          vmx_data_o,
    input  wire [127:0]          vmx_data_i
);

    // CSR addresses
    localparam CSR_MSTATUS    = 12'h300;
    localparam CSR_MISA       = 12'h301;
    localparam CSR_MIE        = 12'h304;
    localparam CSR_MTVEC      = 12'h305;
    localparam CSR_MSCRATCH   = 12'h340;
    localparam CSR_MEPC       = 12'h341;
    localparam CSR_MCAUSE     = 12'h342;
    localparam CSR_MTVAL      = 12'h343;
    localparam CSR_MIP        = 12'h344;

    localparam CSR_CYCLE      = 12'hC00;
    localparam CSR_TIME       = 12'hC01;
    localparam CSR_INSTRET    = 12'hC02;

    localparam CSR_MCYCLEH    = 12'hB80;
    localparam CSR_MINSTRETH  = 12'hB82;

    // Hypervisor CSRs
    localparam CSR_HSTATUS    = 12'h600;
    localparam CSR_HEDELEG    = 12'h602;
    localparam CSR_HIDELEG    = 12'h603;
    localparam CSR_HGATP      = 12'h680;  // Guest address translation

    // CSR registers (per thread)
    reg [XLEN-1:0] mstatus [0:NUM_THREADS-1];
    reg [XLEN-1:0] mtvec [0:NUM_THREADS-1];
    reg [XLEN-1:0] mepc [0:NUM_THREADS-1];
    reg [XLEN-1:0] mcause [0:NUM_THREADS-1];
    reg [XLEN-1:0] mtval [0:NUM_THREADS-1];
    reg [XLEN-1:0] mscratch [0:NUM_THREADS-1];
    reg [XLEN-1:0] mie [0:NUM_THREADS-1];
    reg [XLEN-1:0] mip [0:NUM_THREADS-1];

    // Hypervisor CSRs
    reg [XLEN-1:0] hstatus [0:NUM_THREADS-1];
    reg [XLEN-1:0] hgatp [0:NUM_THREADS-1];

    // Read-only CSRs
    wire [XLEN-1:0] misa = {2'b10, {XLEN-28{1'b0}}, 26'h0000014D};  // RV64IMAFD

    integer i;

    // Initialize CSRs
    initial begin
        for (i = 0; i < NUM_THREADS; i = i + 1) begin
            mstatus[i] = {XLEN{1'b0}};
            mtvec[i] = {XLEN{1'b0}};
            mepc[i] = {XLEN{1'b0}};
            mcause[i] = {XLEN{1'b0}};
            mtval[i] = {XLEN{1'b0}};
            mscratch[i] = {XLEN{1'b0}};
            mie[i] = {XLEN{1'b0}};
            mip[i] = {XLEN{1'b0}};
            hstatus[i] = {XLEN{1'b0}};
            hgatp[i] = {XLEN{1'b0}};
        end
    end

    // CSR read
    always @(*) begin
        case (csr_addr_i)
            CSR_MSTATUS:   csr_rdata_o = mstatus[thread_id_i];
            CSR_MISA:      csr_rdata_o = misa;
            CSR_MIE:       csr_rdata_o = mie[thread_id_i];
            CSR_MTVEC:     csr_rdata_o = mtvec[thread_id_i];
            CSR_MSCRATCH:  csr_rdata_o = mscratch[thread_id_i];
            CSR_MEPC:      csr_rdata_o = mepc[thread_id_i];
            CSR_MCAUSE:    csr_rdata_o = mcause[thread_id_i];
            CSR_MTVAL:     csr_rdata_o = mtval[thread_id_i];
            CSR_MIP:       csr_rdata_o = mip[thread_id_i];
            CSR_CYCLE:     csr_rdata_o = cycle_counter_i;
            CSR_INSTRET:   csr_rdata_o = instr_counter_i;
            CSR_HSTATUS:   csr_rdata_o = hstatus[thread_id_i];
            CSR_HGATP:     csr_rdata_o = hgatp[thread_id_i];
            default:       csr_rdata_o = {XLEN{1'b0}};
        endcase
    end

    // CSR write
    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            for (i = 0; i < NUM_THREADS; i = i + 1) begin
                mstatus[i] <= {XLEN{1'b0}};
                mtvec[i] <= {XLEN{1'b0}};
                mie[i] <= {XLEN{1'b0}};
                mip[i] <= {XLEN{1'b0}};
                hstatus[i] <= {XLEN{1'b0}};
                hgatp[i] <= {XLEN{1'b0}};
            end
        end else begin
            // Update MIP based on interrupts
            if (irq_i)
                mip[thread_id_i][7] <= 1'b1;  // Timer interrupt
            if (nmi_i)
                mip[thread_id_i][11] <= 1'b1; // NMI

            // CSR writes
            if (csr_we_i) begin
                case (csr_addr_i)
                    CSR_MSTATUS:   mstatus[thread_id_i] <= csr_wdata_i;
                    CSR_MIE:       mie[thread_id_i] <= csr_wdata_i;
                    CSR_MTVEC:     mtvec[thread_id_i] <= csr_wdata_i;
                    CSR_MSCRATCH:  mscratch[thread_id_i] <= csr_wdata_i;
                    CSR_MEPC:      mepc[thread_id_i] <= csr_wdata_i;
                    CSR_MCAUSE:    mcause[thread_id_i] <= csr_wdata_i;
                    CSR_MTVAL:     mtval[thread_id_i] <= csr_wdata_i;
                    CSR_MIP:       mip[thread_id_i] <= csr_wdata_i;
                    CSR_HSTATUS:   hstatus[thread_id_i] <= csr_wdata_i;
                    CSR_HGATP:     hgatp[thread_id_i] <= csr_wdata_i;
                endcase
            end

            // Exception handling
            if (exception_i) begin
                mepc[thread_id_i] <= exception_pc_i;
                mcause[thread_id_i] <= {1'b0, {XLEN-8{1'b0}}, exception_cause_i};

                // Disable interrupts
                mstatus[thread_id_i][3] <= 1'b0;  // MIE
            end
        end
    end

    // Exception PC (trap handler)
    always @(*) begin
        if (exception_i || (|mip[thread_id_i] & mstatus[thread_id_i][3])) begin
            // Vectored or direct mode
            if (mtvec[thread_id_i][0])
                exception_pc_o = {mtvec[thread_id_i][XLEN-1:2], 2'b00} +
                               (exception_cause_i << 2);
            else
                exception_pc_o = {mtvec[thread_id_i][XLEN-1:2], 2'b00};
        end else begin
            exception_pc_o = {XLEN{1'b0}};
        end
    end

    // Interrupt pending
    assign interrupt_pending_o = |(mip[thread_id_i] & mie[thread_id_i]) &
                                mstatus[thread_id_i][3];

    // Hypervisor interface
    assign vmx_req_o = 1'b0;  // TODO: Implement VMX operations
    assign vmx_data_o = 128'h0;

endmodule

// ============================================================================
// Debug Interface
// ============================================================================

module dlx_debug #(
    parameter XLEN = 64,
    parameter NUM_THREADS = 1
) (
    input  wire clk,
    input  wire rst_n,

    input  wire                  debug_req_i,
    output reg                   debug_ack_o,
    input  wire [15:0]           debug_addr_i,
    input  wire [XLEN-1:0]       debug_wdata_i,
    output reg  [XLEN-1:0]       debug_rdata_o,
    input  wire                  debug_we_i,

    // Internal state access
    input  wire [XLEN-1:0]       pc_if_i,
    input  wire [XLEN-1:0]       pc_id_i,
    input  wire [XLEN-1:0]       pc_ex_i,
    input  wire [XLEN-1:0]       pc_mem_i,
    input  wire [XLEN-1:0]       pc_wb_i
);

    // Debug address map
    localparam DBG_PC_IF  = 16'h0000;
    localparam DBG_PC_ID  = 16'h0008;
    localparam DBG_PC_EX  = 16'h0010;
    localparam DBG_PC_MEM = 16'h0018;
    localparam DBG_PC_WB  = 16'h0020;

    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            debug_ack_o <= 1'b0;
            debug_rdata_o <= {XLEN{1'b0}};
        end else begin
            if (debug_req_i) begin
                case (debug_addr_i)
                    DBG_PC_IF:  debug_rdata_o <= pc_if_i;
                    DBG_PC_ID:  debug_rdata_o <= pc_id_i;
                    DBG_PC_EX:  debug_rdata_o <= pc_ex_i;
                    DBG_PC_MEM: debug_rdata_o <= pc_mem_i;
                    DBG_PC_WB:  debug_rdata_o <= pc_wb_i;
                    default:    debug_rdata_o <= {XLEN{1'b0}};
                endcase
                debug_ack_o <= 1'b1;
            end else begin
                debug_ack_o <= 1'b0;
            end
        end
    end

endmodule

`default_nettype wire
