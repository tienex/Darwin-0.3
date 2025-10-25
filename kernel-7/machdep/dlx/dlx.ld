/*
 * DLX Kernel Linker Script
 * For Mach-O format on Darwin
 */

OUTPUT_ARCH(dlx)
ENTRY(_start)

SECTIONS
{
	/* Exception vectors at 0x00 - maps to __TEXT,__vectors */
	. = 0x00000000;
	__TEXT_vectors :
	{
		*(__TEXT,__vectors)
		*(__.TEXT.__vectors)
	}

	/* Kernel code starts at 1MB */
	. = 0x00100000;

	__TEXT : ALIGN(0x2000)		/* Align to 8KB page boundary */
	{
		_text = .;
		*(__TEXT,__text)
		*(__.TEXT.__text)
		*(.text)
		*(.text.*)
		*(__TEXT,__const)
		*(__.TEXT.__const)
		*(__TEXT,__cstring)
		*(__.TEXT.__cstring)
		*(.rodata)
		*(.rodata.*)
		_etext = .;
	}

	__DATA : ALIGN(0x2000)
	{
		_data = .;
		*(__DATA,__data)
		*(__.DATA.__data)
		*(.data)
		*(.data.*)
		_edata = .;
	}

	__BSS : ALIGN(0x2000)
	{
		_bss = .;
		*(__DATA,__bss)
		*(__.DATA.__bss)
		*(__DATA,__common)
		*(__.DATA.__common)
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
		*(.debug*)
	}
}
