# DLX Virtualization and I/O Emulation Architecture

## Overview

This document specifies the comprehensive I/O emulation and virtualization features for DLXSIM, transforming it from a simple instruction simulator into a full-featured virtual machine capable of running complete operating systems with modern peripheral support.

## 1. PCIe Bus Architecture

### PCIe Hierarchy
```
Root Complex (RC)
  └── Root Port 0 (Bus 0, Device 0)
      ├── Upstream Port
      │   ├── Downstream Port 0 → Endpoint (Network)
      │   ├── Downstream Port 1 → Endpoint (Storage)
      │   ├── Downstream Port 2 → Endpoint (Graphics)
      │   └── Downstream Port 3 → Endpoint (USB Controller)
      └── Integrated Endpoint (AHCI)
```

### PCIe Configuration Space

#### Type 0 Header (Endpoints)
```c
typedef struct {
    uint16_t vendor_id;           /* 0x00 */
    uint16_t device_id;           /* 0x02 */
    uint16_t command;             /* 0x04 */
    uint16_t status;              /* 0x06 */
    uint8_t  revision_id;         /* 0x08 */
    uint8_t  prog_if;             /* 0x09 */
    uint8_t  subclass;            /* 0x0A */
    uint8_t  class_code;          /* 0x0B */
    uint8_t  cache_line_size;     /* 0x0C */
    uint8_t  latency_timer;       /* 0x0D */
    uint8_t  header_type;         /* 0x0E */
    uint8_t  bist;                /* 0x0F */
    uint32_t bar[6];              /* 0x10-0x27 */
    uint32_t cardbus_cis;         /* 0x28 */
    uint16_t subsystem_vendor_id; /* 0x2C */
    uint16_t subsystem_id;        /* 0x2E */
    uint32_t expansion_rom_base;  /* 0x30 */
    uint8_t  capabilities_ptr;    /* 0x34 */
    uint8_t  reserved[7];         /* 0x35-0x3B */
    uint8_t  interrupt_line;      /* 0x3C */
    uint8_t  interrupt_pin;       /* 0x3D */
    uint8_t  min_grant;           /* 0x3E */
    uint8_t  max_latency;         /* 0x3F */
} pcie_config_type0_t;
```

#### PCIe Capabilities
```c
/* PCIe Capability Structure (offset 0x100+) */
typedef struct {
    uint16_t cap_id;              /* 0x10 = PCIe */
    uint16_t cap_version:4;
    uint16_t dev_port_type:4;
    uint16_t slot_implemented:1;
    uint16_t irq_msg_num:5;
    uint16_t reserved:2;

    uint32_t dev_caps;            /* Device Capabilities */
    uint16_t dev_control;         /* Device Control */
    uint16_t dev_status;          /* Device Status */

    uint32_t link_caps;           /* Link Capabilities */
    uint16_t link_control;        /* Link Control */
    uint16_t link_status;         /* Link Status */

    uint32_t slot_caps;           /* Slot Capabilities */
    uint16_t slot_control;        /* Slot Control */
    uint16_t slot_status;         /* Slot Status */
} pcie_capability_t;
```

### PCIe Memory Mapping
```
0xE000_0000 - 0xEFFF_FFFF   PCIe Configuration Space (256MB)
  0xE000_0000 - 0xE0FF_FFFF     Bus 0 (16MB)
  0xE100_0000 - 0xE1FF_FFFF     Bus 1 (16MB)
  ...

0xF000_0000 - 0xF7FF_FFFF   PCIe Memory-Mapped I/O (128MB)
0xF800_0000 - 0xFBFF_FFFF   PCIe I/O Space (64MB)
0xFC00_0000 - 0xFFFF_FFFF   System Reserved (64MB)
```

### PCIe Transaction Layer Packets (TLP)

```c
typedef enum {
    TLP_MRd32,      /* Memory Read 32-bit */
    TLP_MRd64,      /* Memory Read 64-bit */
    TLP_MWr32,      /* Memory Write 32-bit */
    TLP_MWr64,      /* Memory Write 64-bit */
    TLP_IORd,       /* I/O Read */
    TLP_IOWr,       /* I/O Write */
    TLP_CfgRd0,     /* Configuration Read Type 0 */
    TLP_CfgWr0,     /* Configuration Write Type 0 */
    TLP_Msg,        /* Message */
    TLP_MsgD,       /* Message with Data */
    TLP_Cpl,        /* Completion without Data */
    TLP_CplD,       /* Completion with Data */
} tlp_type_t;

typedef struct {
    uint8_t fmt:2;          /* Format */
    uint8_t type:5;         /* Type */
    uint8_t tc:3;           /* Traffic Class */
    uint8_t attr:2;         /* Attributes */
    uint16_t length:10;     /* Length in DW */
    uint16_t requester_id;  /* Bus:Dev:Func */
    uint8_t tag;            /* Transaction Tag */
    uint8_t first_be:4;     /* First DW Byte Enable */
    uint8_t last_be:4;      /* Last DW Byte Enable */
    uint64_t address;       /* Address */
    uint32_t data[];        /* Payload */
} pcie_tlp_t;
```

## 2. VirtIO Framework

### VirtIO Architecture

VirtIO provides para-virtualized devices with high performance through shared memory rings.

```c
/* VirtIO Device Structure */
typedef struct {
    uint32_t device_features_sel;   /* 0x00 */
    uint32_t device_features;        /* 0x04 */
    uint32_t driver_features_sel;    /* 0x08 */
    uint32_t driver_features;        /* 0x0C */
    uint32_t queue_sel;              /* 0x10 */
    uint32_t queue_num_max;          /* 0x14 */
    uint32_t queue_num;              /* 0x18 */
    uint32_t queue_ready;            /* 0x1C */
    uint32_t queue_notify;           /* 0x20 */
    uint32_t interrupt_status;       /* 0x24 */
    uint32_t interrupt_ack;          /* 0x28 */
    uint32_t status;                 /* 0x2C */
    uint32_t queue_desc_low;         /* 0x30 */
    uint32_t queue_desc_high;        /* 0x34 */
    uint32_t queue_avail_low;        /* 0x38 */
    uint32_t queue_avail_high;       /* 0x3C */
    uint32_t queue_used_low;         /* 0x40 */
    uint32_t queue_used_high;        /* 0x44 */
    uint32_t config_generation;      /* 0x48 */
} virtio_regs_t;
```

### VirtQueue Implementation

```c
/* VirtQueue Descriptor */
typedef struct {
    uint64_t addr;          /* Physical address */
    uint32_t len;           /* Length */
    uint16_t flags;         /* Flags (NEXT, WRITE, INDIRECT) */
    uint16_t next;          /* Next descriptor index */
} virtq_desc_t;

/* VirtQueue Available Ring */
typedef struct {
    uint16_t flags;
    uint16_t idx;
    uint16_t ring[];        /* Available descriptor heads */
    uint16_t used_event;    /* Used for event notification */
} virtq_avail_t;

/* VirtQueue Used Ring */
typedef struct {
    uint16_t flags;
    uint16_t idx;
    struct {
        uint32_t id;        /* Descriptor chain head */
        uint32_t len;       /* Bytes written */
    } ring[];
    uint16_t avail_event;   /* Avail for event notification */
} virtq_used_t;
```

### VirtIO-Net (Network)

```c
/* VirtIO-Net Device Configuration */
typedef struct {
    uint8_t  mac[6];                /* MAC address */
    uint16_t status;                /* Link status */
    uint16_t max_virtqueue_pairs;   /* Maximum queue pairs */
    uint16_t mtu;                   /* MTU */
} virtio_net_config_t;

/* VirtIO-Net Packet Header */
typedef struct {
    uint8_t flags;
    uint8_t gso_type;
    uint16_t hdr_len;
    uint16_t gso_size;
    uint16_t csum_start;
    uint16_t csum_offset;
    uint16_t num_buffers;
} virtio_net_hdr_t;

/* Feature Bits */
#define VIRTIO_NET_F_CSUM           (1 << 0)
#define VIRTIO_NET_F_GUEST_CSUM     (1 << 1)
#define VIRTIO_NET_F_MAC            (1 << 5)
#define VIRTIO_NET_F_GSO            (1 << 6)
#define VIRTIO_NET_F_GUEST_TSO4     (1 << 7)
#define VIRTIO_NET_F_GUEST_TSO6     (1 << 8)
#define VIRTIO_NET_F_STATUS         (1 << 16)
#define VIRTIO_NET_F_MQ             (1 << 22)
```

### VirtIO-Block (Storage)

```c
/* VirtIO-Block Configuration */
typedef struct {
    uint64_t capacity;      /* Device capacity in 512-byte sectors */
    uint32_t size_max;      /* Maximum segment size */
    uint32_t seg_max;       /* Maximum segments */
    struct {
        uint16_t cylinders;
        uint8_t  heads;
        uint8_t  sectors;
    } geometry;
    uint32_t blk_size;      /* Block size */
    uint8_t  physical_block_exp;
    uint8_t  alignment_offset;
    uint16_t min_io_size;
    uint32_t opt_io_size;
    uint8_t  writeback;
    uint8_t  unused0;
    uint16_t num_queues;
    uint32_t max_discard_sectors;
    uint32_t max_discard_seg;
    uint32_t discard_sector_alignment;
    uint32_t max_write_zeroes_sectors;
    uint32_t max_write_zeroes_seg;
    uint8_t  write_zeroes_may_unmap;
} virtio_blk_config_t;

/* VirtIO-Block Request */
typedef struct {
    uint32_t type;          /* READ, WRITE, FLUSH, etc. */
    uint32_t ioprio;        /* I/O priority */
    uint64_t sector;        /* Starting sector */
} virtio_blk_req_t;

#define VIRTIO_BLK_T_IN         0   /* Read */
#define VIRTIO_BLK_T_OUT        1   /* Write */
#define VIRTIO_BLK_T_FLUSH      4   /* Flush */
#define VIRTIO_BLK_T_DISCARD    11  /* Discard */
#define VIRTIO_BLK_T_WRITE_ZEROES 13 /* Write Zeroes */
```

### VirtIO-GPU (Graphics)

```c
/* VirtIO-GPU Configuration */
typedef struct {
    uint32_t events_read;
    uint32_t events_clear;
    uint32_t num_scanouts;
    uint32_t reserved;
} virtio_gpu_config_t;

/* VirtIO-GPU 2D Commands */
typedef enum {
    VIRTIO_GPU_CMD_GET_DISPLAY_INFO = 0x0100,
    VIRTIO_GPU_CMD_RESOURCE_CREATE_2D,
    VIRTIO_GPU_CMD_RESOURCE_UNREF,
    VIRTIO_GPU_CMD_SET_SCANOUT,
    VIRTIO_GPU_CMD_RESOURCE_FLUSH,
    VIRTIO_GPU_CMD_TRANSFER_TO_HOST_2D,
    VIRTIO_GPU_CMD_RESOURCE_ATTACH_BACKING,
    VIRTIO_GPU_CMD_RESOURCE_DETACH_BACKING,
} virtio_gpu_cmd_t;

typedef struct {
    uint32_t resource_id;
    uint32_t width;
    uint32_t height;
    uint32_t format;        /* RGBA, BGRA, etc. */
} virtio_gpu_resource_create_2d_t;
```

## 3. IOMMU (I/O Memory Management Unit)

### IOMMU Architecture

The IOMMU provides DMA remapping and protection, essential for device virtualization and isolation.

```c
/* IOMMU Device */
typedef struct {
    pcie_config_type0_t pci_config;

    /* Capability Registers */
    uint32_t version;           /* IOMMU version */
    uint64_t capabilities;      /* Feature flags */
#define IOMMU_CAP_PAGING        (1ULL << 0)    /* Page table support */
#define IOMMU_CAP_INTERRUPT_REMAP (1ULL << 1)  /* Interrupt remapping */
#define IOMMU_CAP_CACHE_COHERENT (1ULL << 2)   /* Cache coherency */
#define IOMMU_CAP_IOTLB         (1ULL << 3)    /* IOTLB support */
#define IOMMU_CAP_PASID         (1ULL << 4)    /* Process address space ID */
#define IOMMU_CAP_PRI           (1ULL << 5)    /* Page request interface */
#define IOMMU_CAP_ATS           (1ULL << 6)    /* Address translation service */
#define IOMMU_CAP_NESTED        (1ULL << 7)    /* Nested translation */

    /* Control Registers */
    uint32_t global_control;    /* Global control */
#define IOMMU_GCMD_ENABLE       (1 << 0)
#define IOMMU_GCMD_SET_ROOT_PTR (1 << 1)
#define IOMMU_GCMD_IOTLB_INV    (1 << 2)
#define IOMMU_GCMD_CACHE_INV    (1 << 3)
#define IOMMU_GCMD_INTR_REMAP   (1 << 4)

    uint32_t global_status;     /* Global status */
    uint64_t root_table_ptr;    /* Root table address */
    uint64_t context_cmd;       /* Context command */
    uint32_t fault_status;      /* Fault status */
    uint32_t fault_event_ctrl;  /* Fault event control */
    uint64_t fault_event_data;  /* Fault event data */
    uint32_t fault_event_addr;  /* Fault event address */

    /* IOTLB Invalidation */
    uint64_t iotlb_inv;         /* IOTLB invalidate */
    uint64_t iotlb_inv_addr;    /* Address for selective invalidation */

    /* Interrupt Remapping */
    uint64_t intr_remap_table;  /* Interrupt remapping table base */
    uint32_t intr_remap_ctl;    /* Interrupt remapping control */

    /* Fault Recording Registers */
    struct {
        uint64_t fault_addr;    /* Faulting address */
        uint32_t fault_info;    /* Fault information */
#define FAULT_INFO_TYPE_MASK    0xF
#define FAULT_REASON_MASK       0xFF0
        uint16_t source_id;     /* Source device ID (Bus:Dev:Func) */
        uint16_t pasid;         /* PASID if applicable */
    } fault_record[256];

    int num_faults;

    /* Device Context Tables */
    struct iommu_context_table *root_table;

    /* IOTLB Cache */
    struct iotlb_entry {
        uint64_t iova;          /* I/O Virtual Address */
        uint64_t phys_addr;     /* Physical Address */
        uint16_t did;           /* Domain ID */
        uint16_t pasid;         /* PASID */
        uint32_t flags;
        int valid;
    } iotlb[4096];

    /* Statistics */
    uint64_t translations;
    uint64_t iotlb_hits;
    uint64_t iotlb_misses;
    uint64_t faults;
} iommu_t;
```

### IOMMU Page Tables

```c
/* IOMMU uses similar page table structure to CPU MMU */
/* Supports 2-level, 3-level, or 4-level page tables */

typedef struct {
    uint64_t present:1;
    uint64_t read:1;
    uint64_t write:1;
    uint64_t execute:1;         /* Some IOMMUs support execute permission */
    uint64_t supervise:1;       /* Supervisor/user mode */
    uint64_t snoop:1;           /* Cache snoop */
    uint64_t accessed:1;
    uint64_t dirty:1;
    uint64_t page_size:1;       /* 0=4KB, 1=large page */
    uint64_t global:1;
    uint64_t available:3;
    uint64_t addr:40;           /* Physical address [51:12] */
    uint64_t reserved:11;
    uint64_t no_execute:1;
} iommu_pte_t;

/* Domain Context Entry */
typedef struct {
    uint64_t present:1;
    uint64_t fault_disable:1;
    uint64_t translation_type:2;  /* Pass-through, translate, etc. */
#define IOMMU_TT_PASSTHROUGH    0
#define IOMMU_TT_TRANSLATE      1
#define IOMMU_TT_NESTED         2

    uint64_t address_width:3;     /* 39, 48, 57 bits */
    uint64_t hint:1;
    uint64_t reserved1:4;
    uint64_t slptr:52;            /* Second level page table pointer */

    uint64_t domain_id:16;
    uint64_t reserved2:5;
    uint64_t aw:1;                /* Adjusted guest address width */
    uint64_t eim:1;               /* Extended interrupt mode */
    uint64_t reserved3:41;
} iommu_context_entry_t;
```

### DMA Remapping

```c
/* DMA Read/Write through IOMMU */
uint64_t iommu_translate_dma(iommu_t *iommu, uint16_t source_id,
                             uint64_t iova, int access_type) {
    /* 1. Look up device context */
    iommu_context_entry_t *context = lookup_context(iommu, source_id);

    if (!context->present) {
        record_fault(iommu, FAULT_CONTEXT_NOT_PRESENT, source_id, iova);
        return INVALID_ADDR;
    }

    /* 2. Check for pass-through mode */
    if (context->translation_type == IOMMU_TT_PASSTHROUGH) {
        return iova;  /* No translation */
    }

    /* 3. Check IOTLB cache */
    struct iotlb_entry *entry = iotlb_lookup(iommu, context->domain_id, iova);
    if (entry && entry->valid) {
        iommu->iotlb_hits++;
        return entry->phys_addr | (iova & 0xFFF);  /* Combine with offset */
    }

    iommu->iotlb_misses++;

    /* 4. Page table walk */
    iommu_pte_t *pte = walk_page_table(iommu, context->slptr, iova);

    if (!pte || !pte->present) {
        record_fault(iommu, FAULT_PAGE_NOT_PRESENT, source_id, iova);
        return INVALID_ADDR;
    }

    /* 5. Check permissions */
    if (access_type == DMA_WRITE && !pte->write) {
        record_fault(iommu, FAULT_WRITE_PERMISSION, source_id, iova);
        return INVALID_ADDR;
    }

    if (access_type == DMA_READ && !pte->read) {
        record_fault(iommu, FAULT_READ_PERMISSION, source_id, iova);
        return INVALID_ADDR;
    }

    /* 6. Update IOTLB */
    uint64_t phys_addr = (pte->addr << 12);
    iotlb_insert(iommu, context->domain_id, iova, phys_addr);

    /* 7. Return translated address */
    return phys_addr | (iova & 0xFFF);
}
```

### Interrupt Remapping

```c
/* Interrupt Remapping Table Entry */
typedef struct {
    uint64_t present:1;
    uint64_t mode:1;            /* 0=remapped, 1=posted */
    uint64_t trigger_mode:1;    /* 0=edge, 1=level */
    uint64_t delivery_mode:3;   /* Fixed, LP, SMI, NMI, INIT, ExtINT */
    uint64_t reserved1:2;
    uint64_t destination_id:32; /* Target CPU APIC ID */
    uint64_t vector:8;          /* Interrupt vector */
    uint64_t reserved2:16;

    /* Posted interrupt descriptor (if mode=1) */
    uint64_t posted_desc_addr;
} intr_remap_entry_t;

/* Remap MSI/MSI-X interrupt */
int iommu_remap_interrupt(iommu_t *iommu, uint16_t source_id,
                          uint32_t msi_data, uint64_t msi_addr,
                          uint32_t *new_data, uint64_t *new_addr) {
    /* Extract index from MSI address/data */
    int index = extract_irte_index(msi_addr, msi_data);

    /* Look up remapping table entry */
    intr_remap_entry_t *irte = &iommu->intr_remap_table[index];

    if (!irte->present) {
        record_fault(iommu, FAULT_INTERRUPT_REMAP, source_id, 0);
        return -1;
    }

    /* Construct remapped MSI */
    *new_addr = construct_msi_addr(irte->destination_id);
    *new_data = construct_msi_data(irte->vector, irte->delivery_mode,
                                   irte->trigger_mode);

    return 0;
}
```

### PASID (Process Address Space ID)

```c
/* PASID Support for Shared Virtual Memory (SVM) */
typedef struct {
    uint32_t pasid_max;         /* Maximum PASID value */

    /* Per-PASID page tables */
    struct pasid_entry {
        uint64_t slptr;         /* Second level page table */
        int valid;
        int supervisor;
        uint64_t domain_id;
    } pasid_table[65536];       /* Up to 64K PASIDs */

} iommu_pasid_t;

/* PASID TLP Prefix (PCIe extension) */
typedef struct {
    uint16_t type:4;            /* 0x2 = PASID */
    uint16_t length:8;
    uint16_t reserved:4;
    uint32_t pasid:20;
    uint32_t exe:1;             /* Execute requested */
    uint32_t priv:1;            /* Privileged mode */
    uint32_t reserved2:10;
} pcie_pasid_prefix_t;
```

### Page Request Interface (PRI)

```c
/* Page Request for On-Demand Paging */
typedef struct {
    uint16_t function;          /* Requesting function */
    uint32_t pasid:20;          /* PASID */
    uint32_t prg_index:9;       /* Page request group index */
    uint32_t reserved:2;
    uint32_t response:1;        /* 0=request, 1=response */

    uint64_t page_addr;         /* Requested page address */

    uint32_t read:1;
    uint32_t write:1;
    uint32_t exec:1;
    uint32_t priv:1;
    uint32_t last_in_group:1;
    uint32_t reserved2:27;
} page_request_t;

/* Handle page request */
void iommu_handle_page_request(iommu_t *iommu, page_request_t *req) {
    /* Resolve page fault */
    uint64_t phys_addr = resolve_guest_page_fault(req->page_addr, req->pasid);

    /* Update page table */
    install_page_mapping(iommu, req->pasid, req->page_addr, phys_addr,
                        req->read, req->write, req->exec);

    /* Send response */
    send_page_response(req->function, req->prg_index, phys_addr);
}
```

## 4. SR-IOV (Single Root I/O Virtualization)

### SR-IOV Architecture

SR-IOV allows a single PCIe device to appear as multiple separate physical devices to guest VMs.

```c
/* SR-IOV Capability Structure (PCIe Extended Capability) */
typedef struct {
    uint16_t cap_id;            /* 0x0010 = SR-IOV */
    uint16_t cap_version:4;
    uint16_t next_cap:12;

    /* SR-IOV Capabilities */
    uint32_t capabilities;
#define SRIOV_CAP_VF_MIGRATION  (1 << 0)
#define SRIOV_CAP_ARI           (1 << 1)  /* Alternative Routing-ID */
#define SRIOV_CAP_VF_10BIT_TAG  (1 << 2)

    /* SR-IOV Control */
    uint16_t control;
#define SRIOV_CTRL_VF_ENABLE    (1 << 0)
#define SRIOV_CTRL_VF_MSE       (1 << 1)  /* VF Memory Space Enable */
#define SRIOV_CTRL_ARI_ENABLE   (1 << 2)

    /* SR-IOV Status */
    uint16_t status;
#define SRIOV_STATUS_VF_MIGRATION  (1 << 0)

    /* Initial VFs */
    uint16_t initial_vfs;       /* Initial number of VFs */

    /* Total VFs */
    uint16_t total_vfs;         /* Maximum VFs supported */

    /* Number of VFs */
    uint16_t num_vfs;           /* Current number of VFs */

    /* Function Dependency Link */
    uint8_t function_dep_link;

    uint8_t reserved1;

    /* First VF Offset */
    uint16_t first_vf_offset;   /* RID offset from PF */

    /* VF Stride */
    uint16_t vf_stride;         /* RID stride between VFs */

    uint16_t reserved2;

    /* VF Device ID */
    uint16_t vf_device_id;      /* Device ID for VFs */

    /* Supported Page Sizes */
    uint32_t supported_page_sizes;  /* Bitmap of supported page sizes */

    /* System Page Size */
    uint32_t system_page_size;  /* System page size to use */

    /* VF BARs (6 BARs) */
    uint32_t vf_bar[6];         /* Base Address Registers for VFs */

    /* VF Migration State Array Offset */
    uint32_t vf_migration_offset;
} sriov_capability_t;

/* SR-IOV Device */
typedef struct {
    /* Physical Function (PF) */
    pcie_config_type0_t pf_config;
    sriov_capability_t sriov_cap;

    /* Virtual Functions (VFs) */
    struct virtual_function {
        pcie_config_type0_t config;
        int enabled;
        int assigned_vm;        /* Which VM owns this VF */

        /* VF-specific resources */
        uint64_t bar_addr[6];
        uint32_t bar_size[6];

        /* VF mailbox for PF↔VF communication */
        void *mailbox;
        int mailbox_irq;

        /* Statistics */
        uint64_t tx_packets;
        uint64_t rx_packets;
        uint64_t tx_bytes;
        uint64_t rx_bytes;
    } vfs[256];                 /* Up to 256 VFs */

    int num_vfs_enabled;

    /* Resource pools shared among VFs */
    struct {
        int total_queues;
        int queues_per_vf;
        int total_interrupts;
        int interrupts_per_vf;
    } resources;
} sriov_device_t;
```

### SR-IOV Initialization

```c
/* Enable SR-IOV on a device */
int sriov_enable(sriov_device_t *dev, int num_vfs) {
    sriov_capability_t *cap = &dev->sriov_cap;

    if (num_vfs > cap->total_vfs)
        return -EINVAL;

    /* Set number of VFs */
    cap->num_vfs = num_vfs;

    /* Calculate VF BDF (Bus:Device:Function) */
    uint16_t pf_bdf = get_bdf(&dev->pf_config);
    uint8_t pf_bus = (pf_bdf >> 8) & 0xFF;
    uint8_t pf_devfn = pf_bdf & 0xFF;

    for (int i = 0; i < num_vfs; i++) {
        struct virtual_function *vf = &dev->vfs[i];

        /* Calculate VF routing ID */
        uint16_t vf_offset = cap->first_vf_offset + (i * cap->vf_stride);
        uint16_t vf_bdf = pf_bdf + vf_offset;

        /* Initialize VF config space */
        vf->config.vendor_id = dev->pf_config.vendor_id;
        vf->config.device_id = cap->vf_device_id;
        vf->config.class_code = dev->pf_config.class_code;

        /* Map VF BARs */
        for (int bar = 0; bar < 6; bar++) {
            if (cap->vf_bar[bar]) {
                vf->bar_size[bar] = get_bar_size(cap->vf_bar[bar]);
                vf->bar_addr[bar] = allocate_bar_space(vf->bar_size[bar]);
                vf->config.bar[bar] = vf->bar_addr[bar];
            }
        }

        /* Initialize VF mailbox */
        vf->mailbox = allocate_mailbox();
        vf->mailbox_irq = allocate_irq();

        vf->enabled = 1;
        vf->assigned_vm = -1;  /* Not assigned yet */
    }

    /* Enable VFs in SR-IOV control */
    cap->control |= SRIOV_CTRL_VF_ENABLE | SRIOV_CTRL_VF_MSE;

    dev->num_vfs_enabled = num_vfs;

    return 0;
}
```

### VF Assignment to VM

```c
/* Assign VF to a guest VM */
int sriov_assign_vf_to_vm(sriov_device_t *dev, int vf_index, int vm_id) {
    if (vf_index >= dev->num_vfs_enabled)
        return -EINVAL;

    struct virtual_function *vf = &dev->vfs[vf_index];

    if (vf->assigned_vm != -1)
        return -EBUSY;  /* Already assigned */

    /* Configure IOMMU for this VF */
    uint16_t vf_bdf = calculate_vf_bdf(dev, vf_index);
    iommu_setup_domain(&iommu, vf_bdf, vm_id);

    /* Pass through VF to VM */
    vf->assigned_vm = vm_id;

    /* Inject VF into VM's PCIe bus */
    vm_add_pcie_device(vm_id, vf_bdf, &vf->config);

    return 0;
}
```

### PF↔VF Mailbox Communication

```c
/* Mailbox for PF-VF communication */
typedef struct {
    /* Mailbox registers (mapped in both PF and VF) */
    uint32_t pf_to_vf_msg[16];  /* PF writes, VF reads */
    uint32_t vf_to_pf_msg[16];  /* VF writes, PF reads */

    uint32_t pf_to_vf_status;   /* Status/flags */
    uint32_t vf_to_pf_status;

    uint32_t pf_to_vf_irq;      /* IRQ control */
    uint32_t vf_to_pf_irq;
} sriov_mailbox_t;

/* VF requests something from PF */
void vf_send_request(struct virtual_function *vf, uint32_t opcode, void *data) {
    sriov_mailbox_t *mbox = vf->mailbox;

    /* Write message */
    mbox->vf_to_pf_msg[0] = opcode;
    memcpy(&mbox->vf_to_pf_msg[1], data, 60);  /* 60 bytes of data */

    /* Set status and trigger interrupt to PF */
    mbox->vf_to_pf_status = MB_STATUS_VALID;
    mbox->vf_to_pf_irq = 1;
}

/* PF handles VF request */
void pf_handle_vf_mailbox(sriov_device_t *dev, int vf_index) {
    struct virtual_function *vf = &dev->vfs[vf_index];
    sriov_mailbox_t *mbox = vf->mailbox;

    uint32_t opcode = mbox->vf_to_pf_msg[0];

    switch (opcode) {
    case MB_OP_SET_MAC:
        /* VF requests MAC address change */
        memcpy(vf->mac_addr, &mbox->vf_to_pf_msg[1], 6);
        break;

    case MB_OP_SET_VLAN:
        /* VF requests VLAN configuration */
        vf->vlan_id = mbox->vf_to_pf_msg[1];
        break;

    case MB_OP_GET_STATS:
        /* VF requests statistics */
        mbox->pf_to_vf_msg[0] = vf->tx_packets;
        mbox->pf_to_vf_msg[1] = vf->rx_packets;
        mbox->pf_to_vf_msg[2] = vf->tx_bytes;
        mbox->pf_to_vf_msg[3] = vf->rx_bytes;
        break;

    case MB_OP_RESET:
        /* VF requests reset */
        sriov_reset_vf(dev, vf_index);
        break;
    }

    /* Clear VF interrupt and set PF response */
    mbox->vf_to_pf_irq = 0;
    mbox->pf_to_vf_status = MB_STATUS_VALID;
    mbox->pf_to_vf_irq = 1;  /* Interrupt VF with response */
}
```

### VF Migration

```c
/* Live migration of VF between VMs */
typedef struct {
    /* Migration state */
    uint32_t vf_index;
    uint32_t source_vm;
    uint32_t dest_vm;

    /* VF state snapshot */
    struct {
        pcie_config_type0_t config;
        uint64_t bar_addr[6];
        uint8_t mac_addr[6];
        uint16_t vlan_id;

        /* Device-specific state */
        void *device_state;
        size_t state_size;

        /* DMA state */
        uint32_t tx_queue_head;
        uint32_t tx_queue_tail;
        uint32_t rx_queue_head;
        uint32_t rx_queue_tail;
    } snapshot;
} vf_migration_t;

int sriov_migrate_vf(sriov_device_t *dev, int vf_index,
                    int source_vm, int dest_vm) {
    vf_migration_t mig = {0};
    struct virtual_function *vf = &dev->vfs[vf_index];

    /* 1. Pause VF in source VM */
    vm_pause_device(source_vm, vf);

    /* 2. Snapshot VF state */
    snapshot_vf_state(vf, &mig.snapshot);

    /* 3. Unassign from source VM */
    sriov_unassign_vf(dev, vf_index, source_vm);

    /* 4. Assign to destination VM */
    sriov_assign_vf_to_vm(dev, vf_index, dest_vm);

    /* 5. Restore VF state in destination VM */
    restore_vf_state(vf, &mig.snapshot);

    /* 6. Resume VF in destination VM */
    vm_resume_device(dest_vm, vf);

    return 0;
}
```

### SR-IOV Network Example

```c
/* Example: SR-IOV Network Adapter */
typedef struct {
    sriov_device_t sriov;

    /* PF resources */
    struct {
        uint8_t mac_addr[6];
        int num_tx_queues;
        int num_rx_queues;
        void *tx_ring[64];
        void *rx_ring[64];
    } pf;

    /* VF-specific queues */
    struct {
        int tx_queue_start;     /* First TX queue for this VF */
        int num_tx_queues;      /* Number of TX queues */
        int rx_queue_start;     /* First RX queue for this VF */
        int num_rx_queues;      /* Number of RX queues */

        /* Rate limiting */
        uint64_t tx_rate_limit; /* Mbps */
        uint64_t rx_rate_limit;

        /* QoS */
        int priority;
        int vlan_id;
        int qos_class;
    } vf_queues[256];

} sriov_network_t;

/* VF transmits packet */
void vf_tx_packet(sriov_network_t *nic, int vf_index, void *packet, size_t len) {
    struct virtual_function *vf = &nic->sriov.vfs[vf_index];

    /* Rate limiting */
    if (!check_rate_limit(nic, vf_index))
        return;  /* Drop packet - rate limit exceeded */

    /* VLAN tagging (if configured) */
    if (nic->vf_queues[vf_index].vlan_id) {
        insert_vlan_tag(packet, nic->vf_queues[vf_index].vlan_id);
    }

    /* QoS priority */
    set_packet_priority(packet, nic->vf_queues[vf_index].priority);

    /* Send to wire */
    int queue = nic->vf_queues[vf_index].tx_queue_start;
    tx_ring_insert(nic->pf.tx_ring[queue], packet, len);

    vf->tx_packets++;
    vf->tx_bytes += len;
}
```

## 5. Console and Display Emulation

### SDL1 Backend

```c
/* SDL1 Console */
typedef struct {
    SDL_Surface *screen;
    int width;              /* 640, 800, 1024, etc. */
    int height;             /* 480, 600, 768, etc. */
    int bpp;                /* 8, 16, 24, 32 */
    uint32_t *framebuffer;  /* Pixel data */
    SDL_Color palette[256]; /* For 8-bit mode */
} sdl1_console_t;

void sdl1_init(sdl1_console_t *console, int w, int h, int bpp);
void sdl1_update(sdl1_console_t *console);
void sdl1_handle_events(sdl1_console_t *console);
```

### SDL2 Backend

```c
/* SDL2 Console with Hardware Acceleration */
typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    int width;
    int height;
    uint32_t *framebuffer;
    int vsync_enabled;
} sdl2_console_t;

void sdl2_init(sdl2_console_t *console, int w, int h);
void sdl2_update(sdl2_console_t *console);
void sdl2_handle_events(sdl2_console_t *console);
void sdl2_set_fullscreen(sdl2_console_t *console, int enable);
```

### RFB (Remote Framebuffer) / VNC Server

```c
/* RFB Server for Remote Access */
typedef struct {
    int listen_port;        /* Default 5900 */
    int width;
    int height;
    int bpp;
    uint32_t *framebuffer;

    /* Client connections */
    struct rfb_client {
        int socket;
        char *client_name;
        int encoding;       /* Raw, RRE, CoRRE, Hextile, ZRLE */
        int pixel_format;
    } clients[MAX_RFB_CLIENTS];

    int num_clients;
} rfb_server_t;

void rfb_init(rfb_server_t *server, int port, int w, int h);
void rfb_update(rfb_server_t *server);
void rfb_handle_client_input(rfb_server_t *server);
```

### Text Console (VGA-style)

```c
/* 80x25 or 80x50 text console */
typedef struct {
    uint16_t *text_buffer;  /* Character + attribute */
    int rows;               /* 25, 43, 50 */
    int cols;               /* 80, 132 */
    int cursor_row;
    int cursor_col;
    uint8_t default_attr;   /* Foreground/background color */
} text_console_t;

#define VGA_COLOR(fg, bg) (((bg) << 4) | (fg))
```

## 4. Serial Communications (UART)

### RS422 UART Implementation

```c
/* 16550-compatible UART with RS422 support */
typedef struct {
    /* Data Registers */
    uint8_t rbr;            /* Receive Buffer Register */
    uint8_t thr;            /* Transmit Holding Register */
    uint8_t ier;            /* Interrupt Enable Register */
    uint8_t iir;            /* Interrupt Identification Register */
    uint8_t fcr;            /* FIFO Control Register */
    uint8_t lcr;            /* Line Control Register */
    uint8_t mcr;            /* Modem Control Register */
    uint8_t lsr;            /* Line Status Register */
    uint8_t msr;            /* Modem Status Register */
    uint8_t scr;            /* Scratch Register */

    /* Divisor Latch (when LCR.DLAB=1) */
    uint16_t divisor;       /* Baud rate divisor */

    /* FIFO Buffers */
    uint8_t rx_fifo[16];
    uint8_t tx_fifo[16];
    int rx_head, rx_tail;
    int tx_head, tx_tail;

    /* RS422 Specific */
    int rs422_mode;         /* Differential signaling */
    int termination;        /* Line termination enabled */
} uart_rs422_t;

/* Standard baud rates */
#define BAUD_9600   9600
#define BAUD_19200  19200
#define BAUD_38400  38400
#define BAUD_57600  57600
#define BAUD_115200 115200

void uart_init(uart_rs422_t *uart, int base_addr, int irq);
void uart_write(uart_rs422_t *uart, uint8_t data);
int uart_read(uart_rs422_t *uart);
```

## 5. Network Devices

### VMware vmxnet3 Emulation

```c
/* vmxnet3 Adapter */
typedef struct {
    /* PCI Configuration */
    pcie_config_type0_t pci_config;

    /* Device Registers (BAR0) */
    uint32_t vrrs;          /* VMXNET3 Revision Report and Selection */
    uint32_t uvrs;          /* UPT Version Report and Selection */
    uint32_t dsal;          /* Driver Shared Address Low */
    uint32_t dsah;          /* Driver Shared Address High */
    uint32_t cmd;           /* Command */
    uint32_t macl;          /* MAC Address Low */
    uint32_t mach;          /* MAC Address High */
    uint32_t icr;           /* Interrupt Cause Register */
    uint32_t ecr;           /* Event Cause Register */

    /* Shared Data Structures */
    struct vmxnet3_driver_shared {
        uint32_t magic;
        uint32_t size;
        struct vmxnet3_tx_queue tx_queue[8];
        struct vmxnet3_rx_queue rx_queue[8];
        uint32_t num_tx_queues;
        uint32_t num_rx_queues;
    } *shared;

    /* MAC address */
    uint8_t mac_addr[6];

    /* Statistics */
    uint64_t tx_packets;
    uint64_t rx_packets;
    uint64_t tx_bytes;
    uint64_t rx_bytes;
} vmxnet3_t;

/* vmxnet3 Commands */
#define VMXNET3_CMD_ENABLE          0xCAFE0000
#define VMXNET3_CMD_DISABLE         0xCAFE0001
#define VMXNET3_CMD_RESET           0xCAFE0002
#define VMXNET3_CMD_UPDATE_RX_MODE  0xCAFE0003
#define VMXNET3_CMD_GET_STATS       0xCAFE0004
```

### Standard Ethernet Controller (Intel E1000)

```c
/* Intel E1000 for compatibility */
typedef struct {
    pcie_config_type0_t pci_config;

    /* Registers */
    uint32_t ctrl;          /* Device Control */
    uint32_t status;        /* Device Status */
    uint32_t eecd;          /* EEPROM Control */
    uint32_t eerd;          /* EEPROM Read */
    uint32_t fla;           /* Flash Access */
    uint32_t mdic;          /* MDI Control */

    /* Interrupt Registers */
    uint32_t icr;           /* Interrupt Cause Read */
    uint32_t ics;           /* Interrupt Cause Set */
    uint32_t ims;           /* Interrupt Mask Set */
    uint32_t imc;           /* Interrupt Mask Clear */

    /* Receive Registers */
    uint32_t rctl;          /* Receive Control */
    uint64_t rdbal;         /* RX Descriptor Base Low */
    uint64_t rdbah;         /* RX Descriptor Base High */
    uint32_t rdlen;         /* RX Descriptor Length */
    uint32_t rdh;           /* RX Descriptor Head */
    uint32_t rdt;           /* RX Descriptor Tail */

    /* Transmit Registers */
    uint32_t tctl;          /* Transmit Control */
    uint64_t tdbal;         /* TX Descriptor Base Low */
    uint64_t tdbah;         /* TX Descriptor Base High */
    uint32_t tdlen;         /* TX Descriptor Length */
    uint32_t tdh;           /* TX Descriptor Head */
    uint32_t tdt;           /* TX Descriptor Tail */

    uint8_t mac_addr[6];
} e1000_t;
```

## 6. Graphics Devices

### VMware SVGA II

```c
/* VMware SVGA II Graphics Adapter */
typedef struct {
    pcie_config_type0_t pci_config;

    /* I/O Registers (BAR0) */
    uint32_t id;            /* 0x00 - SVGA_ID */
    uint32_t enable;        /* 0x04 - Enable */
    uint32_t width;         /* 0x08 - Width */
    uint32_t height;        /* 0x0C - Height */
    uint32_t max_width;     /* 0x10 */
    uint32_t max_height;    /* 0x14 */
    uint32_t depth;         /* 0x18 - Bits per pixel */
    uint32_t bpp;           /* 0x1C - Bytes per pixel */
    uint32_t pseudocolor;   /* 0x20 */
    uint32_t red_mask;      /* 0x24 */
    uint32_t green_mask;    /* 0x28 */
    uint32_t blue_mask;     /* 0x2C */
    uint32_t bytes_per_line;/* 0x30 */
    uint32_t fb_start;      /* 0x34 - Framebuffer Start */
    uint32_t fb_offset;     /* 0x38 */
    uint32_t vram_size;     /* 0x3C */
    uint32_t fb_size;       /* 0x40 */

    /* FIFO Registers */
    uint32_t capabilities;  /* 0x44 */
    uint32_t mem_start;     /* 0x48 - FIFO Memory Start */
    uint32_t mem_size;      /* 0x4C - FIFO Memory Size */
    uint32_t config_done;   /* 0x50 */
    uint32_t sync;          /* 0x54 */
    uint32_t busy;          /* 0x58 */
    uint32_t guest_id;      /* 0x5C */
    uint32_t cursor_id;     /* 0x60 */
    uint32_t cursor_x;      /* 0x64 */
    uint32_t cursor_y;      /* 0x68 */
    uint32_t cursor_on;     /* 0x6C */
    uint32_t host_bpp;      /* 0x70 */
    uint32_t scratch_size;  /* 0x74 */

    /* Memory regions */
    uint8_t *framebuffer;   /* BAR1 - Framebuffer */
    uint32_t *fifo;         /* BAR2 - Command FIFO */

    /* Capabilities */
    uint32_t caps;
#define SVGA_CAP_RECT_COPY      (1 << 0)
#define SVGA_CAP_CURSOR         (1 << 1)
#define SVGA_CAP_ALPHA_CURSOR   (1 << 2)
#define SVGA_CAP_3D             (1 << 3)
#define SVGA_CAP_EXTENDED_FIFO  (1 << 4)
#define SVGA_CAP_MULTIMON       (1 << 5)
#define SVGA_CAP_PITCHLOCK      (1 << 6)
} svga2_t;

/* SVGA Commands (FIFO) */
typedef enum {
    SVGA_CMD_UPDATE = 1,
    SVGA_CMD_RECT_COPY,
    SVGA_CMD_DEFINE_CURSOR,
    SVGA_CMD_DEFINE_ALPHA_CURSOR,
    SVGA_CMD_UPDATE_VERBOSE,
    SVGA_CMD_FRONT_ROP_FILL,
    SVGA_CMD_FENCE,
    SVGA_CMD_ESCAPE,
    SVGA_CMD_DEFINE_SCREEN,
    SVGA_CMD_DESTROY_SCREEN,
    SVGA_CMD_DEFINE_GMRFB,
    SVGA_CMD_BLIT_GMRFB_TO_SCREEN,
    SVGA_CMD_BLIT_SCREEN_TO_GMRFB,
} svga_cmd_t;
```

## 7. SMP and NUMA Support

### Multi-Processor Configuration

```c
/* CPU Topology */
typedef struct {
    int num_cpus;           /* Total CPUs */
    int num_sockets;        /* Physical sockets */
    int cores_per_socket;   /* Cores per socket */
    int threads_per_core;   /* SMT threads */

    struct cpu_info {
        int cpu_id;         /* Logical CPU ID */
        int socket_id;      /* Physical socket */
        int core_id;        /* Core within socket */
        int thread_id;      /* Thread within core */
        int aic_id;         /* AIC ID */
        int numa_node;      /* NUMA node */
    } cpus[MAX_CPUS];
} cpu_topology_t;

/* Per-CPU State */
typedef struct {
    dlx_cpu_t cpu;          /* CPU registers */
    int cpu_id;
    int running;
    pthread_t thread;       /* Host thread */

    /* Local AIC */
    struct aic_cpu_regs *aic;

    /* Cache */
    struct {
        void *l1_icache;    /* 32KB */
        void *l1_dcache;    /* 32KB */
        void *l2_cache;     /* 256KB */
    } cache;
} vcpu_t;
```

### NUMA (Non-Uniform Memory Access)

```c
/* NUMA Node Configuration */
typedef struct {
    int node_id;
    uint64_t memory_start;
    uint64_t memory_size;
    int num_cpus;
    int cpu_list[MAX_CPUS_PER_NODE];

    /* Distance matrix */
    int distance[MAX_NUMA_NODES];

    /* Memory statistics */
    uint64_t local_accesses;
    uint64_t remote_accesses;
} numa_node_t;

typedef struct {
    int num_nodes;
    numa_node_t nodes[MAX_NUMA_NODES];
} numa_config_t;

/* Example 4-node NUMA configuration:
 * Node 0: 0-16GB, CPUs 0-3   (distance 10)
 * Node 1: 16-32GB, CPUs 4-7  (distance 21 to node 0)
 * Node 2: 32-48GB, CPUs 8-11 (distance 21 to node 0, 32 to node 1)
 * Node 3: 48-64GB, CPUs 12-15 (distance 21 to node 0, 32 to nodes 1,2)
 */
```

## 8. Advanced Interrupt Controller (Apple AIC-inspired)

### AIC Architecture

```c
/* Apple Interrupt Controller */
typedef struct {
    /* Global Registers */
    uint32_t version;           /* 0x0000 - AIC Version */
    uint32_t config;            /* 0x0004 - Configuration */
    uint32_t whoami;            /* 0x0008 - Current CPU ID */

    /* Interrupt Registers (per-CPU, stride 0x8000) */
    struct aic_cpu_regs {
        uint32_t ipi_set;       /* 0x0000 - IPI Send */
        uint32_t ipi_clr;       /* 0x0004 - IPI Clear */
        uint32_t ipi_mask;      /* 0x0008 - IPI Mask */

        uint32_t event[8];      /* 0x0010-0x002F - Event registers */
        uint32_t mask[8];       /* 0x0030-0x004F - Mask registers */

        uint32_t ack;           /* 0x0050 - Acknowledge */
        uint32_t eoi;           /* 0x0054 - End of Interrupt */
        uint32_t pending;       /* 0x0058 - Pending interrupts */

        uint32_t fiq_enable;    /* 0x0060 - FIQ Enable */
        uint32_t fiq_disable;   /* 0x0064 - FIQ Disable */

        /* Per-interrupt routing */
        struct {
            uint32_t target;    /* Target CPU mask */
            uint32_t priority;  /* Priority 0-15 */
        } irq[256];
    } cpu[MAX_CPUS];

    /* Fast IPI mechanism */
    uint32_t ipi_fast[MAX_CPUS];

    /* Die-to-die interrupts (multi-chip) */
    uint32_t d2d_config;
    uint32_t d2d_status;
} aic_t;

/* AIC Interrupt Types */
#define AIC_IRQ_TYPE_LEVEL  0
#define AIC_IRQ_TYPE_EDGE   1

/* Inter-Processor Interrupts */
#define AIC_IPI_RESCHEDULE  0
#define AIC_IPI_CALL_FUNC   1
#define AIC_IPI_TLB_FLUSH   2
#define AIC_IPI_TIMER       3
```

### IPI (Inter-Processor Interrupt) Implementation

```c
void aic_send_ipi(aic_t *aic, int target_cpu, int ipi_type) {
    aic->cpu[target_cpu].ipi_set |= (1 << ipi_type);
    /* Wake up target CPU if sleeping */
}

void aic_handle_ipi(aic_t *aic, int cpu_id) {
    uint32_t ipis = aic->cpu[cpu_id].ipi_set;

    if (ipis & (1 << AIC_IPI_RESCHEDULE)) {
        /* Reschedule current task */
    }
    if (ipis & (1 << AIC_IPI_CALL_FUNC)) {
        /* Execute cross-CPU function call */
    }
    if (ipis & (1 << AIC_IPI_TLB_FLUSH)) {
        /* Flush TLB */
    }

    aic->cpu[cpu_id].ipi_clr = ipis;
}
```

## 9. USB, FireWire, and USB4

### USB 3.0 xHCI Controller

```c
/* xHCI (eXtensible Host Controller Interface) */
typedef struct {
    pcie_config_type0_t pci_config;

    /* Capability Registers (offset 0x00) */
    uint32_t cap_length:8;
    uint32_t reserved:8;
    uint32_t hci_version:16;
    uint32_t hcs_params1;       /* Structural Parameters 1 */
    uint32_t hcs_params2;
    uint32_t hcs_params3;
    uint32_t hcc_params1;       /* Capability Parameters 1 */
    uint32_t db_offset;         /* Doorbell Offset */
    uint32_t rts_offset;        /* Runtime Register Space Offset */
    uint32_t hcc_params2;

    /* Operational Registers (offset CAP_LENGTH) */
    uint32_t usb_cmd;           /* USB Command */
    uint32_t usb_sts;           /* USB Status */
    uint32_t page_size;
    uint32_t dn_ctrl;           /* Device Notification Control */
    uint64_t cmd_ring_ctrl;     /* Command Ring Control */
    uint64_t dcbaa_ptr;         /* Device Context Base Address Array */
    uint32_t config;            /* Configure */

    /* Port Registers (array, one per port) */
    struct xhci_port {
        uint32_t portsc;        /* Port Status and Control */
        uint32_t portpmsc;      /* Port Power Management */
        uint32_t portli;        /* Port Link Info */
        uint32_t porthlpmc;     /* Port Hardware LPM Control */
    } ports[MAX_USB_PORTS];

    /* Runtime Registers */
    uint32_t mfindex;           /* Microframe Index */
    struct {
        uint32_t iman;          /* Interrupter Management */
        uint32_t imod;          /* Interrupter Moderation */
        uint32_t erstsz;        /* Event Ring Segment Table Size */
        uint64_t erstba;        /* Event Ring Segment Table Base */
        uint64_t erdp;          /* Event Ring Dequeue Pointer */
    } interrupter[MAX_INTERRUPTERS];

    /* Doorbell Array */
    uint32_t doorbell[256];

    /* Connected devices */
    struct usb_device *devices[MAX_USB_DEVICES];
} xhci_t;

/* USB Device Classes */
#define USB_CLASS_HID           0x03    /* Keyboard, Mouse */
#define USB_CLASS_MASS_STORAGE  0x08    /* USB drives */
#define USB_CLASS_HUB           0x09
#define USB_CLASS_CDC           0x0A    /* Communications */
#define USB_CLASS_VIDEO         0x0E
#define USB_CLASS_AUDIO         0x01
```

### FireWire (IEEE 1394) OHCI Controller

```c
/* OHCI-1394 (Open Host Controller Interface) */
typedef struct {
    pcie_config_type0_t pci_config;

    /* OHCI Registers */
    uint32_t version;
    uint32_t guid_rom;          /* GUID ROM */
    uint32_t at_retries;        /* Async Transmit Retry */
    uint32_t csr_data;          /* CSR Data */
    uint32_t csr_compare_data;
    uint32_t csr_control;
    uint32_t config_rom_hdr;
    uint32_t bus_id;
    uint32_t bus_options;
    uint32_t guid_hi;           /* GUID High */
    uint32_t guid_lo;           /* GUID Low */

    /* Async TX/RX */
    uint32_t at_req_context_ctrl;
    uint32_t at_req_cmd_ptr;
    uint32_t at_resp_context_ctrl;
    uint32_t at_resp_cmd_ptr;
    uint32_t ar_req_context_ctrl;
    uint32_t ar_req_cmd_ptr;
    uint32_t ar_resp_context_ctrl;
    uint32_t ar_resp_cmd_ptr;

    /* Isochronous */
    uint32_t iso_tx_context_ctrl[32];
    uint32_t iso_rx_context_ctrl[32];

    /* Interrupt registers */
    uint32_t int_event;
    uint32_t int_mask;

    /* Node ID */
    uint16_t node_id;
    uint8_t  phy_id;

    /* Speed (100/200/400/800 Mbps) */
    int speed;
} ohci1394_t;
```

### USB4 Controller

```c
/* USB4 (Thunderbolt 3/4 compatible) */
typedef struct {
    pcie_config_type0_t pci_config;

    /* USB4 Router */
    struct {
        uint32_t router_ops;
        uint32_t route_string;  /* Topology routing */
        uint32_t uuid[4];       /* 128-bit UUID */

        /* Adapters */
        struct usb4_adapter {
            int type;           /* USB3, DP, PCIe, etc. */
            int num;
            uint32_t caps;
            uint64_t bandwidth; /* Gbps */
        } adapters[MAX_USB4_ADAPTERS];
    } router;

    /* Tunneling */
    struct {
        int protocol;           /* USB3, DP, PCIe */
        uint64_t allocated_bw;
        int active;
    } tunnels[MAX_USB4_TUNNELS];

    /* DisplayPort Alt Mode */
    struct {
        int num_lanes;          /* 1, 2, or 4 */
        int max_link_rate;      /* HBR, HBR2, HBR3 */
        int active;
    } dp_alt_mode;

    /* Power delivery */
    int pd_version;             /* USB-PD 3.0 */
    int max_power;              /* Watts */
} usb4_t;
```

## 10. Storage Controllers

### LSI Logic SAS/SCSI Controller

```c
/* LSI Logic SAS2008 (MPT Fusion) */
typedef struct {
    pcie_config_type0_t pci_config;

    /* Message Passing Interface */
    uint32_t doorbell;          /* 0x00 - Doorbell */
    uint32_t write_seq;         /* 0x04 - Write Sequence */
    uint32_t host_diag;         /* 0x08 - Host Diagnostic */
    uint32_t test_base;         /* 0x0C - Test Base */
    uint32_t diag_rw_data;      /* 0x10 - Diagnostic R/W Data */
    uint32_t diag_rw_addr;      /* 0x14 - Diagnostic R/W Address */

    /* Request/Reply Queues */
    uint64_t req_queue_base;
    uint64_t reply_queue_base;
    uint32_t req_queue_depth;
    uint32_t reply_queue_depth;
    uint16_t req_queue_idx;
    uint16_t reply_queue_idx;

    /* I/O Controller (IOC) */
    struct {
        uint16_t who_init;
        uint16_t state;
        uint32_t capabilities;
        uint8_t  max_devices;
        uint8_t  max_buses;
    } ioc;

    /* SAS Topology */
    struct sas_device {
        uint64_t sas_address;
        uint16_t dev_handle;
        uint8_t  phy_num;
        uint8_t  link_rate;     /* 1.5, 3, 6, 12 Gbps */
        int attached;
    } devices[MAX_SAS_DEVICES];

    /* SCSI Emulation */
    struct scsi_device *scsi_devices[16][64]; /* Bus:Target */
} lsi_sas_t;

/* MPT Message Frames */
typedef struct {
    uint8_t  function;
    uint8_t  bus;
    uint8_t  target_id;
    uint8_t  lun;
    uint32_t flags;
    uint32_t data_length;
    uint64_t data_ptr;
    uint8_t  cdb[16];           /* SCSI CDB */
} mpt_scsi_io_request_t;
```

### Intel AHCI (Advanced Host Controller Interface)

```c
/* AHCI SATA Controller */
typedef struct {
    pcie_config_type0_t pci_config;

    /* Generic Host Control */
    uint32_t cap;               /* 0x00 - Host Capabilities */
    uint32_t ghc;               /* 0x04 - Global Host Control */
    uint32_t is;                /* 0x08 - Interrupt Status */
    uint32_t pi;                /* 0x0C - Ports Implemented */
    uint32_t vs;                /* 0x10 - Version */
    uint32_t ccc_ctl;           /* 0x14 - Command Completion Coalescing */
    uint32_t ccc_ports;         /* 0x18 - CCC Ports */
    uint32_t em_loc;            /* 0x1C - Enclosure Management Location */
    uint32_t em_ctl;            /* 0x20 - Enclosure Management Control */
    uint32_t cap2;              /* 0x24 - Host Capabilities Extended */
    uint32_t bohc;              /* 0x28 - BIOS/OS Handoff Control */

    /* Port Registers (offset 0x100 + port * 0x80) */
    struct ahci_port {
        uint64_t clb;           /* Command List Base Address */
        uint64_t fb;            /* FIS Base Address */
        uint32_t is;            /* Interrupt Status */
        uint32_t ie;            /* Interrupt Enable */
        uint32_t cmd;           /* Command and Status */
        uint32_t tfd;           /* Task File Data */
        uint32_t sig;           /* Signature */
        uint32_t ssts;          /* Serial ATA Status */
        uint32_t sctl;          /* Serial ATA Control */
        uint32_t serr;          /* Serial ATA Error */
        uint32_t sact;          /* Serial ATA Active */
        uint32_t ci;            /* Command Issue */
        uint32_t sntf;          /* Serial ATA Notification */
        uint32_t fbs;           /* FIS-based Switching Control */

        /* Attached device */
        struct sata_device *device;
    } ports[32];
} ahci_t;

/* AHCI Command List Entry */
typedef struct {
    uint16_t flags;
    uint16_t prd_table_len;     /* Physical Region Descriptor count */
    uint32_t prd_byte_count;
    uint64_t command_table_base;
} ahci_cmd_header_t;

/* AHCI Command Table */
typedef struct {
    uint8_t  cfis[64];          /* Command FIS */
    uint8_t  acmd[16];          /* ATAPI Command */
    uint8_t  reserved[48];
    struct {
        uint64_t dba;           /* Data Base Address */
        uint32_t reserved;
        uint32_t dbc:22;        /* Data Byte Count */
        uint32_t reserved2:9;
        uint32_t i:1;           /* Interrupt on completion */
    } prdt[];                   /* Physical Region Descriptor Table */
} ahci_cmd_table_t;
```

### NVMe (Non-Volatile Memory Express)

```c
/* NVMe Controller */
typedef struct {
    pcie_config_type0_t pci_config;

    /* Controller Registers (BAR0) */
    uint64_t cap;               /* 0x00 - Controller Capabilities */
    uint32_t vs;                /* 0x08 - Version */
    uint32_t intms;             /* 0x0C - Interrupt Mask Set */
    uint32_t intmc;             /* 0x10 - Interrupt Mask Clear */
    uint32_t cc;                /* 0x14 - Controller Configuration */
    uint32_t csts;              /* 0x1C - Controller Status */
    uint32_t nssr;              /* 0x20 - NVM Subsystem Reset */
    uint32_t aqa;               /* 0x24 - Admin Queue Attributes */
    uint64_t asq;               /* 0x28 - Admin Submission Queue */
    uint64_t acq;               /* 0x30 - Admin Completion Queue */
    uint32_t cmbloc;            /* 0x38 - Controller Memory Buffer Location */
    uint32_t cmbsz;             /* 0x3C - Controller Memory Buffer Size */

    /* Doorbell Registers (offset 0x1000 + queue * 8) */
    uint32_t *sq_doorbell;      /* Submission Queue Tail Doorbell */
    uint32_t *cq_doorbell;      /* Completion Queue Head Doorbell */

    /* Queues */
    struct nvme_queue {
        uint64_t base_addr;
        uint32_t size;
        uint16_t head;
        uint16_t tail;
        uint16_t phase;
    } sq[MAX_NVME_QUEUES];      /* Submission Queues */

    struct nvme_queue cq[MAX_NVME_QUEUES]; /* Completion Queues */

    /* Namespaces */
    struct nvme_namespace {
        uint32_t nsid;
        uint64_t size;          /* In blocks */
        uint32_t block_size;
        uint8_t  *data;         /* Backend storage */
    } namespaces[MAX_NVME_NAMESPACES];

    int num_namespaces;
} nvme_t;

/* NVMe Command Structure */
typedef struct {
    uint8_t  opcode;
    uint8_t  flags;
    uint16_t cid;               /* Command Identifier */
    uint32_t nsid;              /* Namespace ID */
    uint64_t reserved;
    uint64_t mptr;              /* Metadata Pointer */
    uint64_t prp1;              /* Physical Region Page 1 */
    uint64_t prp2;              /* Physical Region Page 2 */
    uint32_t cdw10;             /* Command Dword 10-15 */
    uint32_t cdw11;
    uint32_t cdw12;
    uint32_t cdw13;
    uint32_t cdw14;
    uint32_t cdw15;
} nvme_cmd_t;

/* NVMe Admin Commands */
#define NVME_ADMIN_DELETE_SQ    0x00
#define NVME_ADMIN_CREATE_SQ    0x01
#define NVME_ADMIN_DELETE_CQ    0x04
#define NVME_ADMIN_CREATE_CQ    0x05
#define NVME_ADMIN_IDENTIFY     0x06
#define NVME_ADMIN_ABORT        0x08
#define NVME_ADMIN_SET_FEATURES 0x09
#define NVME_ADMIN_GET_FEATURES 0x0A

/* NVMe I/O Commands */
#define NVME_CMD_FLUSH          0x00
#define NVME_CMD_WRITE          0x01
#define NVME_CMD_READ           0x02
#define NVME_CMD_WRITE_ZEROS    0x08
#define NVME_CMD_COMPARE        0x05
```

## 11. Storage Media Emulation

### Unified Storage Backend

```c
/* Storage Media Types */
typedef enum {
    MEDIA_HDD,              /* Hard Disk Drive */
    MEDIA_SSD,              /* Solid State Drive */
    MEDIA_CDROM,            /* CD-ROM */
    MEDIA_DVD,              /* DVD */
    MEDIA_BLURAY,           /* Blu-ray */
    MEDIA_TAPE,             /* Tape (LTO, DAT) */
    MEDIA_WORM,             /* Write-Once Read-Many */
} media_type_t;

/* Storage Device */
typedef struct {
    media_type_t type;
    char *filename;         /* Backend file */
    FILE *fp;

    /* Geometry */
    uint64_t total_sectors;
    uint32_t sector_size;   /* 512, 2048, 4096 */

    /* For HDD: CHS geometry */
    struct {
        uint32_t cylinders;
        uint32_t heads;
        uint32_t sectors;
    } chs;

    /* Capabilities */
    int read_only;
    int removable;
    int ejected;

    /* Performance characteristics */
    int seek_time_ms;       /* HDD: 8-12ms, SSD: 0 */
    int rotation_rpm;       /* HDD: 5400/7200/10000/15000, SSD: 0 */
    int transfer_rate_mbps; /* Sequential read/write */

    /* Statistics */
    uint64_t reads;
    uint64_t writes;
    uint64_t bytes_read;
    uint64_t bytes_written;
} storage_device_t;
```

### CD/DVD/Blu-ray Emulation

```c
/* Optical Disc Drive */
typedef struct {
    storage_device_t base;

    /* Disc type */
    enum {
        DISC_NONE,
        DISC_CD,            /* 650-700 MB */
        DISC_DVD,           /* 4.7-8.5 GB */
        DISC_BLURAY,        /* 25-100 GB */
    } disc_type;

    /* CD-specific */
    struct {
        int num_tracks;
        struct {
            int type;       /* Audio or Data */
            uint32_t start_lba;
            uint32_t length;
        } tracks[99];
        uint8_t toc[804];   /* Table of Contents */
    } cd;

    /* DVD-specific */
    struct {
        int dual_layer;
        int layer0_size;
        int layer1_size;
    } dvd;

    /* Blu-ray specific */
    struct {
        int num_layers;     /* 1, 2, 3, 4 */
        int bdxl;           /* Extended capacity */
    } bluray;

    /* Drive capabilities */
    int can_write;          /* CD-R, DVD-R, BD-R */
    int can_rewrite;        /* CD-RW, DVD-RW, BD-RE */
    int read_speed;         /* 1x = 150 KB/s (CD) */
    int write_speed;
} optical_drive_t;
```

### Tape Drive Emulation

```c
/* Tape Drive (LTO, DAT) */
typedef struct {
    storage_device_t base;

    /* Tape type */
    enum {
        TAPE_LTO1,          /* 100 GB */
        TAPE_LTO2,          /* 200 GB */
        TAPE_LTO3,          /* 400 GB */
        TAPE_LTO4,          /* 800 GB */
        TAPE_LTO5,          /* 1.5 TB */
        TAPE_LTO6,          /* 2.5 TB */
        TAPE_LTO7,          /* 6 TB */
        TAPE_LTO8,          /* 12 TB */
        TAPE_LTO9,          /* 18 TB */
        TAPE_DAT,
        TAPE_DLT,
    } tape_type;

    /* Tape state */
    uint64_t position;      /* Current position in bytes */
    int beginning_of_tape;
    int end_of_tape;
    int file_mark;

    /* Compression */
    int compression_enabled;
    int compression_ratio;  /* e.g., 2 for 2:1 */

    /* SCSI Streaming Commands */
    int density_code;
    int block_size;         /* 0 = variable */
} tape_drive_t;
```

### Fibre Channel HBA

```c
/* Fibre Channel Host Bus Adapter */
typedef struct {
    pcie_config_type0_t pci_config;

    /* FC Ports */
    struct fc_port {
        uint64_t wwnn;      /* World Wide Node Name */
        uint64_t wwpn;      /* World Wide Port Name */

        /* Port state */
        enum {
            FC_PORT_OFFLINE,
            FC_PORT_LINKDOWN,
            FC_PORT_LINKUP,
            FC_PORT_ONLINE,
        } state;

        /* Speed */
        int speed_gbps;     /* 1, 2, 4, 8, 16, 32 */

        /* Topology */
        enum {
            FC_TOPOLOGY_UNKNOWN,
            FC_TOPOLOGY_PTP,    /* Point-to-Point */
            FC_TOPOLOGY_LOOP,   /* Arbitrated Loop */
            FC_TOPOLOGY_FABRIC, /* Fabric (switch) */
        } topology;

        /* Connected devices */
        struct fc_remote_port {
            uint64_t wwnn;
            uint64_t wwpn;
            uint32_t port_id;
            int scsi_id;
        } remote_ports[MAX_FC_REMOTE_PORTS];

        int num_remote_ports;
    } ports[MAX_FC_PORTS];

    /* Exchange and Sequence management */
    struct fc_exchange {
        uint16_t ox_id;     /* Originator Exchange ID */
        uint16_t rx_id;     /* Responder Exchange ID */
        int active;
    } exchanges[MAX_FC_EXCHANGES];

    /* Frame buffers */
    uint8_t *send_buffer;
    uint8_t *recv_buffer;
} fc_hba_t;

/* FC Frame Header */
typedef struct {
    uint32_t r_ctl:8;       /* Routing Control */
    uint32_t d_id:24;       /* Destination ID */
    uint32_t cs_ctl:8;      /* Class Specific Control */
    uint32_t s_id:24;       /* Source ID */
    uint32_t type:8;        /* Type */
    uint32_t f_ctl:24;      /* Frame Control */
    uint8_t  seq_id;        /* Sequence ID */
    uint8_t  df_ctl;        /* Data Field Control */
    uint16_t seq_cnt;       /* Sequence Count */
    uint16_t ox_id;         /* Originator Exchange ID */
    uint16_t rx_id;         /* Responder Exchange ID */
    uint32_t parameter;     /* Relative Offset or Parameter */
} fc_frame_header_t;
```

## 12. Implementation Plan

### Phase 1: Core Infrastructure
1. **PCIe Bus Framework**
   - Configuration space access
   - Memory-mapped I/O
   - MSI/MSI-X interrupts
   - Device enumeration

2. **Basic Device Framework**
   - Device registration
   - BAR mapping
   - Interrupt routing
   - DMA engine

### Phase 2: Essential I/O
1. **Console** (SDL2 + RFB)
   - Framebuffer emulation
   - Keyboard/mouse input
   - VNC server for remote access

2. **Storage** (NVMe + AHCI)
   - NVMe controller and queues
   - AHCI SATA controller
   - Disk image backend (raw, qcow2)

3. **Network** (VirtIO-Net)
   - VirtIO framework
   - Network device emulation
   - TAP/TUN backend

### Phase 3: Advanced Features
1. **SMP Support**
   - Multi-CPU execution
   - AIC per-CPU interrupt handling
   - IPI mechanism

2. **VirtIO Devices**
   - VirtIO-Block
   - VirtIO-GPU
   - VirtIO-SCSI

3. **USB Controller**
   - xHCI implementation
   - USB device emulation
   - USB mass storage

### Phase 4: Enterprise Features
1. **Storage Controllers**
   - LSI Logic SAS/SCSI
   - NVMe namespace management
   - Optical drive emulation

2. **Networking**
   - VMware vmxnet3
   - Intel E1000
   - Multiple NICs

3. **Graphics**
   - VMware SVGA II
   - VirtIO-GPU 3D
   - Multi-monitor support

### Phase 5: High-End Features
1. **NUMA**
   - NUMA topology
   - Distance matrix
   - Memory affinity

2. **Advanced I/O**
   - FireWire OHCI
   - USB4/Thunderbolt
   - Fibre Channel HBA

3. **Specialized Storage**
   - Tape drives
   - Optical media (CD/DVD/Blu-ray)
   - WORM devices

## 13. Configuration Example

```yaml
# dlxsim.yaml - VM Configuration
vm:
  name: "Darwin DLX VM"
  cpus: 4
  memory: 4096  # MB

  numa:
    nodes: 2
    config:
      - node: 0
        memory: 2048
        cpus: [0, 1]
      - node: 1
        memory: 2048
        cpus: [2, 3]

devices:
  - type: nvme
    namespaces:
      - file: disk0.img
        size: 20G

  - type: ahci
    ports:
      - file: disk1.img
        size: 10G
      - file: cdrom.iso
        media: cdrom
        readonly: true

  - type: virtio-net
    mac: "52:54:00:12:34:56"
    backend: tap0

  - type: vmxnet3
    mac: "00:0c:29:ab:cd:ef"
    backend: vmnet

  - type: svga2
    vram: 128M
    console: sdl2

  - type: xhci
    ports: 4
    devices:
      - type: keyboard
      - type: mouse
      - type: storage
        file: usb-disk.img

  - type: uart
    port: 0x3F8
    irq: 4
    backend: stdio

  - type: rfb
    port: 5900
    password: "secret"

pcie:
  root_complex:
    buses: 256
    devices_per_bus: 32
    functions_per_device: 8
```

## 14. Build System Integration

```makefile
# Makefile additions for I/O emulation

CFLAGS += -DUSE_SDL2 -DUSE_LIBVNC
LDFLAGS += -lSDL2 -lvncserver -lpthread

IO_OBJS = \
    pcie.o \
    virtio.o \
    virtio_net.o \
    virtio_blk.o \
    virtio_gpu.o \
    nvme.o \
    ahci.o \
    lsi_sas.o \
    vmxnet3.o \
    e1000.o \
    svga2.o \
    xhci.o \
    ohci1394.o \
    usb4.o \
    fc_hba.o \
    storage.o \
    optical.o \
    tape.o \
    console_sdl.o \
    console_rfb.o \
    uart.o \
    smp.o \
    numa.o \
    aic.o

dlxsim: $(OBJS) $(IO_OBJS)
    $(CC) -o $@ $^ $(LDFLAGS)
```

---

**Document Status**: Specification Complete
**Implementation Status**: Not Started
**Target**: DLXSIM 3.0
**Complexity**: High - transforms simulator into full VM
**Estimated LOC**: ~50,000 lines
