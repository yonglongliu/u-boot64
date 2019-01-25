#ifndef _FDL_EMMC_OPERATE_H
#define _FDL_EMMC_OPERATE_H

//#include <asm/arch/fdl_stdio.h>
#include "dl_cmd_def.h"
#include "packet.h"
#ifdef CONFIG_EMMC_BOOT
#include "part.h"

#define IMG_BAK_HEADER 0x42544844

#define EFI_SECTOR_SIZE 		(512)
#define EMMC_MAX_MUTIL_WRITE  (0x8000)
#define ERASE_SECTOR_SIZE		((64 * 1024) / EFI_SECTOR_SIZE)

/*buffer must over 50M ,otherwise download userdata.img will fail*/
#define EMMC_BUF_SIZE			(((256 * 1024 * 1024) / EFI_SECTOR_SIZE) * EFI_SECTOR_SIZE)

#define MAX_GPT_PARTITION_SUPPORT  (50)
#define EMMC_ERASE_ALIGN_LENGTH  (0x800)

#define MAGIC_DATA	0xAA55A5A5
#define CHECKSUM_START_OFFSET	0x28
#define MAGIC_DATA_SAVE_OFFSET	(0x20/4)
#define CHECKSUM_SAVE_OFFSET	(0x24/4)

#define UBOOT_PARTITION_NAME   "uboot"
#define SPL_PARTITION_NAME          "splloader"

#define SPRD_DISK_GUID_STR "11111111-2222-3333-4444-000000000000"

/*according to standard ,the first usable LBA is 34,
but actually use 0x800 as the first LBA of all partitions can enhance the erase speed*/
#define STARTING_LBA_OF_FIRST_PARTITION   (0x800)
#define FIRST_USABLE_LBA_FOR_PARTITION    (34)
#define ALTERNATE_GPT_PTE_SIZE (33)

#define MAX_SIZE_FLAG	0xFFFFFFFF

#define ALTERNATIVE_BUFFER_SIZE  0x200000

#define TRUE   1		/* Boolean true value. */
#define FALSE  0		/* Boolean false value. */
typedef enum READ_INFO_METHOD {
	FORCE_READ,
	FLEXIBLE_READ
} READ_INFO_METHOD;

typedef enum _PARTITION_IMG_TYPE {
	IMG_RAW = 0,
	IMG_WITH_SPARSE = 1,
	IMG_TYPE_MAX
} PARTITION_IMG_TYPE;

typedef enum _PARTITION_PURPOSE {
	PARTITION_PURPOSE_NORMAL,
	PARTITION_PURPOSE_NV,
	PARTITION_PURPOSE_MAX
} PARTITION_PURPOSE;

typedef struct DL_EMMC_STATUS_TAG {
	uint32_t part_total_size;
	uint32_t base_sector;
	uchar curUserPartitionName[32];
	PARTITION_PURPOSE partitionpurpose;
	uint8_t curEMMCArea;
	PARTITION_IMG_TYPE curImgType;
} DL_EMMC_STATUS;

typedef struct DL_FILE_STATUS_TAG {
	uint32_t total_size;
	uint32_t total_recv_size;
	uint32_t unsave_recv_size;
} DL_EMMC_FILE_STATUS;

typedef struct _SPECIAL_PARTITION_CFG {
	uchar *partition;
	uchar *bak_partition;
	PARTITION_IMG_TYPE imgattr;
	PARTITION_PURPOSE purpose;
} SPECIAL_PARTITION_CFG;

typedef struct {
	uint32_t version;
	uint32_t magicData;
	uint32_t checkSum;
	uint32_t hashLen;
} EMMC_BootHeader;

typedef struct _ALTER_BUFFER_ATTR {
	uchar* addr;
	uchar* pointer;
	uint32_t size;
	uint32_t used;
	uint32_t spare;
	uint32_t status;
	struct _ALTER_BUFFER_ATTR* next;
} ALTER_BUFFER_ATTR;

typedef enum _ALTERNATIVE_BUFFER_STATUS {
	BUFFER_CLEAN,
	BUFFER_DIRTY
} ALTERNATIVE_BUFFER_STATUS;
#if 0
typedef  struct {
    unsigned int mMagicNum; // "BTHD" == "0x42544844" == "boothead"
    unsigned int mVersion; // 1
    unsigned char mPayloadHash[32];//sha256 hash value
    long long mImgAddr; //image loaded address
    unsigned int mImgSize; //image size
    unsigned char reserved[512-(13*4)]; // reserved\uff0c make sure total size is 512B.
}sys_img_header;
#endif

OPERATE_STATUS dl_download_start(uchar * partition_name, uint32_t size, uint32_t nv_checksum);
OPERATE_STATUS dl_download_midst(uint16_t size, char *buf);
OPERATE_STATUS dl_download_end(void);
OPERATE_STATUS dl_read_start(uchar * partition_name, uint32_t size);

OPERATE_STATUS dl_read_midst(uint32_t size, uint32_t off, uchar * buf);
OPERATE_STATUS dl_read_end(void);
OPERATE_STATUS dl_erase(uchar * partition_name, uint32_t size);
OPERATE_STATUS dl_repartition(uchar * partition_cfg, uint16_t total_partition_num,
									uchar version, uchar size_unit);
OPERATE_STATUS dl_read_ref_info(char *part_name, uint16_t size, uint64_t offset,
	uchar *receive_buf, uchar  *transmit_buf);

#endif //CONFIG_EMMC_BOOT
#endif //_FDL_EMMC_OPERATE_H
