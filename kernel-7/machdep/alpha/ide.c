/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * IDE/ATA disk controller driver for Alpha
 *
 * This driver supports standard IDE/ATA hard drives and ATAPI devices
 * on AlphaStation systems. It provides PIO-based disk I/O.
 */

#include "ide.h"
#include "platform.h"
#include <sys/types.h>
#include <string.h>

/* Global IDE controller */
static struct ide_controller ide_ctrl;

/*
 * Read IDE register (8-bit)
 */
static inline unsigned char
ide_read_reg(struct ide_device *dev, unsigned char reg)
{
    return inb(dev->base + reg);
}

/*
 * Write IDE register (8-bit)
 */
static inline void
ide_write_reg(struct ide_device *dev, unsigned char reg, unsigned char val)
{
    outb(dev->base + reg, val);
}

/*
 * Read IDE data register (16-bit)
 */
static inline unsigned short
ide_read_data(struct ide_device *dev)
{
    return inw(dev->base + IDE_REG_DATA);
}

/*
 * Write IDE data register (16-bit)
 */
static inline void
ide_write_data(struct ide_device *dev, unsigned short val)
{
    outw(dev->base + IDE_REG_DATA, val);
}

/*
 * Read control/alternate status register
 */
static inline unsigned char
ide_read_ctrl(struct ide_device *dev)
{
    return inb(dev->ctrl + IDE_REG_ALTSTATUS);
}

/*
 * Write control register
 */
static inline void
ide_write_ctrl(struct ide_device *dev, unsigned char val)
{
    outb(dev->ctrl + IDE_REG_CONTROL, val);
}

/*
 * Read status register
 */
unsigned char
alpha_ide_read_status(struct ide_device *dev)
{
    return ide_read_reg(dev, IDE_REG_STATUS);
}

/*
 * Read error register
 */
unsigned char
alpha_ide_read_error(struct ide_device *dev)
{
    return ide_read_reg(dev, IDE_REG_ERROR);
}

/*
 * Select drive
 */
void
alpha_ide_select_drive(struct ide_device *dev)
{
    unsigned char drive_select;

    drive_select = (dev->drive == 0) ? IDE_DRIVE_MASTER : IDE_DRIVE_SLAVE;
    ide_write_reg(dev, IDE_REG_DRIVE, drive_select);

    /* 400ns delay */
    ide_read_ctrl(dev);
    ide_read_ctrl(dev);
    ide_read_ctrl(dev);
    ide_read_ctrl(dev);
}

/*
 * Wait for drive to be ready (not busy)
 */
int
alpha_ide_wait_ready(struct ide_device *dev)
{
    unsigned char status;
    int timeout = IDE_TIMEOUT;

    while (timeout--) {
        status = ide_read_ctrl(dev);  /* Use alternate status to avoid interrupt */
        if (!(status & IDE_STATUS_BSY) && (status & IDE_STATUS_DRDY))
            return 0;
    }

    return -1;  /* Timeout */
}

/*
 * Wait for data request
 */
int
alpha_ide_wait_drq(struct ide_device *dev)
{
    unsigned char status;
    int timeout = IDE_TIMEOUT;

    while (timeout--) {
        status = ide_read_ctrl(dev);
        if (status & IDE_STATUS_DRQ)
            return 0;
        if (status & IDE_STATUS_ERR)
            return -1;
    }

    return -1;  /* Timeout */
}

/*
 * Software reset
 */
void
alpha_ide_reset(int controller)
{
    unsigned short ctrl_port;
    int i;

    ctrl_port = (controller == 0) ? IDE_PRIMARY_CTRL : IDE_SECONDARY_CTRL;

    /* Set SRST bit */
    outb(ctrl_port, IDE_CTRL_SRST);

    /* Wait 5 microseconds */
    for (i = 0; i < 50; i++)
        inb(ctrl_port);

    /* Clear SRST bit */
    outb(ctrl_port, 0);

    /* Wait for drives to be ready (up to 5 seconds) */
    for (i = 0; i < IDE_LONG_TIMEOUT; i++) {
        unsigned char status = inb(ctrl_port);
        if (!(status & IDE_STATUS_BSY))
            break;
    }
}

/*
 * Identify device
 */
int
alpha_ide_identify(struct ide_device *dev)
{
    unsigned short *identify = dev->identify;
    int i;
    unsigned char status;

    /* Select drive */
    alpha_ide_select_drive(dev);

    /* Wait for ready */
    if (alpha_ide_wait_ready(dev) < 0)
        return -1;

    /* Send IDENTIFY command */
    ide_write_reg(dev, IDE_REG_COMMAND, IDE_CMD_IDENTIFY);

    /* 400ns delay */
    ide_read_ctrl(dev);

    /* Check if drive exists */
    status = alpha_ide_read_status(dev);
    if (status == 0)
        return -1;  /* No device */

    /* Wait for DRQ or error */
    if (alpha_ide_wait_drq(dev) < 0) {
        /* Try IDENTIFY PACKET for ATAPI */
        alpha_ide_select_drive(dev);
        if (alpha_ide_wait_ready(dev) < 0)
            return -1;

        ide_write_reg(dev, IDE_REG_COMMAND, IDE_CMD_IDENTIFY_PACKET);
        ide_read_ctrl(dev);

        if (alpha_ide_wait_drq(dev) < 0)
            return -1;

        dev->type = IDE_TYPE_ATAPI;
    } else {
        dev->type = IDE_TYPE_ATA;
    }

    /* Read 256 words of identify data */
    for (i = 0; i < 256; i++)
        identify[i] = ide_read_data(dev);

    /* Parse identify data */
    if (dev->type == IDE_TYPE_ATA) {
        /* Get geometry */
        dev->cylinders = identify[IDE_IDENT_CYLINDERS];
        dev->heads = identify[IDE_IDENT_HEADS];
        dev->sectors = identify[IDE_IDENT_SECTORS];

        /* Get total sectors (28-bit LBA) */
        dev->total_sectors = *(unsigned int *)&identify[IDE_IDENT_MAX_LBA];

        /* Check for 48-bit LBA */
        if (identify[IDE_IDENT_COMMANDSETS] & (1 << 10)) {
            dev->lba48 = 1;
            dev->total_sectors_ext = *(unsigned long long *)&identify[IDE_IDENT_MAX_LBA_EXT];
        } else {
            dev->lba48 = 0;
            dev->total_sectors_ext = dev->total_sectors;
        }

        /* Check for LBA support */
        dev->lba = (identify[IDE_IDENT_CAPABILITIES] & (1 << 9)) ? 1 : 0;

        /* Check for DMA support */
        dev->dma = (identify[IDE_IDENT_CAPABILITIES] & (1 << 8)) ? 1 : 0;

        /* Extract model string (swap bytes) */
        for (i = 0; i < 20; i++) {
            unsigned short word = identify[IDE_IDENT_MODEL + i];
            dev->model[i * 2] = (word >> 8) & 0xFF;
            dev->model[i * 2 + 1] = word & 0xFF;
        }
        dev->model[40] = '\0';

        /* Trim trailing spaces */
        for (i = 39; i >= 0 && dev->model[i] == ' '; i--)
            dev->model[i] = '\0';

        /* Extract serial number (swap bytes) */
        for (i = 0; i < 10; i++) {
            unsigned short word = identify[IDE_IDENT_SERIAL + i];
            dev->serial[i * 2] = (word >> 8) & 0xFF;
            dev->serial[i * 2 + 1] = word & 0xFF;
        }
        dev->serial[20] = '\0';

        /* Trim trailing spaces */
        for (i = 19; i >= 0 && dev->serial[i] == ' '; i--)
            dev->serial[i] = '\0';
    }

    dev->exists = 1;
    return 0;
}

/*
 * Detect device on controller
 */
int
alpha_ide_detect(int controller, int drive)
{
    struct ide_device *dev;
    int index = controller * 2 + drive;

    dev = &ide_ctrl.devices[index];

    /* Initialize device structure */
    dev->exists = 0;
    dev->type = IDE_TYPE_NONE;
    dev->drive = drive;

    if (controller == 0) {
        dev->base = IDE_PRIMARY_BASE;
        dev->ctrl = IDE_PRIMARY_CTRL;
        dev->irq = IDE_PRIMARY_IRQ;
    } else {
        dev->base = IDE_SECONDARY_BASE;
        dev->ctrl = IDE_SECONDARY_CTRL;
        dev->irq = IDE_SECONDARY_IRQ;
    }

    /* Try to identify the device */
    return alpha_ide_identify(dev);
}

/*
 * Read sectors using PIO mode
 */
int
alpha_ide_read_sectors(struct ide_device *dev, unsigned long long lba,
                       unsigned char count, void *buffer)
{
    unsigned short *buf = (unsigned short *)buffer;
    int i, j;
    unsigned char status;

    if (!dev->exists || dev->type != IDE_TYPE_ATA)
        return -1;

    /* Select drive */
    alpha_ide_select_drive(dev);

    /* Wait for ready */
    if (alpha_ide_wait_ready(dev) < 0)
        return -1;

    /* Use 28-bit or 48-bit LBA */
    if (dev->lba48 && lba >= 0x10000000) {
        /* 48-bit LBA */
        ide_write_reg(dev, IDE_REG_SECTOR_COUNT, 0);  /* High byte of count */
        ide_write_reg(dev, IDE_REG_LBA_LOW, (lba >> 24) & 0xFF);
        ide_write_reg(dev, IDE_REG_LBA_MID, (lba >> 32) & 0xFF);
        ide_write_reg(dev, IDE_REG_LBA_HIGH, (lba >> 40) & 0xFF);
        ide_write_reg(dev, IDE_REG_SECTOR_COUNT, count);
        ide_write_reg(dev, IDE_REG_LBA_LOW, lba & 0xFF);
        ide_write_reg(dev, IDE_REG_LBA_MID, (lba >> 8) & 0xFF);
        ide_write_reg(dev, IDE_REG_LBA_HIGH, (lba >> 16) & 0xFF);
        ide_write_reg(dev, IDE_REG_DRIVE,
                     (dev->drive == 0 ? IDE_DRIVE_MASTER : IDE_DRIVE_SLAVE) | IDE_DRIVE_LBA);
        ide_write_reg(dev, IDE_REG_COMMAND, IDE_CMD_READ_PIO_EXT);
    } else {
        /* 28-bit LBA */
        ide_write_reg(dev, IDE_REG_SECTOR_COUNT, count);
        ide_write_reg(dev, IDE_REG_LBA_LOW, lba & 0xFF);
        ide_write_reg(dev, IDE_REG_LBA_MID, (lba >> 8) & 0xFF);
        ide_write_reg(dev, IDE_REG_LBA_HIGH, (lba >> 16) & 0xFF);
        ide_write_reg(dev, IDE_REG_DRIVE,
                     (dev->drive == 0 ? IDE_DRIVE_MASTER : IDE_DRIVE_SLAVE) |
                     IDE_DRIVE_LBA | ((lba >> 24) & 0x0F));
        ide_write_reg(dev, IDE_REG_COMMAND, IDE_CMD_READ_PIO);
    }

    /* Read sectors */
    for (i = 0; i < (count == 0 ? 256 : count); i++) {
        /* Wait for DRQ */
        if (alpha_ide_wait_drq(dev) < 0)
            return -1;

        /* Check for errors */
        status = alpha_ide_read_status(dev);
        if (status & IDE_STATUS_ERR)
            return -1;

        /* Read 256 words (512 bytes) */
        for (j = 0; j < 256; j++)
            *buf++ = ide_read_data(dev);
    }

    return 0;
}

/*
 * Write sectors using PIO mode
 */
int
alpha_ide_write_sectors(struct ide_device *dev, unsigned long long lba,
                        unsigned char count, const void *buffer)
{
    const unsigned short *buf = (const unsigned short *)buffer;
    int i, j;
    unsigned char status;

    if (!dev->exists || dev->type != IDE_TYPE_ATA)
        return -1;

    /* Select drive */
    alpha_ide_select_drive(dev);

    /* Wait for ready */
    if (alpha_ide_wait_ready(dev) < 0)
        return -1;

    /* Use 28-bit or 48-bit LBA */
    if (dev->lba48 && lba >= 0x10000000) {
        /* 48-bit LBA */
        ide_write_reg(dev, IDE_REG_SECTOR_COUNT, 0);
        ide_write_reg(dev, IDE_REG_LBA_LOW, (lba >> 24) & 0xFF);
        ide_write_reg(dev, IDE_REG_LBA_MID, (lba >> 32) & 0xFF);
        ide_write_reg(dev, IDE_REG_LBA_HIGH, (lba >> 40) & 0xFF);
        ide_write_reg(dev, IDE_REG_SECTOR_COUNT, count);
        ide_write_reg(dev, IDE_REG_LBA_LOW, lba & 0xFF);
        ide_write_reg(dev, IDE_REG_LBA_MID, (lba >> 8) & 0xFF);
        ide_write_reg(dev, IDE_REG_LBA_HIGH, (lba >> 16) & 0xFF);
        ide_write_reg(dev, IDE_REG_DRIVE,
                     (dev->drive == 0 ? IDE_DRIVE_MASTER : IDE_DRIVE_SLAVE) | IDE_DRIVE_LBA);
        ide_write_reg(dev, IDE_REG_COMMAND, IDE_CMD_WRITE_PIO_EXT);
    } else {
        /* 28-bit LBA */
        ide_write_reg(dev, IDE_REG_SECTOR_COUNT, count);
        ide_write_reg(dev, IDE_REG_LBA_LOW, lba & 0xFF);
        ide_write_reg(dev, IDE_REG_LBA_MID, (lba >> 8) & 0xFF);
        ide_write_reg(dev, IDE_REG_LBA_HIGH, (lba >> 16) & 0xFF);
        ide_write_reg(dev, IDE_REG_DRIVE,
                     (dev->drive == 0 ? IDE_DRIVE_MASTER : IDE_DRIVE_SLAVE) |
                     IDE_DRIVE_LBA | ((lba >> 24) & 0x0F));
        ide_write_reg(dev, IDE_REG_COMMAND, IDE_CMD_WRITE_PIO);
    }

    /* Write sectors */
    for (i = 0; i < (count == 0 ? 256 : count); i++) {
        /* Wait for DRQ */
        if (alpha_ide_wait_drq(dev) < 0)
            return -1;

        /* Check for errors */
        status = alpha_ide_read_status(dev);
        if (status & IDE_STATUS_ERR)
            return -1;

        /* Write 256 words (512 bytes) */
        for (j = 0; j < 256; j++)
            ide_write_data(dev, *buf++);
    }

    /* Wait for completion */
    if (alpha_ide_wait_ready(dev) < 0)
        return -1;

    /* Flush cache */
    ide_write_reg(dev, IDE_REG_COMMAND,
                 dev->lba48 ? IDE_CMD_CACHE_FLUSH_EXT : IDE_CMD_CACHE_FLUSH);
    alpha_ide_wait_ready(dev);

    return 0;
}

/*
 * IDE interrupt handler
 */
void
alpha_ide_interrupt(int irq)
{
    /* TODO: Handle interrupt-driven I/O */
    /* For now, just clear the interrupt by reading status */
    if (irq == IDE_PRIMARY_IRQ) {
        inb(IDE_PRIMARY_BASE + IDE_REG_STATUS);
    } else if (irq == IDE_SECONDARY_IRQ) {
        inb(IDE_SECONDARY_BASE + IDE_REG_STATUS);
    }
}

/*
 * Get device by index
 */
struct ide_device *
alpha_ide_get_device(int index)
{
    if (index < 0 || index >= 4)
        return NULL;

    if (!ide_ctrl.devices[index].exists)
        return NULL;

    return &ide_ctrl.devices[index];
}

/*
 * Initialize IDE subsystem
 */
int
alpha_ide_init(void)
{
    int i;

    /* Initialize controller structure */
    for (i = 0; i < 4; i++) {
        ide_ctrl.devices[i].exists = 0;
        ide_ctrl.devices[i].type = IDE_TYPE_NONE;
    }

    /* Reset controllers */
    alpha_ide_reset(0);  /* Primary */
    alpha_ide_reset(1);  /* Secondary */

    /* Detect devices */
    alpha_ide_detect(0, 0);  /* Primary master */
    alpha_ide_detect(0, 1);  /* Primary slave */
    alpha_ide_detect(1, 0);  /* Secondary master */
    alpha_ide_detect(1, 1);  /* Secondary slave */

    ide_ctrl.initialized = 1;

    return 0;
}
