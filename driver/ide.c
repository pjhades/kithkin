#include <asm/x86.h>
#include <kernel/types.h>
#include <kernel/ide.h>

/*
 * References:
 * http://wiki.osdev.org/PCI_IDE_Controller
 * http://wiki.osdev.org/ATA_PIO_Mode
 */

struct ide_dev idv;

static int ide_poll(void)
{
    int i;
    uint8_t value;

    /* delay ~400ns */
    for (i = 0; i < 4; i++)
        inb(IDE_PRI_ALTSTAT);
    /* wait for BSY to be cleared*/
    while (inb(IDE_PRI_STAT) & IDE_STAT_BSY)
        ;
    value = inb(IDE_PRI_STAT);
    if (value & IDE_STAT_ERR) /* error */
        return IDE_POLL_ERROR;
    if (value & IDE_STAT_DF) /* disk fault */
        return IDE_POLL_DISKFAULT;
    return 0;
}

/* Do detections and preparations here later if needed. */
void ide_init(void)
{
    outb(IDE_PRI_CTRL, IDE_CTRL_nIEN); /* disable IRQ */
}

int ide_read_identity(struct ide_identity *iden)
{
    uint8_t value, id1, id2;
    int count;

    outb(IDE_PRI_DRVSEL, 0xa0); /* select primary channel master drive */
    for (count = 2500; count; count--)
        ide_poll(); /* ~1ms */
    outb(IDE_PRI_CMD, IDE_CMD_IDENTITY); /* read identity */
    for (count = 2500; count; count--)
        ide_poll(); /* ~1ms */
    if (inb(IDE_PRI_STAT) == 0)
        return -1;
    /* wait till data is ready */
    while (1) {
        value = inb(IDE_PRI_STAT);
        if (value & IDE_STAT_ERR)
            return -1;
        if (!(value & IDE_STAT_BSY) && (value & IDE_STAT_DRQ))
            break; /* data is ready */
    }
    id1 = inb(IDE_PRI_LBA1);
    id2 = inb(IDE_PRI_LBA2);
    if ((id1 == 0x14 && id2 == 0xeb) || (id1 == 0x69 && id2 == 0x96))
        return -1;
    insl(IDE_PRI_DATA, iden, 128);
    return 0;
}

/* Only support LBA28 now so that
 * lba_hi should * always be 0.
 */
int ide_read(uint64_t lba, uint8_t n_sec, uint8_t *data)
{
    uint8_t lba_bytes[6] = {0};
    int i, status;

    lba_bytes[0] = lba & 0x000000ff;
    lba_bytes[1] = (lba & 0x0000ff00) >> 8;
    lba_bytes[2] = (lba & 0x00ff0000) >> 16;

    while (inb(IDE_PRI_STAT) & IDE_STAT_BSY)
        ;
    outb(IDE_PRI_DRVSEL, 0xe0 | ((lba >> 24) & 0x0f));
    outb(IDE_PRI_SECCOUNT, n_sec);
    outb(IDE_PRI_LBA0, lba_bytes[0]);
    outb(IDE_PRI_LBA1, lba_bytes[1]);
    outb(IDE_PRI_LBA2, lba_bytes[2]);
    outb(IDE_PRI_CMD, IDE_CMD_PIO_READ);

    for (i = 0; i < n_sec; i++) {
        if (status = ide_poll())
            return status;
        insl(IDE_PRI_DATA, data, 128);
        data += 512;
    }
    return 0;
}

int ide_write(uint64_t lba, uint8_t n_sec, uint8_t *data)
{
    uint8_t lba_bytes[6] = {0};
    int i, status;

    lba_bytes[0] = lba & 0x000000ff;
    lba_bytes[1] = (lba & 0x0000ff00) >> 8;
    lba_bytes[2] = (lba & 0x00ff0000) >> 16;

    while (inb(IDE_PRI_STAT) & IDE_STAT_BSY)
        ;
    outb(IDE_PRI_DRVSEL, 0xe0 | ((lba >> 24) & 0x0f));
    outb(IDE_PRI_SECCOUNT, n_sec);
    outb(IDE_PRI_LBA0, lba_bytes[0]);
    outb(IDE_PRI_LBA1, lba_bytes[1]);
    outb(IDE_PRI_LBA2, lba_bytes[2]);
    outb(IDE_PRI_CMD, IDE_CMD_PIO_READ);

    for (i = 0; i < n_sec; i++) {
        if (status = ide_poll())
            return status;
        outsl(IDE_PRI_DATA, data, 128);
        data += 512;
    }
    outb(IDE_PRI_CMD, IDE_CMD_CACHE_FLUSH);
    ide_poll();

    return 0;
}
