# VOLUME III: ADVANCED FEATURES

# Chapter 15: CHERI Capabilities (X Extension)

## 15.1 Capability Inspection

```assembly
cgetbase    rd, cs              # Get capability base address
cgetlen     rd, cs              # Get capability length
cgetperm    rd, cs              # Get capability permissions
cgettype    rd, cs              # Get capability type
cgettag     rd, cs              # Get capability tag (valid bit)
cgetsealed  rd, cs              # Get sealed status
cgetoffset  rd, cs              # Get capability offset
```

## 15.2 Capability Modification

```assembly
csetaddr    cd, cs, rs          # Set capability address
cincoffset  cd, cs, rs          # Increment capability offset
csetbounds  cd, cs, rs          # Set capability bounds
candperm    cd, cs, rs          # AND permissions (restrict)
cseal       cd, cs, ct          # Seal capability with type
cunseal     cd, cs, ct          # Unseal capability
```

## 15.3 Capability Load/Store

```assembly
clb         rd, offset(cs)      # Capability load byte
clh         rd, offset(cs)      # Capability load halfword
clw         rd, offset(cs)      # Capability load word
cld         rd, offset(cs)      # Capability load doubleword
clc         cd, offset(cs)      # Capability load capability

csb         rs, offset(cd)      # Capability store byte
csh         rs, offset(cd)      # Capability store halfword
csw         rs, offset(cd)      # Capability store word
csd         rs, offset(cd)      # Capability store doubleword
csc         cs, offset(cd)      # Capability store capability
```

## 15.4 Capability Control Flow

```assembly
cjalr       cd, cs              # Capability jump and link register
ccall       cs, selector        # Capability call (domain crossing)
creturn                         # Capability return
```
