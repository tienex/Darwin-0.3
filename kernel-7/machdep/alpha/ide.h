/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * IDE/ATA disk controller driver for Alpha
 */

#ifndef _MACHDEP_ALPHA_IDE_H_
#define _MACHDEP_ALPHA_IDE_H_

/* Standard IDE I/O ports */
#define IDE_PRIMARY_BASE    0x1F0
#define IDE_PRIMARY_CTRL    0x3F6
#define IDE_PRIMARY_IRQ     14

#define IDE_SECONDARY_BASE  0x170
#define IDE_SECONDARY_CTRL  0x376
#define IDE_SECONDARY_IRQ   15

/* IDE register offsets from base */
#define IDE_REG_DATA        0   /* Data register (16-bit) */
#define IDE_REG_ERROR       1   /* Error register (read) */
#define IDE_REG_FEATURES    1   /* Features register (write) */
#define IDE_REG_SECTOR_COUNT 2  /* Sector count */
#define IDE_REG_LBA_LOW     3   /* LBA low (sector number) */
#define IDE_REG_LBA_MID     4   /* LBA mid (cylinder low) */
#define IDE_REG_LBA_HIGH    5   /* LBA high (cylinder high) */
#define IDE_REG_DRIVE       6   /* Drive/Head register */
#define IDE_REG_STATUS      7   /* Status register (read) */
#define IDE_REG_COMMAND     7   /* Command register (write) */

/* Control register (alternate status) */
#define IDE_REG_CONTROL     0   /* Device control register */
#define IDE_REG_ALTSTATUS   0   /* Alternate status (read) */

/* Status register bits */
#define IDE_STATUS_ERR      0x01    /* Error */
#define IDE_STATUS_IDX      0x02    /* Index (obsolete) */
#define IDE_STATUS_CORR     0x04    /* Corrected data */
#define IDE_STATUS_DRQ      0x08    /* Data request */
#define IDE_STATUS_DSC      0x10    /* Drive seek complete */
#define IDE_STATUS_DF       0x20    /* Drive fault */
#define IDE_STATUS_DRDY     0x40    /* Drive ready */
#define IDE_STATUS_BSY      0x80    /* Busy */

/* Error register bits */
#define IDE_ERROR_AMNF      0x01    /* Address mark not found */
#define IDE_ERROR_TK0NF     0x02    /* Track 0 not found */
#define IDE_ERROR_ABRT      0x04    /* Aborted command */
#define IDE_ERROR_MCR       0x08    /* Media change request */
#define IDE_ERROR_IDNF      0x10    /* ID not found */
#define IDE_ERROR_MC        0x20    /* Media changed */
#define IDE_ERROR_UNC       0x40    /* Uncorrectable data error */
#define IDE_ERROR_BBK       0x80    /* Bad block */

/* Control register bits */
#define IDE_CTRL_NIEN       0x02    /* Disable interrupts */
#define IDE_CTRL_SRST       0x04    /* Software reset */
#define IDE_CTRL_HOB        0x80    /* High order byte (48-bit LBA) */

/* Drive/Head register bits */
#define IDE_DRIVE_MASTER    0xA0    /* Master drive */
#define IDE_DRIVE_SLAVE     0xB0    /* Slave drive */
#define IDE_DRIVE_LBA       0x40    /* LBA mode */
#define IDE_DRIVE_CHS       0xA0    /* CHS mode */

/* ATA commands */
#define IDE_CMD_READ_PIO        0x20    /* Read sectors (PIO) */
#define IDE_CMD_READ_PIO_EXT    0x24    /* Read sectors (PIO, 48-bit) */
#define IDE_CMD_READ_DMA        0xC8    /* Read sectors (DMA) */
#define IDE_CMD_READ_DMA_EXT    0x25    /* Read sectors (DMA, 48-bit) */
#define IDE_CMD_WRITE_PIO       0x30    /* Write sectors (PIO) */
#define IDE_CMD_WRITE_PIO_EXT   0x34    /* Write sectors (PIO, 48-bit) */
#define IDE_CMD_WRITE_DMA       0xCA    /* Write sectors (DMA) */
#define IDE_CMD_WRITE_DMA_EXT   0x35    /* Write sectors (DMA, 48-bit) */
#define IDE_CMD_CACHE_FLUSH     0xE7    /* Flush cache */
#define IDE_CMD_CACHE_FLUSH_EXT 0xEA    /* Flush cache (48-bit) */
#define IDE_CMD_PACKET          0xA0    /* ATAPI packet */
#define IDE_CMD_IDENTIFY_PACKET 0xA1    /* Identify ATAPI device */
#define IDE_CMD_IDENTIFY        0xEC    /* Identify device */

/* IDENTIFY data offsets (words) */
#define IDE_IDENT_DEVICETYPE    0
#define IDE_IDENT_CYLINDERS     1
#define IDE_IDENT_HEADS         3
#define IDE_IDENT_SECTORS       6
#define IDE_IDENT_SERIAL        10  /* 10 words (20 bytes) */
#define IDE_IDENT_MODEL         27  /* 20 words (40 bytes) */
#define IDE_IDENT_CAPABILITIES  49
#define IDE_IDENT_FIELDVALID    53
#define IDE_IDENT_MAX_LBA       60  /* 2 words (32-bit) */
#define IDE_IDENT_COMMANDSETS   82
#define IDE_IDENT_MAX_LBA_EXT   100 /* 4 words (48-bit) */

/* Device types */
#define IDE_TYPE_NONE       0
#define IDE_TYPE_ATA        1
#define IDE_TYPE_ATAPI      2

/* Timeout values */
#define IDE_TIMEOUT         30000   /* Standard timeout (30ms) */
#define IDE_LONG_TIMEOUT    5000000 /* Long timeout (5s) */

/* Sector size */
#define IDE_SECTOR_SIZE     512

/* Maximum sectors per transfer */
#define IDE_MAX_SECTORS     256

/* IDE device structure */
struct ide_device {
    int exists;                     /* Device exists */
    int type;                       /* Device type (ATA/ATAPI) */
    unsigned short base;            /* I/O base address */
    unsigned short ctrl;            /* Control register address */
    unsigned char irq;              /* IRQ number */
    unsigned char drive;            /* Drive select (0=master, 1=slave) */

    /* Device geometry */
    unsigned short cylinders;       /* Cylinders (CHS) */
    unsigned short heads;           /* Heads (CHS) */
    unsigned short sectors;         /* Sectors per track (CHS) */
    unsigned int total_sectors;     /* Total sectors (28-bit LBA) */
    unsigned long long total_sectors_ext;  /* Total sectors (48-bit LBA) */

    /* Capabilities */
    unsigned char lba48;            /* 48-bit LBA support */
    unsigned char dma;              /* DMA support */
    unsigned char lba;              /* LBA support */

    /* Device identification */
    char model[41];                 /* Model string */
    char serial[21];                /* Serial number */
    unsigned short identify[256];   /* IDENTIFY data */
};

/* IDE controller structure */
struct ide_controller {
    struct ide_device devices[4];   /* Primary master/slave, secondary master/slave */
    int initialized;
};

/* Function prototypes */
int alpha_ide_init(void);
int alpha_ide_detect(int controller, int drive);
int alpha_ide_identify(struct ide_device *dev);
int alpha_ide_read_sectors(struct ide_device *dev, unsigned long long lba,
                           unsigned char count, void *buffer);
int alpha_ide_write_sectors(struct ide_device *dev, unsigned long long lba,
                            unsigned char count, const void *buffer);
void alpha_ide_interrupt(int irq);
int alpha_ide_wait_ready(struct ide_device *dev);
int alpha_ide_wait_drq(struct ide_device *dev);
unsigned char alpha_ide_read_status(struct ide_device *dev);
unsigned char alpha_ide_read_error(struct ide_device *dev);
void alpha_ide_write_cmd(struct ide_device *dev, unsigned char cmd);
void alpha_ide_select_drive(struct ide_device *dev);
void alpha_ide_reset(int controller);
struct ide_device *alpha_ide_get_device(int index);

#endif /* _MACHDEP_ALPHA_IDE_H_ */
