/*
 * Copyright (c) 2012, NVIDIA CORPORATION.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <config.h>
#include <common.h>
#include <part.h>
#include <ext4fs.h>
#include <fat.h>
#include <fs.h>
#include <sandboxfs.h>
#include <asm/io.h>
#include <lwext4fs.h>

DECLARE_GLOBAL_DATA_PTR;

static block_dev_desc_t *fs_dev_desc;
static disk_partition_t fs_partition;
static int fs_type = FS_TYPE_ANY;

static inline int fs_probe_unsupported(block_dev_desc_t *fs_dev_desc,
				      disk_partition_t *fs_partition)
{
	printf("** Unrecognized filesystem type **\n");
	return -1;
}

static inline int fs_ls_unsupported(const char *dirname)
{
	return -1;
}

static inline int fs_read_unsupported(const char *filename, void *buf,
				      loff_t offset, loff_t len)
{
	return -1;
}

static inline int fs_write_unsupported(const char *filename, void *buf,
				      loff_t offset, loff_t len)
{
	return -1;
}

static inline void fs_close_unsupported(void)
{
}

static inline int fs_mkdir_unsupported(const char *path)
{
	return -1;
}

static inline int fs_delete_unsupported(const char *path)
{
	return -1;
}

static inline int fs_dir_stat_unsupported(const char *dirname)
{
	return -1;
}

static inline int fs_file_stat_unsupported(const char *dirname)
{
	return -1;
}

static inline int fs_dir_rename_unsupported(const char *path, const char *new_path)
{
	return -1;
}

static inline unsigned long long fs_available_size_unsupported(void)
{
	return 0;
}

struct fstype_info {
	int fstype;
	int (*probe)(block_dev_desc_t *fs_dev_desc,
		     disk_partition_t *fs_partition);
	int (*ls)(const char *dirname);
	int (*read)(const char *filename, void *buf, loff_t offset, loff_t len);
	int (*write)(const char *filename, void *buf, loff_t offset, loff_t len);
	int (*mkdir)(const char *path);
	int (*delete)(const char *path);
	int (*dir_stat)(const char *dirname);
	int (*file_stat)(const char *dirname);
	int (*dir_rename)(const char *path, const char *new_path);
	unsigned long long(*available_size)(void);
	void (*close)(void);
};

static struct fstype_info fstypes[] = {
#ifdef CONFIG_FS_FAT
	{
		.fstype = FS_TYPE_FAT,
		.probe = fat_set_blk_dev,
		.close = fat_close,
		.ls = file_fat_ls,
		.read = fat_read_file,
		.write =  fat_write_file,
		.mkdir = do_new_folder,
		.delete = delete_folder,
		.dir_stat = check_stat,
		.file_stat = check_stat,
		.dir_rename = rename_folder,
		.available_size = fat_available_size,
	},
#endif
#ifdef CONFIG_FS_EXT4
	{
		.fstype = FS_TYPE_EXT,
		.probe = ext4fs_probe,
		.close = ext4fs_close,
		.ls = ext4fs_ls,
		.read = ext4_read_file,
	},
#endif
#ifdef CONFIG_FS_LWEXT4
	{
		.fstype = FS_TYPE_LWEXT4,
		.probe = lwext4_probe,
		.write = lwext4_write_to_mem,
		.mkdir = lwext4_dir_mk,
		.delete = lwext4_dremove,
		.dir_stat = lwext4_dir_stat,
		.file_stat = lwext4_file_stat,
		.dir_rename = lwext4_drename,
		.close = lwext4_close,
		.ls = lwext4_ls,
		.available_size = lwext4_getsize,
	},
#endif
#ifdef CONFIG_SANDBOX
	{
		.fstype = FS_TYPE_SANDBOX,
		.probe = sandbox_fs_set_blk_dev,
		.close = sandbox_fs_close,
		.ls = sandbox_fs_ls,
		.read = fs_read_sandbox,
		.write = fs_write_sandbox,
	},
#endif
	{
		.fstype = FS_TYPE_ANY,
		.probe = fs_probe_unsupported,
		.close = fs_close_unsupported,
		.ls = fs_ls_unsupported,
		.read = fs_read_unsupported,
		.write = fs_write_unsupported,
		.mkdir = fs_mkdir_unsupported,
		.delete = fs_delete_unsupported,
		.dir_stat = fs_dir_stat_unsupported,
		.file_stat = fs_file_stat_unsupported,
		.dir_rename = fs_dir_rename_unsupported,
		.available_size = fs_available_size_unsupported
	},
};

static struct fstype_info *fs_get_info(int fstype)
{
	struct fstype_info *info;
	int i;

	for (i = 0, info = fstypes; i < ARRAY_SIZE(fstypes) - 1; i++, info++) {
		if (fstype == info->fstype)
			return info;
	}

	/* Return the 'unsupported' sentinel */
	return info;
}

int fs_set_blk_dev(const char *ifname, const char *dev_part_str, int fstype)
{
	struct fstype_info *info;
	int part, i;
#ifdef CONFIG_NEEDS_MANUAL_RELOC
	static int relocated;

	if (!relocated) {
		for (i = 0, info = fstypes; i < ARRAY_SIZE(fstypes);
				i++, info++) {
			info->probe += gd->reloc_off;
			info->close += gd->reloc_off;
			info->ls += gd->reloc_off;
			info->read += gd->reloc_off;
			info->write += gd->reloc_off;
		}
		relocated = 1;
	}
#endif

	part = get_device_and_partition(ifname, dev_part_str, &fs_dev_desc,
					&fs_partition, 1);
	if (part < 0)
		return -1;

	for (i = 0, info = fstypes; i < ARRAY_SIZE(fstypes); i++, info++) {
		if (fstype != FS_TYPE_ANY && info->fstype != FS_TYPE_ANY &&
				fstype != info->fstype)
			continue;

		if (!info->probe(fs_dev_desc, &fs_partition)) {
			fs_type = info->fstype;
			return 0;
		}
	}

	return -1;
}

int fs_set_blk_by_name(const char *ifname,int dev, uchar * partition_name, int fstype)
{
	struct fstype_info *info;
	int i;
	fs_dev_desc = get_dev(ifname,dev);
	if(get_partition_info_by_name (fs_dev_desc,partition_name,&fs_partition) != 0) {
		return -1;
	}

	for (i = 0, info = fstypes; i < ARRAY_SIZE(fstypes); i++, info++) {
		if (fstype != FS_TYPE_ANY && info->fstype != FS_TYPE_ANY &&
				fstype != info->fstype)
			continue;

		if (!info->probe(fs_dev_desc, &fs_partition)) {
			fs_type = info->fstype;
			return 0;
		}
	}

	return -1;

}

int fs_open(int fstype)
{
	struct fstype_info *info = fs_get_info(fstype);
	if(info == NULL)
		return -1;
	fs_type = fstype;
	return 0;
}

static void fs_close(void)
{
	struct fstype_info *info = fs_get_info(fs_type);

	info->close();

	fs_type = FS_TYPE_ANY;
}

int fs_ls(const char *dirname)
{
	int ret;

	struct fstype_info *info = fs_get_info(fs_type);

	ret = info->ls(dirname);

	fs_type = FS_TYPE_ANY;
	fs_close();

	return ret;
}

int fs_mkdir(const char *path)
{
	int ret;

	struct fstype_info *info = fs_get_info(fs_type);

	ret = info->mkdir(path);
	fs_close();
	return ret;
}

int fs_delete(const char *path)
{
	int ret;

	struct fstype_info *info = fs_get_info(fs_type);

	ret = info->delete(path);
	fs_close();
	return ret;
}

int fs_file_stat(const char *dirname)
{
	int ret;

	struct fstype_info *info = fs_get_info(fs_type);

	ret = info->file_stat(dirname);
	fs_close();
	return ret;
}

int fs_dir_stat(const char *dirname)
{
	int ret;

	struct fstype_info *info = fs_get_info(fs_type);

	ret = info->dir_stat(dirname);
	fs_close();
	return ret;
}

int fs_dir_rename(const char *path, const char *new_path)
{
	int ret;

	struct fstype_info *info = fs_get_info(fs_type);

	ret = info->dir_rename(path, new_path);
	fs_close();
	return ret;
}

unsigned long long fs_available_size(void)
{
	unsigned long long ret;

	struct fstype_info *info = fs_get_info(fs_type);

	ret = info->available_size();
	fs_close();
	return ret;
}

int fs_read(const char *filename, ulong addr, loff_t offset, loff_t len)
{
	struct fstype_info *info = fs_get_info(fs_type);
	void *buf;
	int ret;

	/*
	 * We don't actually know how many bytes are being read, since len==0
	 * means read the whole file.
	 */
	buf = map_sysmem(addr, len);
	ret = info->read(filename, buf, offset, len);
	unmap_sysmem(buf);

	/* If we requested a specific number of bytes, check we got it */
	if (ret >= 0 && len && ret != len) {
		printf("** Unable to read file %s **\n", filename);
		ret = -1;
	}
	fs_close();

	return ret;
}

int fs_write(const char *filename, ulong addr, loff_t offset, loff_t len)
{
	struct fstype_info *info = fs_get_info(fs_type);
	void *buf;
	int ret;

	/*
	 * We don't actually know how many bytes are being read, since len==0
	 * means read the whole file.
	 */
	buf = map_sysmem(addr, len);
	ret = info->write(filename, buf, offset, len);
	unmap_sysmem(buf);

	/* If we requested a specific number of bytes, check we got it */
	if (ret > 0 && len && ret != len) {
		printf("** Unable to write file %s **\n", filename);
		ret = -1;
	}
	fs_close();

	return ret;
}

int do_load(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[],
		int fstype)
{
	unsigned long addr;
	const char *addr_str;
	const char *filename;
	unsigned long bytes;
	unsigned long pos;
	int len_read;
	unsigned long time;

	if (argc < 2)
		return CMD_RET_USAGE;
	if (argc > 7)
		return CMD_RET_USAGE;

	if (fs_set_blk_dev(argv[1], (argc >= 3) ? argv[2] : NULL, fstype))
		return 1;

	if (argc >= 4) {
		addr = simple_strtoul(argv[3], NULL, 16);
	} else {
		addr_str = getenv("loadaddr");
		if (addr_str != NULL)
			addr = simple_strtoul(addr_str, NULL, 16);
		else
			addr = CONFIG_SYS_LOAD_ADDR;
	}
	if (argc >= 5) {
		filename = argv[4];
	} else {
		filename = getenv("bootfile");
		if (!filename) {
			puts("** No boot file defined **\n");
			return 1;
		}
	}
	if (argc >= 6)
		bytes = simple_strtoul(argv[5], NULL, 16);
	else
		bytes = 0;
	if (argc >= 7)
		pos = simple_strtoul(argv[6], NULL, 16);
	else
		pos = 0;

	time = get_timer(0);
	len_read = fs_read(filename, addr, pos, bytes);
	time = get_timer(time);
	if (len_read <= 0)
		return 1;

	printf("%d bytes read in %lu ms", len_read, time);
	if (time > 0) {
		puts(" (");
		print_size(len_read / time * 1000, "/s");
		puts(")");
	}
	puts("\n");

	setenv_hex("filesize", len_read);

	return 0;
}

int do_ls(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[],
	int fstype)
{
	if (argc < 2)
		return CMD_RET_USAGE;
	if (argc > 4)
		return CMD_RET_USAGE;

	if (fs_set_blk_dev(argv[1], (argc >= 3) ? argv[2] : NULL, fstype))
		return 1;

	if (fs_ls(argc >= 4 ? argv[3] : "/"))
		return 1;

	return 0;
}

int do_save(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[],
		int fstype)
{
	unsigned long addr;
	const char *filename;
	unsigned long bytes;
	unsigned long pos;
	int len;
	unsigned long time;

	if (argc < 6 || argc > 7)
		return CMD_RET_USAGE;

	if (fs_set_blk_dev(argv[1], argv[2], fstype))
		return 1;

	filename = argv[3];
	addr = simple_strtoul(argv[4], NULL, 16);
	bytes = simple_strtoul(argv[5], NULL, 16);
	if (argc >= 7)
		pos = simple_strtoul(argv[6], NULL, 16);
	else
		pos = 0;

	time = get_timer(0);
	len = fs_write(filename, addr, pos, bytes);
	time = get_timer(time);
	if (len <= 0)
		return 1;

	printf("%d bytes written in %lu ms", len, time);
	if (time > 0) {
		puts(" (");
		print_size(len / time * 1000, "/s");
		puts(")");
	}
	puts("\n");

	return 0;
}
