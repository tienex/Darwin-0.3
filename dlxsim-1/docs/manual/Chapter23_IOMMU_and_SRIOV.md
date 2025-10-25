# Chapter 23: IOMMU and SR-IOV (I Extension)

## 23.1 IOMMU Operations

```assembly
# IOMMU Configuration
iommu.map       iova, pa, size, prot    # Map IOVA → Physical Address
iommu.unmap     iova, size              # Unmap IOVA
iommu.flush     domain_id               # Flush IOTLB
```

## 23.2 SR-IOV (Single Root I/O Virtualization)

```assembly
# Virtual Function Management
sriov.enable    pf_id, num_vfs  # Enable SR-IOV with N virtual functions
sriov.disable   pf_id           # Disable SR-IOV
sriov.assign    vf_id, vm_id    # Assign VF to VM
```
