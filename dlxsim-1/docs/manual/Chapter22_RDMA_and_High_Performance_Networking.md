# VOLUME IV: SYSTEM ARCHITECTURE

# Chapter 22: RDMA and High-Performance Networking (P Extension)

## 22.1 RDMA Operations

```assembly
# RDMA Send/Receive
rdma.send       qp_id, addr, len, lkey
                # Post send work request

rdma.recv       qp_id, addr, len, lkey
                # Post receive work request

# RDMA Read/Write (one-sided)
rdma.read       local_addr, remote_addr, len, rkey
                # RDMA read from remote memory

rdma.write      remote_addr, local_addr, len, rkey
                # RDMA write to remote memory

rdma.write.imm  remote_addr, local_addr, len, rkey, imm_data
                # RDMA write with immediate data

# RDMA Atomic
rdma.cmp.swap   rd, remote_addr, compare, swap, rkey
                # RDMA atomic compare-and-swap

rdma.fetch.add  rd, remote_addr, add_value, rkey
                # RDMA atomic fetch-and-add
```

## 22.2 InfiniBand Verbs

```assembly
# Queue Pair Management
ib.create.qp    rd, qp_attr     # Create queue pair: rd ← qp_id
ib.modify.qp    qp_id, qp_attr  # Modify queue pair state
ib.destroy.qp   qp_id           # Destroy queue pair

# Completion Queue
ib.poll.cq      rd, cq_id, max_entries  # Poll completion queue
ib.create.cq    rd, cq_entries  # Create completion queue
```
