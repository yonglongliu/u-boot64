# Created by bran.zhang <bran.zhang@spreadtrum.com>


LWCFLAGS = -I$(TOPDIR)/fs/lwext4/include
CFLAGS += $(LWCFLAGS)
CPPFLAGS += $(LWCFLAGS)

obj-y :=  blockdev/blockdev.o src/ext4_blockdev.o src/ext4_debug.o\
		src/ext4.o src/ext4_fs.o src/ext4_bcache.o src/ext4_super.o\
		src/ext4_crc32.o src/ext4_journal.o src/ext4_inode.o src/ext4_mkfs.o\
		src/ext4_dir.o src/ext4_dir_idx.o src/ext4_extent.o src/ext4_trans.o\
		src/ext4_hash.o src/ext4_balloc.o src/ext4_block_group.o src/ext4_bitmap.o\
		src/ext4_ialloc.o lwext4fs.o