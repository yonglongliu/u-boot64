/*
 * Copyright (c) 2009, Google Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <common.h>
#include <asm/errno.h>
#include <malloc.h>
#include <command.h>
#include <android_bootimg.h>
#include <boot_mode.h>
#include <dl_common.h>
#include <dl_cmd_proc.h>
#include "sparse_format.h"
#include <linux/usb/usb_uboot.h>
#ifdef CONFIG_EMMC_BOOT
#include "dl_emmc_operate.h"
#include <asm/arch/chip_releted_def.h>
#include <mmc.h>
#include <dl_emmc_operate.h>
#include <secureboot/sec_common.h>
extern unsigned char *g_eMMCBuf;
extern SPECIAL_PARTITION_CFG const s_special_partition_cfg[];

#endif

#ifdef CONFIG_SECURE_BOOT
#include "secure_verify.h"
#endif

#ifdef CONFIG_ROM_VERIFY_SPL
#define SPL_CHECKSUM_LEN	0x8000
#endif

#define FLASHING_LOCK_PARA_LEN (8)

#ifdef CONFIG_NAND_BOOT
#include <nand.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>

typedef struct {
	char *vol;
	char *bakvol;
} FB_NV_VOL_INFO;

static FB_NV_VOL_INFO s_nv_vol_info[] = {
	{"fixnv1", "fixnv2"},
	{"wfixnv1", "wfixnv2"},
	{"tdfixnv1", "tdfixnv2"},
	{NULL, NULL}
};

#endif

struct dl_image_inf {
	uint8_t *base_address;
	uint64_t max_size;
	uint64_t data_size;
};

#define ROUND_TO_PAGE(x,y) (((x) + (y)) & (~(y)))
#define GFP_ATOMIC ((gfp_t) 0)

struct fastboot_cmd {
	struct fastboot_cmd *next;
	const char *prefix;
	unsigned prefix_len;
	void (*handle) (const char *arg, void *data, uint64_t sz);
};

struct fastboot_var {
	struct fastboot_var *next;
	const char *name;
	const char *value;
};

static struct fastboot_cmd *cmdlist;
struct dl_image_inf ImageInfo;

static unsigned char buffer[4096] __attribute__((aligned(64)));

typedef enum {
	STATE_OFFLINE = 0,
	STATE_COMMAND,
	STATE_COMPLETE,
	STATE_ERROR
} FB_USB_STATE;

static unsigned fastboot_state = STATE_OFFLINE;
static sys_img_header *bakup_header;

extern int usb_fastboot_init(void);
extern int fb_usb_write(void *buf, unsigned len);
extern int fb_usb_read(void *_buf, unsigned len);

void fastboot_register(const char *prefix, void (*handle) (const char *arg, void *data, unsigned sz))
{
	struct fastboot_cmd *cmd;
	cmd = malloc(sizeof(*cmd));
	if (cmd) {
		cmd->prefix = prefix;
		cmd->prefix_len = strlen(prefix);
		cmd->handle = handle;
		cmd->next = cmdlist;
		cmdlist = cmd;
	}
}

static struct fastboot_var *varlist;

void fastboot_publish(const char *name, const char *value)
{
	struct fastboot_var *var;
	var = malloc(sizeof(*var));
	if (var) {
		var->name = name;
		var->value = value;
		var->next = varlist;
		varlist = var;
	}
}
static char response[64] __attribute__((aligned(64)));
void fastboot_ack(const char *code, const char *reason)
{

	if (fastboot_state != STATE_COMMAND)
		return;
	if (reason == 0)
		reason = "";

	//snprintf(response, 64, "%s%s", code, reason);
	if (strlen(code) + strlen(reason) >= 64) {
		debugf("too long string\r\n");
	}
	sprintf(response, "%s%s", code, reason);
	fastboot_state = STATE_COMPLETE;

	fb_usb_write(response, strlen(response));

}

void fastboot_fail(const char *reason)
{
	fastboot_ack("FAIL", reason);
}

void fastboot_okay(const char *info)
{
	fastboot_ack("OKAY", info);
}

static void fb_cmd_getvar(const char *arg, void *data, uint64_t sz)
{
	struct fastboot_var *var;

	for (var = varlist; var; var = var->next) {
		if (!strcmp(var->name, arg)) {
			fastboot_okay(var->value);
			return;
		}
	}
	fastboot_okay("");
}

static void dump_log(char *buf, int len)
{
	int i = 0;

	debugf("**dump log_buf ...addr:0x%08x, len:%d\r\n", buf, len);

	for (i = 0; i < len; i++) {
		debugf("%02x ", *((unsigned char *)buf + i));
		if (i % 0x20 == 0x1f)
			debugf("\n");
	}
}

static void fb_cmd_download(const char *arg, void *data, uint64_t sz)
{
	char response[64];
	uint64_t len = simple_strtoul(arg, NULL, 16);
	int total_rcv;

	debugf("Start fastboot download, image len=0x%x \n", len);
	debugf("buffer base %p, buffer size 0x%x \n", ImageInfo.base_address, ImageInfo.max_size);
	if (len > ImageInfo.max_size) {
		debugf("Image size over the max buffer size,can not accept \n");
		fastboot_fail("data too large");
		return;
	}

	sprintf(response, "DATA%08x", len);
	if (fb_usb_write(response, strlen(response)) < 0) {
		fastboot_state = STATE_ERROR;
		return;
	}

	total_rcv = fb_usb_read(ImageInfo.base_address, len);
	if (total_rcv != len) {
		fastboot_state = STATE_ERROR;
		return;
	}
	ImageInfo.data_size = len;
	fastboot_okay("");
}

#ifdef CONFIG_EMMC_BOOT
unsigned short fastboot_eMMCCheckSum(const unsigned int *src, int len)
{
	unsigned int sum = 0;
	unsigned short *src_short_ptr = NULL;

	while (len > 3) {
		sum += *src++;
		len -= 4;
	}
	src_short_ptr = (unsigned short *)src;
	if (0 != (len & 0x2)) {
		sum += *(src_short_ptr);
		src_short_ptr++;
	}
	if (0 != (len & 0x1)) {
		sum += *((unsigned char *)(src_short_ptr));
	}
	sum = (sum >> 16) + (sum & 0x0FFFF);
	sum += (sum >> 16);

	return (unsigned short)(~sum);
}

void fastboot_splFillCheckData(unsigned int *splBuf, int len)
{

	EMMC_BootHeader *header;
	header = (EMMC_BootHeader *) ((unsigned char *)splBuf + BOOTLOADER_HEADER_OFFSET);
	header->version = 0;
	header->magicData = MAGIC_DATA;
	header->checkSum = (unsigned int)fastboot_eMMCCheckSum((unsigned char *)splBuf + BOOTLOADER_HEADER_OFFSET + sizeof(*header),
							       SPL_CHECKSUM_LEN - (BOOTLOADER_HEADER_OFFSET + sizeof(*header)));
#ifdef CONFIG_SECURE_BOOT
	header->hashLen = CONFIG_SPL_HASH_LEN >> 2;
#else
	header->hashLen = 0;
#endif

}

/**
	Erase the whole partition.
*/
int _fb_erase_partition(uchar * partition_name, unsigned int curArea, unsigned long base, unsigned long count)
{
	if (NULL == partition_name)
		return 0;

	if (count < EMMC_ERASE_ALIGN_LENGTH) {
		unsigned char buf[EMMC_ERASE_ALIGN_LENGTH * EFI_SECTOR_SIZE] = { 0xFF };
		if (!Emmc_Write(curArea, base, count, buf))
			return 0;
	} else {
		if (base % EMMC_ERASE_ALIGN_LENGTH) {
			unsigned char buf[EMMC_ERASE_ALIGN_LENGTH * EFI_SECTOR_SIZE] = { 0xFF };
			unsigned long base_sector_offset = 0;

			base_sector_offset = EMMC_ERASE_ALIGN_LENGTH - base % EMMC_ERASE_ALIGN_LENGTH;
			if (!Emmc_Write(curArea, base, base_sector_offset, buf))
				return 0;
			count = ((count - base_sector_offset) / EMMC_ERASE_ALIGN_LENGTH) * EMMC_ERASE_ALIGN_LENGTH;
			base = base + base_sector_offset;
		} else
			count = (count / EMMC_ERASE_ALIGN_LENGTH) * EMMC_ERASE_ALIGN_LENGTH;

		if (count == 0)
			return 1;

		if (!Emmc_Erase(curArea, base, count))
			return 0;
	}

	return 1;
}

int _fb_emmc_write(unsigned int part_type, uint32_t startBlock, uint32_t count, uint8_t * buf)
{
	uint32_t max_mutil_write = 0x8000;
	uint32_t each, saved = 0;
	uint32_t base = startBlock;
	while (count) {
		each = MIN(count, max_mutil_write);
		if (!Emmc_Write(part_type, base, each, buf + (saved * EFI_SECTOR_SIZE))) {
			debugf("emmc write error\n");
			return 0;
		}
		base += each;
		saved += each;
		count -= each;
	}
	return 1;
}

int _fb_write_sparse_img(uchar * part_name)
{
	void *write_addr = ImageInfo.base_address;
	uint64_t write_size = ImageInfo.data_size;
	int32_t retval = 0;

	debugf("sparse format image write_size 0x%x\n", write_size);
	retval = write_simg2emmc("mmc", 0, part_name, write_addr, write_size);
	if (-1 == retval) {
		debugf("sparse format image write error, retval =%d\n", retval);
		fastboot_fail("eMMC WRITE_ERROR!");
		return -1;
	}

	debugf("sparse format image write end!\n");
	return 0;
}

int _fb_write_nv_img(uchar * part_name, uint32_t startblk)
{
	nv_header_t *header_p = NULL;
	uint8_t header_buf[EMMC_SECTOR_SIZE];
	uint32_t count;
	uchar * backup_partition_name= NULL;

	memset(header_buf, 0, EMMC_SECTOR_SIZE);
	header_p = header_buf;
	header_p->magic = NV_HEAD_MAGIC;
	header_p->len = FIXNV_SIZE;
	header_p->checksum = (uint32_t) calc_checksum(ImageInfo.base_address, FIXNV_SIZE);
	header_p->version = NV_VERSION;
	if (!_fb_emmc_write(PARTITION_USER, startblk, 1, header_buf)) {
		fastboot_fail("eMMC WRITE_NVHEADER_ERROR!");
		return -1;
	}
	startblk++;
	count = ((FIXNV_SIZE + (EMMC_SECTOR_SIZE - 1)) & (~(EMMC_SECTOR_SIZE - 1))) / EMMC_SECTOR_SIZE;

	if (!_fb_emmc_write(PARTITION_USER, startblk, count, ImageInfo.base_address)) {
		fastboot_fail("eMMC WRITE_ERROR!");
		return -1;
	}

	/*write backup nv img*/
	backup_partition_name = _get_backup_partition_name(part_name);
	startblk = efi_GetPartBaseSec(backup_partition_name);
	debugf("backup_partition_name: %s", backup_partition_name);
	if (!_fb_emmc_write(PARTITION_USER, startblk, 1, header_buf)) {
		fastboot_fail("eMMC WRITE_NV_BACKUP_HEADER_ERROR!");
		return -1;
	}
	startblk++;

	if (!_fb_emmc_write(PARTITION_USER, startblk, count, ImageInfo.base_address)) {
		fastboot_fail("eMMC WRITE_NV_BACKUP_ERROR!");
		return -1;
	}

	return 0;

}

int _fb_write_normal_img(uint32_t startblk)
{
	uint32_t data_left = 0;
	uint32_t count;
	/*first part */
	data_left = ImageInfo.data_size % EMMC_SECTOR_SIZE;
	count = ImageInfo.data_size / EMMC_SECTOR_SIZE;
	if (!_fb_emmc_write(PARTITION_USER, startblk, count, ImageInfo.base_address)) {
		debugf("cmd_flash: raw data write frist part error!\n");
		fastboot_fail("write frist part error!");
		return -1;
	}
	debugf("cmd_flash:raw data write first part success\n");
	startblk += count;
	if (0 != data_left) {
		memset(ImageInfo.base_address + ImageInfo.data_size, 0, EMMC_SECTOR_SIZE - data_left);
		if (!_fb_emmc_write(PARTITION_USER, startblk, 1, ImageInfo.base_address + count * EMMC_SECTOR_SIZE)) {
			debugf("cmd_flash: raw data write tail part error!\n");
			fastboot_fail("write tail part error!");
			return -1;
		}
	}

	return 0;

}
int _img_bakup_write(uchar *partition, uint32_t nSectorCount,uint8_t * buf)
{
	uint32_t partition_type = PARTITION_USER;
	uint32_t startblock=0;
	if (0 == strcmp("splloader",  partition)) {
		debugf("partition write BOOT1\n");
		partition_type = PARTITION_BOOT1;
		startblock=  0;
		if(!_fb_emmc_write(partition_type, startblock, nSectorCount, (uint8_t *) buf)){
			debugf("boot1 %s write error! \n", partition);
			return 0;
		}

		//write backup partition
		partition_type = PARTITION_BOOT2;
		startblock =  0;
		if(!_fb_emmc_write(partition_type, startblock, nSectorCount, (uint8_t *) buf)){
			debugf("boot2 %s write error! \n", partition);
			return 0;
		}
	}else{
		partition_type = PARTITION_USER;
		startblock  = efi_GetPartBaseSec(partition);
		if(startblock==0){
			fastboot_fail("eMMC get partition ERROR!");
			return 0;
		}
		if(!_fb_emmc_write(partition_type, startblock, nSectorCount, (uint8_t *) buf)){
			debugf("org  %s write error! \n", partition);
			return 0;
		}
		//write backup partition
		strcat(partition,"_bak");
		startblock = efi_GetPartBaseSec(partition);
		if(startblock==0){
			fastboot_fail("eMMC get partition ERROR!");
			return 0;
		}
		if(!_fb_emmc_write(partition_type, startblock, nSectorCount, (uint8_t *) buf)){
			debugf("bak  %s write error! \n", partition);
			return 0;
		}
	}
      return 1;
}

void fb_cmd_flash(const char *arg, void *data, uint64_t sz)
{
	int i;
	uchar partition_name[PARTNAME_SZ];
	uint32_t partition_type = PARTITION_USER;
	uint32_t partition_purpose = PARTITION_PURPOSE_NORMAL;
	uint32_t img_format = IMG_RAW;
	uint32_t startblock;
	uint32_t count;
	disk_partition_t info;
	block_dev_desc_t *dev = NULL;
	uint32_t total_sz = 0;
	uint64_t *code_addr;
	uint8_t index = 0;
	int ret = 0;
	sparse_header_t sparse_header;
	int bakup_flag=0;
	int offset=0;
	debugf("data = %p,ImageInfo[0].base_address = %p\n", data, ImageInfo.base_address);

	dev = get_dev("mmc", 0);
	if (NULL == dev) {
		fastboot_fail("Block device not supported!");
		return;
	}

	if(!get_fblockflag()) {
		fastboot_fail("Flashing Lock Flag is locked. Please unlock it first!");
		return;
	}

	debugf("Cmd Flash partition:%s \n", arg);
	for (i = 0; i < PARTNAME_SZ; i++) {
		partition_name[i] = arg[i];
		if (0 == arg[i])
			break;
	}
		//for spl uboot bakup
	bakup_header = malloc(sizeof(sys_img_header));
	if (NULL == bakup_header) {
		debugf("No space to store bakup_header!\n");
		return ;
	}
	memcpy(bakup_header,ImageInfo.base_address,EMMC_SECTOR_SIZE);
	printf("bakup_header->mMagicNum=0x%x\n",bakup_header->mMagicNum);
	#if defined (CONFIG_SECBOOT)
	if((bakup_header->mMagicNum!=IMG_BAK_HEADER)
		&&((0 == strcmp("splloader", partition_name))
		||(0 == strcmp("sml", partition_name))
		||(0 == strcmp("trustos", partition_name))
		||(0 == strcmp("uboot", partition_name))))
		{
			fastboot_fail("image without image header");
			return;
		}

	#endif
	if((bakup_header->mMagicNum==IMG_BAK_HEADER)
		&&((0 == strcmp("splloader", partition_name))
		||(0 == strcmp("sml", partition_name))
		||(0 == strcmp("trustos", partition_name))
		||(0 == strcmp("uboot", partition_name)))){
		#if defined (CONFIG_SECBOOT)
		if (0 == strcmp(partition_name, "splloader")){
			//if(secboot_enable_check()){
				dl_secure_verify("splloader0", data, 0);
				//}
			}
		if (0 == strcmp(partition_name, "uboot")) {
			dl_secure_verify("splloader", data, 0);
			}
		#endif
		bakup_flag=1;
		memset(bakup_header, 0, EMMC_SECTOR_SIZE);
		free(bakup_header);
		count = ((sz + (EMMC_SECTOR_SIZE - 1)) & (~(EMMC_SECTOR_SIZE - 1))) / EMMC_SECTOR_SIZE;
		goto emmc_write;
	}


	if (0 == strcmp(partition_name, "splloader")) {
		fastboot_splFillCheckData(data, sz);
#if defined (CONFIG_SECURE_BOOT)
		//if(secboot_enable_check()){
		dl_secure_verify("splloader0", data, 0);
		//}
#endif
		count = ((SPL_CHECKSUM_LEN + (EMMC_SECTOR_SIZE - 1)) & (~(EMMC_SECTOR_SIZE - 1))) / EMMC_SECTOR_SIZE;
		startblock = 0;
		partition_type = PARTITION_BOOT1;
		goto emmc_write;
	}

	if (0 == strcmp(partition_name, "uboot")) {
		count = ((sz + (EMMC_SECTOR_SIZE - 1)) & (~(EMMC_SECTOR_SIZE - 1))) / EMMC_SECTOR_SIZE;
#if defined (CONFIG_SECURE_BOOT)
		dl_secure_verify("splloader", data, 0);
#endif
		startblock = 0;
		partition_type = PARTITION_BOOT2;
		goto emmc_write;
	}

	/*get the special partition info */
	for (i = 0; NULL != s_special_partition_cfg[i].partition; i++) {
		if (strcmp(s_special_partition_cfg[i].partition, partition_name) == 0) {
			partition_purpose = s_special_partition_cfg[i].purpose;
			img_format = s_special_partition_cfg[i].imgattr;
			break;
		}
	}

	memset(&sparse_header, 0, sizeof(sparse_header_t));
	memcpy(&sparse_header, ImageInfo.base_address, sizeof(sparse_header));
	if ((sparse_header.magic == SPARSE_HEADER_MAGIC) && (sparse_header.major_version == SPARSE_HEADER_MAJOR_VER)) {
		img_format = IMG_WITH_SPARSE;
		debugf("img_format =FB_IMG_WITH_SPARSE\n");
	}
	count = ((sz + (EMMC_SECTOR_SIZE - 1)) & (~(EMMC_SECTOR_SIZE - 1))) / EMMC_SECTOR_SIZE;

	if(get_fblockflag()) {
		/*do nothing, because flag is unlocked*/
	}
	else {
		/*Check boot&recovery img's magic */
		if (!strcmp(arg, "boot") || !strcmp(arg, "recovery")) {
#ifdef CONFIG_SECURE_BOOT
#ifdef CONFIG_ROM_VERIFY_SPL
			code_addr = (uint64_t *) get_code_addr("uboot", data);
#else
			code_addr = (uint64_t *) get_code_addr(NULL, data);
#endif

			if (memcmp((void *)code_addr, BOOT_MAGIC, BOOT_MAGIC_SIZE)) {
				fastboot_fail("image is not a boot image");
				return;
			}
			dl_secure_verify("uboot", data, 0);
#else
#ifdef CONFIG_SECBOOT
			//dl_secure_verify("fdl2", data, 0);
			secboot_verify(NULL,data,NULL,SPRD_FLAG);
			offset=sizeof(sys_img_header);
#endif
			if (memcmp((void *)(data+offset), BOOT_MAGIC, BOOT_MAGIC_SIZE)) {
				fastboot_fail("image is not a boot image");
				return;
			}
#endif
		}
	}

#ifdef CONFIG_SECURE_BOOT
#ifdef CONFIG_ROM_VERIFY_SPL
	if (!strcmp(arg, "system")) {
#ifdef CONFIG_SYSTEM_VERIFY
		Sec_SHA1_Start();
		do {
			secure_verify_system_start("uboot", ImageInfo[index].base_address, ImageInfo[index].data_size);
			index++;

		} while (index < 2);
		Sec_SHA1_Finish();
		if (secure_verify_system_end("uboot", ImageInfo[0].base_address, 0) == 0) {
			fastboot_fail("system.image secuer verify failed\n");
			return;
		}

		ImageInfo[0].base_address += VLR_CODE_OFF;
		ImageInfo[0].data_size -= VLR_CODE_OFF;
#endif
	}
#endif
#endif
	/*get partition info from emmc */
	if (0 != get_partition_info_by_name(dev, partition_name, &info)) {
		fastboot_fail("eMMC get partition ERROR!");
		return;
	}

	startblock = info.start;

	if (IMG_WITH_SPARSE == img_format) {
		debugf("flash sparse data\n");
		ret = _fb_write_sparse_img(partition_name);
		if (0 == ret)
			goto end;
		else
			return;
	} else if (IMG_RAW == img_format) {

		debugf("flash raw data\n");
		if (PARTITION_PURPOSE_NV == partition_purpose) {
			ret = _fb_write_nv_img(partition_name, startblock);
			if (0 == ret)
				goto end;
			else
				return;
		} else {
			ret = _fb_write_normal_img(startblock);
			if (0 == ret)
				goto end;
			else
				return;
		}
	} else {
		fastboot_fail("Image Format Unkown!");
		return;
	}

emmc_write:
	if(bakup_flag==1){
		bakup_flag=0;
		if(!_img_bakup_write(partition_name,count,(uint8_t *) data)){
			fastboot_fail("eMMC WRITE_ERROR!");
			return;
		}
	}else{
		if (!_fb_emmc_write(partition_type, startblock, count, (uint8_t *) data)) {
			fastboot_fail("eMMC WRITE_ERROR!");
			return;
		}
	}
end:
	fastboot_okay("");
}

void fb_cmd_erase(const char *arg, void *data, uint64_t sz)
{
	int i;
	uchar partition_name[PARTNAME_SZ];
	unsigned long count = 0, base_sector = 0;
	unsigned int curArea = 0;
	disk_partition_t info;
	block_dev_desc_t *dev = NULL;

	dev = get_dev("mmc", 0);
	if (NULL == dev) {
		fastboot_fail("Block device not supported!");
		return;
	}

	debugf("Cmd Erase partition:%s \n", arg);

	if (strcmp("params", arg) == 0) {

		count = Emmc_GetCapacity(PARTITION_BOOT1);
		curArea = PARTITION_BOOT1;
		base_sector = 0;
	} else if (strcmp("2ndbl", arg) == 0) {
		count = Emmc_GetCapacity(PARTITION_BOOT2);
		curArea = PARTITION_BOOT2;
		base_sector = 0;
	} else {
		for (i = 0; i < PARTNAME_SZ; i++) {
			partition_name[i] = arg[i];
			if (0 == arg[i])
				break;
		}
		/*get partition info from emmc */
		if (0 != get_partition_info_by_name(dev, partition_name, &info)) {
			fastboot_fail("eMMC get partition ERROR!");
			return;
		}
		curArea = PARTITION_USER;
		count = info.size;
		base_sector = info.start;
	}

	if (!_fb_erase_partition(partition_name, curArea, base_sector, count)) {
		fastboot_fail("eMMC Erase Partition ERROR!");
		return;
	}
	debugf("Cmd Erase OK\n");
	fastboot_okay("");
	return;
}

#endif

/*for nand this can not work,TODO*/
#ifdef CONFIG_NAND_BOOT
void fb_cmd_flash(const char *arg, void *data, uint64_t sz)
{
	int ret = -1;
	int i;
	uint64_t *code_addr;
	debugf("arg:%x date: 0x%x, sz 0x%x\n", arg, data, sz);

	if (!strcmp(arg, "boot") || !strcmp(arg, "recovery")) {
#ifdef CONFIG_SECURE_BOOT
		code_addr = (uint64_t *) get_code_addr(NULL, data);
		if (memcmp((void *)code_addr, BOOT_MAGIC, BOOT_MAGIC_SIZE)) {
			fastboot_fail("image is not a boot image");
			return;
		}
		secure_verify("uboot", data, 0);
#else
		if (memcmp((void *)data, BOOT_MAGIC, BOOT_MAGIC_SIZE)) {
			fastboot_fail("image is not a boot image");
			return;
		}
#endif
	}
#ifdef CONFIG_SECURE_BOOT
	if (strcmp(arg, "spl") == 0) {
		secure_verify("splloader0", data, 0);
	} else if (strcmp(arg, "2ndbl") == 0) {
		secure_verify("splloader", data, 0);
	}
#endif

	/**
	 *	FIX ME!
	 *	assume first image buffer is big enough for nv
	 */
	for (i = 0; s_nv_vol_info[i].vol != NULL; i++) {
		if (!strcmp(arg, s_nv_vol_info[i].vol)) {
			nv_header_t *header = NULL;
			uint8_t tmp[NV_HEAD_LEN];

			memset(tmp, 0x00, NV_HEAD_LEN);
			header = tmp;
			header->magic = NV_HEAD_MAGIC;
			header->len = FIXNV_SIZE;
			header->checksum = (uint32_t) calc_checksum((unsigned char *)data, FIXNV_SIZE);
			header->version = NV_VERSION;
			/*write org nv */
			ret = do_raw_data_write(arg, FIXNV_SIZE + NV_HEAD_LEN, NV_HEAD_LEN, 0, tmp);
			if (ret)
				goto end;
			ret = do_raw_data_write(arg, 0, FIXNV_SIZE, NV_HEAD_LEN, data);
			if (ret)
				goto end;
			/*write bak nv */
			ret = do_raw_data_write(s_nv_vol_info[i].bakvol, FIXNV_SIZE + NV_HEAD_LEN, NV_HEAD_LEN, 0, tmp);
			if (ret)
				goto end;
			ret = do_raw_data_write(s_nv_vol_info[i].bakvol, 0, FIXNV_SIZE, NV_HEAD_LEN, data);
			goto end;
		}
	}

	if (ImageInfo[1].data_size) {
		uint32_t total_sz = ImageInfo[0].data_size + ImageInfo[1].data_size;
		ret = do_raw_data_write(arg, total_sz, ImageInfo[0].data_size, 0, ImageInfo[0].base_address);
		if (ret)
			goto end;
		ret = do_raw_data_write(arg, 0, ImageInfo[1].data_size, ImageInfo[0].data_size, ImageInfo[1].base_address);
	} else {
		ret = do_raw_data_write(arg, ImageInfo[0].data_size, ImageInfo[0].data_size, 0, ImageInfo[0].base_address);
	}

end:
	if (!ret)
		fastboot_okay("");
	else
		fastboot_fail("flash error");
	return;
}

void fb_cmd_erase(const char *arg, void *data, uint64_t sz)
{
	struct mtd_info *nand;
	struct mtd_device *dev;
	struct part_info *part;
	nand_erase_options_t opts;
	u8 pnum;
	int ret;
	char buf[1024];

	debugf("\n");

	ret = find_dev_and_part(arg, &dev, &pnum, &part);
	if (!ret) {
		nand = &nand_info[dev->id->num];
		memset(&opts, 0, sizeof(opts));
		opts.offset = (loff_t) part->offset;
		opts.length = (loff_t) part->size;
		opts.jffs2 = 0;
		opts.quiet = 1;
		ret = nand_erase_opts(nand, &opts);
		if (ret)
			goto end;
	}

	/*just erase 1k now */
	memset(buf, 0x0, 1024);
	ret = do_raw_data_write(arg, 1024, 1024, 0, buf);

end:
	if (ret)
		fastboot_fail("nand erase error");
	else
		fastboot_okay("");
	return;
}
#endif

extern void udc_power_off(void);

void boot_linux(unsigned kaddr, unsigned taddr)
{
	void (*theKernel) (void *dtb_addr, int zero, int arch, int reserved)=(void*)kaddr;

	invalidate_dcache_all();

	theKernel(DT_ADR, 0, 0, 0);
}

extern unsigned char raw_header[8192];

void fb_cmd_getlockflag(const char *arg, void *data, uint64_t sz)
{
	unsigned int fb_cmdtmpflag = 0;

	//extern unsigned int get_fblockflag(void);
	fb_cmdtmpflag = get_fblockflag();

	if(fb_cmdtmpflag == 0)
		debugf("lock flag is lock.\n");
	else
		debugf("lock flag is unlock.\n");

	fastboot_okay("");
}

extern int set_fblockflag(unsigned int flag);
void fb_cmd_setlockflag(const char *arg, void *data, uint64_t sz)
{
	char * search = arg;
	unsigned int fb_cmdtmpflag = 0;
	int tmp_ret = 0;
	char flag_name[FLASHING_LOCK_PARA_LEN];
	int i;

	debugf("arg#%s#, data: %p, sz: 0x%x\n", arg, data, sz);

	memset(flag_name, 0, sizeof(flag_name));
	for (i = 0; i < FLASHING_LOCK_PARA_LEN; i++) {
		flag_name[i] = arg[i];
		if (0 == arg[i])
			break;
	}

	debugf("flag_name#%s#\n", flag_name);

	/*check bootimage(recoveryimage) when flash or bootup*/
	if(0 == strcmp(" lock", flag_name))	{
		/*normal secure check*/
		fb_cmdtmpflag = 0;
	}
	else if(0 == strcmp(" unlock", flag_name)) {
		/*jump secure check*/
		fb_cmdtmpflag = 1;
	}
	else	{
		fastboot_fail("invalid arg.");
		return;
	}

	tmp_ret = set_fblockflag(fb_cmdtmpflag);
	if(tmp_ret)
		debugf("set_fblockflag failed, ret:%d\n", tmp_ret);

	if(fb_cmdtmpflag == 0)
		debugf("set lock flag to lock.\n");
	else
		debugf("set lock flag to unlock.\n");

	fastboot_okay("");
}

void fb_cmd_boot(const char *arg, void *data, uint64_t sz)
{
	boot_img_hdr *hdr = raw_header;
	unsigned kernel_actual;
	unsigned ramdisk_actual;
	unsigned kernel_addr;
	unsigned ramdisk_addr;

	debugf("arg: %s, data: %p, sz: 0x%x\n", arg, data, sz);

	if (memcmp(hdr->magic, BOOT_MAGIC, BOOT_MAGIC_SIZE)) {
		debugf("boot image headr: %s\n", hdr->magic);
		fastboot_fail("bad boot image header");
		return;
	}
	kernel_actual = ROUND_TO_PAGE(hdr->kernel_size, (KERNL_PAGE_SIZE - 1));
	if (kernel_actual <= 0) {
		fastboot_fail("kernel image should not be zero");
		return;
	}
	ramdisk_actual = ROUND_TO_PAGE(hdr->ramdisk_size, (KERNL_PAGE_SIZE - 1));
	if (0 == ramdisk_actual) {
		fastboot_fail("ramdisk size error");
		return;
	}

	memcpy((void *)hdr->kernel_addr, (void *)data + KERNL_PAGE_SIZE, kernel_actual);
	memcpy((void *)hdr->ramdisk_addr, (void *)data + KERNL_PAGE_SIZE + kernel_actual, ramdisk_actual);

	debugf("kernel @0x%08x (0x%08x bytes)\n", hdr->kernel_addr, kernel_actual);
	debugf("ramdisk @0x%08x (0x%08x bytes)\n", hdr->ramdisk_addr, ramdisk_actual);

	fastboot_okay("");
	usb_driver_exit();
	boot_linux(hdr->kernel_addr, hdr->tags_addr);
}

void fb_cmd_continue(const char *arg, void *data, uint64_t sz)
{
	fastboot_okay("");
	usb_driver_exit();
	normal_mode();
}

void fb_cmd_reboot(const char *arg, void *data, uint64_t sz)
{
	fastboot_okay("");
	reboot_devices(CMD_NORMAL_MODE);
}

void fb_cmd_reboot_bootloader(const char *arg, void *data, uint64_t sz)
{
	fastboot_okay("");
	usb_driver_exit();
	reboot_devices(CMD_FASTBOOT_MODE);
}

void fb_cmd_powerdown(const char *arg, void *data, uint64_t sz)
{
	fastboot_okay("");
	power_down_devices(0);

}

static void fastboot_command_loop(void)
{
	struct fastboot_cmd *cmd;
	int r;
	debugf("fastboot: processing commands\n");

again:
	while (fastboot_state != STATE_ERROR) {
		memset(buffer, 0, 64);
		r = fb_usb_read(buffer, 64);
		if (r < 0)
			break;
		buffer[r] = 0;
		debugf("fastboot: %s, r:%d\n", buffer, r);

		for (cmd = cmdlist; cmd; cmd = cmd->next) {
			if (memcmp(buffer, cmd->prefix, cmd->prefix_len))
				continue;
			debugf("Receive cmd from host :%s \n", cmd->prefix);
			fastboot_state = STATE_COMMAND;
			cmd->handle((const char *)buffer + cmd->prefix_len, ImageInfo.base_address, ImageInfo.data_size);
			if (fastboot_state == STATE_COMMAND)
				fastboot_fail("unknown reason");
			goto again;
		}

		fastboot_fail("unknown command");

	}
	fastboot_state = STATE_OFFLINE;
	debugf("fastboot: oops!\n");
}

static int fastboot_handler(void *arg)
{
	for (;;) {
		fastboot_command_loop();
	}
	return 0;
}

int do_fastboot()
{
	int ret = 0;
	debugf("start fastboot\n");

	ret = usb_fastboot_init();
	if (ret < 0)
		return ret;

	ImageInfo.base_address = (uint8_t *) CONFIG_SYS_SDRAM_BASE;
	/*except the u-boot occupied area , all RAM can be used as fastboot buffer */
	ImageInfo.max_size = (uint64_t) CONFIG_SYS_TEXT_BASE - CONFIG_SYS_SDRAM_BASE;
	ImageInfo.data_size = 0;

	fastboot_register("getvar:", fb_cmd_getvar);
	/*when you input cmd"flash" in host, we will rcv cmd"download" first,then the "flash",
	   so even if we can't see cmd"download" in host fastboot cmd list,it is also used */
	fastboot_register("download:", fb_cmd_download);
	fastboot_publish("version", "1.0");

	fastboot_register("flash:", fb_cmd_flash);
	fastboot_register("erase:", fb_cmd_erase);
	fastboot_register("boot", fb_cmd_boot);
	fastboot_register("reboot", fb_cmd_reboot);
	fastboot_register("powerdown", fb_cmd_powerdown);
	fastboot_register("continue", fb_cmd_continue);
	fastboot_register("reboot-bootloader", fb_cmd_reboot_bootloader);
	fastboot_register("flashing", fb_cmd_setlockflag);
	fastboot_register("getflag", fb_cmd_getlockflag);

	fastboot_handler(0);

	return 0;

}
