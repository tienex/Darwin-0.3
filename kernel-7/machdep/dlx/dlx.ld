/*
 * DLX Kernel Linker Script
 */

OUTPUT_FORMAT("elf32-dlx")
OUTPUT_ARCH(dlx)
ENTRY(_start)

SECTIONS
{
	/* Exception vectors at 0x00 */
	.vectors 0x00000000 :
	{
		*(.vectors)
	}

	/* Kernel code starts at 1MB */
	. = 0x00100000;

	.text : ALIGN(0x2000)		/* Align to 8KB */
	{
		_text = .;
		*(.text)
		*(.text.*)
		*(.rodata)
		*(.rodata.*)
		_etext = .;
	}

	.data : ALIGN(0x2000)
	{
		_data = .;
		*(.data)
		*(.data.*)
		_edata = .;
	}

	.bss : ALIGN(0x2000)
	{
		_bss = .;
		*(.bss)
		*(.bss.*)
		*(COMMON)
		_ebss = .;
	}

	_end = .;

	/* Discard unneeded sections */
	/DISCARD/ :
	{
		*(.comment)
		*(.note)
		*(.eh_frame)
	}
}
