# Chapter 16: Secure Enclaves (E Extension)

## 16.1 Enclave Lifecycle

```assembly
ecreate     rd, code_ptr, code_size, data_ptr, data_size
            # Create secure enclave: rd ← enclave_id

eenter      enclave_id, entry_offset, params
            # Enter enclave (world switch to secure)

eexit       result_ptr
            # Exit enclave (world switch to normal)

edestroy    enclave_id
            # Destroy enclave and free resources
```

## 16.2 Enclave Memory Management

```assembly
eadd        enclave_id, vaddr, page_type, prot
            # Add page to enclave

eremove     enclave_id, vaddr
            # Remove page from enclave

eextend     enclave_id, vaddr
            # Extend enclave measurement with page hash

eblock      enclave_id, vaddr
            # Block page (make inaccessible)

etrack      enclave_id
            # Track TLB flushes for enclave pages
```

## 16.3 Attestation and Sealing

```assembly
eattest     rd, enclave_id, user_data, user_data_len
            # Generate attestation report: rd ← report_ptr

eseal       rd_sealed, data_ptr, data_len, enclave_id
            # Seal data to enclave: rd_sealed ← sealed_blob_ptr

eunseal     rd_data, sealed_ptr, sealed_len, enclave_id
            # Unseal data (only works in same enclave)
```
