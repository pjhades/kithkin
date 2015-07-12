#include <string.h>
#include <ide.h>
#include <asm.h>
#include <kernel/types.h>

struct ata_device disk_primaster = {.channel=ATA_PRIMARY, .disk=ATA_MASTER},
                  disk_prislave  = {.channel=ATA_PRIMARY, .disk=ATA_SLAVE},
                  disk_secmaster = {.channel=ATA_SECONDARY, .disk=ATA_MASTER},
                  disk_secslave  = {.channel=ATA_SECONDARY, .disk=ATA_SLAVE};

static struct ata_device *devlist[] = {
    &disk_primaster,
    &disk_prislave,
    &disk_secmaster,
    &disk_secslave,
    NULL
};

/*
 * References:
 *   http://wiki.osdev.org/PCI_ATA_Controller
 *   http://wiki.osdev.org/ATA_PIO_Mode
 */

static int ata_poll(struct ata_device *dev)
{
    int i;
    u8 value;

    /* delay ~400ns */
    for (i = 0; i < 4; i++)
        inb(ata_port_altstat(dev->channel));
    /* wait for BSY to be cleared*/
    while (inb(ata_port_altstat(dev->channel)) & ATA_STAT_BSY)
        ;
    value = inb(ata_port_stat(dev->channel));
    if (value & ATA_STAT_ERR) /* error */
        return ATA_POLL_ERROR;
    if (value & ATA_STAT_DF) /* disk fault */
        return ATA_POLL_DISKFAULT;
    return 0;
}

/* Do detections and preparations here later if needed. */
void ata_init(void)
{
    int i;

    for (i = 0; devlist[i]; i++) {
        /* disable IRQ */
        // outb(ata_port_ctrl(dev->channel), ATA_CTRL_nIEN);
        ata_identify_device(devlist[i]);
    }
}

int ata_identify_device(struct ata_device *dev)
{
    int count;
    u8 value, lba1, lba2;
    u16 info16[256];
    u32 *info32;

    if (dev->channel != ATA_PRIMARY && dev->channel != ATA_SECONDARY)
        return -1;

    outb(ata_port_drvsel(dev->channel), dev->disk);
    outb(ata_port_seccount(dev->channel), 0);
    outb(ata_port_lba0(dev->channel), 0);
    outb(ata_port_lba1(dev->channel), 0);
    outb(ata_port_lba2(dev->channel), 0);
    for (count = 2500; count; count--)
        ata_poll(dev); /* ~1ms */

    /* read identity */
    outb(ata_port_cmd(dev->channel), ATA_CMD_IDENTITY); 
    for (count = 2500; count; count--)
        ata_poll(dev); /* ~1ms */

    /* drive does not exist */
    if (inb(ata_port_stat(dev->channel)) == 0)
        return -1;

    /* wait till data is ready */
    while (1) {
        value = inb(ata_port_stat(dev->channel));
        if (value & ATA_STAT_ERR)
            return -1;
        if (!(value & ATA_STAT_BSY) && (value & ATA_STAT_DRQ))
            break;
        /* check if not ATA device */
        lba1 = inb(ata_port_lba1(dev->channel));
        lba2 = inb(ata_port_lba2(dev->channel));
        if ((lba1 == 0x14 && lba2 == 0xeb)
                || (lba1 == 0x69 && lba2 == 0x96))
            return -1;
    }

    insl(ata_port_data(dev->channel), info16, 128);
    info32 = (u32 *)info16;

    /* TODO
     * LBA28 is forced here. We should set the fallback to CHS
     * and check if LBA28 and LBA48 is supported, then do the
     * communication accordingly.
     *
     * Print warning messages.
     */

    /*
     * References:
     *   grub source code: grub-core/disk/ata.c
     */
    dev->addrmode = ATA_ADDRESS_LBA28;
    if (!(info16[49] & (1 << 9)))
        return -1;
    dev->size = (u64)info32[30];
    dev->n_cylinder = info16[1];
    dev->n_head = info16[3];
    dev->n_sector_pertrack = info16[6];

    return 0;
}

/* Only support LBA28 now so that
 * lba_hi should * always be 0.
 */
int ata_read(struct ata_device *dev, u64 lba, u8 n_sector,
        u8 *data)
{
    u8 lba_bytes[6] = {0};
    int i, status;

    lba_bytes[0] = lba & 0x000000ff;
    lba_bytes[1] = (lba & 0x0000ff00) >> 8;
    lba_bytes[2] = (lba & 0x00ff0000) >> 16;

    while (inb(ata_port_stat(dev->channel)) & ATA_STAT_BSY)
        ;
    outb(ata_port_drvsel(dev->channel), 0xe0 | ((lba >> 24) & 0x0f));
    outb(ata_port_seccount(dev->channel), n_sector);
    outb(ata_port_lba0(dev->channel), lba_bytes[0]);
    outb(ata_port_lba1(dev->channel), lba_bytes[1]);
    outb(ata_port_lba2(dev->channel), lba_bytes[2]);
    outb(ata_port_cmd(dev->channel), ATA_CMD_PIO_READ);

    for (i = 0; i < n_sector; i++) {
        if ((status = ata_poll(dev)))
            return status;
        insl(ata_port_data(dev->channel), data, 128);
        data += 512;
    }
    return 0;
}

int ata_write(struct ata_device *dev, u64 lba, u8 n_sector,
        u8 *data)
{
    u8 lba_bytes[6] = {0};
    int i, status;

    lba_bytes[0] = lba & 0x000000ff;
    lba_bytes[1] = (lba & 0x0000ff00) >> 8;
    lba_bytes[2] = (lba & 0x00ff0000) >> 16;

    while (inb(ata_port_stat(dev->channel)) & ATA_STAT_BSY)
        ;
    outb(ata_port_drvsel(dev->channel), 0xe0 | ((lba >> 24) & 0x0f));
    outb(ata_port_seccount(dev->channel), n_sector);
    outb(ata_port_lba0(dev->channel), lba_bytes[0]);
    outb(ata_port_lba1(dev->channel), lba_bytes[1]);
    outb(ata_port_lba2(dev->channel), lba_bytes[2]);
    outb(ata_port_cmd(dev->channel), ATA_CMD_PIO_READ);

    for (i = 0; i < n_sector; i++) {
        if ((status = ata_poll(dev)))
            return status;
        outsl(ata_port_data(dev->channel), data, 128);
        data += 512;
    }
    outb(ata_port_cmd(dev->channel), ATA_CMD_CACHE_FLUSH);
    ata_poll(dev);

    return 0;
}

/* TODO
 * here n must be a multiple of the sector size
 */
size_t loader_disk_read(u64 diskoff, size_t n, void *buf)
{
    return ata_read(&disk_primaster, off_to_lba(diskoff),
            (n >> ATA_SECTOR_BITS), buf);
}

