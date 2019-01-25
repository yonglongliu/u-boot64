#include <common.h>
#include <linux/types.h>
#include <linux/string.h>

#include "root_inspect.h"
#include "loader_common.h"

#ifdef CONFIG_EMMC_BOOT
#include <mmc.h>
#endif

u32 get_rootflag(root_stat_t *stat)
{
	u32 result = 0;

	if (do_raw_data_read(PRODUCTINFO_FILE_PATITION, sizeof(root_stat_t), ROOT_OFFSET, stat)) {
		debugf("read miscdata error.\n");
		return result;
	}

	if (stat->magic == ROOT_MAGIC) {
		debugf("ROOT_MAGIC matched\n");
		debugf("stat.root_flag = %d\n", stat->root_flag);
		result = stat->root_flag;
	}
	return result;
}

u32 erase_rootflag(root_stat_t *stat)
{
	unsigned long erasesize = 0, base_sector = 0;
	unsigned int curArea = 0;
	disk_partition_t info;
	block_dev_desc_t *dev = NULL;

	dev = get_dev("mmc", 0);
	if (NULL == dev) {
		debugf("Block device not supported!,erase rootflag failed.\n");
		return -1;
	}
	if (0 != get_partition_info_by_name(dev, PRODUCTINFO_FILE_PATITION, &info)) {
		debugf("eMMC get partition ERROR!\n");
		return -1;
	}

	curArea = PARTITION_USER;
	base_sector = info.start + ROOT_OFFSET/EMMC_SECTOR_SIZE;
	erasesize = sizeof(root_stat_t);
	char *buf = malloc(erasesize);
	memset(buf, 0, erasesize);

	debugf("Erase miscdata partition.\n");

	Emmc_Write(curArea, base_sector, 1, buf);

	memset(stat, 0, erasesize);

	free(buf);
	return 0;
}
