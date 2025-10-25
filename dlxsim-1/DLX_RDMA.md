# DLX RDMA Emulation Architecture

## Overview

This document specifies comprehensive RDMA (Remote Direct Memory Access) emulation for DLXSIM, enabling high-performance, low-latency network communication with zero-copy data transfer and kernel bypass.

RDMA allows direct memory access from one computer into the memory of another without involving either operating system, enabling high-throughput, low-latency networking used in storage, clustering, and high-performance computing.

## 1. RDMA Fundamentals

### Key Concepts

**RDMA Benefits:**
- **Zero-copy**: Data transferred directly between application memory and network
- **Kernel bypass**: No OS involvement in data path
- **CPU offload**: Network adapter handles protocol processing
- **Low latency**: < 1 microsecond for small messages
- **High bandwidth**: 100-400 Gbps with modern adapters

**RDMA Protocols:**
1. **InfiniBand (IB)**: Native RDMA protocol, lossless fabric
2. **RoCE (RDMA over Converged Ethernet)**: RDMA over Ethernet with priority flow control
   - RoCEv1: L2 only (same subnet)
   - RoCEv2: L3 routable (UDP encapsulation)
3. **iWARP**: RDMA over TCP/IP, works on standard Ethernet

### RDMA Verbs API

```c
/* Core RDMA operations */
typedef enum {
    IBV_WR_SEND,            /* Send operation */
    IBV_WR_SEND_WITH_IMM,   /* Send with immediate data */
    IBV_WR_RECV,            /* Receive operation */
    IBV_WR_RDMA_WRITE,      /* RDMA Write */
    IBV_WR_RDMA_WRITE_WITH_IMM, /* RDMA Write with immediate */
    IBV_WR_RDMA_READ,       /* RDMA Read */
    IBV_WR_ATOMIC_CMP_AND_SWP, /* Compare and swap */
    IBV_WR_ATOMIC_FETCH_AND_ADD, /* Fetch and add */
    IBV_WR_LOCAL_INV,       /* Local invalidate */
    IBV_WR_BIND_MW,         /* Bind memory window */
} ibv_wr_opcode_t;
```

## 2. InfiniBand Architecture

### InfiniBand HCA (Host Channel Adapter)

```c
/* InfiniBand HCA Device */
typedef struct {
    pcie_config_type0_t pci_config;

    /* Device Attributes */
    struct {
        uint64_t node_guid;     /* Node GUID (globally unique) */
        uint64_t sys_image_guid;

        char fw_ver[64];        /* Firmware version */
        uint16_t hw_ver;        /* Hardware version */

        int max_qp;             /* Maximum Queue Pairs: 16K-1M */
        int max_qp_wr;          /* Max WRs per QP: 16K */
        int max_sge;            /* Max SGE per WR: 30 */
        int max_cq;             /* Maximum Completion Queues */
        int max_cqe;            /* Max CQEs per CQ: 4M */
        int max_mr;             /* Maximum Memory Regions */
        uint64_t max_mr_size;   /* Max MR size */
        int max_pd;             /* Maximum Protection Domains */
        int max_ah;             /* Maximum Address Handles */
        int max_srq;            /* Max Shared Receive Queues */
        int max_srq_wr;         /* Max WRs per SRQ */
        int max_srq_sge;        /* Max SGE per SRQ WR */

        /* Port capabilities */
        int phys_port_cnt;      /* Number of physical ports: 1-2 */
        int max_pkeys;          /* Max partition keys: 128 */
        int max_gids;           /* Max GIDs per port: 256 */

        /* Atomic capabilities */
        int atomic_cap;         /* None, HCA, Global */

        /* Features */
        uint64_t device_cap_flags;
#define IBV_DEVICE_RESIZE_MAX_WR        (1 << 0)
#define IBV_DEVICE_BAD_PKEY_CNTR        (1 << 1)
#define IBV_DEVICE_BAD_QKEY_CNTR        (1 << 2)
#define IBV_DEVICE_AUTO_PATH_MIG        (1 << 3)
#define IBV_DEVICE_CHANGE_PHY_PORT      (1 << 4)
#define IBV_DEVICE_UD_AV_PORT_ENFORCE   (1 << 5)
#define IBV_DEVICE_CURR_QP_STATE_MOD    (1 << 6)
#define IBV_DEVICE_SHUTDOWN_PORT        (1 << 7)
#define IBV_DEVICE_INIT_TYPE            (1 << 8)
#define IBV_DEVICE_PORT_ACTIVE_EVENT    (1 << 9)
#define IBV_DEVICE_SYS_IMAGE_GUID       (1 << 10)
#define IBV_DEVICE_RC_RNR_NAK_GEN       (1 << 11)
#define IBV_DEVICE_SRQ_RESIZE           (1 << 12)
#define IBV_DEVICE_MEM_WINDOW           (1 << 13)
#define IBV_DEVICE_UD_IP_CSUM           (1 << 14)
#define IBV_DEVICE_XRC                  (1 << 15)
#define IBV_DEVICE_MEM_MGT_EXTENSIONS   (1 << 16)
#define IBV_DEVICE_MEM_WINDOW_TYPE_2A   (1 << 17)
#define IBV_DEVICE_MEM_WINDOW_TYPE_2B   (1 << 18)
#define IBV_DEVICE_RC_IP_CSUM           (1 << 19)
#define IBV_DEVICE_RAW_IP_CSUM          (1 << 20)
#define IBV_DEVICE_MANAGED_FLOW_STEERING (1 << 21)
    } attr;

    /* Ports */
    struct ib_port {
        uint64_t port_guid;

        /* Port state */
        enum {
            IB_PORT_NOP         = 0,
            IB_PORT_DOWN        = 1,
            IB_PORT_INIT        = 2,
            IB_PORT_ARMED       = 3,
            IB_PORT_ACTIVE      = 4,
            IB_PORT_ACTIVE_DEFER = 5,
        } state;

        /* Physical state */
        enum {
            IB_PHYS_STATE_SLEEP         = 1,
            IB_PHYS_STATE_POLLING       = 2,
            IB_PHYS_STATE_DISABLED      = 3,
            IB_PHYS_STATE_PORT_CONFIG   = 4,
            IB_PHYS_STATE_LINK_UP       = 5,
            IB_PHYS_STATE_LINK_ERROR_RECOVERY = 6,
            IB_PHYS_STATE_PHY_TEST      = 7,
        } phys_state;

        /* Link properties */
        int width;              /* 1x, 4x, 8x, 12x */
        int speed;              /* SDR, DDR, QDR, FDR, EDR, HDR, NDR */
#define IB_SPEED_SDR    2.5     /* 2.5 Gbps per lane */
#define IB_SPEED_DDR    5       /* 5 Gbps */
#define IB_SPEED_QDR    10      /* 10 Gbps */
#define IB_SPEED_FDR    14      /* 14.0625 Gbps (FDR10: 10.3125) */
#define IB_SPEED_EDR    25      /* 25.78125 Gbps */
#define IB_SPEED_HDR    50      /* 50 Gbps */
#define IB_SPEED_NDR    100     /* 100 Gbps */
#define IB_SPEED_XDR    200     /* 200 Gbps (future) */

        /* LID (Local Identifier) */
        uint16_t lid;           /* 16-bit for base LID */
        uint16_t lmc;           /* LID mask count for multi-path */
        uint16_t sm_lid;        /* Subnet Manager LID */
        uint8_t  sm_sl;         /* SM Service Level */

        /* GID (Global Identifier) Table */
        union ib_gid {
            uint8_t  raw[16];
            struct {
                uint64_t subnet_prefix;
                uint64_t interface_id;
            } global;
        } gid_table[256];

        /* PKey (Partition Key) Table */
        uint16_t pkey_table[128];

        /* Counters */
        uint64_t rx_packets;
        uint64_t tx_packets;
        uint64_t rx_bytes;
        uint64_t tx_bytes;
        uint64_t rx_errors;
        uint64_t tx_errors;
    } ports[2];

    /* Resource tracking */
    struct ib_qp *qp_table[1024 * 1024];    /* Queue Pairs */
    struct ib_cq *cq_table[1024 * 1024];    /* Completion Queues */
    struct ib_mr *mr_table[1024 * 1024];    /* Memory Regions */
    struct ib_pd *pd_table[256 * 1024];     /* Protection Domains */
    struct ib_ah *ah_table[256 * 1024];     /* Address Handles */
    struct ib_srq *srq_table[65536];        /* Shared Receive Queues */

    /* Event handling */
    int async_event_fd;
    pthread_t event_thread;

    /* Firmware command interface */
    struct {
        void *cmd_box;          /* Command mailbox */
        void *event_box;        /* Event mailbox */
        pthread_mutex_t lock;
    } fw;
} ib_hca_t;
```

### Queue Pair (QP) - Core RDMA Object

```c
/* Queue Pair Types */
typedef enum {
    IB_QPT_RC,              /* Reliable Connection */
    IB_QPT_UC,              /* Unreliable Connection */
    IB_QPT_UD,              /* Unreliable Datagram */
    IB_QPT_RAW_PACKET,      /* Raw Ethernet */
    IB_QPT_XRC_SEND,        /* Extended RC (send) */
    IB_QPT_XRC_RECV,        /* Extended RC (recv) */
    IB_QPT_DRIVER,          /* Driver-specific */
} ib_qp_type_t;

/* Queue Pair State Machine */
typedef enum {
    IB_QPS_RESET,           /* Reset state */
    IB_QPS_INIT,            /* Initialized */
    IB_QPS_RTR,             /* Ready to Receive */
    IB_QPS_RTS,             /* Ready to Send */
    IB_QPS_SQD,             /* Send Queue Drained */
    IB_QPS_SQE,             /* Send Queue Error */
    IB_QPS_ERR,             /* Error */
} ib_qp_state_t;

/* Queue Pair Structure */
typedef struct ib_qp {
    uint32_t qp_num;        /* QP number (24-bit) */
    ib_qp_type_t qp_type;
    ib_qp_state_t state;

    struct ib_pd *pd;       /* Protection Domain */
    struct ib_cq *send_cq;  /* Send Completion Queue */
    struct ib_cq *recv_cq;  /* Receive Completion Queue */
    struct ib_srq *srq;     /* Optional Shared Receive Queue */

    /* Send Queue */
    struct {
        struct ib_wqe *wqe;     /* Work Queue Elements */
        uint32_t head;
        uint32_t tail;
        uint32_t max_wr;        /* Max outstanding WRs */
        uint32_t max_sge;       /* Max SGEs per WR */
        int signaled;           /* All or selective signaling */
    } sq;

    /* Receive Queue */
    struct {
        struct ib_wqe *wqe;     /* Work Queue Elements */
        uint32_t head;
        uint32_t tail;
        uint32_t max_wr;
        uint32_t max_sge;
    } rq;

    /* QP Attributes (for RC/UC) */
    struct {
        uint32_t dest_qp_num;   /* Destination QP number */
        uint32_t psn;           /* Packet Sequence Number */
        uint16_t dlid;          /* Destination LID */
        uint8_t  src_path_bits; /* Source path bits */
        uint8_t  port_num;      /* Physical port */
        uint8_t  timeout;       /* Local ack timeout */
        uint8_t  retry_cnt;     /* Retry count */
        uint8_t  rnr_retry;     /* RNR retry count */
        uint8_t  max_rd_atomic; /* Max RDMA reads/atomic ops */
        uint8_t  min_rnr_timer; /* Min RNR NAK timer */
        union ib_gid dgid;      /* Destination GID */
    } remote;

    /* Capabilities */
    uint32_t max_inline_data;   /* Max inline data size */

    /* Statistics */
    uint64_t send_completions;
    uint64_t recv_completions;
    uint64_t errors;
} ib_qp_t;
```

### Work Request (WR) and Scatter/Gather

```c
/* Scatter/Gather Element */
typedef struct {
    uint64_t addr;          /* Virtual address */
    uint32_t length;        /* Length in bytes */
    uint32_t lkey;          /* Local key (from MR) */
} ib_sge_t;

/* Send Work Request */
typedef struct {
    uint64_t wr_id;         /* User-defined WR ID */
    struct ib_send_wr *next; /* Next WR in chain */

    ib_sge_t *sg_list;      /* Scatter/gather array */
    int num_sge;

    ibv_wr_opcode_t opcode;

    int send_flags;
#define IBV_SEND_FENCE      (1 << 0)    /* Fence operation */
#define IBV_SEND_SIGNALED   (1 << 1)    /* Generate completion */
#define IBV_SEND_SOLICITED  (1 << 2)    /* Solicited event */
#define IBV_SEND_INLINE     (1 << 3)    /* Inline data */
#define IBV_SEND_IP_CSUM    (1 << 4)    /* Offload IP checksum */

    /* For RDMA operations */
    union {
        struct {
            uint64_t remote_addr;   /* Remote virtual address */
            uint32_t rkey;          /* Remote key */
        } rdma;

        struct {
            uint64_t remote_addr;
            uint32_t rkey;
            uint64_t compare_add;   /* Compare or add value */
            uint64_t swap;          /* Swap value */
        } atomic;
    };

    uint32_t imm_data;      /* Immediate data (32-bit) */
} ib_send_wr_t;

/* Receive Work Request */
typedef struct {
    uint64_t wr_id;
    struct ib_recv_wr *next;

    ib_sge_t *sg_list;
    int num_sge;
} ib_recv_wr_t;
```

### Completion Queue (CQ)

```c
/* Completion Queue */
typedef struct ib_cq {
    uint32_t cq_num;
    int cqe;                /* Number of CQEs */

    /* CQ buffer (ring) */
    struct ib_wc *cq_buf;
    uint32_t head;
    uint32_t tail;

    /* Notification mechanism */
    int comp_vector;        /* MSI-X vector */
    int solicited_only;     /* Event on solicited only */

    /* Event channel */
    int event_fd;           /* For async notification */

    /* Statistics */
    uint64_t completions;
    uint64_t events;
    uint64_t overruns;
} ib_cq_t;

/* Work Completion (WC) */
typedef struct {
    uint64_t wr_id;         /* From WR */

    enum {
        IBV_WC_SUCCESS,
        IBV_WC_LOC_LEN_ERR,
        IBV_WC_LOC_QP_OP_ERR,
        IBV_WC_LOC_EEC_OP_ERR,
        IBV_WC_LOC_PROT_ERR,
        IBV_WC_WR_FLUSH_ERR,
        IBV_WC_MW_BIND_ERR,
        IBV_WC_BAD_RESP_ERR,
        IBV_WC_LOC_ACCESS_ERR,
        IBV_WC_REM_INV_REQ_ERR,
        IBV_WC_REM_ACCESS_ERR,
        IBV_WC_REM_OP_ERR,
        IBV_WC_RETRY_EXC_ERR,
        IBV_WC_RNR_RETRY_EXC_ERR,
        IBV_WC_LOC_RDD_VIOL_ERR,
        IBV_WC_REM_INV_RD_REQ_ERR,
        IBV_WC_REM_ABORT_ERR,
        IBV_WC_INV_EECN_ERR,
        IBV_WC_INV_EEC_STATE_ERR,
        IBV_WC_FATAL_ERR,
        IBV_WC_RESP_TIMEOUT_ERR,
        IBV_WC_GENERAL_ERR,
    } status;

    ibv_wr_opcode_t opcode;
    uint32_t byte_len;      /* Bytes transferred */
    uint32_t imm_data;      /* Immediate data (if present) */
    uint32_t qp_num;        /* QP number */

    int wc_flags;
#define IBV_WC_GRH          (1 << 0)    /* GRH present (UD) */
#define IBV_WC_WITH_IMM     (1 << 1)    /* Immediate data present */
#define IBV_WC_IP_CSUM_OK   (1 << 2)    /* IP checksum OK */

    uint16_t slid;          /* Source LID (UD) */
    uint8_t  sl;            /* Service Level */
    uint8_t  dlid_path_bits;
} ib_wc_t;
```

### Memory Region (MR) and Protection

```c
/* Protection Domain */
typedef struct ib_pd {
    uint32_t pd_num;
    struct ib_hca *hca;

    /* Resource limits */
    int max_qp;
    int max_mr;
    int max_ah;
} ib_pd_t;

/* Memory Region */
typedef struct ib_mr {
    uint32_t lkey;          /* Local key */
    uint32_t rkey;          /* Remote key */

    struct ib_pd *pd;

    uint64_t addr;          /* Virtual address */
    uint64_t length;        /* Length in bytes */

    /* Access flags */
    int access_flags;
#define IBV_ACCESS_LOCAL_WRITE      (1 << 0)
#define IBV_ACCESS_REMOTE_WRITE     (1 << 1)
#define IBV_ACCESS_REMOTE_READ      (1 << 2)
#define IBV_ACCESS_REMOTE_ATOMIC    (1 << 3)
#define IBV_ACCESS_MW_BIND          (1 << 4)
#define IBV_ACCESS_ZERO_BASED       (1 << 5)
#define IBV_ACCESS_ON_DEMAND        (1 << 6)

    /* Physical memory mapping */
    void *host_addr;        /* Host virtual address */

    /* For DMA */
    uint64_t *dma_pages;    /* Array of physical pages */
    int num_pages;
} ib_mr_t;

/* Memory Window (Type 1 and 2) */
typedef struct ib_mw {
    uint32_t rkey;
    struct ib_pd *pd;
    enum {
        IBV_MW_TYPE_1,      /* Bound via post_send */
        IBV_MW_TYPE_2,      /* Bound via bind_mw */
    } type;
} ib_mw_t;
```

### Address Handle (AH) for UD

```c
/* Address Handle - for UD QPs */
typedef struct ib_ah {
    struct ib_pd *pd;

    struct {
        union ib_gid dgid;      /* Destination GID */
        uint16_t dlid;          /* Destination LID */
        uint8_t  sl;            /* Service Level (0-15) */
        uint8_t  src_path_bits; /* Source path bits */
        uint8_t  static_rate;   /* Static rate */
        uint8_t  port_num;      /* Physical port */
        int is_global;          /* Use GRH */
        uint8_t  traffic_class;
        uint8_t  hop_limit;
        uint32_t flow_label;
    } attr;
} ib_ah_t;
```

## 3. RoCE (RDMA over Converged Ethernet)

### RoCE Network Adapter

```c
/* RoCE Device (Mellanox ConnectX-style) */
typedef struct {
    ib_hca_t ib;            /* Inherits IB HCA */

    /* Ethernet-specific */
    struct {
        uint8_t mac_addr[6];
        int mtu;            /* 1500 or 9000 (jumbo) */

        /* RoCE version */
        enum {
            ROCE_V1,        /* L2 only, EtherType 0x8915 */
            ROCE_V2,        /* UDP encapsulation, port 4791 */
        } version;

        /* Priority Flow Control (PFC) */
        int pfc_enabled;
        uint8_t pfc_priority;   /* 802.1p priority */

        /* ECN (Explicit Congestion Notification) */
        int ecn_enabled;

        /* DCQCN (Data Center QCN) */
        struct {
            int enabled;
            int g;              /* Weight for rate increase */
            int min_rate;       /* Minimum rate */
            int rate_reduce;    /* Rate reduction on CNP */
        } dcqcn;

        /* VLAN */
        int vlan_enabled;
        uint16_t vlan_id;
        uint8_t  vlan_prio;
    } eth;

    /* GID to MAC/IP mapping */
    struct gid_entry {
        union ib_gid gid;
        uint8_t mac[6];
        uint32_t ipv4;
        uint8_t ipv6[16];
        int roce_version;
    } gid_map[256];
} roce_hca_t;
```

### RoCEv2 Packet Format

```c
/* RoCEv2 Packet (UDP encapsulation) */
typedef struct {
    /* Ethernet Header */
    uint8_t  dst_mac[6];
    uint8_t  src_mac[6];
    uint16_t ethertype;     /* 0x0800 for IPv4, 0x86DD for IPv6 */

    /* IP Header */
    union {
        struct {
            uint8_t  version_ihl;
            uint8_t  tos;
            uint16_t total_length;
            uint16_t id;
            uint16_t flags_offset;
            uint8_t  ttl;
            uint8_t  protocol;  /* 17 = UDP */
            uint16_t checksum;
            uint32_t src_ip;
            uint32_t dst_ip;
        } ipv4;

        struct {
            uint32_t version_tc_fl;
            uint16_t payload_len;
            uint8_t  next_header;   /* 17 = UDP */
            uint8_t  hop_limit;
            uint8_t  src_ip[16];
            uint8_t  dst_ip[16];
        } ipv6;
    } ip;

    /* UDP Header */
    uint16_t src_port;      /* Ephemeral */
    uint16_t dst_port;      /* 4791 */
    uint16_t length;
    uint16_t checksum;

    /* InfiniBand Transport Header */
    union {
        struct {
            uint8_t  opcode;
            uint8_t  flags;
            uint16_t pkey;
            uint32_t qpn_reserved;
            uint32_t ack_psn;
            /* RC-specific fields */
        } rc;

        struct {
            uint8_t  opcode;
            uint8_t  flags;
            uint16_t pkey;
            uint32_t qpn;
            uint32_t psn;
            /* UD-specific fields */
        } ud;
    } bth;

    /* Payload */
    uint8_t data[];
} rocev2_packet_t;
```

## 4. iWARP (Internet Wide Area RDMA Protocol)

### iWARP Network Adapter

```c
/* iWARP Device */
typedef struct {
    ib_hca_t ib;            /* Inherits IB verbs interface */

    /* TCP/IP stack integration */
    struct {
        uint8_t mac_addr[6];
        uint32_t ipv4_addr;
        uint8_t  ipv6_addr[16];

        /* TCP connections per QP */
        struct iwarp_connection {
            int socket_fd;
            uint32_t local_ip;
            uint32_t remote_ip;
            uint16_t local_port;
            uint16_t remote_port;

            /* MPA (Marker PDU Aligned) framing */
            struct {
                int markers_enabled;
                int crc_enabled;
                uint16_t marker_interval;
            } mpa;

            /* DDP (Direct Data Placement) */
            struct {
                uint32_t msn;   /* Message Sequence Number */
                uint32_t mo;    /* Message Offset */
            } ddp;

            /* RDMAP state */
            struct {
                uint32_t next_tag;
                int rdma_read_credits;
            } rdmap;
        } *connections;

        int num_connections;
    } tcp;
} iwarp_hca_t;

/* iWARP Protocol Stack */
typedef struct {
    /* MPA (Marker PDU Aligned Framing) Header */
    struct {
        uint16_t flags;
        uint16_t private_data_size;
        uint8_t  private_data[512];
    } mpa_header;

    /* DDP (Direct Data Placement) Header */
    struct {
        uint8_t  control;
        uint8_t  rsvd_ub;
        uint16_t rsvd_bits;
        uint32_t stag;      /* Steering Tag */
        uint64_t to;        /* Tagged Offset */
    } ddp_header;

    /* RDMAP (Remote Direct Memory Access Protocol) Header */
    struct {
        uint8_t  version;
        uint8_t  opcode;
        uint16_t control;
        uint32_t inv_stag;  /* For SEND_INV */
        uint32_t queue_number;
        uint32_t msn;
        uint32_t mo;
    } rdmap_header;
} iwarp_packet_t;
```

## 5. RDMA Operations Implementation

### RDMA Write

```c
/* RDMA Write - one-sided operation */
void rdma_write(ib_qp_t *qp, ib_send_wr_t *wr) {
    /* Local scatter/gather */
    uint8_t *local_buf = malloc(wr->sg_list[0].length);
    for (int i = 0; i < wr->num_sge; i++) {
        ib_mr_t *mr = lookup_mr(wr->sg_list[i].lkey);
        memcpy(local_buf + offset,
               mr->host_addr + (wr->sg_list[i].addr - mr->addr),
               wr->sg_list[i].length);
        offset += wr->sg_list[i].length;
    }

    /* Create RDMA Write packet */
    ib_packet_t *pkt = create_packet();
    pkt->bth.opcode = IB_OPCODE_RC_RDMA_WRITE_ONLY;
    pkt->bth.dest_qp = qp->remote.dest_qp_num;
    pkt->bth.psn = qp->sq.psn++;

    pkt->reth.va = wr->rdma.remote_addr;
    pkt->reth.rkey = wr->rdma.rkey;
    pkt->reth.dma_len = total_length;

    memcpy(pkt->payload, local_buf, total_length);

    /* Send packet (no ACK required unless signaled) */
    send_packet(qp, pkt);

    /* Generate completion if signaled */
    if (wr->send_flags & IBV_SEND_SIGNALED) {
        post_completion(qp->send_cq, wr->wr_id, IBV_WC_SUCCESS);
    }
}
```

### RDMA Read

```c
/* RDMA Read - one-sided operation with response */
void rdma_read(ib_qp_t *qp, ib_send_wr_t *wr) {
    /* Create RDMA Read Request */
    ib_packet_t *req = create_packet();
    req->bth.opcode = IB_OPCODE_RC_RDMA_READ_REQUEST;
    req->bth.dest_qp = qp->remote.dest_qp_num;
    req->bth.psn = qp->sq.psn++;

    req->reth.va = wr->rdma.remote_addr;
    req->reth.rkey = wr->rdma.rkey;
    req->reth.dma_len = wr->sg_list[0].length;

    send_packet(qp, req);

    /* Response will be handled by receive handler */
    /* Data will be DMA'd directly to local SGE addresses */
}

/* Handle RDMA Read Response */
void handle_rdma_read_response(ib_qp_t *qp, ib_packet_t *resp) {
    /* Find pending read WR */
    ib_send_wr_t *wr = find_pending_read(qp, resp->bth.psn);

    /* DMA data directly to local memory */
    int offset = 0;
    for (int i = 0; i < wr->num_sge; i++) {
        ib_mr_t *mr = lookup_mr(wr->sg_list[i].lkey);
        memcpy(mr->host_addr + (wr->sg_list[i].addr - mr->addr),
               resp->payload + offset,
               wr->sg_list[i].length);
        offset += wr->sg_list[i].length;
    }

    /* Post completion */
    post_completion(qp->send_cq, wr->wr_id, IBV_WC_SUCCESS);
}
```

### Atomic Operations

```c
/* Compare and Swap */
uint64_t rdma_atomic_cmp_swp(ib_qp_t *qp, ib_send_wr_t *wr) {
    ib_packet_t *pkt = create_packet();
    pkt->bth.opcode = IB_OPCODE_RC_COMPARE_SWAP;
    pkt->bth.dest_qp = qp->remote.dest_qp_num;
    pkt->bth.psn = qp->sq.psn++;

    pkt->atomiceth.va = wr->atomic.remote_addr;
    pkt->atomiceth.rkey = wr->atomic.rkey;
    pkt->atomiceth.swap_dt = wr->atomic.swap;
    pkt->atomiceth.cmp_dt = wr->atomic.compare_add;

    send_packet(qp, pkt);

    /* Wait for response with original value */
    ib_packet_t *resp = wait_for_response(qp);
    uint64_t original = resp->atomicacketh.orig_data;

    post_completion(qp->send_cq, wr->wr_id, IBV_WC_SUCCESS);

    return original;
}

/* Fetch and Add */
uint64_t rdma_atomic_fetch_add(ib_qp_t *qp, ib_send_wr_t *wr) {
    ib_packet_t *pkt = create_packet();
    pkt->bth.opcode = IB_OPCODE_RC_FETCH_ADD;
    pkt->bth.dest_qp = qp->remote.dest_qp_num;
    pkt->bth.psn = qp->sq.psn++;

    pkt->atomiceth.va = wr->atomic.remote_addr;
    pkt->atomiceth.rkey = wr->atomic.rkey;
    pkt->atomiceth.swap_dt = wr->atomic.compare_add;

    send_packet(qp, pkt);

    ib_packet_t *resp = wait_for_response(qp);
    uint64_t original = resp->atomicacketh.orig_data;

    post_completion(qp->send_cq, wr->wr_id, IBV_WC_SUCCESS);

    return original;
}
```

## 6. Advanced Features

### Shared Receive Queue (SRQ)

```c
/* Shared Receive Queue - shared among multiple QPs */
typedef struct ib_srq {
    uint32_t srq_num;
    struct ib_pd *pd;

    /* Receive WQEs */
    struct {
        struct ib_recv_wr *wqe;
        uint32_t head;
        uint32_t tail;
        uint32_t max_wr;
        uint32_t max_sge;
    } rq;

    /* Limit for SRQ event */
    uint32_t limit;

    /* QPs using this SRQ */
    struct ib_qp **qps;
    int num_qps;
} ib_srq_t;

/* Post recv to SRQ instead of individual QP */
void ibv_post_srq_recv(ib_srq_t *srq, ib_recv_wr_t *wr) {
    /* Add WR to SRQ */
    /* Any QP using this SRQ can consume it */
}
```

### Extended Reliable Connection (XRC)

```c
/* XRC - allows one-to-many connections with shared receive */
typedef struct ib_xrcd {
    int xrcd_num;
    int fd;             /* File descriptor for sharing */
} ib_xrcd_t;

typedef struct ib_xrc_qp {
    ib_qp_t base;
    struct ib_xrcd *xrcd;
    uint32_t xrc_remote_srq_num;
} ib_xrc_qp_t;
```

### On-Demand Paging (ODP)

```c
/* On-Demand Paging - no need to pin memory */
typedef struct {
    ib_mr_t base;

    /* Page fault handling */
    struct {
        int enabled;
        pthread_t fault_handler;

        /* Page table */
        struct {
            uint64_t va;
            uint64_t pa;
            int present;
        } *page_table;

        int num_pages;
    } odp;
} ib_odp_mr_t;

/* Handle page fault */
void handle_page_fault(ib_odp_mr_t *mr, uint64_t addr) {
    /* Resolve virtual to physical */
    /* Update page table */
    /* Resume RDMA operation */
}
```

### GPUDirect RDMA

```c
/* GPUDirect RDMA - direct GPU memory access */
typedef struct {
    ib_mr_t base;

    /* GPU-specific */
    struct {
        int gpu_id;
        void *gpu_ptr;      /* GPU device pointer */
        uint64_t gpu_bar;   /* GPU BAR address */

        /* Peer-to-peer DMA */
        int p2p_enabled;
    } gpu;
} ib_gpu_mr_t;

/* Register GPU memory for RDMA */
ib_gpu_mr_t *ibv_reg_gpu_mr(ib_pd_t *pd, void *gpu_ptr, size_t length) {
    ib_gpu_mr_t *mr = calloc(1, sizeof(*mr));

    /* Get GPU physical address */
    mr->gpu.gpu_bar = get_gpu_physical_addr(gpu_ptr);
    mr->gpu.gpu_ptr = gpu_ptr;

    /* Enable P2P DMA */
    enable_p2p_dma(pd->hca, mr->gpu.gpu_id);

    /* Register with HCA */
    mr->base.addr = (uint64_t)gpu_ptr;
    mr->base.length = length;
    mr->base.lkey = allocate_lkey();
    mr->base.rkey = allocate_rkey();

    return mr;
}
```

### Managed Flow Steering

```c
/* Flow Steering - hardware packet filtering */
typedef struct {
    enum {
        IBV_FLOW_SPEC_ETH,
        IBV_FLOW_SPEC_IPV4,
        IBV_FLOW_SPEC_TCP,
        IBV_FLOW_SPEC_UDP,
    } type;

    union {
        struct {
            uint8_t dst_mac[6];
            uint8_t src_mac[6];
            uint16_t ether_type;
            uint16_t vlan_tag;
        } eth;

        struct {
            uint32_t src_ip;
            uint32_t dst_ip;
            uint8_t  tos;
            uint8_t  protocol;
        } ipv4;

        struct {
            uint16_t src_port;
            uint16_t dst_port;
        } tcp_udp;
    };

    /* Mask for matching */
    uint8_t mask[64];
} ib_flow_spec_t;

typedef struct {
    int priority;
    ib_flow_spec_t *specs;
    int num_specs;

    /* Action */
    struct ib_qp *dest_qp;
    int drop;
} ib_flow_rule_t;
```

### Congestion Control

```c
/* Congestion Control (DCQCN for RoCE) */
typedef struct {
    /* Rate limiting */
    struct {
        uint64_t current_rate;  /* Gbps */
        uint64_t target_rate;
        uint64_t timer_us;      /* Update period */

        /* DCQCN parameters */
        double g;               /* 1/256 typically */
        double alpha;           /* Current alpha */
        double alpha_timer;
        int f;                  /* Fast recovery */
    } rate_limiter;

    /* ECN marking */
    struct {
        int enabled;
        uint32_t threshold;     /* Marking threshold (bytes) */
    } ecn;

    /* CNP (Congestion Notification Packet) */
    struct {
        uint64_t sent;
        uint64_t received;
    } cnp_stats;
} ib_congestion_ctrl_t;
```

## 7. Multi-Network Topology

### RDMA Switch/Router Emulation

```c
/* InfiniBand Switch */
typedef struct {
    int num_ports;          /* 12, 24, 36, 48 ports typical */

    struct ib_switch_port {
        int port_num;
        enum ib_port_state state;
        int width;          /* 1x, 4x, 8x, 12x */
        int speed;          /* SDR, DDR, QDR, FDR, EDR, HDR */

        /* Connected device */
        void *connected_device;
        int connected_port;

        /* Forwarding */
        uint16_t lid_table[49152];  /* LID → output port */
    } ports[48];

    /* Subnet Manager functionality */
    struct {
        int is_sm;
        uint16_t lid_base;      /* LID allocation base */
        uint16_t lid_next;
    } sm;

    /* Multicast support */
    struct {
        uint16_t mlid;
        uint64_t port_mask;     /* Which ports are members */
    } mcast_table[256];
} ib_switch_t;
```

### RDMA Network Fabric

```c
/* Complete RDMA Fabric */
typedef struct {
    /* Devices */
    ib_hca_t *hcas[256];
    ib_switch_t *switches[64];

    /* Topology */
    enum {
        FABRIC_TOPO_FLAT,       /* Single switch */
        FABRIC_TOPO_TREE,       /* Tree of switches */
        FABRIC_TOPO_FAT_TREE,   /* Fat tree (Clos) */
        FABRIC_TOPO_DRAGONFLY,  /* Dragonfly */
        FABRIC_TOPO_MESH,       /* 2D/3D mesh */
    } topology;

    /* Subnet Manager */
    struct {
        ib_hca_t *master_sm;
        uint16_t sm_lid;

        /* LID assignment */
        uint16_t next_lid;

        /* Path records */
        struct {
            uint16_t slid;
            uint16_t dlid;
            uint8_t  hop_count;
            uint8_t  path[64];  /* Port hops */
        } *path_cache;
    } sm;

    /* Routing */
    enum {
        ROUTE_MIN_HOP,
        ROUTE_UP_DOWN,
        ROUTE_FAT_TREE,
        ROUTE_ADAPTIVE,
    } routing_algorithm;
} rdma_fabric_t;
```

## 8. Performance Modeling

### Latency Model

```c
/* RDMA Latency Components */
typedef struct {
    /* Software overhead */
    int post_send_ns;       /* 50-100 ns */
    int poll_cq_ns;         /* 50-100 ns */

    /* Hardware processing */
    int hca_processing_ns;  /* 100-200 ns */

    /* Network */
    int serialization_ns;   /* Message size / link speed */
    int propagation_ns;     /* Cable length / speed of light */
    int switch_ns;          /* Per-hop: 100-200 ns */

    /* Total one-way */
    int total_ns;           /* Typically 1-5 us */
} rdma_latency_t;

int calculate_rdma_latency(int msg_size, int num_hops) {
    rdma_latency_t lat = {0};

    lat.post_send_ns = 75;
    lat.hca_processing_ns = 150;
    lat.serialization_ns = (msg_size * 8 * 1000) / 100;  /* 100 Gbps */
    lat.propagation_ns = 5 * num_hops;  /* 5 ns per meter, assume 1m cables */
    lat.switch_ns = 150 * num_hops;
    lat.poll_cq_ns = 75;

    lat.total_ns = lat.post_send_ns + lat.hca_processing_ns +
                   lat.serialization_ns + lat.propagation_ns +
                   lat.switch_ns + lat.poll_cq_ns;

    return lat.total_ns;
}
```

### Bandwidth Model

```c
/* RDMA Bandwidth */
typedef struct {
    int link_speed_gbps;    /* 25, 50, 100, 200, 400 */

    /* Efficiency factors */
    double protocol_overhead;   /* 0.95-0.98 */
    double pfc_overhead;        /* 0.98-1.0 (RoCE only) */

    /* Effective bandwidth */
    double effective_gbps;
} rdma_bandwidth_t;

double calculate_effective_bandwidth(int link_speed, int msg_size) {
    rdma_bandwidth_t bw = {0};
    bw.link_speed_gbps = link_speed;

    /* Small messages have lower efficiency */
    if (msg_size < 1024) {
        bw.protocol_overhead = 0.80;
    } else if (msg_size < 8192) {
        bw.protocol_overhead = 0.92;
    } else {
        bw.protocol_overhead = 0.97;
    }

    bw.pfc_overhead = 0.99;

    bw.effective_gbps = bw.link_speed_gbps *
                        bw.protocol_overhead *
                        bw.pfc_overhead;

    return bw.effective_gbps;
}
```

## 9. Integration with DLXSIM

### Memory Management Integration

```c
/* Hook RDMA into DLX MMU */
void dlx_rdma_dma_read(dlx_sim_t *sim, uint64_t addr, void *buf, size_t len) {
    /* Translate virtual to physical */
    for (size_t i = 0; i < len; i += 4096) {
        uint64_t phys = dlx_mmu_translate(sim, addr + i);
        memcpy(buf + i, sim->memory.mem + phys,
               MIN(4096, len - i));
    }
}

void dlx_rdma_dma_write(dlx_sim_t *sim, uint64_t addr, void *buf, size_t len) {
    for (size_t i = 0; i < len; i += 4096) {
        uint64_t phys = dlx_mmu_translate(sim, addr + i);
        memcpy(sim->memory.mem + phys, buf + i,
               MIN(4096, len - i));
    }
}
```

### Interrupt Integration with AIC

```c
/* RDMA interrupts via AIC */
void rdma_raise_interrupt(dlx_sim_t *sim, int cpu_id, int vector) {
    aic_t *aic = &sim->aic;

    /* Set interrupt pending */
    aic->cpu[cpu_id].event[vector / 32] |= (1 << (vector % 32));

    /* Wake CPU if in WFI */
    if (sim->vcpus[cpu_id].cpu.status & DLX_STATUS_WFI) {
        sim->vcpus[cpu_id].cpu.status &= ~DLX_STATUS_WFI;
    }
}

/* MSI-X for completion queue events */
void rdma_cq_event(ib_cq_t *cq) {
    rdma_raise_interrupt(cq->sim, cq->comp_vector % sim->num_cpus,
                        RDMA_CQ_VECTOR_BASE + cq->comp_vector);
}
```

### Device Configuration

```yaml
# dlxsim.yaml RDMA configuration
devices:
  - type: infiniband
    model: connectx6
    ports: 2
    port_config:
      - port: 1
        speed: HDR  # 200 Gbps
        width: 4x
        guid: 0x506b4b0300ca0000
      - port: 2
        speed: HDR
        width: 4x
        guid: 0x506b4b0300ca0001

    capabilities:
      max_qp: 262144
      max_cq: 262144
      max_mr: 16777216
      max_pd: 262144
      atomic: global
      odp: true
      gpudirect: true

    features:
      - xrc
      - srq
      - flow_steering
      - congestion_control

  - type: roce
    model: connectx6-dx
    mac: "00:0c:29:ab:cd:ef"
    roce_version: v2
    pfc: true
    ecn: true
    dcqcn: true

fabric:
  topology: fat_tree
  switches:
    - type: ib_switch
      ports: 36
      speed: HDR

  routing: fat_tree

  subnet_manager:
    enabled: true
    lid_base: 1
```

## 10. Testing and Benchmarks

### Built-in Benchmarks

```c
/* RDMA Ping-Pong latency test */
void rdma_pingpong_test(ib_qp_t *qp) {
    uint64_t start, end;
    int iterations = 10000;

    for (int i = 0; i < iterations; i++) {
        start = rdtsc();

        /* Post send */
        ibv_post_send(qp, &send_wr);

        /* Poll for completion */
        while (!ibv_poll_cq(qp->send_cq, &wc));

        end = rdtsc();

        latencies[i] = cycles_to_ns(end - start);
    }

    printf("Latency: avg=%.2f ns, min=%d ns, max=%d ns\n",
           avg(latencies), min(latencies), max(latencies));
}

/* RDMA Bandwidth test */
void rdma_bandwidth_test(ib_qp_t *qp, int msg_size) {
    uint64_t start, end;
    int iterations = 10000;
    uint64_t total_bytes = (uint64_t)iterations * msg_size;

    start = rdtsc();

    for (int i = 0; i < iterations; i++) {
        ibv_post_send(qp, &send_wr);

        /* Batch completions */
        if (i % 16 == 0) {
            while (ibv_poll_cq(qp->send_cq, &wc) < 16);
        }
    }

    end = rdtsc();

    double seconds = cycles_to_seconds(end - start);
    double gbps = (total_bytes * 8.0) / (seconds * 1e9);

    printf("Bandwidth: %.2f Gbps (%.2f MB/s)\n",
           gbps, (total_bytes / seconds) / (1024 * 1024));
}
```

---

**Document Status**: Specification Complete
**Implementation Target**: DLXSIM 3.0
**Complexity**: Very High - requires networking, DMA, and verbs API
**Estimated LOC**: ~30,000 lines
**Key Dependencies**: PCIe framework, DMA engine, network backend
**Performance Target**: < 2 μs latency, > 50 Gbps bandwidth (simulated)
