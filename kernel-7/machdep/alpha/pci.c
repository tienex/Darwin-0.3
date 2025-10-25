/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * Alpha PCI Bus Support
 */

#include <mach/mach_types.h>
#include <architecture/alpha/cpu.h>
#include "platform.h"

/*
 * PCI device structure
 */
struct pci_device {
	int		bus;
	int		dev;
	int		func;
	unsigned short	vendor_id;
	unsigned short	device_id;
	unsigned char	class_code;
	unsigned char	subclass;
	unsigned char	prog_if;
	unsigned char	revision;
	unsigned char	header_type;
	unsigned int	bar[6];		/* Base Address Registers */
	unsigned char	irq_line;
	unsigned char	irq_pin;
};

#define MAX_PCI_DEVICES 64
static struct pci_device pci_devices[MAX_PCI_DEVICES];
static int num_pci_devices = 0;

/*
 * PCI configuration space register offsets
 */
#define PCI_REG_VENDOR_ID	0x00
#define PCI_REG_DEVICE_ID	0x02
#define PCI_REG_COMMAND		0x04
#define PCI_REG_STATUS		0x06
#define PCI_REG_REVISION	0x08
#define PCI_REG_PROG_IF		0x09
#define PCI_REG_SUBCLASS	0x0A
#define PCI_REG_CLASS_CODE	0x0B
#define PCI_REG_HEADER_TYPE	0x0E
#define PCI_REG_BAR0		0x10
#define PCI_REG_IRQ_LINE	0x3C
#define PCI_REG_IRQ_PIN		0x3D

/*
 * PCI command register bits
 */
#define PCI_CMD_IO_ENABLE	0x0001
#define PCI_CMD_MEM_ENABLE	0x0002
#define PCI_CMD_MASTER_ENABLE	0x0004
#define PCI_CMD_PARITY_ENABLE	0x0040

/*
 * Read PCI configuration register (8-bit)
 */
static unsigned char
pci_config_read_byte(int bus, int dev, int func, int reg)
{
	unsigned int val = pci_config_read(bus, dev, func, reg & ~3);
	return (val >> ((reg & 3) * 8)) & 0xFF;
}

/*
 * Read PCI configuration register (16-bit)
 */
static unsigned short
pci_config_read_word(int bus, int dev, int func, int reg)
{
	unsigned int val = pci_config_read(bus, dev, func, reg & ~3);
	return (val >> ((reg & 2) * 8)) & 0xFFFF;
}

/*
 * Write PCI configuration register (16-bit)
 */
static void
pci_config_write_word(int bus, int dev, int func, int reg, unsigned short value)
{
	unsigned int val = pci_config_read(bus, dev, func, reg & ~3);
	unsigned int shift = (reg & 2) * 8;
	val = (val & ~(0xFFFF << shift)) | ((unsigned int)value << shift);
	pci_config_write(bus, dev, func, reg & ~3, val);
}

/*
 * Probe for PCI device
 */
static int
pci_probe_device(int bus, int dev, int func)
{
	unsigned short vendor_id;

	vendor_id = pci_config_read_word(bus, dev, func, PCI_REG_VENDOR_ID);

	/* 0xFFFF means no device */
	if (vendor_id == 0xFFFF)
		return 0;

	return 1;
}

/*
 * Scan PCI bus for devices
 */
static void
pci_scan_bus(int bus)
{
	int dev, func;
	struct pci_device *pdev;

	for (dev = 0; dev < 32; dev++) {
		if (!pci_probe_device(bus, dev, 0))
			continue;

		if (num_pci_devices >= MAX_PCI_DEVICES) {
			printf("Warning: Too many PCI devices\n");
			return;
		}

		pdev = &pci_devices[num_pci_devices++];
		pdev->bus = bus;
		pdev->dev = dev;
		pdev->func = 0;

		/* Read device information */
		pdev->vendor_id = pci_config_read_word(bus, dev, 0, PCI_REG_VENDOR_ID);
		pdev->device_id = pci_config_read_word(bus, dev, 0, PCI_REG_DEVICE_ID);
		pdev->class_code = pci_config_read_byte(bus, dev, 0, PCI_REG_CLASS_CODE);
		pdev->subclass = pci_config_read_byte(bus, dev, 0, PCI_REG_SUBCLASS);
		pdev->prog_if = pci_config_read_byte(bus, dev, 0, PCI_REG_PROG_IF);
		pdev->revision = pci_config_read_byte(bus, dev, 0, PCI_REG_REVISION);
		pdev->header_type = pci_config_read_byte(bus, dev, 0, PCI_REG_HEADER_TYPE);
		pdev->irq_line = pci_config_read_byte(bus, dev, 0, PCI_REG_IRQ_LINE);
		pdev->irq_pin = pci_config_read_byte(bus, dev, 0, PCI_REG_IRQ_PIN);

		/* Read base address registers */
		for (func = 0; func < 6; func++) {
			pdev->bar[func] = pci_config_read(bus, dev, 0,
							   PCI_REG_BAR0 + func * 4);
		}

		/* Check for multi-function device */
		if (pdev->header_type & 0x80) {
			/* Scan additional functions */
			for (func = 1; func < 8; func++) {
				if (pci_probe_device(bus, dev, func)) {
					/* Would add additional function here */
				}
			}
		}
	}
}

/*
 * Initialize PCI bus
 */
void
alpha_pci_init(void)
{
	int bus;

	printf("Initializing PCI bus...\n");

	num_pci_devices = 0;

	/*
	 * Scan all PCI buses
	 */
	for (bus = 0; bus < current_platform.num_pci_buses; bus++) {
		pci_scan_bus(bus);
	}

	printf("  Found %d PCI devices\n", num_pci_devices);
}

/*
 * Enable PCI device
 */
void
pci_enable_device(struct pci_device *dev)
{
	unsigned short cmd;

	cmd = pci_config_read_word(dev->bus, dev->dev, dev->func, PCI_REG_COMMAND);

	/* Enable I/O and memory space */
	cmd |= PCI_CMD_IO_ENABLE | PCI_CMD_MEM_ENABLE;

	/* Enable bus mastering for DMA */
	cmd |= PCI_CMD_MASTER_ENABLE;

	pci_config_write_word(dev->bus, dev->dev, dev->func, PCI_REG_COMMAND, cmd);
}

/*
 * Print PCI device information
 */
void
pci_print_devices(void)
{
	int i;
	struct pci_device *dev;
	const char *class_name;

	printf("\nPCI Devices:\n");
	printf("  Bus Dev Func  Vendor Device  Class                   IRQ\n");
	printf("  --- --- ----  ------ ------  ----------------------  ---\n");

	for (i = 0; i < num_pci_devices; i++) {
		dev = &pci_devices[i];

		/* Determine class name */
		switch (dev->class_code) {
		case 0x00: class_name = "Legacy Device"; break;
		case 0x01: class_name = "Mass Storage"; break;
		case 0x02: class_name = "Network Controller"; break;
		case 0x03: class_name = "Display Controller"; break;
		case 0x04: class_name = "Multimedia Device"; break;
		case 0x05: class_name = "Memory Controller"; break;
		case 0x06: class_name = "Bridge Device"; break;
		case 0x07: class_name = "Communication Device"; break;
		case 0x08: class_name = "System Peripheral"; break;
		case 0x09: class_name = "Input Device"; break;
		case 0x0A: class_name = "Docking Station"; break;
		case 0x0B: class_name = "Processor"; break;
		case 0x0C: class_name = "Serial Bus"; break;
		default:   class_name = "Unknown"; break;
		}

		printf("  %3d %3d  %3d  0x%04X 0x%04X  %-22s  %3d\n",
		       dev->bus, dev->dev, dev->func,
		       dev->vendor_id, dev->device_id,
		       class_name, dev->irq_line);
	}
}

/*
 * Find PCI device by vendor/device ID
 */
struct pci_device *
pci_find_device(unsigned short vendor_id, unsigned short device_id)
{
	int i;

	for (i = 0; i < num_pci_devices; i++) {
		if (pci_devices[i].vendor_id == vendor_id &&
		    pci_devices[i].device_id == device_id) {
			return &pci_devices[i];
		}
	}

	return NULL;
}

/*
 * Find PCI device by class code
 */
struct pci_device *
pci_find_class(unsigned char class_code, unsigned char subclass)
{
	int i;

	for (i = 0; i < num_pci_devices; i++) {
		if (pci_devices[i].class_code == class_code &&
		    pci_devices[i].subclass == subclass) {
			return &pci_devices[i];
		}
	}

	return NULL;
}
