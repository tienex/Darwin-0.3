# Chapter 30: Moxie Compatibility Mode

DLX can execute Moxie binaries natively through hardware translation:

## 30.1 Moxie Mode Control

```assembly
moxie.enable                    # Enable Moxie compatibility mode
moxie.disable                   # Disable Moxie mode
```

## 30.2 Moxie Register Mapping

Moxie's 16 registers map to DLX r0-r15:
- Moxie $r0 → DLX r0
- Moxie $sp → DLX r1 (sp)
- Moxie $fp → DLX r8 (s0/fp)

## 30.3 Moxie64 Support

128-bit Moxie with 32 registers:
- Direct mapping to DLX128 registers
- 64-bit addressing
- Hardware instruction translation
