#ifndef __EXT2_H__
#define __EXT2_H__

#define EXT2_ROOT_INODE 2

struct ext2_superblock {
    uint32_t sb_n_inodes;
    uint32_t sb_n_blocks;
    uint32_t sb_n_blocks_reserved;
    uint32_t sb_n_free_blocks;
    uint32_t sb_n_free_inodes;
    uint32_t sb_first_data_block; /* Block number of block containing superblock */
    uint32_t sb_log_block_size; /* blocksize = 1024 << x */
    uint32_t sb_log_frag_size; /* fragsize = + ? 1024 << x : 1024 >> -x */
    uint32_t sb_n_blocks_per_blkgrp;
    uint32_t sb_n_frags_per_blkgrp;
    uint32_t sb_n_inodes_per_blkgrp;
    uint32_t sb_last_mount_time;
    uint32_t sb_last_write_time;
    uint16_t sb_mount_count;
    uint16_t sb_max_mount;
    uint16_t sb_magic;
    uint16_t sb_fs_state;
    uint16_t sb_on_error; /* Error handling methods */
    uint16_t sb_ver_minor; /* Minor version */
    uint32_t sb_last_check_time; /* Time of last consistency check */
    uint32_t sb_check_interval; /* Interval between forced consistency check */
    uint32_t sb_os_id; /* Creator OS ID */
    uint32_t sb_ver_major; /* Major version */
    uint16_t sb_uid; /* User that can use reserved blocks */
    uint16_t sb_gid; /* Group that can use reserved blocks */
    /* extended fields ... */
    uint32_t sb_first_inode; /* First non-reserved inode */
    uint32_t sb_inode_size;
    uint8_t  __unused[932];
} __attribute__((packed));

struct ext2_block_group_desc {
    uint32_t bg_block_bitmap;
    uint32_t bg_inode_bitmap;
    uint32_t bg_inode_table; /* Block address of inode table */
    uint16_t bg_n_free_blocks;
    uint16_t bg_n_free_inodes;
    uint16_t bg_n_dirs;
    uint16_t __pad;
    uint32_t __reserved[3];
} __attribute__((packed));

#define EXT2_N_DIRECT_BLK_PTR 12
#define EXT2_1_INDIRECT_BLK_PTR EXT2_N_DIRECT_BLK_PTR
#define EXT2_2_INDIRECT_BLK_PTR (EXT2_1_INDIRECT_BLK_PTR + 1)
#define EXT2_3_INDIRECT_BLK_PTR (EXT2_2_INDIRECT_BLK_PTR + 1)
#define EXT2_N_BLK_PTRS (EXT2_3_INDIRECT_BLK_PTR + 1)

struct ext2_inode {
    uint16_t i_mode;
    uint16_t i_uid;
    uint32_t i_size_lo32;
    uint32_t i_atime;
    uint32_t i_ctime;
    uint32_t i_mtime;
    uint32_t i_dtime;
    uint16_t i_gid;
    uint16_t i_n_links;
    uint32_t i_n_sectors;
    uint32_t i_flags;
    uint32_t i_os_value1;
    uint32_t i_blocks[EXT2_N_BLK_PTRS];
    uint32_t i_generation;
    uint32_t i_file_acl;
    uint32_t i_dir_acl;
    uint32_t i_frag_blk;
    uint32_t i_os_value2[3];
} __attribute__((packed));

struct ext2_fsinfo {
    struct ext2_superblock sb;
    struct ext2_inode root;
    uint64_t disk_start;
};

int ext2_read_block(struct ext2_fsinfo *fs, uint64_t blk_id, uint8_t *block);
int ext2_get_fsinfo(struct ext2_fsinfo *fs);

#endif
