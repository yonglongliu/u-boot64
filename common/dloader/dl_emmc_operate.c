
#include "common.h"

#include <mmc.h>
#include "packet.h"
#include <malloc.h>
#include <asm/arch/chip_releted_def.h>
#include "dl_common.h"
#include "dl_cmd_proc.h"
#include "dl_emmc_operate.h"
#include <ext_common.h>
#include "dl_crc.h"
#include <asm/sizes.h>
#include <sparse_format.h>

#include <secureboot/sec_common.h>
//#ifdef CONFIG_SECURE_BOOT
#include "secure_verify.h"
#include "secure_boot.h"
//#endif
static DL_EMMC_FILE_STATUS g_status;
static DL_EMMC_STATUS g_dl_eMMCStatus = { 0, 0, 0, 0, 0, 0, 0 };

static unsigned long g_checksum;
static unsigned long g_sram_addr;
static sys_img_header *bakup_header;
static unsigned int img_backup_flag = 0;

unsigned char *g_eMMCBuf = (unsigned char *)0x82000000;

//#ifdef CONFIG_SECURE_BOOT
#if defined (CONFIG_SECURE_BOOT) || defined (CONFIG_SECBOOT)
static int check_secure_flag = 0;
static int secure_image_flag = 0;
typedef enum SECURE_IMAGE {
	NORMALE_IMAGE = 1,
	SPL_IMAGE,
	SYSTEM_IMAGE
} SIGNED_IMAGE;
static uchar *const s_force_secure_check[] = {
	"boot",
	"recovery",
	"uboot",
	"wl_modem",
	"wl_ldsp",
	"wmodem",
	"wl_gdsp",
	"wl_warm",
	"pm_sys",
	"sml",
	"tl_ldsp",
	"tl_tgdsp",
	"tl_modem",
	"l_modem",
	"l_ldsp",
	"l_gdsp",
	"l_warm",
	"l_tgdsp",
	"l_agdsp",
	"trustos",
	"wdsp",
	"vmm",
	#ifdef CONFIG_SUPPORT_WIFI
	"wcnmodem",
	#endif
	NULL
};
#endif

/**
	partitions not for raw data or normal usage(e.g. nv and prodinfo) should config here.
	partitions not list here mean raw data/normal usage.
*/
SPECIAL_PARTITION_CFG const s_special_partition_cfg[] = {
	{{"fixnv1"}, {"fixnv2"}, IMG_RAW, PARTITION_PURPOSE_NV},
	{{"runtimenv1"}, {"runtimenv2"}, IMG_RAW, PARTITION_PURPOSE_NV},
	{{"tdfixnv1"}, {"tdfixnv2"}, IMG_RAW, PARTITION_PURPOSE_NV},
	{{"tdruntimenv1"}, {"tdruntimenv2"}, IMG_RAW, PARTITION_PURPOSE_NV},
	{{"g_fixnv1"}, {"g_fixnv2"}, IMG_RAW, PARTITION_PURPOSE_NV},
	{{"g_runtimenv1"}, {"g_runtimenv2"}, IMG_RAW, PARTITION_PURPOSE_NV},
	{{"l_fixnv1"}, {"l_fixnv2"}, IMG_RAW, PARTITION_PURPOSE_NV},
	{{"l_runtimenv1"}, {"l_runtimenv2"}, IMG_RAW, PARTITION_PURPOSE_NV},
	{{"tl_fixnv1"}, {"tl_fixnv2"}, IMG_RAW, PARTITION_PURPOSE_NV},
	{{"tl_runtimenv1"}, {"tl_runtimenv2"}, IMG_RAW, PARTITION_PURPOSE_NV},
	{{"lf_fixnv1"}, {"lf_fixnv2"}, IMG_RAW, PARTITION_PURPOSE_NV},
	{{"lf_runtimenv1"}, {"lf_runtimenv2"}, IMG_RAW, PARTITION_PURPOSE_NV},
	{{"wfixnv1"}, {"wfixnv2"}, IMG_RAW, PARTITION_PURPOSE_NV},
	{{"wruntimenv1"}, {"wruntimenv2"}, IMG_RAW, PARTITION_PURPOSE_NV},
	{{"wl_fixnv1"}, {"wl_fixnv2"}, IMG_RAW, PARTITION_PURPOSE_NV},
	{{"wl_runtimenv1"}, {"wl_runtimenv2"}, IMG_RAW, PARTITION_PURPOSE_NV},
	{{"wcnfixnv1"}, {"wcnfixnv2"}, IMG_RAW, PARTITION_PURPOSE_NV},
	{{"wcnruntimenv1"}, {"wcnruntimenv2"}, IMG_RAW, PARTITION_PURPOSE_NV},
	{{"system"}, NULL, IMG_RAW, PARTITION_PURPOSE_NORMAL},
	{{"userdata"}, NULL, IMG_WITH_SPARSE, PARTITION_PURPOSE_NORMAL},
	{{"cache"}, NULL, IMG_WITH_SPARSE, PARTITION_PURPOSE_NORMAL},
	{{"prodnv"}, NULL, IMG_RAW, PARTITION_PURPOSE_NORMAL},
	{NULL, NULL, IMG_TYPE_MAX, PARTITION_PURPOSE_MAX}
};


ALTER_BUFFER_ATTR alter_buffer1;
ALTER_BUFFER_ATTR alter_buffer2;
ALTER_BUFFER_ATTR* current_buffer;


/***********************************************************/

uint16_t eMMCCheckSum(const uint32_t * src, int32_t len)
{
	uint32_t sum = 0;
	unsigned short *src_short_ptr = NULL;

	while (len > 3) {
		sum += *src++;
		len -= 4;
	}

	src_short_ptr = (uint16_t *) src;

	if (0 != (len & 0x2)) {
		sum += *(src_short_ptr);
		src_short_ptr++;
	}

	if (0 != (len & 0x1)) {
		sum += *((unsigned char *)(src_short_ptr));
	}

	sum = (sum >> 16) + (sum & 0x0FFFF);
	sum += (sum >> 16);

	return (uint16_t) (~sum);
}

uint32_t get_pad_data(const uint32_t * src, int32_t len, int32_t offset, uint16_t sum)
{
	uint32_t sum_tmp;
	uint32_t sum1 = 0;
	uint32_t pad_data;
	uint32_t i;
	sum = ~sum;
	sum_tmp = sum & 0xffff;
	sum1 = 0;
	for (i = 0; i < offset; i++) {
		sum1 += src[i];
	}
	for (i = (offset + 1); i < len; i++) {
		sum1 += src[i];
	}
	pad_data = sum_tmp - sum1;
	return pad_data;
}

void splFillCheckData(uint32_t * splBuf, int32_t len)
{
	EMMC_BootHeader *header;
	uint32_t pad_data;
	uint32_t w_len;
	uint32_t w_offset;
	w_len = (SPL_CHECKSUM_LEN - (BOOTLOADER_HEADER_OFFSET + sizeof(*header))) / 4;
	w_offset = w_len - 1;
	/*pad the data inorder to make check sum to 0 */
	pad_data = (uint32_t) get_pad_data((unsigned char *)splBuf + BOOTLOADER_HEADER_OFFSET + sizeof(*header), w_len, w_offset, 0);
	*(volatile unsigned int *)((uchar *) splBuf + SPL_CHECKSUM_LEN - 4) = pad_data;
	header = (EMMC_BootHeader *) ((uchar *) splBuf + BOOTLOADER_HEADER_OFFSET);
	header->version = 0;
	header->magicData = MAGIC_DATA;
	header->checkSum = (uint32_t) eMMCCheckSum((uchar *) splBuf + BOOTLOADER_HEADER_OFFSET + sizeof(*header),
						   SPL_CHECKSUM_LEN - (BOOTLOADER_HEADER_OFFSET + sizeof(*header)));
#ifdef CONFIG_SECURE_BOOT
	header->hashLen = CONFIG_SPL_HASH_LEN >> 2;
#else
	header->hashLen = 0;
#endif
}

int32_t _uefi_get_part_info(disk_partition_t ** part_info_arr, uint32_t * total_part_num, READ_INFO_METHOD method)
{
	block_dev_desc_t *dev_desc = NULL;
	int part_index = 0;
	int res = 0;
	disk_partition_t *part_info;

	static disk_partition_t part_info_internal[MAX_GPT_PARTITION_SUPPORT] = { 0 };
	static uint32_t read_info_already = FALSE;
	static uint32_t part_num_internal = 0;

	if (!part_info_arr || !total_part_num) {
		debugf("Invalid argument !\n");
		return 0;
	}

	if ((TRUE == read_info_already) && (FLEXIBLE_READ == method)) {
		//debugf("Get partiton info from local .\n");
		*part_info_arr = part_info_internal;
		*total_part_num = part_num_internal;
		return 1;
	}

	part_info = malloc(sizeof(disk_partition_t));
	if (NULL == part_info) {
		debugf("No spare space to store partition_info!\n");
		return 0;
	}

	dev_desc = get_dev("mmc", 0);
	if (NULL == dev_desc) {
		free(part_info);
		return 0;
	}

	debugf("Get partiton info from efi \n");

	part_num_internal = 0;

	for (part_index = 0; part_index < MAX_GPT_PARTITION_SUPPORT; part_index++) {
		res = get_partition_info_efi(dev_desc, part_index + 1, part_info);
		/*get partition info success ,then write the static data */
		if (0 == res) {
			part_info_internal[part_index] = *part_info;
			part_num_internal++;
		} else {
			/*get fail means there is no more partition */
			break;
		}
	}

	if (MAX_GPT_PARTITION_SUPPORT != part_index)
		memset(&part_info_internal[part_index], 0, sizeof(disk_partition_t));

	read_info_already = TRUE;
	*part_info_arr = part_info_internal;
	*total_part_num = part_num_internal;
	free(part_info);

	return 1;
}

uint32_t efi_GetPartBaseSec(uchar * partition_name)
{
	int i;

	disk_partition_t *all_part_info;
	uint32_t total_part_num = 0;

	/*Get user partition info from eMMC */
	_uefi_get_part_info(&all_part_info, &total_part_num, FLEXIBLE_READ);

	for (i = 0; i < total_part_num; i++) {
		if (0 == strcmp(partition_name, all_part_info[i].name)) {
			return all_part_info[i].start;
		}
	}

	debugf("Can't find partition:%s!\n", partition_name);
	/*return 0 means find the specified partition fail */
	return 0;
}

uint32_t efi_GetPartSize(uchar * partition_name)
{
	int i;

	disk_partition_t *all_part_info;
	uint32_t total_part_num = 0;

	if (NULL == partition_name) {
		debugf("Invalid argument!\n");
		return 0;
	}

	/*Get user partition info from eMMC */
	_uefi_get_part_info(&all_part_info, &total_part_num, FLEXIBLE_READ);
	/*
	debugf("Get part info complete ,total num=%d!\n", total_part_num);
	for (i = 0; i < total_part_num; i++) {
		debugf("old partition_name:%s,partition_size:%ld,partiton_start:%ld,\n",
		all_part_info[i].name , all_part_info[i].size, all_part_info[i].start);
	}
	 */

	for (i = 0; i < total_part_num; i++) {
		if (0 == strcmp(partition_name, all_part_info[i].name)) {
			debugf("Find partition:%s!\n", all_part_info[i].name);
			/*return the block number*/
			return all_part_info[i].size;
		}
	}

	debugf("Can't find partition:%s!\n", partition_name);
	/*return 0 means find the specified partition fail */
	return 0;
}

int _parser_repartition_cfg_v0(disk_partition_t * partition_info, const uchar * partition_cfg, uint16_t total_partition_num)
{
	uint16_t i = 0;
	uint16_t j = 0;
	uint32_t partition_size_m = 0;
	lbaint_t partition_start_lba = STARTING_LBA_OF_FIRST_PARTITION;
	uint32_t lbas_per_m = SZ_1M /EFI_SECTOR_SIZE;

	/*Decode String: Partition Name(72Byte)+SIZE(4Byte)+... */
	for (i = 0; i < total_partition_num; i++) {
		partition_info[i].blksz = EFI_SECTOR_SIZE;
		strcpy(partition_info[i].type, "U-boot");	/*no use */

		partition_size_m = *(uint32_t *) (partition_cfg + 76 * (i + 1) - 4);
		/*the last partition and partition_size_m is 0xFFFFFFFF means this partition use all the spare lba */
		if ((i == total_partition_num - 1) && (MAX_SIZE_FLAG == partition_size_m)) {
			/*size=0 represent use all the spare lba */
			partition_info[i].size = 0;
		} else {
			/*calc the partition size of lba , raw data unit is Mb */
			partition_info[i].size = (lbaint_t) partition_size_m * lbas_per_m;
		}

		/*in raw data rcv from download tool,partition_name is 38 uint16_t length;
		 **in part.h ,disk_partition_t.name is 32 uchar length;
		 **in part_efi.h ,gpt_entry.partition_name is 36 uint16_t length,
		 **we convert raw data to disk_partition_t format here , gpt_fill_pte() in part_efi.c will
		 **convert disk_partition_t to gpt_entry*/
		for (j = 0; j < 32 - 1; j++) {
			/*transform 64 bytes uint16_t to 32 bytes uchar ,and discard the last 8 bytes */
			partition_info[i].name[j] = *((uint16_t *) partition_cfg + 38 * i + j) & 0xFF;
		}
		partition_info[i].name[j] = '\0';
		partition_info[i].start = partition_start_lba;

		partition_start_lba += partition_info[i].size;

		debugf("partition name:%s,partition_size:0x%lx,partiton_start:0x%lx,\n", partition_info[i].name, partition_info[i].size,
		       partition_info[i].start);
	}

	return 0;
}


int _parser_repartition_cfg_v1(disk_partition_t * partition_info, uchar* partition_cfg, uint16_t total_partition_num,
									uchar size_unit)
{
	int i, j;

	uint64_t partition_size_raw;
	uint64_t partition_size_lba;
	/*attention here, we use int64 instead of uint64 to support minus gap size, such as "-2048"*/
	int64_t gap_raw;
	int64_t gap_lba;
	uint64_t partition_start_lba = 0;

	/*V1 partition table format: Partition Name(72Byte)+SIZE(8Byte)+GAP(8Byte)...*/
	for (i = 0; i < total_partition_num; i++) {
		partition_size_raw = *(uint64_t *)(partition_cfg + 88 * (i + 1) - 16);
		gap_raw = *(int64_t *)(partition_cfg + 88 * (i + 1) - 8);
		debugf("partition_size_raw = 0x%llx, gap_raw = 0x%llx\n", partition_size_raw, gap_raw);

		switch (size_unit) {
		/*unit is MB*/
		case 0:
			partition_size_lba = partition_size_raw * SZ_1M / EFI_SECTOR_SIZE;
			gap_lba = gap_raw * SZ_1M / EFI_SECTOR_SIZE;
			break;
		/*unit is 512K*/
		case 1:
			partition_size_lba = partition_size_raw * SZ_1K;
			gap_lba = gap_raw * SZ_1K;
			break;
		/*unit is KB*/
		case 2:
			partition_size_lba = partition_size_raw * SZ_1K / EFI_SECTOR_SIZE;
			gap_lba = gap_raw * SZ_1K / EFI_SECTOR_SIZE;
			break;
		/*unit is bytes, min partition unit supported is lba(512byte), so we pad it if neccessary*/
		case 3:
			if (partition_size_raw % EFI_SECTOR_SIZE != 0)
				partition_size_lba = partition_size_raw / EFI_SECTOR_SIZE + 1;
			else
				partition_size_lba = partition_size_raw / EFI_SECTOR_SIZE;
			if (gap_raw % EFI_SECTOR_SIZE != 0)
				gap_lba = gap_raw / EFI_SECTOR_SIZE + 1;
			else
				gap_lba = gap_raw / EFI_SECTOR_SIZE;
			break;
		/*unit is sector*/
		case 4:
			partition_size_lba = partition_size_raw;
			gap_lba = gap_raw;
			break;
		/*unsupported unit we consider it as MB*/
		default:
			partition_size_lba = partition_size_raw * SZ_1M / EFI_SECTOR_SIZE;
			gap_lba = gap_raw * SZ_1M / EFI_SECTOR_SIZE;
			break;
		}

		/*check the gap of the first partition*/
		if (0 == i) {
			if (0 == gap_lba) {
				gap_lba = STARTING_LBA_OF_FIRST_PARTITION;
			} else if (gap_lba < FIRST_USABLE_LBA_FOR_PARTITION) {
				printf("%s: FATAL ERROR, gap of the first partition counted in lba CAN NOT less than 34\n", __FUNCTION__);
				return -1;
			}
		}

		/*all the 8 bytes are 0xFF in the packet, but to compatible with the old version,
			we only consider the lower 4 bytes.*/
		if (MAX_SIZE_FLAG == (partition_size_raw & MAX_SIZE_FLAG)) {
			partition_size_lba = 0;
			debugf("the last partition use all the left lba\n");
		}
		partition_start_lba += gap_lba;
		partition_info[i].start = (lbaint_t)partition_start_lba;
		partition_info[i].size = (lbaint_t)partition_size_lba;
		if (0 != partition_size_lba)
			partition_start_lba += partition_size_lba;

		strcpy(partition_info[i].type, "U-boot");	/*no use */

		for (j = 0; j < 32 - 1; j++)
			partition_info[i].name[j] = *((uint16_t *)partition_cfg + 44 * i + j) & 0xFF;

		debugf("partition name:%s,partition_size:0x%lx, partition_offset:0x%lx\n",
			partition_info[i].name, partition_info[i].size, partition_info[i].start);
	}
	return 0;
}

int _parser_repartition_cfg(disk_partition_t * partition_info, const uchar* partition_cfg, uint16_t total_partition_num,
								unsigned char version, unsigned char size_unit)
{
	int ret = 0;

	switch (version) {
	case 0:
		debugf("Handle repartition packet version 0  \n");
		ret = _parser_repartition_cfg_v0(partition_info, partition_cfg, total_partition_num);
		break;
	case 1:
	default:
		debugf("Handle repartition packet version 1  \n");
		ret = _parser_repartition_cfg_v1(partition_info, partition_cfg, total_partition_num, size_unit);
		break;
	}
	return ret;
}


/**
	Get the partition's target image type(raw data or others) and purpose(nv/prodinfo/normal).
*/
void _get_partition_attribute(uchar * partition_name)
{
	int i;

	for (i = 0; s_special_partition_cfg[i].partition != NULL; i++) {
		if (0 == strcmp(s_special_partition_cfg[i].partition, partition_name)) {
			g_dl_eMMCStatus.curImgType = s_special_partition_cfg[i].imgattr;
			g_dl_eMMCStatus.partitionpurpose = s_special_partition_cfg[i].purpose;
			debugf("partition %s image type is %d,partitionpurpose:%d\n", partition_name, g_dl_eMMCStatus.curImgType,
			       g_dl_eMMCStatus.partitionpurpose);
			return;
		}
	}

	/*default type is IMG_RAW */
	g_dl_eMMCStatus.curImgType = IMG_RAW;
	g_dl_eMMCStatus.partitionpurpose = PARTITION_PURPOSE_NORMAL;
	debugf("partition %s image type is RAW, normal partition!\n", partition_name);

	return;
}

/**
	Check whether the partition to be operated is compatible.
*/
int32_t _get_compatible_partition(uchar * partition_name)
{
	uint32_t i;
	disk_partition_t *all_part_info;
	uint32_t total_part_num = 0;

	/*get special partition attr */
	_get_partition_attribute(partition_name);

	/*Get user partition info from eMMC */
	_uefi_get_part_info(&all_part_info, &total_part_num, FLEXIBLE_READ);

	/*Try to find the partition specified */
	if (0 == strcmp(partition_name, "uboot")) {
		strcpy(g_dl_eMMCStatus.curUserPartitionName, UBOOT_PARTITION_NAME);
		return TRUE;
	}

	if (0 == strcmp(partition_name, "splloader")) {
		strcpy(g_dl_eMMCStatus.curUserPartitionName, SPL_PARTITION_NAME);
		return TRUE;
	}

	for (i = 0; i < total_part_num; i++) {
		if (0 == strcmp(partition_name, all_part_info[i].name)) {
			strcpy(g_dl_eMMCStatus.curUserPartitionName, all_part_info[i].name);
			//debugf("%s:g_dl_eMMCStatus.curUserPartitionName=%s \n", __FUNCTION__,g_dl_eMMCStatus.curUserPartitionName);
			return TRUE;
		}
	}
	/*Can't find the specified partition */
	strcpy(g_dl_eMMCStatus.curUserPartitionName, "\0");
	debugf("Can't find partition:%s \n", partition_name);

	return FALSE;
}

/**
	Get the backup partition name
*/
uchar *_get_backup_partition_name(uchar * partition_name)
{
	int i = 0;

	for (i = 0; s_special_partition_cfg[i].partition != NULL; i++) {
		if (0 == strcmp(partition_name, s_special_partition_cfg[i].partition)) {
			return s_special_partition_cfg[i].bak_partition;
		}
	}

	return NULL;
}

/*check whether the new partition table and old one are same*/
int32_t _check_partition_table(disk_partition_t * new_part_info, uint16_t total_partition_num)
{
	uint16_t part_index = 0;
	int32_t ret_val = PART_SAME;
	disk_partition_t *old_part_info;
	uint32_t old_part_num = 0;
	int i = 0;
	uint64_t emmc_user_capacity = 0;
	lbaint_t new_last_part_size = 0;

	debugf("_check_partition_table -----\n");

	/*Get user partition info from eMMC */
	if (!_uefi_get_part_info(&old_part_info, &old_part_num, FORCE_READ)) {
		debugf("get partition info fail\n");
		return PART_DIFF;
	}
	/*
	for (i = 0; i < old_part_num; i++) {
		debugf("old partition_name:%s,partition_size:0x%lx,partiton_start:0x%lx,\n", old_part_info[i].name, old_part_info[i].size,
		       old_part_info[i].start);
	}
	*/
	if (old_part_num != total_partition_num) {
		return PART_DIFF;
	}

	/*check all partitions */
	for (part_index = 0; part_index < total_partition_num; part_index++) {
		if (0 != strcmp(new_part_info[part_index].name, old_part_info[part_index].name)) {
			ret_val = PART_DIFF;
			break;
		}

		if (new_part_info[part_index].start != old_part_info[part_index].start) {
			ret_val = PART_DIFF;
			break;
		}

		/*attention ,the last partition size=0 means use all spare lba,so don't check the size */
		if (0 == new_part_info[part_index].size && part_index == total_partition_num - 1) {
			emmc_user_capacity = Emmc_GetCapacity(PARTITION_USER);
			emmc_user_capacity = emmc_user_capacity / EFI_SECTOR_SIZE;
			new_last_part_size = emmc_user_capacity - new_part_info[part_index - 1].start - new_part_info[part_index - 1].size - ALTERNATE_GPT_PTE_SIZE;
			debugf("new_last_part_size=0x%x, old_last_part_size=0x%x\n", new_last_part_size, old_part_info[part_index].size);
			if (new_last_part_size != old_part_info[part_index].size)
				ret_val = PART_DIFF;
			else
				ret_val = PART_SAME;
			break;
		}

		if (new_part_info[part_index].size != old_part_info[part_index].size) {
			ret_val = PART_DIFF;
			break;
		}
	}

	return ret_val;
}

/**
	Erase the whole partition.
*/
int _emmc_real_erase_partition(uchar * partition_name)
{
	uint32_t count = 0;
	uint32_t base_sector = 0;
	uint8_t curArea = 0;

	debugf("Real erase partition %s! \n", partition_name);
	if (NULL == partition_name)
		return 0;

	if (strcmp("splloader", partition_name) == 0) {
//#ifdef CONFIG_SECURE_BOOT
#if defined (CONFIG_SECURE_BOOT) || defined (CONFIG_SECBOOT)
		//if (secureboot_enabled())
		if(secboot_enable_check())
		{
			return 1;
		}
#endif
		count = Emmc_GetCapacity(PARTITION_BOOT1);
		count = count / EFI_SECTOR_SIZE;	/* partition size : in blocks */
		curArea = PARTITION_BOOT1;
		base_sector = 0;
	} else if (strcmp("uboot", partition_name) == 0) {
		count = Emmc_GetCapacity(PARTITION_BOOT2);
		count = count / EFI_SECTOR_SIZE;	/* partition size : in blocks */
		curArea = PARTITION_BOOT2;
		base_sector = 0;
	} else {
		curArea = PARTITION_USER;
		count = efi_GetPartSize(partition_name);	/* partition size : in blocks */
		if (0 == count)
			return 0;
		base_sector = efi_GetPartBaseSec(partition_name);
		if (0 == base_sector)
			return 0;
	}

	 if (!Emmc_Erase(curArea, base_sector, count))
			return 0;

	return 1;
}

/**
	Fast erase userpartition use emmc_write .
*/
int _emmc_fast_erase_userpartition(disk_partition_t * part_info)
{
	uint32_t count = 0;
	uint32_t base_sector = 0;

	count = part_info->size;
	base_sector = part_info->start;
	count = (count > ERASE_SECTOR_SIZE) ? ERASE_SECTOR_SIZE : count;

	memset(g_eMMCBuf, 0xff, count * EFI_SECTOR_SIZE);
	if (!Emmc_Write(PARTITION_USER, base_sector, count, (unsigned char *)g_eMMCBuf))
		return 0;

	return 1;
}

int32_t _emmc_erase_allflash(void)
{
	uint64_t count;
	count = Emmc_GetCapacity(PARTITION_USER);
	count = count / EFI_SECTOR_SIZE;
	if (!Emmc_Erase(PARTITION_USER, 0,count))
			return 0;
	/*erase boot1 partition */
	if (!_emmc_real_erase_partition(SPL_PARTITION_NAME))
		return 0;

	/*erase boot2 partition */
	if (!_emmc_real_erase_partition(UBOOT_PARTITION_NAME))
		return 0;

	return 1;
}

void prepare_alternative_buffers(void)
{

	alter_buffer1.addr = g_eMMCBuf + EMMC_BUF_SIZE+ALTERNATIVE_BUFFER_SIZE;
	alter_buffer1.pointer = alter_buffer1.addr;
	alter_buffer1.size = ALTERNATIVE_BUFFER_SIZE;
	alter_buffer1.used = 0;
	alter_buffer1.spare = alter_buffer1.size;
	alter_buffer1.status = BUFFER_CLEAN;
	alter_buffer1.next = &alter_buffer2;

	alter_buffer2.addr = alter_buffer1.addr + 2*ALTERNATIVE_BUFFER_SIZE;
	alter_buffer2.pointer = alter_buffer2.addr;
	alter_buffer2.size = ALTERNATIVE_BUFFER_SIZE;
	alter_buffer2.used = 0;
	alter_buffer2.spare = alter_buffer2.size;
	alter_buffer2.status = BUFFER_CLEAN;
	alter_buffer2.next = &alter_buffer1;

	current_buffer = &alter_buffer1;

	return;

}

int speedup_download_process(uint16_t size, char *buf)
{
	uint32_t blk_count = 0;
	uint32_t num = current_buffer->size / EFI_SECTOR_SIZE;
	uint32_t last_size = 0;
	if (current_buffer->spare > size)  {
		memcpy(current_buffer->pointer, buf, size);
		current_buffer->used += size;
		current_buffer->spare -= size;
		current_buffer->pointer += size;

		if (g_status.total_recv_size == g_status.total_size) {
			if (0 == (current_buffer->used % EFI_SECTOR_SIZE))
				blk_count = current_buffer->used / EFI_SECTOR_SIZE;
			else
				blk_count = current_buffer->used / EFI_SECTOR_SIZE + 1;

			if (BUFFER_DIRTY == current_buffer->next->status)
				if (!Emmc_Query_Backstage(g_dl_eMMCStatus.curEMMCArea, num, current_buffer->next->addr))
					return OPERATE_WRITE_ERROR;

			if (!Emmc_Write(g_dl_eMMCStatus.curEMMCArea, g_dl_eMMCStatus.base_sector, blk_count, current_buffer->addr))
				return OPERATE_WRITE_ERROR;
			g_status.unsave_recv_size = 0;
		}
	} else {
		memcpy(current_buffer->pointer, buf, current_buffer->spare);
		last_size = size - current_buffer->spare;
		if (BUFFER_DIRTY == current_buffer->next->status)
			if (!Emmc_Query_Backstage(g_dl_eMMCStatus.curEMMCArea, num, current_buffer->next->addr))
				return OPERATE_WRITE_ERROR;
		if (!Emmc_Write_Backstage(PARTITION_USER, g_dl_eMMCStatus.base_sector, num, current_buffer->addr))
			return OPERATE_WRITE_ERROR;
		g_dl_eMMCStatus.base_sector += num;
		if (0 != last_size)
			memcpy(current_buffer->next->addr, buf + current_buffer->spare, last_size);
		current_buffer->used = 0;
		current_buffer->spare = current_buffer->size;
		current_buffer->pointer = current_buffer->addr;
		current_buffer->status = BUFFER_DIRTY;

		current_buffer = current_buffer->next;
		current_buffer->pointer = current_buffer->addr + last_size;
		current_buffer->used = last_size;
		current_buffer->spare = current_buffer->size - last_size;
		if (g_status.total_recv_size == g_status.total_size) {
			if (!Emmc_Query_Backstage(g_dl_eMMCStatus.curEMMCArea, num, current_buffer->next->addr))
				return OPERATE_WRITE_ERROR;
			if (0 != current_buffer->used) {
				if (0 == (current_buffer->used % EFI_SECTOR_SIZE))
					blk_count = current_buffer->used / EFI_SECTOR_SIZE;
				else
					blk_count = current_buffer->used / EFI_SECTOR_SIZE + 1;
				if (!Emmc_Write(g_dl_eMMCStatus.curEMMCArea, g_dl_eMMCStatus.base_sector, blk_count, current_buffer->addr))
					return OPERATE_WRITE_ERROR;
			}
			g_status.unsave_recv_size = 0;
		}
	}

	return OPERATE_SUCCESS;
}


int regular_download_process(uint16_t size, char *buf)
{
	uint32_t lastSize;
	uint32_t nSectorCount;
	uint32_t each_write_block = EMMC_MAX_MUTIL_WRITE;
	uint32_t fix_nv_checksum;

	if (EMMC_BUF_SIZE >= (g_status.unsave_recv_size + size)) {
		memcpy((uchar *) g_sram_addr, buf, size);
		g_sram_addr += size;
		g_status.unsave_recv_size += size;

		if (EMMC_BUF_SIZE == g_status.unsave_recv_size) {
			debugf("Unsaved buf size equal the whole buffer size (0x%x)\n", EMMC_BUF_SIZE);
			if (0 == (EMMC_BUF_SIZE % EFI_SECTOR_SIZE))
				nSectorCount = EMMC_BUF_SIZE / EFI_SECTOR_SIZE;
			else
				nSectorCount = EMMC_BUF_SIZE / EFI_SECTOR_SIZE + 1;

			if (!_emmc_download_image(nSectorCount, each_write_block, FALSE))
				return OPERATE_WRITE_ERROR;
		} else if (g_status.total_recv_size == g_status.total_size) {
			if (PARTITION_PURPOSE_NV == g_dl_eMMCStatus.partitionpurpose) {
				debugf("Get all data of partition<%s>,start write to EMMC\n", g_dl_eMMCStatus.curUserPartitionName);
				fix_nv_checksum = Get_CheckSum((uchar *) g_eMMCBuf, g_status.total_recv_size);
				if (fix_nv_checksum != g_checksum) {
					/*may data transfer error */
					debugf("nv data transfer error,checksum error!\n");
					return OPERATE_CHECKSUM_DIFF;
				}
				if (!_nv_img_check_and_write(g_dl_eMMCStatus.curUserPartitionName, FIXNV_SIZE))
					return OPERATE_WRITE_ERROR;
			} else {
				if (g_status.unsave_recv_size != 0) {
					if (0 == (g_status.unsave_recv_size % EFI_SECTOR_SIZE))
						nSectorCount = g_status.unsave_recv_size / EFI_SECTOR_SIZE;
					else
						nSectorCount = g_status.unsave_recv_size / EFI_SECTOR_SIZE + 1;

					if (0 == strcmp("splloader", g_dl_eMMCStatus.curUserPartitionName)) {
						if (g_status.total_recv_size < SPL_CHECKSUM_LEN)
							nSectorCount = SPL_CHECKSUM_LEN / EFI_SECTOR_SIZE;
						/*when the downloading spl not match the fix size ,fill the space with pad data */
#ifndef CONFIG_SPL_BAK
						splFillCheckData((uint32_t *) g_eMMCBuf, (int32_t) g_status.total_recv_size);
#endif
					}

						bakup_header = malloc(sizeof(sys_img_header));
						if (NULL == bakup_header) {
							debugf("No space to store bakup_header!\n");
							return OPERATE_WRITE_ERROR;
						}
						memcpy(bakup_header,(uchar *)g_eMMCBuf,EFI_SECTOR_SIZE);
						debugf("bakup_header->mMagicNum=0x%x\n",bakup_header->mMagicNum);
						if((bakup_header->mMagicNum==IMG_BAK_HEADER)
							&&((0 == strcmp("splloader",g_dl_eMMCStatus.curUserPartitionName))
							||(0 == strcmp("sml",g_dl_eMMCStatus.curUserPartitionName))
							||(0 == strcmp("trustos",g_dl_eMMCStatus.curUserPartitionName))
							||(0 == strcmp("uboot",  g_dl_eMMCStatus.curUserPartitionName)))){
								memset(bakup_header, 0, EFI_SECTOR_SIZE);
								free(bakup_header);
								if(!_img_bakup_check_and_write(g_dl_eMMCStatus.curUserPartitionName,nSectorCount,each_write_block))
									return OPERATE_WRITE_ERROR;
						}else{
							free(bakup_header);
							if(!_emmc_download_image(nSectorCount, each_write_block, TRUE))
								return OPERATE_WRITE_ERROR;
						}
				}
			}
		}
	} else {
		lastSize = EMMC_BUF_SIZE - g_status.unsave_recv_size;
		debugf("Unsaved buf size overflow the whole buffer size,lastsize=%u,unsavedsize=%u\n", lastSize, g_status.unsave_recv_size);
		memcpy((unsigned char *)g_sram_addr, buf, lastSize);

		g_status.unsave_recv_size = EMMC_BUF_SIZE;
		if (0 == (EMMC_BUF_SIZE % EFI_SECTOR_SIZE))
			nSectorCount = EMMC_BUF_SIZE / EFI_SECTOR_SIZE;
		else
			nSectorCount = EMMC_BUF_SIZE / EFI_SECTOR_SIZE + 1;

		if (!_emmc_download_image(nSectorCount, each_write_block, FALSE))
			return OPERATE_WRITE_ERROR;

		g_sram_addr = (uint32_t) (g_eMMCBuf + g_status.unsave_recv_size);
		memcpy((uchar *) g_sram_addr, (char *)(&buf[lastSize]), size - lastSize);
		g_status.unsave_recv_size += size - lastSize;
		g_sram_addr = (uint32_t) (g_eMMCBuf + g_status.unsave_recv_size);
		debugf("After write,unsaved recv size=%d\n", g_status.unsave_recv_size);
	}
	return OPERATE_SUCCESS;
}



//#ifdef CONFIG_SECURE_BOOT
#if defined (CONFIG_SECURE_BOOT)||defined (CONFIG_SECBOOT)
int _check_secure_part(uchar * partition_name)
{
	int i = 0;
	do {
		if (0 == strcmp(s_force_secure_check[i], partition_name))
			return 1;
		i++;
	}
	while (s_force_secure_check[i] != 0);

	return 0;
}

static int _emmc_secure_download(uchar * partition_name)
{
	unsigned long count = 0;
	unsigned int saved = 0;
	unsigned int each;
	unsigned int base_sector = g_dl_eMMCStatus.base_sector;
	unsigned int ret = 0;

	debugf("enter secure_image_flag = %d partition:%s \n", secure_image_flag, partition_name);
	switch (secure_image_flag) {
	case NORMALE_IMAGE:	// 1 boot image,uboot image, etc...
		if (strcmp("uboot", partition_name) == 0 || strcmp("uboot_bak",partition_name) == 0) {
			dl_secure_verify("splloader",g_eMMCBuf,0);
		}
#ifdef CONFIG_SECBOOT
	#ifdef CONFIG_SMLBOOT
		else if(strcmp("sml",partition_name) == 0 || strcmp("trustos",partition_name) == 0 || strcmp("sml_bak",partition_name) == 0 ||
			strcmp("trustos_bak",partition_name) == 0){
			dl_secure_verify("fdl1",g_eMMCBuf,0);
		}
	#endif
#endif
		else {
			dl_secure_verify("fdl2",g_eMMCBuf,0);
		}
		break;
	case SPL_IMAGE:	//  2 spl image
		dl_secure_verify("splloader0",g_eMMCBuf,0);
		break;
	case SYSTEM_IMAGE:	//  3 system image
#ifdef CONFIG_SYSTEM_VERIFY
		debugf("Check system image \n");
		Sec_SHA1_Finish();
		ret = secure_verify_system_end("fdl2", g_eMMCBuf, 0);
		debugf("secure_verify_system  ret = %d \n", ret);
		if (ret == 0) {
			debugf("verify system failed! Erase it! \n", ret);
			ret = dl_erase("system", 0);
			debugf("fdl2_erase ret = %d \n", ret);
			return 0;
		} else {
			debugf("secure_verify_system success. \n", ret);
			return 1;
		}
#endif
		return 1;
	default:
		// do nothing
		return 1;
	}

	//download the image
	if (0 == (g_status.unsave_recv_size % EFI_SECTOR_SIZE))
		count = g_status.unsave_recv_size / EFI_SECTOR_SIZE;
	else
		count = g_status.unsave_recv_size / EFI_SECTOR_SIZE + 1;
	if(img_backup_flag==1)
	{
		if(!_img_bakup_check_and_write(g_dl_eMMCStatus.curUserPartitionName,count,EMMC_MAX_MUTIL_WRITE)){
			g_status.unsave_recv_size = 0;
			return 0;
			}

	}
	else{
		//write code
		while (count) {
			each = MIN(count, EMMC_MAX_MUTIL_WRITE);
			if (!Emmc_Write(g_dl_eMMCStatus.curEMMCArea, base_sector, each, (unsigned char *)(g_eMMCBuf + (saved * EFI_SECTOR_SIZE)))) {
				g_status.unsave_recv_size = 0;
				return 0;
			}
			base_sector += each;
			saved += each;
			count -= each;
		}
	}
	g_status.unsave_recv_size = 0;
	secure_image_flag = UNABLE_SECURE;	// 0;
	return 1;
}
#endif
int _emmc_download_image(uint32_t nSectorCount, uint32_t each_write_block, int is_total_recv)
{
	uint32_t cnt;
	uint32_t base_sector;
	uint32_t trans_times;
	uint32_t remain_block;
	uchar *point;
	int32_t retval;
//#ifdef CONFIG_SECURE_BOOT
#if defined (CONFIG_SECURE_BOOT) || defined (CONFIG_SECBOOT)
	unsigned char is_header = FALSE;
	unsigned int code_off = 0;

	debugf("enter secure_image_flag = %d \n", secure_image_flag);
	switch (secure_image_flag) {
	case NORMALE_IMAGE:	// 1 boot image,uboot image, etc...
		// no break here
	case SPL_IMAGE:	// 2 spl image
		return 1;
	case SYSTEM_IMAGE:	//  3 system image
#ifdef CONFIG_SYSTEM_VERIFY
		debugf("Calculate SHA \n");
		is_header = secure_verify_system_start("fdl2", g_eMMCBuf, g_status.unsave_recv_size);

		debugf("is_header = %d \n", is_header);
		if (is_header) {
			code_off = VLR_CODE_OFF;
			g_status.unsave_recv_size -= VLR_CODE_OFF;
			if (0 == (g_status.unsave_recv_size % EFI_SECTOR_SIZE))
				nSectorCount = g_status.unsave_recv_size / EFI_SECTOR_SIZE;
			else
				nSectorCount = g_status.unsave_recv_size / EFI_SECTOR_SIZE + 1;
		}
#endif
		break;
	default:
		// do nothing
		break;
	}
#endif

	if (IMG_WITH_SPARSE == g_dl_eMMCStatus.curImgType) {
		debugf("Handle the saving of image with sparse,name=%s,buf start at 0x%x,size=0x%x\n",
		       g_dl_eMMCStatus.curUserPartitionName, g_eMMCBuf, g_status.unsave_recv_size);
		retval = write_simg2emmc("mmc", 0, g_dl_eMMCStatus.curUserPartitionName, g_eMMCBuf, g_status.unsave_recv_size);
		if (-1 == retval) {
			g_status.unsave_recv_size = 0;
			debugf("Write sparse img fail\n");
			return 0;
		}
		debugf("Write packed img success,return value=%d\n", retval);
	} else {
		uint32_t count = nSectorCount;
		uint32_t saved = 0;
		uint32_t each;
		uint32_t base_sector = g_dl_eMMCStatus.base_sector;
		while (count) {
			each = MIN(count, each_write_block);
#ifdef CONFIG_SECURE_BOOT
			if (!Emmc_Write(g_dl_eMMCStatus.curEMMCArea, base_sector, each, (unsigned char *)(g_eMMCBuf + (saved * EFI_SECTOR_SIZE) + code_off)))
#else
			if (!Emmc_Write(g_dl_eMMCStatus.curEMMCArea, base_sector, each, (uchar *) (g_eMMCBuf + (saved * EFI_SECTOR_SIZE))))
#endif
			{
				g_status.unsave_recv_size = 0;
				return 0;
			}
			base_sector += each;
			saved += each;
			count -= each;
		}
	}

	if (IMG_WITH_SPARSE == g_dl_eMMCStatus.curImgType) {
		if (retval > 0) {
			memmove(g_eMMCBuf, g_eMMCBuf + retval, g_status.unsave_recv_size - retval);
			g_status.unsave_recv_size -= retval;
			g_sram_addr = (uint32_t) (g_eMMCBuf + g_status.unsave_recv_size);
			debugf("After simg , unsave_recv_size=%d, saved value=%d\n", g_status.unsave_recv_size, retval);
		} else {
			g_status.unsave_recv_size = 0;
			g_sram_addr = (uint32_t) g_eMMCBuf;
		}
	} else {
		g_status.unsave_recv_size = 0;
		if (!is_total_recv) {
			g_dl_eMMCStatus.base_sector += nSectorCount;
			g_sram_addr = (uint32_t) g_eMMCBuf;
		}
	}
	return 1;
}

/**
	Function used for reading nv partition which has crc protection.
*/
int32_t _read_nv_with_backup(uchar * partition_name, uint8_t * buf, uint32_t size)
{
	uchar *backup_partition_name = NULL;
	uint8_t header_buf[EFI_SECTOR_SIZE];
	uint32_t base_sector = 0;
	uint16_t checkSum = 0;
	uint32_t magic = 0;
	uint32_t len = 0;
	nv_header_t *header_p = NULL;

	header_p = header_buf;
	base_sector = efi_GetPartBaseSec(partition_name);
	if (0 == base_sector)
		return 0;
	/*read origin image header */
	memset(header_buf, 0, EFI_SECTOR_SIZE);
	if (!Emmc_Read(PARTITION_USER, base_sector, 1, header_buf)) {
		debugf("_read_nv_with_backup read origin image header failed\n");
		return 0;
	}
	memcpy(&magic, header_buf, 4);
	if (NV_HEAD_MAGIC == magic) {
		base_sector++;
	}
	debugf("_read_nv_with_backup origin image magic = 0x%x\n", magic);

	/*read origin image */
	memset(buf, 0xFF, size);
	if (Emmc_Read(PARTITION_USER, base_sector, (size >> 9) + 1, (uint8_t *) buf)) {
		/* get length and checksum */
		if (NV_HEAD_MAGIC == magic) {
			len = header_p->len;
			checkSum = header_p->checksum;
		} else {
			len = size - 4;
			checkSum = (uint16_t) ((((uint16_t) buf[size - 3]) << 8) | ((uint16_t) buf[size - 4]));
		}
		/*check crc */
		if (fdl_check_crc(buf, len, checkSum)) {
			return 1;
		}
	}

	/*get the backup partition name */
	backup_partition_name = _get_backup_partition_name(partition_name);
	if (NULL == backup_partition_name) {
		return 0;
	}
	base_sector = efi_GetPartBaseSec(backup_partition_name);

	/*read backup header */
	memset(header_buf, 0, EFI_SECTOR_SIZE);
	if (!Emmc_Read(PARTITION_USER, base_sector, 1, header_buf)) {
		debugf("_read_nv_with_backup read backup image header failed\n");
		return 0;
	}
	memcpy(&magic, header_buf, 4);
	if (NV_HEAD_MAGIC == magic) {
		base_sector++;
	}
	debugf("_read_nv_with_backup backup image magic = 0x%x\n", magic);

	/*read bakup image */
	memset(buf, 0xFF, size);
	if (Emmc_Read(PARTITION_USER, base_sector, (size >> 9) + 1, (uint8_t *) buf)) {
		/*get length and checksum */
		if (NV_HEAD_MAGIC == magic) {
			len = header_p->len;
			checkSum = header_p->checksum;
		} else {
			len = size - 4;
			checkSum = (uint16_t) ((((uint16_t) buf[size - 3]) << 8) | ((uint16_t) buf[size - 4]));
		}
		/*check crc */
		if (!fdl_check_crc(buf, len, checkSum)) {
			debugf("read backup image checksum error \n");
			return 0;
		}
	}
	return 1;
}

int _nv_img_check_and_write(uchar * partition, uint32_t size)
{
	uint32_t nSectorCount;
	uint32_t nSectorBase;
	uint16_t sum = 0;
	uint16_t *dataaddr;
	uchar *backup_partition_name = NULL;
	uint8_t header_buf[EFI_SECTOR_SIZE];
	nv_header_t *nv_header_p = NULL;
	//uint32 checksum = 0;

	if (0 == ((size + 4) % EFI_SECTOR_SIZE))
		nSectorCount = (size + 4) / EFI_SECTOR_SIZE;
	else
		nSectorCount = (size + 4) / EFI_SECTOR_SIZE + 1;

	memset(header_buf, 0x00, EFI_SECTOR_SIZE);
	nv_header_p = header_buf;
	nv_header_p->magic = NV_HEAD_MAGIC;
	nv_header_p->len = size;
	nv_header_p->checksum = (uint32_t) fdl_calc_checksum(g_eMMCBuf, size);
	nv_header_p->version = NV_VERSION;
	/*write the original partition */
	_emmc_real_erase_partition(partition);
	debugf("Start to write first block of NV partition\n");
	if (!Emmc_Write(g_dl_eMMCStatus.curEMMCArea, g_dl_eMMCStatus.base_sector, 1, (unsigned char *)header_buf)) {
		debugf("original header %s write error! \n", partition);
		return 0;
	}
	g_dl_eMMCStatus.base_sector++;
	debugf("Start to write remain blocks of NV partition\n");
	if (!Emmc_Write(g_dl_eMMCStatus.curEMMCArea, g_dl_eMMCStatus.base_sector, nSectorCount, (unsigned char *)g_eMMCBuf)) {
		debugf("original %s write error! \n", partition);
		return 0;
	}

	/*write the backup partition */
	backup_partition_name = _get_backup_partition_name(partition);
	nSectorBase = efi_GetPartBaseSec(backup_partition_name);
	if (0 == nSectorBase)
		return 0;
	_emmc_real_erase_partition(backup_partition_name);
	if (!Emmc_Write(g_dl_eMMCStatus.curEMMCArea, nSectorBase, 1, (uchar *) header_buf)) {
		debugf("backup header %s write error! \n", partition);
		return 0;
	}
	nSectorBase++;
	if (!Emmc_Write(g_dl_eMMCStatus.curEMMCArea, nSectorBase, nSectorCount, (unsigned char *)g_eMMCBuf)) {
		debugf("backup %s write error! \n", partition);
		return 0;
	}
	g_status.unsave_recv_size = 0;
	return 1;
}
int _write_img_to_partition(uint32_t nSectorCount, uint32_t each_write_block)
{
	uint32_t count = nSectorCount;
	uint32_t saved = 0;
	uint32_t each=0;
	uint32_t base_sector=g_dl_eMMCStatus.base_sector;
	while(count) {
		each = MIN(count,each_write_block);
		if (!Emmc_Write(g_dl_eMMCStatus.curEMMCArea, base_sector,
			each, (uchar *) (g_eMMCBuf + (saved * EFI_SECTOR_SIZE)))) {
			g_status.unsave_recv_size = 0;
			return 0;
		}
	base_sector += each;
	saved += each;
	count -= each;
	}
}

int _img_bakup_check_and_write(uchar *partition, uint32_t nSectorCount, uint32_t each_write_block)
{
	if(img_backup_flag==0){
#if defined CONFIG_SECBOOT
		img_backup_flag=1;
		return 1;
#endif
	}
	img_backup_flag=0;
	if (0 == strcmp("splloader",  partition)) {
		debugf("partition write BOOT1\n");
		g_dl_eMMCStatus.curEMMCArea = PARTITION_BOOT1;
		g_dl_eMMCStatus.base_sector =  0;
		if( !_write_img_to_partition(nSectorCount,each_write_block)){
			debugf("boot1 %s write error! \n", partition);
			return 0;
		}

	//write backup partition
		g_dl_eMMCStatus.curEMMCArea = PARTITION_BOOT2;
		g_dl_eMMCStatus.base_sector =  0;
		if(!_write_img_to_partition(nSectorCount,each_write_block)){
			debugf("boot2 %s write error! \n", partition);
			return 0;
		}
	}else{
		g_dl_eMMCStatus.curEMMCArea = PARTITION_USER;
		g_dl_eMMCStatus.base_sector  = efi_GetPartBaseSec(partition);
		if(!_write_img_to_partition(nSectorCount,each_write_block)){
			debugf("org  %s write error! \n", partition);
			return 0;
		}
	//write backup partition
		strcat(partition,"_bak");
		g_dl_eMMCStatus.base_sector  = efi_GetPartBaseSec(partition);
		if( !_write_img_to_partition(nSectorCount,each_write_block)){
			debugf("bak  %s write error! \n", partition);
			return 0;
		}

	}
	g_status.unsave_recv_size = 0;
	return 1;
}

void _checkNVPartition(void)
{
	uint8_t *ori_buf;
	uint8_t *backup_buf;
	uchar *backup_partition_name = NULL;
	uint8_t ori_header_buf[EFI_SECTOR_SIZE];
	uint8_t backup_header_buf[EFI_SECTOR_SIZE];
	uint32_t base_sector;
	uint16_t checkSum = 0;
	uint32_t len = 0;
	nv_header_t *header_p = NULL;
	uint8_t status = 0;
	uint32_t size = FIXNV_SIZE;

	debugf("check nv partition enter\n");
	ori_buf = malloc(size + EFI_SECTOR_SIZE);
	if (!ori_buf) {
		debugf("check nv partition malloc oribuf failed\n");
		return;
	}
	debugf("check nv partition ori_buf 0x%x\n", ori_buf);
	backup_buf = malloc(size + EFI_SECTOR_SIZE);
	if (!backup_buf) {
		debugf("check nv partition malloc backup_buf failed\n");
		free(ori_buf);
		return;
	}
	debugf("check nv partition backup_buf 0x%x\n", backup_buf);
	header_p = ori_header_buf;
	base_sector = efi_GetPartBaseSec(g_dl_eMMCStatus.curUserPartitionName);
	if (0 == base_sector) {
		free(ori_buf);
		free(backup_buf);
		return;
	}
	/*read origin image header */
	memset(ori_header_buf, 0, EFI_SECTOR_SIZE);
	if (!Emmc_Read(PARTITION_USER, base_sector, 1, ori_header_buf)) {
		debugf("_checkNVPartition read origin image header failed\n");
		free(ori_buf);
		free(backup_buf);
		return;
	}
	if (NV_HEAD_MAGIC == header_p->magic) {
		base_sector++;
	}
	debugf("_checkNVPartition origin image magic = 0x%x\n", header_p->magic);

	/*read origin image */
	memset(ori_buf, 0xFF, size + EFI_SECTOR_SIZE);
	if (Emmc_Read(PARTITION_USER, base_sector, ((size + EFI_SECTOR_SIZE - 1) & ~(EFI_SECTOR_SIZE - 1)) >> 9, (uint8_t *) ori_buf)) {
		/*get length and checksum */
		if (NV_HEAD_MAGIC == header_p->magic) {
			len = header_p->len;
			checkSum = header_p->checksum;
		} else {
			len = size - 4;
			checkSum = (uint16_t) ((((uint16_t) ori_buf[size - 3]) << 8) | ((uint16_t) ori_buf[size - 4]));
		}
		/*check ecc */
		if (fdl_check_crc(ori_buf, len, checkSum)) {
			status |= 1;
		}
	}

	/*get the backup partition name */
	backup_partition_name = _get_backup_partition_name(g_dl_eMMCStatus.curUserPartitionName);
	if (NULL == backup_partition_name) {
		free(ori_buf);
		free(backup_buf);
		return;
	}
	base_sector = efi_GetPartBaseSec(backup_partition_name);
	/*read backup header */
	header_p = backup_header_buf;
	memset(backup_header_buf, 0, EFI_SECTOR_SIZE);
	if (!Emmc_Read(PARTITION_USER, base_sector, 1, backup_header_buf)) {
		debugf("_read_nv_with_backup read backup image header failed\n");
		free(ori_buf);
		free(backup_buf);
		return;
	}

	if (NV_HEAD_MAGIC == header_p->magic) {
		base_sector++;
	}
	debugf("_read_nv_with_backup backup image magic = 0x%x\n", header_p->magic);

	/*read bakup image */
	memset(backup_buf, 0xFF, size + EFI_SECTOR_SIZE);
	if (Emmc_Read(PARTITION_USER, base_sector, ((size + EFI_SECTOR_SIZE - 1) & ~(EFI_SECTOR_SIZE - 1)) >> 9, (uint8_t *) backup_buf)) {
		/*get length and checksum */
		if (NV_HEAD_MAGIC == header_p->magic) {
			len = header_p->len;
			checkSum = header_p->checksum;
		} else {
			len = size - 4;
			checkSum = (uint16_t) ((((uint16_t) backup_buf[size - 3]) << 8) | ((uint16_t) backup_buf[size - 4]));
		}
		/*check ecc */
		if (fdl_check_crc(backup_buf, len, checkSum)) {
			status |= 2;	//four status:00,01,10,11
		}
	}
	switch (status) {
	case 0:
		debugf("both org and bak partition are damaged!\n");
		break;
	case 1:
		debugf("bak partition is damaged!\n");
		base_sector = efi_GetPartBaseSec(backup_partition_name);
		header_p = ori_header_buf;
		if (NV_HEAD_MAGIC == header_p->magic) {
			if (Emmc_Write(PARTITION_USER, base_sector, 1, ori_header_buf)) {
				debugf("write backup nv header success\n");
				base_sector++;
			}
		}
		/*write one more sector */
		if (Emmc_Write(PARTITION_USER, base_sector, ((size + EFI_SECTOR_SIZE - 1) & ~(EFI_SECTOR_SIZE - 1)) >> 9, (uint8_t *) ori_buf)) {
			debugf("write backup nv body success\n");
		}
		break;
	case 2:
		debugf("org partition is damaged!\n!");
		base_sector = efi_GetPartBaseSec(g_dl_eMMCStatus.curUserPartitionName);
		header_p = backup_header_buf;
		if (NV_HEAD_MAGIC == header_p->magic) {
			if (Emmc_Write(PARTITION_USER, base_sector, 1, backup_header_buf)) {
				debugf("write original partition header success\n");
				base_sector++;
			}
		}
		if (Emmc_Write(PARTITION_USER, base_sector, ((size + EFI_SECTOR_SIZE - 1) & ~(EFI_SECTOR_SIZE - 1)) >> 9, (uint8_t *) backup_buf)) {
			debugf("write original partition body success\n");
		}
		break;
	case 3:
		debugf("both org and bak partition are ok!\n");
		break;
	default:
		debugf("status error!\n");
		break;
	}
	free(backup_buf);
	free(ori_buf);
	return;
}

OPERATE_STATUS dl_read_start(uchar * partition_name, uint32_t size)
{
	uint32_t size_in_lba = 0;
	uint32_t len = 0;
	static uint32_t part_offset = 0;

	 if (NULL != strstr(partition_name, "userdata")) {
		if (0 == strcmp("userdata_size", partition_name)) {
			strcpy(g_dl_eMMCStatus.curUserPartitionName, "userdata_size");
		} else {
			len =  strlen(partition_name) -  strlen("userdata");
			partition_name = partition_name + strlen("userdata");
			while (len > 0) {
				if(*partition_name <'0' || *partition_name >'9')
					return OPERATE_INCOMPATIBLE_PART;
				len--;
				partition_name++;
			}
			debugf("%s : part_offset:%d\n", partition_name, part_offset);
			g_dl_eMMCStatus.base_sector = efi_GetPartBaseSec("userdata") + part_offset;
			part_offset += size / EFI_SECTOR_SIZE;
			strcpy(g_dl_eMMCStatus.curUserPartitionName, "userdata");
	 	}
		g_dl_eMMCStatus.curEMMCArea = PARTITION_USER;
		g_dl_eMMCStatus.part_total_size = MAX_SIZE_FLAG;
		return OPERATE_SUCCESS;
	}

	if (!_get_compatible_partition(partition_name))
		return OPERATE_INCOMPATIBLE_PART;

	g_dl_eMMCStatus.curEMMCArea = PARTITION_USER;

	if (0 == strcmp("splloader", g_dl_eMMCStatus.curUserPartitionName)) {
		g_dl_eMMCStatus.curEMMCArea = PARTITION_BOOT1;
		g_dl_eMMCStatus.part_total_size = Emmc_GetCapacity(PARTITION_BOOT1);
		g_dl_eMMCStatus.base_sector = 0;
	} else if (0 == strcmp("uboot", g_dl_eMMCStatus.curUserPartitionName)) {
		g_dl_eMMCStatus.curEMMCArea = PARTITION_BOOT2;
		g_dl_eMMCStatus.part_total_size = Emmc_GetCapacity(PARTITION_BOOT2);
		g_dl_eMMCStatus.base_sector = 0;
	} else {
		size_in_lba = efi_GetPartSize(g_dl_eMMCStatus.curUserPartitionName);
		/*max size of uint32 is 4GB, so we take care of the overflow*/
		if (size_in_lba > (MAX_SIZE_FLAG / EFI_SECTOR_SIZE))
			g_dl_eMMCStatus.part_total_size = MAX_SIZE_FLAG;
		else
			g_dl_eMMCStatus.part_total_size = size_in_lba * EFI_SECTOR_SIZE;
		g_dl_eMMCStatus.base_sector = efi_GetPartBaseSec(g_dl_eMMCStatus.curUserPartitionName);
	}

	if (size > g_dl_eMMCStatus.part_total_size) {
		debugf("size(0x%x) beyond max size:0x%x!\n", size, g_dl_eMMCStatus.part_total_size);
		return OPERATE_INVALID_SIZE;
	}

	if (0 == strcmp("prodnv", g_dl_eMMCStatus.curUserPartitionName)) {
		struct ext2_sblock *sblock = NULL;
		sblock = malloc(MAX(EFI_SECTOR_SIZE, sizeof(struct ext2_sblock)));
		if (!sblock) {
			debugf("malloc sblock failed\n");
			goto err;
		}

		if (!Emmc_Read(g_dl_eMMCStatus.curEMMCArea, g_dl_eMMCStatus.base_sector + 2,	/*superblock offset 2 sect */
			       1,	/*superblock take one sect */
			       sblock)) {
			free(sblock);
			goto err;
		}

		if (sblock->magic != EXT2_MAGIC) {
			debugf("bad prodnv image magic\n");
			free(sblock);
			goto err;
		}
		free(sblock);
	}

	return OPERATE_SUCCESS;
err:
	return OPERATE_SYSTEM_ERROR;
}

OPERATE_STATUS dl_read_midst(uint32_t size, uint32_t off, uchar * buf)
{
	uint32_t nSectorCount;
	uint32_t nSectorOffset;
	uint64_t part_size = 0;

	if ((size + off) > g_dl_eMMCStatus.part_total_size) {
		debugf("size(0x%x)+off(0x%x) beyond max size(0x%x)!\n", size, off, g_dl_eMMCStatus.part_total_size);
		return OPERATE_INVALID_SIZE;
	}

	if (PARTITION_PURPOSE_NV == g_dl_eMMCStatus.partitionpurpose) {
		if (_read_nv_with_backup(g_dl_eMMCStatus.curUserPartitionName, g_eMMCBuf, FIXNV_SIZE)) {
			memcpy(buf, (uchar *) (g_eMMCBuf + off), size);
			return OPERATE_SUCCESS;
		} else {
			return OPERATE_SYSTEM_ERROR;
		}
	} else if (0 == strcmp(g_dl_eMMCStatus.curUserPartitionName, "userdata_size")) {
		part_size = (uint64_t)efi_GetPartSize("userdata")* EFI_SECTOR_SIZE;
		sprintf((char *)buf, "0x%llx\n", part_size);
		debugf("userdata partition size: %s\n", buf);
	} else {
		if (0 == (size % EFI_SECTOR_SIZE))
			nSectorCount = size / EFI_SECTOR_SIZE;
		else
			nSectorCount = size / EFI_SECTOR_SIZE + 1;

		nSectorOffset = off / EFI_SECTOR_SIZE;
		if (!Emmc_Read(g_dl_eMMCStatus.curEMMCArea, g_dl_eMMCStatus.base_sector + nSectorOffset, nSectorCount, buf)) {
			debugf("read error!\n");
			return OPERATE_SYSTEM_ERROR;
		}
	}

	return OPERATE_SUCCESS;
}

OPERATE_STATUS dl_read_end(void)
{
	/*Just send ack to tool in emmc */
	debugf("g_dl_eMMCStatus.partitionpurpose = %d\n", g_dl_eMMCStatus.partitionpurpose);
	if (PARTITION_PURPOSE_NV == g_dl_eMMCStatus.partitionpurpose)
		_checkNVPartition();

	return OPERATE_SUCCESS;
}

OPERATE_STATUS dl_download_start(uchar * partition_name, uint32_t size, uint32_t nv_checksum)
{
	int i = 0;
	uint32_t size_in_lba = 0;

	g_status.total_size = size;

//#ifdef CONFIG_SECURE_BOOT
#if defined (CONFIG_SECURE_BOOT) || defined (CONFIG_SECBOOT)
	if (_check_secure_part(partition_name)) {
		check_secure_flag = ENABLE_SECURE;	//1;
		secure_image_flag = ENABLE_SECURE;	//1;
	} else {
		check_secure_flag = UNABLE_SECURE;	//0;
		secure_image_flag = UNABLE_SECURE;	//0;
	}
#endif
	if (!_get_compatible_partition(partition_name)) {
		return OPERATE_INCOMPATIBLE_PART;
	}
	if ((g_status.total_size > EMMC_BUF_SIZE) && (IMG_WITH_SPARSE == g_dl_eMMCStatus.curImgType)) {
		debugf("g_status.total_size>EMMC_BUF_SIZE!\n");
		return OPERATE_INVALID_SIZE;
	}
	if (PARTITION_PURPOSE_NV == g_dl_eMMCStatus.partitionpurpose) {
		debugf("partition purpose is NV\n");
		g_dl_eMMCStatus.curEMMCArea = PARTITION_USER;
		g_dl_eMMCStatus.part_total_size = EFI_SECTOR_SIZE * efi_GetPartSize(g_dl_eMMCStatus.curUserPartitionName);
		if (size > FIXNV_SIZE) {
			debugf("size(0x%x) beyond FIXNV_SIZE:0x%x !\n", size, FIXNV_SIZE);
			return OPERATE_INVALID_SIZE;
		}
		g_dl_eMMCStatus.base_sector = efi_GetPartBaseSec(g_dl_eMMCStatus.curUserPartitionName);
		debugf("NV partition:%s, base sector: 0x%x, size: 0x%x\n", partition_name, g_dl_eMMCStatus.base_sector, g_dl_eMMCStatus.part_total_size);
		memset(g_eMMCBuf, 0xff, FIXNV_SIZE + EFI_SECTOR_SIZE);
		g_checksum = nv_checksum;
	} else if (0 == strcmp("splloader", g_dl_eMMCStatus.curUserPartitionName)) {
//#ifdef CONFIG_SECURE_BOOT
#if defined (CONFIG_SECURE_BOOT)||defined (CONFIG_SECBOOT)
		secure_image_flag = SPL_CHECK;	//2;
#endif
		g_dl_eMMCStatus.curEMMCArea = PARTITION_BOOT1;
		g_dl_eMMCStatus.part_total_size = Emmc_GetCapacity(PARTITION_BOOT1);
		debugf("SPL total size: 0x%x\n", g_dl_eMMCStatus.part_total_size);
		g_dl_eMMCStatus.base_sector = 0;
		memset(g_eMMCBuf, 0xff, g_dl_eMMCStatus.part_total_size);
	} else if (0 == strcmp("uboot", g_dl_eMMCStatus.curUserPartitionName)) {
		g_dl_eMMCStatus.curEMMCArea = PARTITION_BOOT2;
		g_dl_eMMCStatus.part_total_size = Emmc_GetCapacity(PARTITION_BOOT2);
		debugf("UBOOT total size: 0x%x\n", g_dl_eMMCStatus.part_total_size);
		g_dl_eMMCStatus.base_sector = 0;
		memset(g_eMMCBuf, 0xff, g_dl_eMMCStatus.part_total_size);
	} else {
		debugf("partition purpose is normal\n");
#if defined (CONFIG_SECURE_BOOT)||defined (CONFIG_SECBOOT)
		if (strcmp("system", g_dl_eMMCStatus.curUserPartitionName) == 0) {

			debugf("Downloading system: Set secure_image_flag to 3. \n");
			secure_image_flag = SYSTEM_CHECK;	//3;
#ifdef CONFIG_SYSTEM_VERIFY
			Sec_SHA1_Start();
#endif
		}
#endif
		g_dl_eMMCStatus.curEMMCArea = PARTITION_USER;

/*
		if (IMG_WITH_SPARSE != g_dl_eMMCStatus.curImgType)
			_emmc_real_erase_partition(g_dl_eMMCStatus.curUserPartitionName);
		else
			memset(g_eMMCBuf, 0, EMMC_BUF_SIZE);
*/
		size_in_lba = efi_GetPartSize(g_dl_eMMCStatus.curUserPartitionName);
		/*max size of uint32 is 4GB, so we take care of the overflow*/
		if (size_in_lba > (MAX_SIZE_FLAG / EFI_SECTOR_SIZE))
			g_dl_eMMCStatus.part_total_size = MAX_SIZE_FLAG;
		else
			g_dl_eMMCStatus.part_total_size = size_in_lba * EFI_SECTOR_SIZE;
		g_dl_eMMCStatus.base_sector = efi_GetPartBaseSec(g_dl_eMMCStatus.curUserPartitionName);
	}
	if (size > g_dl_eMMCStatus.part_total_size) {
			debugf("size(0x%x) beyond max size:0x%x!\n", size, g_dl_eMMCStatus.part_total_size);
			return OPERATE_INVALID_SIZE;
	}
	g_status.total_recv_size = 0;
	g_status.unsave_recv_size = 0;
	g_sram_addr = (uint32_t) g_eMMCBuf;

	prepare_alternative_buffers();
	return OPERATE_SUCCESS;
}

OPERATE_STATUS dl_download_midst(uint16_t size, char *buf)
{
	uint32_t lastSize;
	uint32_t nSectorCount;
	uint32_t each_write_block = EMMC_MAX_MUTIL_WRITE;
	uint32_t fix_nv_checksum;
	int ret = 0;

	int local_buf_index;
	sparse_header_t sparse_header_test_head;

	/*adjust the image type via the header magic*/
	if (0 == g_status.total_recv_size ) {
		memset(&sparse_header_test_head, 0, sizeof(sparse_header_t));
		memcpy(&sparse_header_test_head, buf, sizeof(sparse_header_test_head));

		if (sparse_header_test_head.magic != SPARSE_HEADER_MAGIC)
			g_dl_eMMCStatus.curImgType = IMG_RAW;
		else
			g_dl_eMMCStatus.curImgType = IMG_WITH_SPARSE;
	}

	if ((g_status.total_recv_size + size) > g_status.total_size) {
		debugf("size+recvd>total_size!\n");
		return OPERATE_INVALID_SIZE;
	}

	g_status.total_recv_size += size;

#ifdef CONFIG_SECURE_BOOT
	if (check_secure_flag == ENABLE_SECURE /*1 */ ) {
		check_secure_flag = UNABLE_SECURE;	//0;
		if (secure_header_parser(buf) != 1) {
			debugf("check secure image failed!! \n");
			secure_image_flag = UNABLE_SECURE;	//0;
			return OPERATE_WRITE_ERROR;
		}
	}
#endif
#ifdef CONFIG_SECBOOT
	if(check_secure_flag == ENABLE_SECURE)
		check_secure_flag = UNABLE_SECURE;
#endif

#if defined (CONFIG_SECURE_BOOT)||defined (CONFIG_SECBOOT)

#ifdef CONFIG_SYSTEM_VERIFY
	ret = regular_download_process(size, buf);
#else

#if 1
	ret = regular_download_process(size, buf);
#else
	if (SYSTEM_CHECK == secure_image_flag)
		ret = speedup_download_process(size, buf);
	else
		ret = regular_download_process(size, buf);
#endif
#endif

#else
	if ((IMG_RAW == g_dl_eMMCStatus.curImgType) && (g_status.total_size > ALTERNATIVE_BUFFER_SIZE))
		ret = speedup_download_process(size, buf);
	else
		ret = regular_download_process(size, buf);
#endif

	return ret;
}



OPERATE_STATUS dl_download_end(void)
{
//#ifdef CONFIG_SECURE_BOOT
#if defined (CONFIG_SECURE_BOOT) || defined (CONFIG_SECBOOT)
	if (_emmc_secure_download(g_dl_eMMCStatus.curUserPartitionName) != 1) {
		debugf("%s:_emmc_secure_download error!\n");
		return OPERATE_WRITE_ERROR;
	}
#endif
	if (g_status.unsave_recv_size != 0) {
		debugf("unsaved size is not zero!\n");
		return OPERATE_SYSTEM_ERROR;
	}

	g_status.total_size = 0;
	return OPERATE_SUCCESS;
}

OPERATE_STATUS dl_erase(uchar * partition_name, uint32_t size)
{
	int32_t retval;
	uint32_t part_size;
	uchar *backup_partition_name = NULL;

	if ((0 == strcmp(partition_name, "erase_all")) && (0xffffffff == size)) {
		debugf("Erase all!\n");
		if (!_emmc_erase_allflash()) {
			return OPERATE_WRITE_ERROR;
		}
	} else {
		debugf("erase partition %s!\n", partition_name);
		if (!_get_compatible_partition(partition_name)) {
			return OPERATE_INCOMPATIBLE_PART;
		}

		if (!_emmc_real_erase_partition(g_dl_eMMCStatus.curUserPartitionName)) {
			return OPERATE_WRITE_ERROR;
		}

		backup_partition_name = _get_backup_partition_name(g_dl_eMMCStatus.curUserPartitionName);

		if (NULL != backup_partition_name) {
			if (!_emmc_real_erase_partition(backup_partition_name)) {
				return OPERATE_WRITE_ERROR;
			}
		}

	}

	return OPERATE_SUCCESS;
}

/*Now we use the original disk partition code in part_efi.c to do the repartition */
OPERATE_STATUS dl_repartition(uchar * partition_cfg, uint16_t total_partition_num,
									uchar version, uchar size_unit)
{
	int16_t i = 0;
	block_dev_desc_t *dev_desc;
	disk_partition_t *partition_info;
	int32_t res = 0;

	/*get device description */
	dev_desc = get_dev("mmc", 0);
	if (NULL == dev_desc) {
		debugf("Get no storage device for repartition!\n");
		dev_desc = get_dev("mmc", 0);
		if (NULL == dev_desc) {
			printf("%s:Still get no storage device for repartition,return!\n", __FUNCTION__);
			return OPERATE_DEVICE_INIT_ERROR;
		}
		debugf("get storage device for repartition successfull in second time!\n");
	}

	/*prepare mem to store <total_partition_num> partitions' info */
	partition_info = malloc(sizeof(disk_partition_t) * total_partition_num);
	if (NULL == partition_info) {
		debugf("No space to store partition_info!\n");
		return OPERATE_SYSTEM_ERROR;
	}

	res = _parser_repartition_cfg(partition_info, partition_cfg, total_partition_num, version, size_unit);
	if (res < 0) {
		free(partition_info);
		return OPERATE_SYSTEM_ERROR;
	}

	res = _check_partition_table(partition_info, total_partition_num);
	if (PART_SAME == res) {
		debugf("Partition Config same with before!\n");
		free(partition_info);
		return OPERATE_SUCCESS;
	}

	/*retry for three times */
	for (i = 0; i < 3; i++) {
		gpt_restore(dev_desc, SPRD_DISK_GUID_STR, partition_info, total_partition_num);
		if (PART_SAME == _check_partition_table(partition_info, total_partition_num)) {
			/*restore success and break */
			break;
		}
	}

	free(partition_info);

	if (i < 3) {
		/*update the partition type after real repartition in disk*/
		init_part(dev_desc);
		return OPERATE_SUCCESS;
	} else {
		/*i==3 means restore fail */
		return OPERATE_SYSTEM_ERROR;
	}
}

OPERATE_STATUS dl_read_ref_info(char *part_name, uint16_t size, uint64_t offset,
	uchar *receive_buf, uchar  *transmit_buf)
{
	uint32_t base_sector = 0;

	base_sector = efi_GetPartBaseSec(part_name) + offset/EFI_SECTOR_SIZE;
	if(!Emmc_Read(PARTITION_USER, base_sector, (size >> 9), transmit_buf)) {
		/* read old info from flash fail, transmit the receive info back to tool */
		debugf("read old info from flash fail\n");
		memcpy(transmit_buf, receive_buf, (size_t)size);
	}
	/* update receive ref info to flash */
	Emmc_Write(PARTITION_USER, base_sector, (size >> 9), receive_buf);
	return OPERATE_SUCCESS;
}

