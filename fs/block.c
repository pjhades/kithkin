#include <common.h>
#include <fs/ext2.h>
#include <driver/ide.h>

//extern uint8_t cursor_row, cursor_col;
// TODO replace this with console functions
#if 0
static void pm_printc(char ch)
{
    if (ch == ' ') {
        if (++cursor_col == 80)
            goto newline;
        return;
    }
    if (ch == '\n') {
newline:
        ++cursor_row;
        cursor_col = 0;
        return;
    }
    if (ch == '\r') {
        cursor_col = 0;
        return;
    }
    *((uint8_t *)0x0b8000 + ((cursor_row * 80 + cursor_col) << 1)) = ch;
    *((uint8_t *)0x0b8001 + ((cursor_row * 80 + cursor_col) << 1)) = 0x0e;
    if (++cursor_col == 80) {
        cursor_col = 0;
        ++cursor_row;
    }
}

// TODO replace this with console functions
static void pm_print(char *s, int len)
{
    for (; len; s++, len--)
        pm_printc(*s);
}

// TODO replace this with console functions
static void pm_printhex(uint32_t hex, int unit)
{
    /* unit = 1, 2, 4 */
    char *table = "0123456789abcdef";
    uint32_t v32;
    uint16_t v16;
    uint8_t v8;

    if (unit == 1) {
        if (hex == 0)
            pm_print("00 ", 3);
        else {
            v32 = hex;
            while (v32) {
                v8 = v32 % 256;
                pm_printc(table[(v8 & 0xf0) >> 4]);
                pm_printc(table[v8 & 0x0f]);
                pm_printc(' ');
                v32 >>= 8;
            }
        }
    }
    else if (unit == 2) {
        if (hex == 0)
            pm_print("0000 ", 5);
        else {
            v32 = hex;
            while (v32) {
                v16 = v32 % 65536;
                pm_printc(table[(v16 & 0xf000) >> 12]);
                pm_printc(table[(v16 & 0x0f00) >> 8]);
                pm_printc(table[(v16 & 0x00f0) >> 4]);
                pm_printc(table[v16 & 0x0f]);
                pm_printc(' ');
                v32 >>= 16;
            }
        }
    }
    else {
        if (hex == 0)
            pm_print("00000000 ", 9);
        else {
            v32 = hex;
            pm_printc(table[(v32 & 0xf0000000) >> 28]);
            pm_printc(table[(v32 & 0x0f000000) >> 24]);
            pm_printc(table[(v32 & 0x00f00000) >> 20]);
            pm_printc(table[(v32 & 0x000f0000) >> 16]);
            pm_printc(table[(v32 & 0x0000f000) >> 12]);
            pm_printc(table[(v32 & 0x00000f00) >> 8]);
            pm_printc(table[(v32 & 0x000000f0) >> 4]);
            pm_printc(table[v32 & 0x0f]);
            pm_printc(' ');
        }
    }
}
#endif

int ext2_read_block(struct ext2_fsinfo *fs, uint64_t blk_id, uint8_t *block)
{
    uint32_t blk_size;
    uint64_t offset;

    blk_size = 1024 << fs->sb.sb_log_block_size;
    offset = fs->disk_start + blk_id * blk_size;
    return ide_read(PTR2LBA(offset), blk_size >> 9, block);
}

#include <lib/string.h>
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
