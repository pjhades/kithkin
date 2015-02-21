#include <common.h>
#include <fs/ext2.h>
#include <driver/ide.h>
#include <lib/string.h>

int ext2_read_block(struct ext2_fsinfo *fs, uint64_t blk_id, uint8_t *block)
{
    uint32_t blk_size;
    uint64_t offset;

    blk_size = 1024 << fs->sb.sb_log_block_size;
    offset = fs->disk_start + blk_id * blk_size;
    return ide_read(PTR2LBA(offset), blk_size >> 9, block);
}

int ext2_get_root(struct ext2_fsinfo *fsinfo)
{
    int i;
    uint8_t block[4096];
    uint32_t n_blkgrps, grp_id, blk_size, n_desc_blks, blk_id, inode_idx;
    struct ext2_block_group_desc *desc_table, desc;
    struct ext2_inode *inodes;

    /* Find the group descriptor table for the descriptor
     * of the group where the inode of the root directory resides.
     */
    blk_size = 1024 << fsinfo->sb.sb_log_block_size;
    n_blkgrps = (fsinfo->sb.sb_n_blocks + fsinfo->sb.sb_n_blocks_per_blkgrp - 1)
        / fsinfo->sb.sb_n_blocks_per_blkgrp;
    grp_id = (EXT2_ROOT_INODE - 1) / fsinfo->sb.sb_n_inodes_per_blkgrp;
    /* blk_id is the block ID starting from the descriptor table,
     * so we add 1 for the super block
     */
    blk_id = grp_id / (blk_size / sizeof(struct ext2_block_group_desc));

    if (ext2_read_block(fsinfo, fsinfo->sb.sb_first_data_block + blk_id + 1,
                block))
        return -1;

    desc_table = (struct ext2_block_group_desc *)block;
    i = grp_id % (blk_size / sizeof(struct ext2_block_group_desc));
    memcpy(&desc, &desc_table[i], sizeof(struct ext2_block_group_desc));

    /* Find the block which contains the inode
     * of the root directory.
     */
    inode_idx = (EXT2_ROOT_INODE - 1) % fsinfo->sb.sb_n_inodes_per_blkgrp;
    blk_id = (inode_idx * fsinfo->sb.sb_inode_size) / blk_size;
    if (ext2_read_block(fsinfo, desc.bg_inode_table + blk_id, block))
        return -1;

    inodes = (struct ext2_inode *)block;
    i = inode_idx % (blk_size / fsinfo->sb.sb_inode_size);
    memcpy(&fsinfo->root_inode, &inodes[i], sizeof(struct ext2_inode));

    return 0;
}

int ext2_get_fsinfo(struct ext2_fsinfo *fsinfo)
{
    if (ide_read(PTR2LBA(fsinfo->disk_start + 1024), 2, (uint8_t *)&fsinfo->sb))
        return -1;
    if (ext2_get_root(fsinfo))
        return -1;
    return 0;
}
