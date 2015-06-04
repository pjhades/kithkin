#include <kernel/types.h>
#include <kernel/ext2.h>
#include <kernel/ide.h>
#include <kernel/console.h>
#include <string.h>

static int loader_ext2_read_block(struct ext2_fsinfo *fs, uint32_t blkid,
        uint8_t *block)
{
    uint32_t blksz;
    uint64_t offset;

    blksz = 1024 << fs->sb.sb_log_block_size;
    offset = fs->disk_start + blkid * blksz;
    return ide_read(off_to_lba(offset), blksz >> 9, block);
}

/*
 * Find an inode according to the given inode ID
 */
static int loader_ext2_find_inode(struct ext2_fsinfo *fs, uint32_t id,
        struct ext2_inode *inode)
{
    int i;
    uint8_t block[4096];
    uint32_t grpid, blksz, blkid, inode_idx;
    struct ext2_block_group_desc *desc_table, desc;
    struct ext2_inode *inodes;

    /* Find the group descriptor table */
    blksz = 1024 << fs->sb.sb_log_block_size;
    grpid = (id - 1) / fs->sb.sb_n_inodes_per_blkgrp;
    blkid = grpid / (blksz / sizeof(struct ext2_block_group_desc));

    if (loader_ext2_read_block(fs, fs->sb.sb_first_data_block + blkid + 1,
                block) < 0)
        return -1;

    desc_table = (struct ext2_block_group_desc *)block;
    i = grpid % (blksz / sizeof(struct ext2_block_group_desc));
    memcpy(&desc, &desc_table[i], sizeof(struct ext2_block_group_desc));

    /* Find the block */
    inode_idx = (id - 1) % fs->sb.sb_n_inodes_per_blkgrp;
    blkid = (inode_idx * fs->sb.sb_inode_size) / blksz;

    if (loader_ext2_read_block(fs, desc.bg_inode_table + blkid, block) < 0)
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
static int loader_ext2_search_dir_indirect(struct ext2_fsinfo *fs,
        uint32_t blkid, const char *name, struct ext2_inode *inode,
        int level)
{
    int i, found;
    uint8_t block[4096], *p;
    uint32_t *blkids, n_blkid, blksz;
    struct ext2_direntry *entry;

    blksz = 1024 << fs->sb.sb_log_block_size;
    n_blkid = blksz >> 2;

    if (loader_ext2_read_block(fs, blkid, block) < 0)
        return -1;

    if (level <= 0) {
        p = block;
        while (p < block + blksz) {
            entry = (struct ext2_direntry *)p;
            if (entry->d_inode == 0)
                return 0;
            if (strncmp(entry->d_name, name, entry->d_name_len) == 0) {
                loader_ext2_find_inode(fs, entry->d_inode, inode);
                return 1;
            }
            p += entry->d_rec_len;
        }
        return 0;
    }

    blkids = (uint32_t *)block;
    for (i = 0; i < n_blkid; i++) {
        found = loader_ext2_search_dir_indirect(fs, blkids[i], name, inode,
                level - 1);
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
static int loader_ext2_search_dir(struct ext2_fsinfo *fs,
        struct ext2_inode *dir, const char *name,
        struct ext2_inode *inode)
{
    int i, found, level;

    if (!(dir->i_mode & EXT2_TYPE_DIR)) {
        cons_puts("Current inode is not a directory\n");
        return -1;
    }

    for (i = 0; i < EXT2_N_BLK_PTRS; i++) {
        level = i < EXT2_N_DIRECT_BLK_PTR ? 0 : i - EXT2_N_DIRECT_BLK_PTR + 1;
        found = loader_ext2_search_dir_indirect(fs, dir->i_blocks[i], name,
                inode, level);
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
int loader_ext2_find_file(struct ext2_fsinfo *fs, const char *path,
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

        found = loader_ext2_search_dir(fs, cur, name, inode);
        if (found <= 0)
            return found;

        cur = inode;
        p = q;
    }
    return found;
}

int loader_ext2_get_fsinfo(struct ext2_fsinfo *fs)
{
    if (ide_read(off_to_lba(fs->disk_start + 1024), 2, (uint8_t *)&fs->sb))
        return -1;
    if (loader_ext2_find_inode(fs, EXT2_ROOT_INODE, &fs->root_inode))
        return -1;
    return 0;
}

static int loader_ext2_read_indirect(struct ext2_fsinfo *fs, uint32_t blkid,
        int level, struct ext2_fshelp *help)
{
    int i;
    uint8_t block[4096];
    uint32_t *blkids, n_blkid, blksz, sz;

    blksz = 1024 << fs->sb.sb_log_block_size;
    n_blkid = blksz >> 2;

    if (level <= 0) {
        if (loader_ext2_read_block(fs, blkid, block) < 0)
            return -1;
        sz = blksz - help->index[0] < help->total - help->count ?
             blksz - help->index[0] : help->total - help->count;
        memcpy(help->buf, block + help->index[0], sz);
        help->index[0] = 0;
        help->count += sz;
        help->buf += sz;
        return 0;
    }

    if (loader_ext2_read_block(fs, blkid, block) < 0)
        return -1;
    blkids = (uint32_t *)block;
    i = help->index[level];
    help->index[level] = 0;
    for (; i < n_blkid; i++) {
        if (loader_ext2_read_indirect(fs, blkids[i], level - 1, help) < 0)
            return -1;
        if (help->count >= help->total)
            return 0;
    }
    return 0;
}

/*
 * Read @total bytes from @inode, starting from the very beginning
 * of the file. Put all read data in @buf. Return the bytes read.
 */
ssize_t loader_ext2_read(struct ext2_fsinfo *fs, struct ext2_inode *inode, void *buf,
        size_t total)
{
    int i, level;
    struct ext2_fshelp help = {buf, total, 0};

    for (i = 0; i < EXT2_N_BLK_PTRS; i++) {
        level = i < EXT2_N_DIRECT_BLK_PTR ? 0 : i - EXT2_N_DIRECT_BLK_PTR + 1;
        if (loader_ext2_read_indirect(fs, inode->i_blocks[i], level, &help)
                == -1)
            return -1;
        if (help.count >= total)
            return total;
    }
    return help.count;
}

/*
 * Read @total bytes from @inode, starting from byte @offset.
 * Put all read data in @buf. Return the bytes read.
 */
ssize_t loader_ext2_pread(struct ext2_fsinfo *fs, struct ext2_inode *inode,
        void *buf, size_t total, off_t offset)
{
    int i;
    uint32_t blksz, bbits, bmask, quater, qbits, fileblock;
    uint64_t filesz;
    struct ext2_fshelp help = {buf, total, 0};

    filesz = ((uint64_t)inode->i_dir_acl << 32) | inode->i_size_lo32;
    if (offset + total > filesz)
        total = filesz - offset;

    blksz = 1024 << fs->sb.sb_log_block_size;
    bbits = 10 + fs->sb.sb_log_block_size;
    bmask = blksz - 1;

    quater = blksz >> 2;
    qbits = bbits - 2;

    fileblock = offset >> bbits;

    if (fileblock < EXT2_N_DIRECT_BLK_PTR) {
        for (i = fileblock; i < EXT2_N_DIRECT_BLK_PTR; i++) {
            help.index[0] = (i == fileblock ? offset & bmask : 0);
            if (loader_ext2_read_indirect(fs, inode->i_blocks[i], 0, &help) < 0)
                return -1;
            if (help.count >= total)
                return total;
        }
        return help.count;
    }

    fileblock -= EXT2_N_DIRECT_BLK_PTR;
    offset -= EXT2_N_DIRECT_BLK_PTR * blksz;
    if (fileblock < quater) {
        help.index[1] = offset >> bbits;
        help.index[0] = offset & bmask;
        i = EXT2_1_INDIRECT_BLK_PTR;
        goto indirect;
    }

    fileblock -= quater;
    offset -= quater * blksz;
    if (fileblock < quater * quater) {
        uint64_t mask = (1LL << (bbits + qbits)) - 1;
        help.index[2] = offset >> (bbits + qbits);
        help.index[1] = (offset & mask) >> bbits;
        help.index[0] = (offset & mask) & bmask;
        i = EXT2_2_INDIRECT_BLK_PTR;
        goto indirect;
    }

    fileblock -= quater * quater;
    offset -= quater * quater * blksz;
    if (fileblock < quater * quater * quater) {
        uint64_t mask1 = (1LL << (bbits + qbits)) - 1,
                 mask2 = (1LL << (bbits*2 + qbits)) - 1;
        help.index[3] = offset >> (bbits*2 + qbits);
        help.index[2] = (offset & mask2) >> (bbits + qbits);
        help.index[1] = ((offset & mask2) & mask1) >> bbits;
        help.index[0] = ((offset & mask2) & mask1) & bmask;
        i = EXT2_3_INDIRECT_BLK_PTR;
    }

indirect:
    for (; i < EXT2_N_BLK_PTRS; i++) {
        loader_ext2_read_indirect(fs, inode->i_blocks[i],
                i - EXT2_N_DIRECT_BLK_PTR + 1, &help);
        if (help.count >= total)
            return total;
    }
    return help.count;
}
