#ifndef __EXT2_H__
#define __EXT2_H__

#define EXT2_ROOT_INODE 2
#define EXT2_NAME_MAX 255

#define EXT2_SUPERBLOCK_SIZE 1024

struct ext2_superblock {
    u32 sb_n_inodes;
    u32 sb_n_blocks;
    u32 sb_n_blocks_reserved;
    u32 sb_n_free_blocks;
    u32 sb_n_free_inodes;
    u32 sb_first_data_block; /* Block number of block containing superblock */
    u32 sb_log_block_size;   /* blocksize = 1024 << x */
    u32 sb_log_frag_size;    /* fragsize = + ? 1024 << x : 1024 >> -x */
    u32 sb_n_blocks_per_blkgrp;
    u32 sb_n_frags_per_blkgrp;
    u32 sb_n_inodes_per_blkgrp;
    u32 sb_last_mount_time;
    u32 sb_last_write_time;
    u16 sb_mount_count;
    u16 sb_max_mount;
    u16 sb_magic;
    u16 sb_fs_state;
    u16 sb_on_error; /* Error handling methods */
    u16 sb_ver_minor; /* Minor version */
    u32 sb_last_check_time; /* Time of last consistency check */
    u32 sb_check_interval; /* Interval between forced consistency check */
    u32 sb_os_id; /* Creator OS ID */
    u32 sb_ver_major; /* Major version */
    u16 sb_uid; /* User that can use reserved blocks */
    u16 sb_gid; /* Group that can use reserved blocks */
    /* extended fields ... */
    u32 sb_first_inode; /* First non-reserved inode */
    u16 sb_inode_size;
    u8  __unused[934];
} __attribute__((packed));

struct ext2_block_group_desc {
    u32 bg_block_bitmap;
    u32 bg_inode_bitmap;
    u32 bg_inode_table; /* Block address of inode table */
    u16 bg_n_free_blocks;
    u16 bg_n_free_inodes;
    u16 bg_n_dirs;
    u16 __pad;
    u32 __reserved[3];
} __attribute__((packed));

#define EXT2_N_DIRECT_BLK_PTR 12
#define EXT2_1_INDIRECT_BLK_PTR EXT2_N_DIRECT_BLK_PTR
#define EXT2_2_INDIRECT_BLK_PTR (EXT2_1_INDIRECT_BLK_PTR + 1)
#define EXT2_3_INDIRECT_BLK_PTR (EXT2_2_INDIRECT_BLK_PTR + 1)
#define EXT2_N_BLK_PTRS (EXT2_3_INDIRECT_BLK_PTR + 1)

#define EXT2_TYPE_SOCK 0xc000
#define EXT2_TYPE_LINK 0xa000
#define EXT2_TYPE_REG  0x8000
#define EXT2_TYPE_BLK  0x6000
#define EXT2_TYPE_DIR  0x4000
#define EXT2_TYPE_CHR  0x2000
#define EXT2_TYPE_FIFO 0x1000

struct ext2_inode {
    u16 i_mode;
    u16 i_uid;
    u32 i_size_lo32;
    u32 i_atime;
    u32 i_ctime;
    u32 i_mtime;
    u32 i_dtime;
    u16 i_gid;
    u16 i_n_links;
    u32 i_n_sectors;
    u32 i_flags;
    u32 i_os_value1;
    u32 i_blocks[EXT2_N_BLK_PTRS];
    u32 i_generation;
    u32 i_file_acl;
    u32 i_dir_acl;
    u32 i_frag_blk;
    u32 i_os_value2[3];
} __attribute__((packed));

struct ext2_direntry {
    u32 d_inode;
    u16 d_rec_len;
    u8  d_name_len;
    u8  d_type;
    char     d_name[];
};

struct ext2_fsinfo {
    struct ext2_superblock sb;
    struct ext2_inode root_inode;
    u64 disk_start;
};

/*
 * Hold information we need to pass along the way we
 * read the blocks of an inode.
 */
struct ext2_fshelp {
    void *buf;
    size_t total; /* number of bytes to read */
    size_t count; /* number of bytes we've read */
    int index[4]; /* start offset in each level of indirect block */
};

int loader_ext2_get_fsinfo(struct ext2_fsinfo *fs);
int loader_ext2_find_file(struct ext2_fsinfo *fs, const char *path,
        struct ext2_inode *inode);
ssize_t loader_ext2_read(struct ext2_fsinfo *fs, struct ext2_inode *inode, void *buf,
        size_t count);
ssize_t loader_ext2_pread(struct ext2_fsinfo *fs, struct ext2_inode *inode, void *buf,
        size_t count, off_t offset);

#endif
