# Chapter 17: Nested Virtualization (H Extension)

## 17.1 VMX Operations

```assembly
vmxon       addr                # Enable VMX operation
vmxoff                          # Disable VMX operation

vmclear     vmcs_addr           # Clear VMCS state
vmptrld     vmcs_addr           # Load VMCS pointer
vmptrst     mem_addr            # Store VMCS pointer

vmread      rd, field           # Read VMCS field
vmwrite     field, rs           # Write VMCS field

vmlaunch                        # Launch VM (initial entry)
vmresume                        # Resume VM (re-entry after VM exit)

vmcall                          # Hypercall from guest to hypervisor
```

## 17.2 EPT (Extended Page Tables)

```assembly
invept      type, descriptor    # Invalidate EPT entries
invvpid     type, descriptor    # Invalidate VPID entries
```

EPT provides two-dimensional paging:
- **First level**: Guest virtual → Guest physical (guest page tables)
- **Second level**: Guest physical → Host physical (EPT)
