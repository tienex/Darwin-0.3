/*
 * DLX Device Configuration
 *
 * Device configuration and initialization for DLX architecture.
 * Manages device tables and provides device discovery/initialization.
 */

#include <mach/dlx/vm_types.h>
#include <mach/dlx/boolean.h>
#include "trap.h"

/* Device types */
#define DEV_TYPE_CHAR		1	/* Character device */
#define DEV_TYPE_BLOCK		2	/* Block device */
#define DEV_TYPE_NETWORK	3	/* Network device */

/* Device flags */
#define DEV_FLAG_INITED		0x0001	/* Device initialized */
#define DEV_FLAG_ENABLED	0x0002	/* Device enabled */
#define DEV_FLAG_IRQ		0x0004	/* Device uses interrupts */

/*
 * Device descriptor
 */
struct device_desc {
	const char *name;		/* Device name */
	int type;			/* Device type */
	int flags;			/* Device flags */
	unsigned int base_addr;		/* Base I/O address */
	int irq;			/* IRQ number */
	void (*init)(void);		/* Init function */
	void (*reset)(void);		/* Reset function */
	void *private_data;		/* Driver private data */
};

/* Forward declarations of init functions */
extern void console_init(void);
extern void keyboard_init(void);
extern void clock_init(void);
extern void interrupt_init(void);

/*
 * Device table
 * Lists all devices in the system
 */
static struct device_desc device_table[] = {
	{
		.name = "console",
		.type = DEV_TYPE_CHAR,
		.flags = DEV_FLAG_ENABLED | DEV_FLAG_IRQ,
		.base_addr = 0xfff00100,	/* Keyboard I/O base */
		.irq = 0x48,			/* TRAP_KBD */
		.init = console_init,
		.reset = NULL,
		.private_data = NULL
	},
	{
		.name = "keyboard",
		.type = DEV_TYPE_CHAR,
		.flags = DEV_FLAG_ENABLED | DEV_FLAG_IRQ,
		.base_addr = 0xfff00100,	/* Keyboard I/O base */
		.irq = 0x48,			/* TRAP_KBD */
		.init = keyboard_init,
		.reset = NULL,
		.private_data = NULL
	},
	{
		.name = "timer",
		.type = DEV_TYPE_CHAR,
		.flags = DEV_FLAG_ENABLED | DEV_FLAG_IRQ,
		.base_addr = 0xfff00010,	/* Timer base */
		.irq = 0x40,			/* TRAP_TIMER */
		.init = clock_init,
		.reset = NULL,
		.private_data = NULL
	},
	/* Null terminator */
	{ NULL, 0, 0, 0, 0, NULL, NULL, NULL }
};

/*
 * Initialize single device
 */
static int
device_init_one(struct device_desc *dev)
{
	if (!dev || !dev->name)
		return -1;

	if (dev->flags & DEV_FLAG_INITED)
		return 0;	/* Already initialized */

	printf("Initializing device: %s at 0x%x", dev->name, dev->base_addr);
	if (dev->flags & DEV_FLAG_IRQ)
		printf(" (IRQ 0x%x)", dev->irq);
	printf("\n");

	/* Call device init function */
	if (dev->init)
		dev->init();

	dev->flags |= DEV_FLAG_INITED;

	return 0;
}

/*
 * Initialize all devices
 * Called during kernel startup
 */
void
device_init_all(void)
{
	struct device_desc *dev;
	int count = 0;

	printf("\n=== Device Initialization ===\n");

	/* Initialize interrupt controller first */
	interrupt_init();

	/* Initialize all configured devices */
	for (dev = device_table; dev->name != NULL; dev++) {
		if (dev->flags & DEV_FLAG_ENABLED) {
			if (device_init_one(dev) == 0)
				count++;
		}
	}

	printf("=== %d devices initialized ===\n\n", count);
}

/*
 * Find device by name
 */
struct device_desc *
device_find(const char *name)
{
	struct device_desc *dev;

	if (!name)
		return NULL;

	for (dev = device_table; dev->name != NULL; dev++) {
		/* Simple string comparison */
		const char *p1 = dev->name;
		const char *p2 = name;
		while (*p1 && *p2 && *p1 == *p2) {
			p1++;
			p2++;
		}
		if (*p1 == *p2)  /* Both reached end */
			return dev;
	}

	return NULL;
}

/*
 * Get device by index
 */
struct device_desc *
device_get(int index)
{
	int i;

	for (i = 0; device_table[i].name != NULL; i++) {
		if (i == index)
			return &device_table[i];
	}

	return NULL;
}

/*
 * Get device count
 */
int
device_count(void)
{
	int count = 0;

	while (device_table[count].name != NULL)
		count++;

	return count;
}

/*
 * Print device table
 */
void
device_print_table(void)
{
	struct device_desc *dev;
	const char *type_str;

	printf("\nDevice Table:\n");
	printf("%-12s %-8s %-10s %-12s %s\n",
	       "Name", "Type", "Address", "IRQ", "Status");
	printf("--------------------------------------------------------\n");

	for (dev = device_table; dev->name != NULL; dev++) {
		/* Determine type string */
		if (dev->type == DEV_TYPE_CHAR)
			type_str = "CHAR";
		else if (dev->type == DEV_TYPE_BLOCK)
			type_str = "BLOCK";
		else if (dev->type == DEV_TYPE_NETWORK)
			type_str = "NET";
		else
			type_str = "UNKNOWN";

		printf("%-12s %-8s 0x%08x", dev->name, type_str, dev->base_addr);

		if (dev->flags & DEV_FLAG_IRQ)
			printf(" 0x%02x      ", dev->irq);
		else
			printf(" --        ");

		if (dev->flags & DEV_FLAG_INITED)
			printf(" INIT");
		if (dev->flags & DEV_FLAG_ENABLED)
			printf(" ENABLED");

		printf("\n");
	}
}

/*
 * Reset device
 */
int
device_reset(const char *name)
{
	struct device_desc *dev;

	dev = device_find(name);
	if (!dev)
		return -1;

	printf("Resetting device: %s\n", name);

	if (dev->reset)
		dev->reset();

	return 0;
}

/*
 * Enable device
 */
int
device_enable(const char *name)
{
	struct device_desc *dev;

	dev = device_find(name);
	if (!dev)
		return -1;

	dev->flags |= DEV_FLAG_ENABLED;
	return 0;
}

/*
 * Disable device
 */
int
device_disable(const char *name)
{
	struct device_desc *dev;

	dev = device_find(name);
	if (!dev)
		return -1;

	dev->flags &= ~DEV_FLAG_ENABLED;
	return 0;
}
