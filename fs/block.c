#include <common.h>
#include <fs/ext2.h>
#include <driver/ide.h>

int ext2_read_block(struct ext2_superblock *sb, uint64_t offset, uint8_t *block)
{
    return ide_read(PTR2LBA(offset), (1024 << sb->sb_log_block_size) >> 9, block);
}
