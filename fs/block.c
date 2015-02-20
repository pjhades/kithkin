#include <common.h>
#include <fs/ext2.h>
#include <driver/ide.h>

int ext2_read_block(struct ext2_fsinfo *fs, uint64_t blk_id, uint8_t *block)
{
    uint32_t blk_size;
    uint64_t offset;

    blk_size = 1024 << fs->sb.sb_log_block_size;
    offset = fs->disk_start + blk_id * blk_size;
    return ide_read(PTR2LBA(offset), blk_size >> 9, block);
}

int ext2_get_fsinfo(struct ext2_fsinfo *fsinfo)
{
    if (ide_read(PTR2LBA(fsinfo->disk_start + 1024), 2, (uint8_t *)&fsinfo->sb))
        return -1;
    return 0;
}
