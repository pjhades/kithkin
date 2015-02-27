#include <common.h>
#include <fs/ext2.h>
#include <driver/ide.h>
#include <driver/tty.h>
#include <lib/string.h>

static int ext2_read_block(struct ext2_fsinfo *fs, uint32_t blk_id, uint8_t *block)
{
    uint32_t blk_size;
    uint64_t offset;

    blk_size = 1024 << fs->sb.sb_log_block_size;
    offset = fs->disk_start + blk_id * blk_size;
    return ide_read(PTR2LBA(offset), blk_size >> 9, block);
}

/*
 * Find an inode according to the given inode ID
 */
static int ext2_find_inode(struct ext2_fsinfo *fs, uint32_t id,
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
    inode_idx = (id - 1) % fs->sb.sb_n_inodes_per_blkgrp;
    blk_id = (inode_idx * fs->sb.sb_inode_size) / blk_size;

    if (ext2_read_block(fs, desc.bg_inode_table + blk_id, block))
        return -1;

    inodes = (struct ext2_inode *)block;
    i = inode_idx % (blk_size / fs->sb.sb_inode_size);
    memcpy(inode, &inodes[i], sizeof(struct ext2_inode));

    return 0;
}

/*
 * Search the block @blk_id as the block of indirect level @level
 * for the file @name, store the result inode in @inode.
 */
static int ext2_search_dir_indirect(struct ext2_fsinfo *fs, uint32_t blk_id,
        const char *name, struct ext2_inode *inode, int level)
{
    int i, found;
    uint8_t block[4096], *p;
    uint32_t *blkids, n_blkid, blk_size;
    struct ext2_direntry *entry;
    
    blk_size = 1024 << fs->sb.sb_log_block_size;
    n_blkid = blk_size >> 2;

    if (ext2_read_block(fs, blk_id, block))
        return -1;

    if (level <= 0) {
        p = block;
        while (p < block + blk_size) {
            entry = (struct ext2_direntry *)p;
            if (entry->d_inode == 0)
                return 0;
            if (strncmp(entry->d_name, name, entry->d_name_len) == 0) {
                ext2_find_inode(fs, entry->d_inode, inode);
                return 1;
            }
            p += entry->d_rec_len;
        }
        return 0;
    }

    blkids = (uint32_t *)block;
    for (i = 0; i < n_blkid; i++) {
        found = ext2_search_dir_indirect(fs, blkids[i], name, inode, level - 1);
        /* continue only if not found */
        if (found)
            return found;
    }
    return 0;
}

/*
 * Search the directory @dir for @name, store the result
 * inode in @inode. Return 1 if found, 0 if not found,
 * and -1 if error occurred.
 */
static int ext2_search_dir(struct ext2_fsinfo *fs, struct ext2_inode *dir,
        const char *name, struct ext2_inode *inode)
{
    int i, found;
    uint8_t block[4096], *p;
    uint32_t blk_size = 1024 << fs->sb.sb_log_block_size;
    struct ext2_direntry *entry;

    if (!(dir->i_mode & EXT2_TYPE_DIR)) {
        cons_puts("Current inode is not a directory\n");
        return -1;
    }
    /* direct blocks */
    for (i = 0; i < EXT2_N_DIRECT_BLK_PTR; i++) {
        found = ext2_search_dir_indirect(fs, dir->i_blocks[i], name, inode, 0);
        if (found)
            return found;
    }
    /* indirect blocks */
    for (i = 1; i <= 3; i++) {
        found = ext2_search_dir_indirect(fs,
                dir->i_blocks[EXT2_N_DIRECT_BLK_PTR + i - 1], name, inode, i);
        if (found)
            return found;
    }

    return 0;
}

/*
 * Find the inode according to the given file @path.
 * Return 1 if found, 0 if not found, and -1 if error
 * occurred.
 */
int ext2_find_file(struct ext2_fsinfo *fs, const char *path,
        struct ext2_inode *inode)
{
    int i, found = 0;
    char name[EXT2_NAME_MAX];
    const char *p, *q;
    struct ext2_inode *cur;

    if (!path || path[0] != '/')
        return -1;

    p = path;
    cur = &fs->root_inode;
    while (*p) {
        for (i = 0, q = p + 1; *q && *q != '/'; q++, i++)
            name[i] = *q;
        name[i] = '\0';

        found = ext2_search_dir(fs, cur, name, inode);
        if (found <= 0)
            return found;

        cur = inode;
        p = q;
    }
    return found;
}

int ext2_get_fsinfo(struct ext2_fsinfo *fs)
{
    if (ide_read(PTR2LBA(fs->disk_start + 1024), 2, (uint8_t *)&fs->sb))
        return -1;
    if (ext2_find_inode(fs, EXT2_ROOT_INODE, &fs->root_inode))
        return -1;
    return 0;
}

static size_t ext2_read_file_indirect(struct ext2_fsinfo *fs, uint32_t blk_id,
        int level, void **buf, size_t count)
{
    int i;
    uint8_t block[4096];
    uint32_t *blkids, n_blkid, blk_size;
    size_t total = 0, n;

    blk_size = 1024 << fs->sb.sb_log_block_size;
    n_blkid = blk_size >> 2;

    if (level <= 0) {
        if (ext2_read_block(fs, blk_id, *buf))
            return -1;
        total += blk_size;
        *buf += blk_size;
        if (total >= count)
            return count;
        return total;
    }

    if (ext2_read_block(fs, blk_id, block))
        return -1;
    blkids = (uint32_t *)block;
    for (i = 0; i < n_blkid; i++) {
        n = ext2_read_file_indirect(fs, blkids[i], level - 1, buf, count - total);
        total += n;
        *buf += n;
        if (total >= count)
            return count;
    }
    return total;
}

size_t ext2_read_file(struct ext2_fsinfo *fs, struct ext2_inode *inode,
        void *buf, size_t count)
{
    int i;
    size_t total = 0, n;

    for (i = 0; i < EXT2_N_BLK_PTRS; i++) {
        n = ext2_read_file_indirect(fs, inode->i_blocks[i],
                i < EXT2_N_DIRECT_BLK_PTR ? 0 : i - EXT2_N_DIRECT_BLK_PTR + 1,
                &buf, count - total);
        total += n;
        buf += n;
        if (total >= count)
            return count;
    }
    return total;
}
