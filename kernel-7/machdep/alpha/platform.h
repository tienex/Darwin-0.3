/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * Alpha Platform Definitions
 *
 * This file defines platform-specific information for various Alpha
 * systems, with initial focus on AlphaStation 500 and 600 series.
 */

#ifndef _MACHDEP_ALPHA_PLATFORM_H_
#define _MACHDEP_ALPHA_PLATFORM_H_

/*
 * Platform types
 */
#define ALPHA_PLATFORM_UNKNOWN		0
#define ALPHA_PLATFORM_ALPHASTATION_500	1	/* AlphaStation 500 (Maverick) */
#define ALPHA_PLATFORM_ALPHASTATION_600	2	/* AlphaStation 600 (Alcor) */
#define ALPHA_PLATFORM_ALPHASTATION_200	3	/* AlphaStation 200 (Avanti) */
#define ALPHA_PLATFORM_ALPHASTATION_255	4	/* AlphaStation 255 (Avanti) */
#define ALPHA_PLATFORM_ALPHASTATION_400	5	/* AlphaStation 400 (Avanti) */
#define ALPHA_PLATFORM_EB164		6	/* EB164 evaluation board */
#define ALPHA_PLATFORM_PC164		7	/* PC164 (Durango) */
#define ALPHA_PLATFORM_LX164		8	/* LX164 (Durango) */
#define ALPHA_PLATFORM_SX164		9	/* SX164 (Ruffian) */
#define ALPHA_PLATFORM_ALPHASERVER_800	10	/* AlphaServer 800 */
#define ALPHA_PLATFORM_ALPHASERVER_1000	11	/* AlphaServer 1000 */
#define ALPHA_PLATFORM_ALPHASERVER_2100	12	/* AlphaServer 2100 */

/*
 * System chipsets
 */
#define ALPHA_CHIPSET_UNKNOWN		0
#define ALPHA_CHIPSET_CIA		1	/* 21171/21172 (Alcor/Pyxis) */
#define ALPHA_CHIPSET_PYXIS		2	/* 21174 (Miata) */
#define ALPHA_CHIPSET_TSUNAMI		3	/* 21272 (Typhoon) */
#define ALPHA_CHIPSET_IRONGATE		4	/* AMD-751 (Nautilus) */
#define ALPHA_CHIPSET_T2		5	/* T2 (Sable) */
#define ALPHA_CHIPSET_APECS		6	/* APECS (Avanti/Mustang) */

/*
 * Platform information structure
 */
struct alpha_platform {
	int		type;			/* Platform type */
	const char	*name;			/* Platform name */
	const char	*model;			/* Model name */
	int		chipset;		/* Core chipset */
	int		cpu_type;		/* CPU type (EV4/EV5/EV6) */
	unsigned long	memory_base;		/* Memory base address */
	unsigned long	memory_size;		/* Memory size */
	unsigned long	io_base;		/* I/O space base */
	unsigned long	pci_config_base;	/* PCI config space base */
	unsigned long	pci_mem_base;		/* PCI memory base */
	unsigned long	pci_io_base;		/* PCI I/O base */
	int		num_pci_buses;		/* Number of PCI buses */
	int		isa_present;		/* ISA bus present */
	int		rtc_port;		/* RTC I/O port */
};

/*
 * AlphaStation 500 (Maverick) - EV5/EV56 based workstation
 *
 * Features:
 * - 21164 (EV5) or 21164A (EV56) CPU at 266-500 MHz
 * - 21172 CIA chipset
 * - PCI bus
 * - ISA bus
 * - Up to 1GB RAM
 * - Built-in S3 graphics
 */
#define AS500_PCI_CONFIG_BASE		0x8700000000UL
#define AS500_PCI_MEM_BASE		0x8000000000UL
#define AS500_PCI_IO_BASE		0x8580000000UL
#define AS500_MEMORY_BASE		0x0000000000UL
#define AS500_RTC_PORT			0x70

/*
 * AlphaStation 600 (Alcor) - EV5 based workstation
 *
 * Features:
 * - 21164 (EV5) CPU at 266-333 MHz
 * - 21171 CIA chipset
 * - PCI bus
 * - ISA bus
 * - Up to 2GB RAM
 * - Better expansion capabilities than AS500
 */
#define AS600_PCI_CONFIG_BASE		0x8700000000UL
#define AS600_PCI_MEM_BASE		0x8000000000UL
#define AS600_PCI_IO_BASE		0x8580000000UL
#define AS600_MEMORY_BASE		0x0000000000UL
#define AS600_RTC_PORT			0x70

/*
 * CIA (21171/21172) Chipset Registers
 *
 * The CIA is the core logic chipset for AlphaStation 500/600.
 * It provides:
 * - PCI bus interface
 * - Memory controller
 * - Interrupt controller
 */
#define CIA_CSR_BASE			0x8740000000UL

/* CIA General registers */
#define CIA_REG_CSR_GEN			(CIA_CSR_BASE + 0x000)
#define CIA_REG_CSR_LED			(CIA_CSR_BASE + 0x010)
#define CIA_REG_CSR_CNFG		(CIA_CSR_BASE + 0x040)
#define CIA_REG_CSR_HAE_MEM		(CIA_CSR_BASE + 0x400)
#define CIA_REG_CSR_HAE_IO		(CIA_CSR_BASE + 0x440)

/* CIA Diagnostic registers */
#define CIA_REG_CIA_DIAG		(CIA_CSR_BASE + 0x2000)
#define CIA_REG_CIA_CTRL		(CIA_CSR_BASE + 0x2100)
#define CIA_REG_CIA_ERR			(CIA_CSR_BASE + 0x8200)

/* CIA PCI configuration */
#define CIA_PCI_CONFIG_ADDR		(CIA_CSR_BASE + 0x8300)
#define CIA_PCI_CONFIG_DATA		(CIA_CSR_BASE + 0x8320)

/* CIA Scatter-Gather TLB */
#define CIA_PCI_TBIA			(CIA_CSR_BASE + 0x8600)
#define CIA_PCI_W0_BASE			(CIA_CSR_BASE + 0x400)
#define CIA_PCI_W0_MASK			(CIA_CSR_BASE + 0x440)
#define CIA_PCI_T0_BASE			(CIA_CSR_BASE + 0x480)
#define CIA_PCI_W1_BASE			(CIA_CSR_BASE + 0x500)
#define CIA_PCI_W1_MASK			(CIA_CSR_BASE + 0x540)
#define CIA_PCI_T1_BASE			(CIA_CSR_BASE + 0x580)

/*
 * Platform detection from HWRPB
 */
#define HWRPB_SYSTYPE_ALPHASTATION_500	20	/* AlphaStation 500 */
#define HWRPB_SYSTYPE_ALPHASTATION_600	6	/* AlphaStation 600 (Alcor) */
#define HWRPB_SYSTYPE_ALPHASTATION_200	11	/* AlphaStation 200 */

/*
 * ISA bus I/O ports
 */
#define ISA_PORT_RTC_ADDR		0x70
#define ISA_PORT_RTC_DATA		0x71
#define ISA_PORT_NVRAM_ADDR		0x74
#define ISA_PORT_NVRAM_DATA		0x75
#define ISA_PORT_KBD_STATUS		0x64
#define ISA_PORT_KBD_DATA		0x60

#ifndef __ASSEMBLER__

/*
 * Current platform
 */
extern struct alpha_platform current_platform;

/*
 * Platform detection and initialization
 */
int alpha_platform_detect(void);
void alpha_platform_init(void);
const char *alpha_platform_name(void);

/*
 * Platform-specific operations
 */
void platform_poweroff(void);
void platform_reboot(void);
void platform_halt(void);

/*
 * I/O port access for ISA devices
 */
unsigned char inb(unsigned short port);
void outb(unsigned char value, unsigned short port);
unsigned short inw(unsigned short port);
void outw(unsigned short value, unsigned short port);
unsigned int inl(unsigned short port);
void outl(unsigned int value, unsigned short port);

/*
 * PCI configuration space access
 */
unsigned int pci_config_read(int bus, int dev, int func, int reg);
void pci_config_write(int bus, int dev, int func, int reg, unsigned int value);

#endif /* !__ASSEMBLER__ */

#endif /* _MACHDEP_ALPHA_PLATFORM_H_ */
