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

int ext2_find_inode(struct ext2_fsinfo *fs, uint32_t id,
        struct ext2_inode *inode)
{
    int i;
    uint8_t block[4096];
    uint32_t n_blkgrps, grp_id, blk_size, blk_id, inode_idx;
    struct ext2_block_group_desc *desc_table, desc;
    struct ext2_inode *inodes;

    /* Find the group descriptor table */
    blk_size = 1024 << fs->sb.sb_log_block_size;
    n_blkgrps = (fs->sb.sb_n_blocks + fs->sb.sb_n_blocks_per_blkgrp - 1)
        / fs->sb.sb_n_blocks_per_blkgrp;
    grp_id = (id - 1) / fs->sb.sb_n_inodes_per_blkgrp;
    blk_id = grp_id / (blk_size / sizeof(struct ext2_block_group_desc));

    if (ext2_read_block(fs, fs->sb.sb_first_data_block + blk_id + 1,
                block))
        return -1;

    desc_table = (struct ext2_block_group_desc *)block;
    i = grp_id % (blk_size / sizeof(struct ext2_block_group_desc));
    memcpy(&desc, &desc_table[i], sizeof(struct ext2_block_group_desc));

    /* Find the block */
    inode_idx = (EXT2_ROOT_INODE - 1) % fs->sb.sb_n_inodes_per_blkgrp;
    blk_id = (inode_idx * fs->sb.sb_inode_size) / blk_size;

    if (ext2_read_block(fs, desc.bg_inode_table + blk_id, block))
        return -1;

    inodes = (struct ext2_inode *)block;
    i = inode_idx % (blk_size / fs->sb.sb_inode_size);
    memcpy(inode, &inodes[i], sizeof(struct ext2_inode));
    return 0;
}

//int ext2_find_file(struct ext2_fsinfo *fs, const char *path,
//        struct ext2_inode *inode)
//{
//    int i, found = 0;
//    char *p, *q, name[EXT2_NAME_MAX];
//    struct ext2_inode *cur;
//
//    if (!path || path[0] != '/')
//        return -1;
//
//    p = path + 1;
//    for (i = 0, q = p; *q && *q != '/'; q++, i++)
//        name[i] = *q;
//    name[i] = '\0';
//
//    cur = fs->root_inode;
//    while (!found) {
//    }
//
//    return -1;
//}

int ext2_get_fsinfo(struct ext2_fsinfo *fs)
{
    if (ide_read(PTR2LBA(fs->disk_start + 1024), 2, (uint8_t *)&fs->sb))
        return -1;
    if (ext2_find_inode(fs, EXT2_ROOT_INODE, &fs->root_inode))
        return -1;
    return 0;
}
