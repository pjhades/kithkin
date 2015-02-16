#ifndef __IDE_H__
#define __IDE_H__

/* define but ignore the secondary channel */
#define IDE_PRI_DATA     0x1f0
#define IDE_PRI_ERROR    0x1f1
#define IDE_PRI_FEATURE  0x1f1
#define IDE_PRI_SECCOUNT 0x1f2
#define IDE_PRI_LBA0     0x1f3
#define IDE_PRI_LBA1     0x1f4
#define IDE_PRI_LBA2     0x1f5
#define IDE_PRI_DRVSEL   0x1f6
#define IDE_PRI_CMD      0x1f7
#define IDE_PRI_STAT     0x1f7

#define IDE_SEC_DATA     0x170
#define IDE_SEC_ERROR    0x171
#define IDE_SEC_FEATURE  0x171
#define IDE_SEC_SECCOUNT 0x172
#define IDE_SEC_LBALO    0x173
#define IDE_SEC_LBAMI    0x174
#define IDE_SEC_LBAHI    0x175
#define IDE_SEC_DRVSEL   0x176
#define IDE_SEC_CMD      0x177
#define IDE_SEC_STAT     0x177

#define IDE_PRI_CTRL    0x3f6
#define IDE_PRI_ALTSTAT IDE_PRI_CTRL
#define IDE_SEC_CTRL    0x376
#define IDE_SEC_ALTSTAT IDE_SEC_CTRL

/* status register */
#define IDE_STAT_ERR 0x00
#define IDE_STAT_DRQ 0x08
#define IDE_STAT_SRV 0x10
#define IDE_STAT_DF  0x20
#define IDE_STAT_RDY 0x40
#define IDE_STAT_BSY 0x80

/* control register instructions */
#define IDE_CTRL_nIEN 0x02
#define IDE_CTRL_SRST 0x04
#define IDE_CTRL_HOB  0x80

/* commands */
#define IDE_CMD_IDENTITY    0xec
#define IDE_CMD_PIO_READ    0x20
#define IDE_CMD_PIO_WRITE   0x30
#define IDE_CMD_CACHE_FLUSH 0xe7

/* Ignore most of the fields */
struct ide_identity {
    uint16_t config;
    uint8_t  __ignore1[18];
    uint8_t  serial_no[20];
    uint8_t  __ignore2[6];
    uint8_t  fw_rev[8];
    uint8_t  model[40];
    uint8_t  __ignore3[72];
    uint16_t command_set;
    uint8_t  __ignore4[342];
    uint16_t integrity;
} __attribute__((packed));

struct ide_dev {
    struct ide_identity iden;
};

void ide_init(void);
int ide_read_identity(struct ide_identity *iden);
int ide_read(uint64_t lba, uint8_t n_sec, uint8_t *data);
int ide_write(uint64_t lba, uint8_t n_sec, uint8_t *data);

#endif
