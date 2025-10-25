# Chapter 24: Single-Level Storage (S Extension)

## 24.1 Persistent Memory Operations

```assembly
# Persistent memory stores
pmemst.b    rs, offset(rd)      # Persistent store byte
pmemst.w    rs, offset(rd)      # Persistent store word
pmemst.d    rs, offset(rd)      # Persistent store doubleword

# Persistent memory fence
pmemfence                       # Ensure persistence

# Persistent memory flush
pmemflush   addr, size          # Flush range to persistent media
```

## 24.2 Object Addressing

```assembly
# Direct object references (128-bit addresses)
ldobj       rd, oid             # Load object by OID
stobj       rs, oid             # Store object by OID
```
