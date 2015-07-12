#ifndef __ATA_H__
#define __ATA_H__

#define ATA_SECTOR_BITS 9
#define ATA_SECTOR_SIZE (1 << ATA_SECTOR_BITS)
#define ATA_SECTOR_MASK (ATA_SECTOR_SIZE - 1)

#define ATA_PRIMARY                0x1f0
#define ATA_SECONDARY              0x170
#define ATA_MASTER                 0xa0
#define ATA_SLAVE                  0xb0
#define ata_port_data(channel)     (channel)
#define ata_port_error(channel)    ((channel) + 0x1)
#define ata_port_feature(channel)  ((channel) + 0x1)
#define ata_port_seccount(channel) ((channel) + 0x2)
#define ata_port_lba0(channel)     ((channel) + 0x3)
#define ata_port_lba1(channel)     ((channel) + 0x4)
#define ata_port_lba2(channel)     ((channel) + 0x5)
#define ata_port_drvsel(channel)   ((channel) + 0x6)
#define ata_port_cmd(channel)      ((channel) + 0x7)
#define ata_port_stat(channel)     ((channel) + 0x7)
#define ata_port_ctrl(channel)     ((channel) + 0x206)
#define ata_port_altstat(channel)  ata_port_ctrl(channel)

/* status register */
#define ATA_STAT_ERR 0x00
#define ATA_STAT_DRQ 0x08
#define ATA_STAT_SRV 0x10
#define ATA_STAT_DF  0x20
#define ATA_STAT_RDY 0x40
#define ATA_STAT_BSY 0x80

/* control register instructions */
#define ATA_CTRL_nIEN 0x02
#define ATA_CTRL_SRST 0x04
#define ATA_CTRL_HOB  0x80

/* commands */
#define ATA_CMD_IDENTITY    0xec
#define ATA_CMD_PIO_READ    0x20
#define ATA_CMD_PIO_WRITE   0x30
#define ATA_CMD_CACHE_FLUSH 0xe7

#define off_to_lba(ptr) ((ptr) >> 9)

/* return value */
#define ATA_POLL_ERROR     -1
#define ATA_POLL_DISKFAULT -2

enum {
    ATA_ADDRESS_CHS,
    ATA_ADDRESS_LBA28,
    ATA_ADDRESS_LBA48,
};

struct ata_device {
    u16 channel;
    u16 disk;
    u8 addrmode;
    u64 size;
    u16 nr_cylinder;
    u16 nr_head;
    u16 nr_sector_pertrack;
};

void ata_init(void);
int ata_identify_device(struct ata_device *dev);
int ata_read(struct ata_device *dev, u64 lba, u8 nr_sector,
        u8 *data);
int ata_write(struct ata_device *dev, u64 lba, u8 nr_sector,
        u8 *data);

size_t loader_disk_read(u64 diskoff, size_t n, void *buf);

#endif
