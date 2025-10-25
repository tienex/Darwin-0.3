/*
 * Linker script for Darwin IA64 EFI bootloader
 *
 * EFI applications use PE/COFF format with specific section layout
 */

OUTPUT_FORMAT("elf64-ia64-little", "elf64-ia64-little", "elf64-ia64-little")
OUTPUT_ARCH(ia64)
ENTRY(_start)

SECTIONS
{
  . = 0;
  ImageBase = .;

  /* PE/COFF header is placed here by objcopy */

  . = ALIGN(4096);
  .text : {
    _text = .;
    *(.text)
    *(.text.*)
    *(.gnu.linkonce.t.*)
    . = ALIGN(16);
    _etext = .;
  }

  . = ALIGN(4096);
  .sdata : {
    _sdata = .;
    *(.sdata)
    *(.sdata.*)
    *(.gnu.linkonce.s.*)
    _esdata = .;
  }

  . = ALIGN(4096);
  .data : {
    _data = .;
    *(.data)
    *(.data.*)
    *(.gnu.linkonce.d.*)
    . = ALIGN(8);
    _gp = .;  /* Global pointer for IA64 */
    . = ALIGN(16);
    _edata = .;
  }

  . = ALIGN(4096);
  .dynamic : {
    *(.dynamic)
  }

  . = ALIGN(4096);
  .dynsym : {
    *(.dynsym)
  }

  . = ALIGN(4096);
  .dynstr : {
    *(.dynstr)
  }

  . = ALIGN(4096);
  .rel : {
    *(.rel)
    *(.rel.*)
  }

  . = ALIGN(4096);
  .rela : {
    *(.rela)
    *(.rela.*)
  }

  . = ALIGN(4096);
  .reloc : {
    *(.reloc)
  }

  . = ALIGN(4096);
  .rodata : {
    _rodata = .;
    *(.rodata)
    *(.rodata.*)
    *(.gnu.linkonce.r.*)
    _erodata = .;
  }

  . = ALIGN(4096);
  .got : {
    *(.got)
    *(.got.plt)
  }

  . = ALIGN(4096);
  .sbss : {
    _sbss = .;
    *(.sbss)
    *(.scommon)
    _esbss = .;
  }

  . = ALIGN(4096);
  .bss : {
    _bss = .;
    *(.bss)
    *(COMMON)
    _ebss = .;
  }

  . = ALIGN(4096);
  .ia64.unwind : {
    *(.IA_64.unwind)
    *(.IA_64.unwind_info)
  }

  . = ALIGN(4096);
  _end = .;

  /* Discard debug sections */
  /DISCARD/ : {
    *(.note.*)
    *(.comment)
    *(.eh_frame)
    *(.debug*)
  }
}
