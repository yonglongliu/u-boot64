#include <common.h>
#include <lwext4fs.h>
#include <fs.h>
#include <asm/sizes.h>
#include <../fs/lwext4/include/ext4.h>
#include <../fs/lwext4/include/ext4_debug.h>
#include <../fs/lwext4/blockdev/blockdev.h>
#define MOUNT_POINT "/userdata/"

static char *entry_to_str(uint8_t type)
{
    switch (type) {
    case EXT4_DE_UNKNOWN:
        return "[unk] ";
    case EXT4_DE_REG_FILE:
        return "[fil] ";
    case EXT4_DE_DIR:
        return "[dir] ";
    case EXT4_DE_CHRDEV:
        return "[cha] ";
    case EXT4_DE_BLKDEV:
        return "[blk] ";
    case EXT4_DE_FIFO:
        return "[fif] ";
    case EXT4_DE_SOCK:
        return "[soc] ";
    case EXT4_DE_SYMLINK:
        return "[sym] ";
    default:
        break;
    }
    return "[???]";
}

int lwext4_probe(block_dev_desc_t *fs_dev_desc,disk_partition_t *fs_partition)
{
    int lwret;
    struct ext4_blockdev *lw_ext4dev = NULL;
    lwext4_set_blk_dev(fs_dev_desc,fs_partition);
    
    lw_ext4dev = ext4_blockdev_get();
    /*for debug to open all log*/
    ext4_dmask_set(DEBUG_ALL);
    ext4_device_register(lw_ext4dev,0,"ext4_fs");
    if (lwret != EOK) {
        debugf("ext4_device_register: lwret = %d\n", lwret);
        return lwret;
    }
    lwret = ext4_mount("ext4_fs", MOUNT_POINT, false);
    if (lwret != 0) {
        debugf("ext4_mount lwret : %d\n",lwret);
        return lwret;
    }
    lwret = ext4_cache_write_back(MOUNT_POINT, 1);
    return lwret;
}

int lwext4_dir_mk(const char *path)
{
    int ret;
    char lwpath[72];
    if(path == NULL)
        return -1;
    sprintf(lwpath, MOUNT_POINT"%s",path);
    debugf("lwpath : %s\n",lwpath);
    ret = ext4_dir_mk(lwpath);
    debugf("lwext4_dir_mk = %d\n",ret);
    return ret;
}

int lwext4_write_to_mem(char *filename,void *memaddr, int offset,unsigned long memsize)
{
    int ret;
    size_t read_cnt;
    ext4_file f;
    char lwpath[255];
    debugf("filename = %s, memaddr = %p, memsize = %lx\n",\
        filename, memaddr, memsize);
    sprintf(lwpath, MOUNT_POINT"%s",filename);
    ret = ext4_fopen(&f,lwpath,"wb");
    debugf("ext4_fopen = %d\n",ret);
    ret = ext4_fwrite(&f,memaddr, memsize, &read_cnt);
    debugf("ext4_fwrite = %d,read_cnt = %lx\n",ret,read_cnt);
    ret = ext4_fclose(&f);
    debugf("ext4_fclose = %d\n",ret);
    return ret;
}

int lwext4_dir_stat(const char *dirname)
{
    /*char sss[255];*/
    char lwpath[72];
    ext4_dir d;
    const ext4_direntry *de;
    int ret = -1;
    sprintf(lwpath, MOUNT_POINT"%s",dirname);
    ret = ext4_dir_open(&d, lwpath);
    debugf("lwpath : %s\n",lwpath);
    if (ret > 0)
        return ret;
    ret = ext4_dir_close(&d);

    return ret;
}

int lwext4_file_stat(const char *filename)
{
    char lwfile[72];
    ext4_file f;
    int ret = -1;
    sprintf(lwfile, MOUNT_POINT"%s",filename);
    ret = ext4_fopen(&f, lwfile, "rb");
    debugf("lwfile : %s\n",lwfile);
    if (ret > 0)
        return ret;
    ret = ext4_fclose(&f);

    return ret;
}

int lwext4_drename(const char *path, const char *new_path)
{
    char lwpath[72],new_lwpath[72];
    int ret;
    if(path == NULL || new_path == NULL)
        return -1;
    sprintf(lwpath, MOUNT_POINT"%s", path);
    sprintf(new_lwpath, MOUNT_POINT"%s",new_path);
    debugf("lwpath : %s\n",lwpath);
    debugf("new_lwpath : %s\n",new_lwpath);
    ret = ext4_frename(lwpath,new_lwpath);
    return ret;
}

int lwext4_dremove(const char *path)
{
    char lwpath[72];
    if(path == NULL)
        return -1;
    sprintf(lwpath, MOUNT_POINT"%s", path);
    debugf("lwpath : %s\n",lwpath);
    return ext4_dir_rm(lwpath);
}

unsigned long long lwext4_getsize()
{
    unsigned long long size = 0;
    struct ext4_mount_stats stats;
    ext4_mount_point_stats(MOUNT_POINT, &stats);
    debugf("free_blocks_count = %llu\n",stats.free_blocks_count);
    debugf("block_size = %lu\n", stats.block_size);

    size = (unsigned long long)stats.free_blocks_count * (unsigned long long)stats.block_size;
    return size;
}

void lwext4_close()
{
    ext4_cache_write_back(MOUNT_POINT, 0);
}

int lwext4_ls(const char *dirname)
{
    char sss[255];
    char lwpath[72];
    ext4_dir d;
    const ext4_direntry *de;
    int ret = -1;
    debugf("ls %s\n", MOUNT_POINT);
    sprintf(lwpath, MOUNT_POINT"%s",dirname);
    ext4_dir_open(&d, lwpath);
    de = ext4_dir_entry_next(&d);
    while (de) {
        memcpy(sss, de->name, de->name_length);
        sss[de->name_length] = 0;
        debugf("%s%s\n", entry_to_str(de->inode_type), sss);
        de = ext4_dir_entry_next(&d);
    }
    ext4_dir_close(&d);

    return ret;
}

#define BOOTLOADER_LOG_PATH "/userdata/ylog/bootloader.log"
static int init_flag;
static int before_uboot_log()
{
    if (fs_set_blk_by_name("mmc",0,"userdata",4) < 0) {
        init_flag = 0;
        return -1;
    }
    if (lwext4_getsize() < SZ_1M) {
       init_flag = 0;
       return -1;
    }
    init_flag = 1;
    return 0;
}

static int uboot_flag;
int save_uboot_log(const char *s)
{
    int ret;
    size_t read_cnt;
    ext4_file f;
    unsigned long memsize = strlen(s);

    uboot_flag ++;
    if (uboot_flag == 1) {
        if (!init_flag) {
            if (before_uboot_log()) {
                uboot_flag = 0;
                return 0;
            }
        }
         if (ext4_fopen(&f,BOOTLOADER_LOG_PATH,"a+")) {
                uboot_flag = 0;
                return 0;
        }

        if (ext4_fwrite(&f,s, memsize, &read_cnt)) {
            uboot_flag = 0;
            return 0;
        }
        if (ext4_fclose(&f)) {
            uboot_flag = 0;
            return 0;
        }
        if (ext4_cache_write_back(MOUNT_POINT, 0)) {
            uboot_flag = 0;
            return 0;
        }
        uboot_flag = 0;
        return 0;
    } else
        return -1;
}
