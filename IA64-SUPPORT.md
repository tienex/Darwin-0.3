# Darwin-0.3 IA64 (Intel Itanium) Architecture Support

**Date**: 2026-06-10
**Branch**: `claude/add-mmix-support-011CUT198Ha67uTFbXZqwn2A`
**Status**: ✅ Architecture plumbing complete

---

## Overview

This adds IA-64 (Intel Itanium) as a recognized 64-bit architecture across
Darwin-0.3, following the same integration pattern established for MMIX.
IA-64 binaries use the 64-bit Mach-O format (`MH_MAGIC_64`, `LC_SEGMENT_64`,
`nlist_64`) whose generic support was implemented previously across the
toolchain, kernel and libraries.

### Architecture Identity

| Property | Value |
|----------|-------|
| CPU type | `CPU_TYPE_IA64` = `(20 \| CPU_ARCH_ABI64)` = `0x01000014` |
| Subtypes | `IA64_ALL` (0), `IA64_ITANIUM` (1), `IA64_ITANIUM2` (2) |
| Byte order | Little-endian |
| ABI | 64-bit (`CPU_ARCH_ABI64`) |
| Stack | Memory stack grows down; register stack (RSE) grows up |
| Default segment alignment | 16K (`0x4000`) |
| arch flags | `-arch ia64`, `-arch itanium`, `-arch itanium2` |

---

## Files Changed

### CPU Type Definitions
- `cctools-2/include/mach/machine.h` — `CPU_TYPE_IA64`, three subtypes
- `kernel-7/mach/machine.h` — same definitions for the kernel

### Architecture Tables
- `cctools-2/libstuff/arch.c`
  - `arch_flags[]`: `ia64`, `itanium`, `itanium2` entries
  - `get_byte_sex_from_flag()`: IA64 → little-endian
  - `get_stack_direction_from_flag()`: IA64 → grows down
  - `get_stack_addr_from_flag()`: IA64 → `0x8000000000000000`
  - `get_segalign_from_flag()`: IA64 → 16K
- `cctools-2/libmacho/arch.c` — `NXArchInfo` entries for all three subtypes

### New Architecture Headers
- `cctools-2/include/mach-o/ia64/reloc.h` — relocation types
  (`IA64_RELOC_VANILLA/PAIR/IMM14/IMM22/IMM64/PCREL21B/PCREL60B/`
  `GPREL22/LTOFF22/SECTDIFF/LOCAL_SECTDIFF`)
- `cctools-2/include/mach/ia64/thread_status.h` — thread, exception and
  float state (`ia64_thread_state_t` with ip/psr/cfm, static GRs r0–r31
  + NaT bits, branch registers, predicates, and the application
  registers needed to resume a thread including RSE state)
- `cctools-2/include/mach-o/ia64/swap.h` — byte-swap declarations
- `cctools-2/libmacho/ia64_swap.c` — byte-swap implementation
- `architecture-1/ia64/byte_order.h` — NXSwap* inlines
- `architecture-1/ia64/alignment.h` — natural-alignment accessors
- `architecture-1/ia64/cpu.h` — register file constants
  (128 GRs/FRs, 64 predicates, 8 BRs, fixed-role registers, AR
  indices, bundle geometry)

### Integration
- `architecture-1/byte_order.h`, `architecture-1/alignment.h` —
  `__ia64__` dispatch branches
- `cctools-2/libmacho/Makefile` — builds `ia64_swap.c` (and the
  previously missing `mmix_swap.c`)
- `cctools-2/file/magdir/mach` — `ia64` (0x01000014) recognized in all
  four Mach-O magic blocks (32/64-bit × little/big-endian)

---

## What Works Now

- `-arch ia64` (also `itanium`, `itanium2`) is accepted by every tool
  that uses libstuff/libmacho arch parsing (ld, lipo, otool, nm, …)
- `file` identifies IA64 Mach-O binaries: `Mach-O 64-bit executable ia64`
- Fat (universal) binaries can carry an IA64 slice (`lipo -arch ia64`)
- The kernel recognizes `CPU_TYPE_IA64` and, via the generic
  64-bit Mach-O loader, can load IA64 `MH_MAGIC_64` images
- Thread state is defined and byte-swappable for cross-tools

## Not Included (Future Work)

Full code generation requires architecture backends comparable to the
MMIX ones — these are substantial, separate efforts:

1. **Assembler backend** (`as/ia64.c`) — EPIC bundle packing,
   templates, predication, ~all instruction formats
2. **Linker relocator** (`ld/ia64_reloc.c`) — apply the relocation
   types defined in `mach-o/ia64/reloc.h`
3. **Disassembler** (`otool/ia64_disasm.c`) — bundle/slot decoding
4. **Kernel pmap/trap support** — actual execution on Itanium hardware

The plumbing added here is everything those backends plug into.

---

Generated with [Claude Code](https://claude.com/claude-code)

Co-Authored-By: Claude <noreply@anthropic.com>
