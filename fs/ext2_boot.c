#include <common.h>
#include <fs/ext2.h>
#include <driver/ide.h>
#include <driver/tty.h>
#include <lib/string.h>

static int ext2_read_block(struct ext2_fsinfo *fs, uint32_t blkid, uint8_t *block)
{
    uint32_t blksz;
    uint64_t offset;

    blksz = 1024 << fs->sb.sb_log_block_size;
    offset = fs->disk_start + blkid * blksz;
    return ide_read(PTR2LBA(offset), blksz >> 9, block);
}

/*
 * Find an inode according to the given inode ID
 */
static int ext2_find_inode(struct ext2_fsinfo *fs, uint32_t id,
        struct ext2_inode *inode)
{
    int i;
    uint8_t block[4096];
    uint32_t n_grps, grpid, blksz, blkid, inode_idx;
    struct ext2_block_group_desc *desc_table, desc;
    struct ext2_inode *inodes;

    /* Find the group descriptor table */
    blksz = 1024 << fs->sb.sb_log_block_size;
    n_grps = (fs->sb.sb_n_blocks + fs->sb.sb_n_blocks_per_blkgrp - 1)
        / fs->sb.sb_n_blocks_per_blkgrp;
    grpid = (id - 1) / fs->sb.sb_n_inodes_per_blkgrp;
    blkid = grpid / (blksz / sizeof(struct ext2_block_group_desc));

    if (ext2_read_block(fs, fs->sb.sb_first_data_block + blkid + 1,
                block))
        return -1;

    desc_table = (struct ext2_block_group_desc *)block;
    i = grpid % (blksz / sizeof(struct ext2_block_group_desc));
    memcpy(&desc, &desc_table[i], sizeof(struct ext2_block_group_desc));

    /* Find the block */
    inode_idx = (id - 1) % fs->sb.sb_n_inodes_per_blkgrp;
    blkid = (inode_idx * fs->sb.sb_inode_size) / blksz;

    if (ext2_read_block(fs, desc.bg_inode_table + blkid, block))
        return -1;

    inodes = (struct ext2_inode *)block;
    i = inode_idx % (blksz / fs->sb.sb_inode_size);
    memcpy(inode, &inodes[i], sizeof(struct ext2_inode));

    return 0;
}

/*
 * Search the block @blkid as the block of indirect level @level
 * for the file @name, store the result inode in @inode.
 */
static int ext2_search_dir_indirect(struct ext2_fsinfo *fs, uint32_t blkid,
        const char *name, struct ext2_inode *inode, int level)
{
    int i, found;
    uint8_t block[4096], *p;
    uint32_t *blkids, n_blkid, blksz;
    struct ext2_direntry *entry;
    
    blksz = 1024 << fs->sb.sb_log_block_size;
    n_blkid = blksz >> 2;

    if (ext2_read_block(fs, blkid, block))
        return -1;

    if (level <= 0) {
        p = block;
        while (p < block + blksz) {
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
    int i, found, level;
    uint8_t block[4096], *p;
    uint32_t blksz = 1024 << fs->sb.sb_log_block_size;
    struct ext2_direntry *entry;

    if (!(dir->i_mode & EXT2_TYPE_DIR)) {
        cons_puts("Current inode is not a directory\n");
        return -1;
    }

    for (i = 0; i < EXT2_N_BLK_PTRS; i++) {
        level = i < EXT2_N_DIRECT_BLK_PTR ? 0 : i - EXT2_N_DIRECT_BLK_PTR + 1;
        found = ext2_search_dir_indirect(fs, dir->i_blocks[i], name, inode, level);
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
int boot_ext2_find_file(struct ext2_fsinfo *fs, const char *path,
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

int boot_ext2_get_fsinfo(struct ext2_fsinfo *fs)
{
    if (ide_read(PTR2LBA(fs->disk_start + 1024), 2, (uint8_t *)&fs->sb))
        return -1;
    if (ext2_find_inode(fs, EXT2_ROOT_INODE, &fs->root_inode))
        return -1;
    return 0;
}

static int ext2_read_indirect(struct ext2_fsinfo *fs, uint32_t blkid,
        int level, struct ext2_fshelp *help)
{
    int i;
    uint8_t block[4096];
    uint32_t *blkids, n_blkid, blksz, sz;

    blksz = 1024 << fs->sb.sb_log_block_size;
    n_blkid = blksz >> 2;

    if (level <= 0) {
        if (ext2_read_block(fs, blkid, block))
            return -1;
        sz = blksz - help->index[0] < help->count - help->total ?
             blksz - help->index[0] : help->count - help->total;
        memcpy(help->buf, block + help->index[0], sz);
        help->index[0] = 0;
        help->total += sz;
        help->buf += sz;
        return 0;
    }

    if (ext2_read_block(fs, blkid, block))
        return -1;
    blkids = (uint32_t *)block;
    i = help->index[level];
    help->index[level] = 0;
    for (; i < n_blkid; i++) {
        ext2_read_indirect(fs, blkids[i], level - 1, help);
        if (help->total >= help->count)
            return 0;
    }
    return 0;
}

/*
 * Read @count bytes from @inode, starting from the very beginning
 * of the file. Put all read data in @buf. Return the bytes read.
 */
ssize_t boot_ext2_read(struct ext2_fsinfo *fs, struct ext2_inode *inode, void *buf,
        size_t count)
{
    int i, level;
    struct ext2_fshelp help = {buf, count, 0};

    for (i = 0; i < EXT2_N_BLK_PTRS; i++) {
        level = i < EXT2_N_DIRECT_BLK_PTR ? 0 : i - EXT2_N_DIRECT_BLK_PTR + 1;
        if (ext2_read_indirect(fs, inode->i_blocks[i], level, &help) == -1)
            return -1;
        if (help.total >= count)
            return count;
    }
    return help.total;
}

/*
 * Read @count bytes from @inode, starting from byte @offset.
 * Put all read data in @buf. Return the bytes read.
 */
ssize_t boot_ext2_pread(struct ext2_fsinfo *fs, struct ext2_inode *inode, void *buf,
        size_t count, off_t offset)
{
    int i, off, level;
    size_t total = 0, n;
    uint32_t blksz, n_blkid;
    struct ext2_fshelp help = {buf, count, 0};

    blksz = 1024 << fs->sb.sb_log_block_size;
    n_blkid = blksz >> 2;

    if (offset >= 0 && offset < EXT2_N_DIRECT_BLK_PTR * blksz) {
        for (i = offset / blksz; i < EXT2_N_BLK_PTRS; i++) {
            help.index[0] = offset % blksz;
            level = i < EXT2_N_DIRECT_BLK_PTR ? 0 : i - EXT2_N_DIRECT_BLK_PTR + 1;
            ext2_read_indirect(fs, inode->i_blocks[i], level, &help);
            if (help.total >= count)
                return count;
        }
        return help.total;
    }

    offset -= EXT2_N_DIRECT_BLK_PTR * blksz;
    if (offset >= 0 && offset < n_blkid * blksz) {
        help.index[1] = offset / blksz;
        help.index[0] = offset % blksz;
        i = EXT2_1_INDIRECT_BLK_PTR;
        goto indirect;
    }
    offset -= n_blkid * blksz;
    if (offset >= 0 && offset < n_blkid * n_blkid * blksz) {
        help.index[2] = offset / (n_blkid * blksz);
        help.index[1] = offset % (n_blkid * blksz) / blksz;
        help.index[0] = offset % (n_blkid * blksz) % blksz;
        i = EXT2_2_INDIRECT_BLK_PTR;
        goto indirect;
    }
    offset -= n_blkid * n_blkid * blksz;
    if (offset >= 0 && offset < n_blkid * n_blkid * n_blkid) {
        help.index[3] = offset / (n_blkid * n_blkid * blksz);
        help.index[2] = offset % (n_blkid * n_blkid * blksz) / (n_blkid * blksz);
        help.index[1] = offset % (n_blkid * n_blkid * blksz) % (n_blkid * blksz) / blksz;
        help.index[0] = offset % (n_blkid * n_blkid * blksz) % (n_blkid * blksz) % blksz;
        i = EXT2_3_INDIRECT_BLK_PTR;
    }
indirect:
    for (; i < EXT2_N_BLK_PTRS; i++) {
        ext2_read_indirect(fs, inode->i_blocks[i],
                i - EXT2_N_DIRECT_BLK_PTR + 1, &help);
        if (help.total >= count)
            return count;
    }
    return help.total;
}
