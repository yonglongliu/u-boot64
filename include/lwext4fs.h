#ifndef LWEXT4_H_
#define LWEXT4_H_
#include <part.h>

int lwext4_probe(block_dev_desc_t *fs_dev_desc,disk_partition_t *fs_partition);
int lwext4_dir_mk(const char *path);
int lwext4_write_to_mem(char *filename,void *memaddr, int offset,unsigned long memsize);
int lwext4_dir_stat(const char *dirname);
int lwext4_file_stat(const char *filename);
int lwext4_drename(const char *path, const char *new_path);
int lwext4_dremove(const char *path);
unsigned long long lwext4_getsize();
void lwext4_close();
int lwext4_ls(const char *dirname);
int save_uboot_log(const char *s);
#endif
