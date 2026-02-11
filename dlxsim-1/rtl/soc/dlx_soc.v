// ============================================================================
// DLX System-on-Chip (SoC) - Multi-Core SMP Configuration
// ============================================================================
//
// Features:
// - Up to 16 DLX cores
// - SMT support (up to 16 threads per core)
// - Wishbone interconnect with arbiter
// - Shared memory
// - Advanced interrupt controller (APIC)
// - Cache coherence support
// - Pluggable extensions per core
//
// ============================================================================

`default_nettype none

// ============================================================================
// Top-Level SoC
// ============================================================================

module dlx_soc #(
    parameter NUM_CORES = 4,
    parameter NUM_THREADS_PER_CORE = 4,
    parameter XLEN = 64,
    parameter ADDR_WIDTH = 64,
    parameter DATA_WIDTH = 64,
    parameter MEM_SIZE_KB = 1024,        // 1 MB shared memory
    parameter ENABLE_HYPERVISOR = 1,
    parameter ENABLE_VECTOR = 1,
    parameter ENABLE_BITMANIP = 1
) (
    input  wire clk,
    input  wire rst_n,

    // External memory interface (Wishbone master)
    output wire [ADDR_WIDTH-1:0]  ext_mem_adr_o,
    output wire [DATA_WIDTH-1:0]  ext_mem_dat_o,
    input  wire [DATA_WIDTH-1:0]  ext_mem_dat_i,
    output wire                   ext_mem_we_o,
    output wire [DATA_WIDTH/8-1:0] ext_mem_sel_o,
    output wire                   ext_mem_stb_o,
    input  wire                   ext_mem_ack_i,
    output wire                   ext_mem_cyc_o,

    // External interrupts
    input  wire [255:0]           ext_irq_i,

    // Debug/JTAG interface
    input  wire                   debug_req_i,
    output wire                   debug_ack_o,
    input  wire [3:0]             debug_core_sel_i,
    input  wire [15:0]            debug_addr_i,
    input  wire [DATA_WIDTH-1:0]  debug_wdata_i,
    output wire [DATA_WIDTH-1:0]  debug_rdata_o,
    input  wire                   debug_we_i,

    // Status outputs
    output wire [NUM_CORES-1:0]   core_halted_o,
    output wire [63:0]            total_cycles_o,
    output wire [63:0]            total_instrs_o
);

    // ========================================================================
    // Internal Signals
    // ========================================================================

    // Wishbone buses from cores (instruction)
    wire [ADDR_WIDTH-1:0]  core_iwb_adr   [0:NUM_CORES-1];
    wire [DATA_WIDTH-1:0]  core_iwb_dat_o [0:NUM_CORES-1];
    wire [DATA_WIDTH-1:0]  core_iwb_dat_i [0:NUM_CORES-1];
    wire                   core_iwb_we    [0:NUM_CORES-1];
    wire [DATA_WIDTH/8-1:0] core_iwb_sel  [0:NUM_CORES-1];
    wire                   core_iwb_stb   [0:NUM_CORES-1];
    wire                   core_iwb_ack   [0:NUM_CORES-1];
    wire                   core_iwb_cyc   [0:NUM_CORES-1];
    wire [2:0]             core_iwb_cti   [0:NUM_CORES-1];
    wire [1:0]             core_iwb_bte   [0:NUM_CORES-1];

    // Wishbone buses from cores (data)
    wire [ADDR_WIDTH-1:0]  core_dwb_adr   [0:NUM_CORES-1];
    wire [DATA_WIDTH-1:0]  core_dwb_dat_o [0:NUM_CORES-1];
    wire [DATA_WIDTH-1:0]  core_dwb_dat_i [0:NUM_CORES-1];
    wire                   core_dwb_we    [0:NUM_CORES-1];
    wire [DATA_WIDTH/8-1:0] core_dwb_sel  [0:NUM_CORES-1];
    wire                   core_dwb_stb   [0:NUM_CORES-1];
    wire                   core_dwb_ack   [0:NUM_CORES-1];
    wire                   core_dwb_cyc   [0:NUM_CORES-1];
    wire [2:0]             core_dwb_cti   [0:NUM_CORES-1];
    wire [1:0]             core_dwb_bte   [0:NUM_CORES-1];

    // Interrupts
    wire [NUM_CORES-1:0]   core_irq;
    wire [7:0]             core_irq_vector [0:NUM_CORES-1];

    // Cache coherence (snoop bus)
    wire                   snoop_valid [0:NUM_CORES-1];
    wire [ADDR_WIDTH-1:0]  snoop_addr [0:NUM_CORES-1];
    wire [1:0]             snoop_type [0:NUM_CORES-1];
    wire                   snoop_ack [0:NUM_CORES-1];

    // Performance counters
    wire [63:0]            core_cycles [0:NUM_CORES-1];
    wire [63:0]            core_instrs [0:NUM_CORES-1];

    // Extension interfaces
    wire                   core_ext_vector_req [0:NUM_CORES-1];
    wire                   core_ext_vector_ack [0:NUM_CORES-1];
    wire [255:0]           core_ext_vector_data_o [0:NUM_CORES-1];
    wire [255:0]           core_ext_vector_data_i [0:NUM_CORES-1];

    wire                   core_ext_vmx_req [0:NUM_CORES-1];
    wire                   core_ext_vmx_ack [0:NUM_CORES-1];
    wire [127:0]           core_ext_vmx_data_o [0:NUM_CORES-1];
    wire [127:0]           core_ext_vmx_data_i [0:NUM_CORES-1];

    // Active threads
    wire [$clog2(NUM_THREADS_PER_CORE)-1:0] core_active_thread [0:NUM_CORES-1];

    // Interconnect signals
    wire [ADDR_WIDTH-1:0]  mem_wb_adr;
    wire [DATA_WIDTH-1:0]  mem_wb_dat_o;
    wire [DATA_WIDTH-1:0]  mem_wb_dat_i;
    wire                   mem_wb_we;
    wire [DATA_WIDTH/8-1:0] mem_wb_sel;
    wire                   mem_wb_stb;
    wire                   mem_wb_ack;
    wire                   mem_wb_cyc;

    wire [31:0]            apic_wb_adr;
    wire [63:0]            apic_wb_dat_o;
    wire [63:0]            apic_wb_dat_i;
    wire                   apic_wb_we;
    wire [7:0]             apic_wb_sel;
    wire                   apic_wb_stb;
    wire                   apic_wb_ack;
    wire                   apic_wb_cyc;

    genvar i;

    // ========================================================================
    // Instantiate DLX Cores
    // ========================================================================

    generate
    for (i = 0; i < NUM_CORES; i = i + 1) begin : gen_cores
        dlx_core #(
            .CORE_ID(i),
            .NUM_THREADS(NUM_THREADS_PER_CORE),
            .ENABLE_HYPERVISOR(ENABLE_HYPERVISOR),
            .ENABLE_VECTOR(ENABLE_VECTOR),
            .ENABLE_BITMANIP(ENABLE_BITMANIP),
            .DATA_WIDTH(DATA_WIDTH),
            .ADDR_WIDTH(ADDR_WIDTH),
            .XLEN(XLEN)
        ) u_core (
            .clk(clk),
            .rst_n(rst_n),

            // Wishbone instruction bus
            .iwb_adr_o(core_iwb_adr[i]),
            .iwb_dat_o(core_iwb_dat_o[i]),
            .iwb_dat_i(core_iwb_dat_i[i]),
            .iwb_we_o(core_iwb_we[i]),
            .iwb_sel_o(core_iwb_sel[i]),
            .iwb_stb_o(core_iwb_stb[i]),
            .iwb_ack_i(core_iwb_ack[i]),
            .iwb_cyc_o(core_iwb_cyc[i]),
            .iwb_err_i(1'b0),
            .iwb_rty_i(1'b0),
            .iwb_cti_o(core_iwb_cti[i]),
            .iwb_bte_o(core_iwb_bte[i]),

            // Wishbone data bus
            .dwb_adr_o(core_dwb_adr[i]),
            .dwb_dat_o(core_dwb_dat_o[i]),
            .dwb_dat_i(core_dwb_dat_i[i]),
            .dwb_we_o(core_dwb_we[i]),
            .dwb_sel_o(core_dwb_sel[i]),
            .dwb_stb_o(core_dwb_stb[i]),
            .dwb_ack_i(core_dwb_ack[i]),
            .dwb_cyc_o(core_dwb_cyc[i]),
            .dwb_err_i(1'b0),
            .dwb_rty_i(1'b0),
            .dwb_cti_o(core_dwb_cti[i]),
            .dwb_bte_o(core_dwb_bte[i]),

            // Interrupts
            .irq(core_irq[i]),
            .nmi(1'b0),
            .irq_vector(core_irq_vector[i]),

            // Cache coherence
            .snoop_valid(|snoop_valid),    // OR of all snoop signals
            .snoop_addr(snoop_addr[0]),     // Simplified
            .snoop_type(snoop_type[0]),
            .snoop_ack(snoop_ack[i]),

            // Debug
            .debug_req(debug_req_i && (debug_core_sel_i == i)),
            .debug_ack(/* Connected below */),
            .debug_addr(debug_addr_i),
            .debug_wdata(debug_wdata_i),
            .debug_rdata(/* Connected below */),
            .debug_we(debug_we_i),

            // Extensions
            .ext_vector_req(core_ext_vector_req[i]),
            .ext_vector_ack(core_ext_vector_ack[i]),
            .ext_vector_data_o(core_ext_vector_data_o[i]),
            .ext_vector_data_i(core_ext_vector_data_i[i]),

            .ext_vmx_req(core_ext_vmx_req[i]),
            .ext_vmx_ack(core_ext_vmx_ack[i]),
            .ext_vmx_data_o(core_ext_vmx_data_o[i]),
            .ext_vmx_data_i(core_ext_vmx_data_i[i]),

            // Performance
            .perfmon_cycles(core_cycles[i]),
            .perfmon_instrs(core_instrs[i]),

            // Status
            .halted(core_halted_o[i]),
            .active_thread(core_active_thread[i])
        );

        // Instantiate Vector Extension (if enabled)
        if (ENABLE_VECTOR) begin : gen_vector_ext
            dlx_vector_extension #(
                .XLEN(XLEN),
                .VLEN(256)
            ) u_vector_ext (
                .clk(clk),
                .rst_n(rst_n),
                .vec_req_i(core_ext_vector_req[i]),
                .vec_ack_o(core_ext_vector_ack[i]),
                .vec_data_i(core_ext_vector_data_o[i]),
                .vec_data_o(core_ext_vector_data_i[i]),
                .vtype_i(16'h0),
                .vl_i(64'h8)
            );
        end else begin
            assign core_ext_vector_ack[i] = 1'b0;
            assign core_ext_vector_data_i[i] = 256'h0;
        end

        // Instantiate Hypervisor Extension (if enabled)
        if (ENABLE_HYPERVISOR) begin : gen_vmx_ext
            dlx_vmx_extension #(
                .XLEN(XLEN),
                .ADDR_WIDTH(ADDR_WIDTH)
            ) u_vmx_ext (
                .clk(clk),
                .rst_n(rst_n),
                .vmx_req_i(core_ext_vmx_req[i]),
                .vmx_ack_o(core_ext_vmx_ack[i]),
                .vmx_data_i(core_ext_vmx_data_o[i]),
                .vmx_data_o(core_ext_vmx_data_i[i]),

                // EPT memory access (stub)
                .wb_adr_o(),
                .wb_dat_o(),
                .wb_dat_i(64'h0),
                .wb_we_o(),
                .wb_sel_o(),
                .wb_stb_o(),
                .wb_ack_i(1'b0),
                .wb_cyc_o(),

                .vmx_mode_o(),
                .guest_pc_o(),
                .vmcs_ptr_o()
            );
        end else begin
            assign core_ext_vmx_ack[i] = 1'b0;
            assign core_ext_vmx_data_i[i] = 128'h0;
        end
    end
    endgenerate

    // ========================================================================
    // Wishbone Interconnect / Arbiter
    // ========================================================================

    dlx_wb_interconnect #(
        .NUM_MASTERS(NUM_CORES * 2),  // I + D per core
        .NUM_SLAVES(2),                // Memory + APIC
        .ADDR_WIDTH(ADDR_WIDTH),
        .DATA_WIDTH(DATA_WIDTH)
    ) u_interconnect (
        .clk(clk),
        .rst_n(rst_n),

        // Master ports (cores)
        .m_wb_adr_i({core_dwb_adr[3], core_iwb_adr[3],
                    core_dwb_adr[2], core_iwb_adr[2],
                    core_dwb_adr[1], core_iwb_adr[1],
                    core_dwb_adr[0], core_iwb_adr[0]}),
        .m_wb_dat_i({core_dwb_dat_o[3], core_iwb_dat_o[3],
                    core_dwb_dat_o[2], core_iwb_dat_o[2],
                    core_dwb_dat_o[1], core_iwb_dat_o[1],
                    core_dwb_dat_o[0], core_iwb_dat_o[0]}),
        .m_wb_dat_o({core_dwb_dat_i[3], core_iwb_dat_i[3],
                    core_dwb_dat_i[2], core_iwb_dat_i[2],
                    core_dwb_dat_i[1], core_iwb_dat_i[1],
                    core_dwb_dat_i[0], core_iwb_dat_i[0]}),
        .m_wb_we_i({core_dwb_we[3], core_iwb_we[3],
                   core_dwb_we[2], core_iwb_we[2],
                   core_dwb_we[1], core_iwb_we[1],
                   core_dwb_we[0], core_iwb_we[0]}),
        .m_wb_sel_i({core_dwb_sel[3], core_iwb_sel[3],
                    core_dwb_sel[2], core_iwb_sel[2],
                    core_dwb_sel[1], core_iwb_sel[1],
                    core_dwb_sel[0], core_iwb_sel[0]}),
        .m_wb_stb_i({core_dwb_stb[3], core_iwb_stb[3],
                    core_dwb_stb[2], core_iwb_stb[2],
                    core_dwb_stb[1], core_iwb_stb[1],
                    core_dwb_stb[0], core_iwb_stb[0]}),
        .m_wb_ack_o({core_dwb_ack[3], core_iwb_ack[3],
                    core_dwb_ack[2], core_iwb_ack[2],
                    core_dwb_ack[1], core_iwb_ack[1],
                    core_dwb_ack[0], core_iwb_ack[0]}),
        .m_wb_cyc_i({core_dwb_cyc[3], core_iwb_cyc[3],
                    core_dwb_cyc[2], core_iwb_cyc[2],
                    core_dwb_cyc[1], core_iwb_cyc[1],
                    core_dwb_cyc[0], core_iwb_cyc[0]}),

        // Slave ports
        .s_wb_adr_o({apic_wb_adr, mem_wb_adr[31:0]}),
        .s_wb_dat_o({apic_wb_dat_o, mem_wb_dat_o}),
        .s_wb_dat_i({apic_wb_dat_i, mem_wb_dat_i}),
        .s_wb_we_o({apic_wb_we, mem_wb_we}),
        .s_wb_sel_o({apic_wb_sel, mem_wb_sel}),
        .s_wb_stb_o({apic_wb_stb, mem_wb_stb}),
        .s_wb_ack_i({apic_wb_ack, mem_wb_ack}),
        .s_wb_cyc_o({apic_wb_cyc, mem_wb_cyc})
    );

    // ========================================================================
    // Shared Memory
    // ========================================================================

    dlx_memory #(
        .SIZE_KB(MEM_SIZE_KB),
        .DATA_WIDTH(DATA_WIDTH),
        .ADDR_WIDTH(32)
    ) u_memory (
        .clk(clk),
        .rst_n(rst_n),

        .wb_adr_i(mem_wb_adr),
        .wb_dat_i(mem_wb_dat_o),
        .wb_dat_o(mem_wb_dat_i),
        .wb_we_i(mem_wb_we),
        .wb_sel_i(mem_wb_sel),
        .wb_stb_i(mem_wb_stb),
        .wb_ack_o(mem_wb_ack),
        .wb_cyc_i(mem_wb_cyc)
    );

    // ========================================================================
    // Advanced Interrupt Controller
    // ========================================================================

    dlx_apic #(
        .NUM_CORES(NUM_CORES),
        .NUM_THREADS_PER_CORE(NUM_THREADS_PER_CORE),
        .NUM_IRQS(256)
    ) u_apic (
        .clk(clk),
        .rst_n(rst_n),

        // Wishbone slave interface
        .wb_adr_i(apic_wb_adr),
        .wb_dat_i(apic_wb_dat_o),
        .wb_dat_o(apic_wb_dat_i),
        .wb_we_i(apic_wb_we),
        .wb_sel_i(apic_wb_sel),
        .wb_stb_i(apic_wb_stb),
        .wb_ack_o(apic_wb_ack),
        .wb_cyc_i(apic_wb_cyc),

        // External interrupts
        .irq_sources_i(ext_irq_i),

        // Interrupt outputs to cores
        .core_irq_o(core_irq),
        .core_irq_vector_o(core_irq_vector)
    );

    // ========================================================================
    // Performance Counter Aggregation
    // ========================================================================

    assign total_cycles_o = core_cycles[0] + core_cycles[1] +
                           core_cycles[2] + core_cycles[3];
    assign total_instrs_o = core_instrs[0] + core_instrs[1] +
                           core_instrs[2] + core_instrs[3];

    // ========================================================================
    // Debug Interface Multiplexing
    // ========================================================================

    assign debug_ack_o = 1'b0;  // TODO: Implement debug mux
    assign debug_rdata_o = 64'h0;

endmodule

// ============================================================================
// Wishbone Interconnect/Arbiter (Simplified)
// ============================================================================

module dlx_wb_interconnect #(
    parameter NUM_MASTERS = 8,
    parameter NUM_SLAVES = 2,
    parameter ADDR_WIDTH = 64,
    parameter DATA_WIDTH = 64
) (
    input  wire clk,
    input  wire rst_n,

    // Master ports (concatenated)
    input  wire [ADDR_WIDTH*NUM_MASTERS-1:0]       m_wb_adr_i,
    input  wire [DATA_WIDTH*NUM_MASTERS-1:0]       m_wb_dat_i,
    output wire [DATA_WIDTH*NUM_MASTERS-1:0]       m_wb_dat_o,
    input  wire [NUM_MASTERS-1:0]                  m_wb_we_i,
    input  wire [(DATA_WIDTH/8)*NUM_MASTERS-1:0]   m_wb_sel_i,
    input  wire [NUM_MASTERS-1:0]                  m_wb_stb_i,
    output wire [NUM_MASTERS-1:0]                  m_wb_ack_o,
    input  wire [NUM_MASTERS-1:0]                  m_wb_cyc_i,

    // Slave ports (concatenated)
    output wire [32*NUM_SLAVES-1:0]                s_wb_adr_o,
    output wire [DATA_WIDTH*NUM_SLAVES-1:0]        s_wb_dat_o,
    input  wire [DATA_WIDTH*NUM_SLAVES-1:0]        s_wb_dat_i,
    output wire [NUM_SLAVES-1:0]                   s_wb_we_o,
    output wire [(DATA_WIDTH/8)*NUM_SLAVES-1:0]    s_wb_sel_o,
    output wire [NUM_SLAVES-1:0]                   s_wb_stb_o,
    input  wire [NUM_SLAVES-1:0]                   s_wb_ack_i,
    output wire [NUM_SLAVES-1:0]                   s_wb_cyc_o
);

    // Simplified: Round-robin arbiter, broadcast to all slaves
    // Real implementation would have proper address decoding

    reg [$clog2(NUM_MASTERS)-1:0] grant;
    reg [NUM_MASTERS-1:0] grant_oh;  // One-hot

    integer i;

    // Round-robin arbiter
    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            grant <= '0;
            grant_oh <= {{(NUM_MASTERS-1){1'b0}}, 1'b1};
        end else begin
            // Find next requesting master
            grant <= (grant + 1) % NUM_MASTERS;
            grant_oh <= (grant_oh << 1) | (grant_oh >> (NUM_MASTERS-1));
        end
    end

    // Connect granted master to all slaves (simplified)
    // In a real design, use address decoding to select specific slave

    genvar m, s;
    generate
        for (s = 0; s < NUM_SLAVES; s = s + 1) begin : gen_slaves
            assign s_wb_adr_o[s*32 +: 32] =
                m_wb_adr_i[grant*ADDR_WIDTH +: 32];
            assign s_wb_dat_o[s*DATA_WIDTH +: DATA_WIDTH] =
                m_wb_dat_i[grant*DATA_WIDTH +: DATA_WIDTH];
            assign s_wb_we_o[s] = m_wb_we_i[grant];
            assign s_wb_sel_o[s*(DATA_WIDTH/8) +: (DATA_WIDTH/8)] =
                m_wb_sel_i[grant*(DATA_WIDTH/8) +: (DATA_WIDTH/8)];
            assign s_wb_stb_o[s] = m_wb_stb_i[grant];
            assign s_wb_cyc_o[s] = m_wb_cyc_i[grant];
        end

        for (m = 0; m < NUM_MASTERS; m = m + 1) begin : gen_masters
            assign m_wb_dat_o[m*DATA_WIDTH +: DATA_WIDTH] =
                s_wb_dat_i[0*DATA_WIDTH +: DATA_WIDTH];  // From slave 0
            assign m_wb_ack_o[m] = grant_oh[m] & s_wb_ack_i[0];
        end
    endgenerate

endmodule

// ============================================================================
// Wishbone Memory (SRAM)
// ============================================================================

module dlx_memory #(
    parameter SIZE_KB = 1024,
    parameter DATA_WIDTH = 64,
    parameter ADDR_WIDTH = 32
) (
    input  wire clk,
    input  wire rst_n,

    input  wire [ADDR_WIDTH-1:0]  wb_adr_i,
    input  wire [DATA_WIDTH-1:0]  wb_dat_i,
    output reg  [DATA_WIDTH-1:0]  wb_dat_o,
    input  wire                   wb_we_i,
    input  wire [DATA_WIDTH/8-1:0] wb_sel_i,
    input  wire                   wb_stb_i,
    output reg                    wb_ack_o,
    input  wire                   wb_cyc_i
);

    localparam MEM_WORDS = (SIZE_KB * 1024) / (DATA_WIDTH/8);

    // Memory array
    reg [DATA_WIDTH-1:0] mem [0:MEM_WORDS-1];

    wire [ADDR_WIDTH-1:0] word_addr = wb_adr_i[ADDR_WIDTH-1:$clog2(DATA_WIDTH/8)];

    integer i;

    // Initialize memory to zero
    initial begin
        for (i = 0; i < MEM_WORDS; i = i + 1)
            mem[i] = {DATA_WIDTH{1'b0}};
    end

    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            wb_ack_o <= 1'b0;
            wb_dat_o <= {DATA_WIDTH{1'b0}};
        end else begin
            if (wb_cyc_i && wb_stb_i) begin
                if (wb_we_i) begin
                    // Write with byte select
                    for (i = 0; i < DATA_WIDTH/8; i = i + 1) begin
                        if (wb_sel_i[i])
                            mem[word_addr][i*8 +: 8] <= wb_dat_i[i*8 +: 8];
                    end
                end else begin
                    // Read
                    wb_dat_o <= mem[word_addr];
                end
                wb_ack_o <= 1'b1;
            end else begin
                wb_ack_o <= 1'b0;
            end
        end
    end

endmodule

`default_nettype wire
