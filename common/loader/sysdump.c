/*
 * Copyright (C) 2013 Spreadtrum Communications Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

//#include "normal_mode.h"
#include <rtc.h>
#include <asm/sizes.h>
#include <boot_mode.h>
#include <common.h>

#include <linux/input.h>
#include "sysdump.h"
#include <linux/mtd/mtd.h>
#include <asm/arch/check_reboot.h>
#include <mmc.h>
#include <fat.h>
#include <exfat.h>
#include "loader_common.h"

#include <libfdt.h>
#include <fdt.h>
#include <linux/ctype.h>
#include <fdtdec.h>
#include <asm/arch/sdio_cfg.h>
#include "crc32_simple.h"
#include <fs.h>

enum {
	FS_INVALID,
	FS_FAT32,
	FS_EXFAT
};

extern char __image_copy_start[0];
extern void MMU_DisableIDCM(void);
extern void set_backlight(uint32_t value);
extern void lcd_printf(const char *fmt, ...);
extern  int sprd_host_init(int sdio_type);
extern struct rtc_time get_time_by_sec(void);

static int display_crashinfo(struct sysdump_info *sinfo, int rst_mode);

#define ALIGN_SIZE 0X100000
#define ROUND_UP(x)       ((x + ALIGN_SIZE - 1) & ~(ALIGN_SIZE - 1))
#define YLOG_PATH "ylog/"
#define STORAGE_PATH "ylog/sysdump/"
#define UBOOT_START __image_copy_start
#define UBOOT_SIZE SZ_16M  //EXP

void display_writing_sysdump(void)
{
	debugf("%s\n", __FUNCTION__);
	vibrator_hw_init();
	set_vibrator(1);
	lcd_printf("   -------------------------------  \n"
		   "   Sysdumpping now, keep power on.  \n"
		   "   -------------------------------  \n");
	set_backlight(255);
	set_vibrator(0);
}
void display_special_mode(void)
{
	debugf("%s\n", __FUNCTION__);
	vibrator_hw_init();
	set_vibrator(1);
	lcd_printf("   -------------------------------  \n"
		   "   Restart now, keep power on.  \n"
		   "   -------------------------------  \n");
	set_backlight(255);
	set_vibrator(0);
}
/*display without character*/
void display_sysdump(void)
{
	debugf("%s\n", __FUNCTION__);
	vibrator_hw_init();
	set_vibrator(1);
	set_backlight(255);
	set_vibrator(0);
}

static void wait_for_keypress(void)
{
	int key_code;

	do {
		udelay(50 * 1000);
		key_code = board_key_scan();
		//printf("key_code: %d, (vd:%d,vu:%d,p:%d)\n", key_code, KEY_VOLUMEDOWN, KEY_VOLUMEUP, KEY_POWER);
		if (key_code == KEY_VOLUMEDOWN || key_code == KEY_VOLUMEUP || key_code == KEY_HOME)
			break;
	} while (1);
	debugf("Pressed key: %d\n", key_code);
	lcd_printf("Pressed key: %d\n", key_code);
}

static unsigned long sysdump_crc32(const char *filename, ulong addr, unsigned long len)
{
	unsigned long crc32 = 0;
	unsigned long nocrc32_start = UBOOT_START;
	unsigned long nocrc32_end = UBOOT_START + UBOOT_SIZE;
	unsigned long dump_file_end = addr + len;
	debugf("nocrc32_start = %lx\n", nocrc32_start);
	debugf("nocrc32_end = %lx\n", nocrc32_end);
	debugf("dump_file_end = %lx\n", dump_file_end);

	if (nocrc32_start > addr && dump_file_end > nocrc32_end) {
		//nocrc32 area inside dumpfile
		crc32 = crc32_simple(0, addr, nocrc32_start - addr);
		crc32 = crc32_simple(crc32, nocrc32_end, dump_file_end - nocrc32_end);
	}else if (nocrc32_start > addr && nocrc32_start < dump_file_end && dump_file_end < nocrc32_end) {
			//nocrc32 area below dumpfile
			crc32 = crc32_simple(0, addr, nocrc32_start - addr);
		}else if ( nocrc32_start < addr && addr < nocrc32_end && nocrc32_end < dump_file_end) {
			//nocrc32 area above dumpfile
				crc32 = crc32_simple(0, nocrc32_end, len - nocrc32_end + addr);
			}else if (nocrc32_end < addr || dump_file_end < nocrc32_start) {
					crc32 = crc32_simple(0, addr, len);
			}
	debugf("%s crc32 = %lx\n", filename, crc32);
	return crc32;
}

static unsigned long sysdump_write(int fs_type, const char *filename, ulong addr, unsigned long offset, unsigned long len)
{
	unsigned long ret;

	debugf("writing 0x%lx bytes to sd file %s\n",len, filename);
	lcd_printf("writing 0x%lx bytes to sd file %s\n", len, filename);
	if (fs_type == FS_FAT32) {
		debugf("sysdump_write & file system is FAT32\n");
		if (fs_open(fs_type) < 0)
			return 0;
		fs_write(filename, addr, offset, len);
	}
        else if (fs_type == FS_EXFAT) {
		debugf("sysdump_write & file system is exFAT\n");
		ret = file_exfat_write(filename, addr, len);
	}

	ret = sysdump_crc32(filename, addr, len);
	return ret;
}
static int SD_init(int *fs_type)
{
	struct mmc *mmc;
	block_dev_desc_t *dev_desc = NULL;
	int ret;

	*fs_type = FS_INVALID;
	sprd_host_init(SD);
	mmc = find_mmc_device(SD);// 1 is  sd, 0 is emmc
	if(mmc) {
		ret = mmc_init(mmc);
		if(ret < 0){
			debugf("mmc init failed %d\n", ret);
			return -1;
		}
	} else {
		debugf("no mmc card found\n");
		return -1;
	}
	dev_desc = get_dev("mmc",SD);
	if(dev_desc == NULL){
		debugf("no mmc block device found\n");
		return -1;
	}
	if (dev_desc ->part_type != PART_TYPE_DOS){
		debugf(" no part_type_dos!\n");
		return -1;
	}
	ret = fat_register_device(dev_desc, SD);
	if (ret == 0) {
		ret = file_fat_detectfs();
		if(ret) {
			debugf("detect fs fat failed\n");
			return -1;
		}
		*fs_type = FS_FAT32;
		return 0;
	}

	debugf("Fat register fail. Try exFat. \n");
	ret = exfat_register_device(dev_desc, 1);
	if(ret < 0) {
		debugf("exFat register fail %d.\n", ret);
		return -1;
	}

	ret = file_exfat_detectfs();
	if(ret) {
		debugf("Detect fs exfat failed\n");
		return -1;
	}
	*fs_type = FS_EXFAT;
	return 0;
}

static size_t get_elfhdr_size(int nphdr)
{
	size_t elfhdr_len;

	elfhdr_len = sizeof(struct elfhdr) +
		(nphdr + 1) * sizeof(struct elf_phdr);
#if SETUP_NOTE
	elfhdr_len += ((sizeof(struct elf_note)) +
		roundup(sizeof(CORE_STR), 4)) * 3 +
		roundup(sizeof(struct elf_prstatus), 4) +
		roundup(sizeof(struct elf_prpsinfo), 4) +
		roundup(sizeof(struct task_struct), 4);
#endif
	elfhdr_len = PAGE_ALIGN(elfhdr_len); //why?

	return elfhdr_len;
}

#if SETUP_NOTE
static int notesize(struct memelfnote *en)
{
	int sz;

	sz = sizeof(struct elf_note);
	sz += roundup((strlen(en->name) + 1), 4);
	sz += roundup(en->datasz, 4);

	return sz;
}

static char *storenote(struct memelfnote *men, char *bufp)
{
	struct elf_note en;

#define DUMP_WRITE(addr,nr) do { memcpy(bufp,addr,nr); bufp += nr; } while(0)

	en.n_namesz = strlen(men->name) + 1;
	en.n_descsz = men->datasz;
	en.n_type = men->type;

	DUMP_WRITE(&en, sizeof(en));
	DUMP_WRITE(men->name, en.n_namesz);

	/* XXX - cast from long long to long to avoid need for libgcc.a */
	bufp = (char*) roundup((unsigned long)bufp,4);
	DUMP_WRITE(men->data, men->datasz);
	bufp = (char*) roundup((unsigned long)bufp,4);

#undef DUMP_WRITE

	return bufp;
}

#endif

static void sysdump_fill_core_hdr(struct pt_regs *regs,
						struct sysdump_mem *sysmem, int mem_num,
						char *bufp, int nphdr, int dataoff)
{
#if 0
	struct elf_prstatus prstatus;	/* NT_PRSTATUS */
	struct elf_prpsinfo prpsinfo;	/* NT_PRPSINFO */
#endif
	struct elf_phdr *nhdr, *phdr;
	struct elfhdr *elf;
	struct memelfnote notes[3];
	off_t offset = 0;
	Elf_Off f_offset = dataoff;
	int i;

	/* setup ELF header */
	elf = (struct elfhdr *) bufp;
	bufp += sizeof(struct elfhdr); //printk("sizeof(struct elfhdr): %d\n");
	offset += sizeof(struct elfhdr); //printk("sizeof(struct elfhdr): %d\n");
	memcpy(elf->e_ident, ELFMAG, SELFMAG); //printk("ELFMAG: %s, SELFMAG:%d\n", ELFMAG, SELFMAG);
	elf->e_ident[EI_CLASS]	= ELF_CLASS;//printk("EI_CLASS:%d, ELF_CLASS: %d", EI_CLASS, ELF_CLASS);
	elf->e_ident[EI_DATA]	= ELF_DATA;//printk("EI_DATA:%");
	elf->e_ident[EI_VERSION]= EV_CURRENT;
	elf->e_ident[EI_OSABI] = ELF_OSABI;
	memset(elf->e_ident+EI_PAD, 0, EI_NIDENT-EI_PAD);
	elf->e_type	= ET_CORE;
	elf->e_machine	= ELF_ARCH;
	elf->e_version	= EV_CURRENT;
	elf->e_entry	= 0;
	elf->e_phoff	= sizeof(struct elfhdr);
	elf->e_shoff	= 0;
	elf->e_flags	= ELF_CORE_EFLAGS;
	elf->e_ehsize	= sizeof(struct elfhdr);
	elf->e_phentsize= sizeof(struct elf_phdr);
	elf->e_phnum	= nphdr;
	elf->e_shentsize= 0;
	elf->e_shnum	= 0;
	elf->e_shstrndx	= 0;

	/* setup ELF PT_NOTE program header */
	nhdr = (struct elf_phdr *) bufp;
	bufp += sizeof(struct elf_phdr);
	offset += sizeof(struct elf_phdr);
	nhdr->p_type	= PT_NOTE;
	nhdr->p_offset	= 0;
	nhdr->p_vaddr	= 0;
	nhdr->p_paddr	= 0;
	nhdr->p_filesz	= 0;
	nhdr->p_memsz	= 0;
	nhdr->p_flags	= 0;
	nhdr->p_align	= 0;

	/* setup ELF PT_LOAD program header for every area */
	for (i = 0; i < mem_num; i++) {
		phdr = (struct elf_phdr *) bufp;
		bufp += sizeof(struct elf_phdr);
		offset += sizeof(struct elf_phdr);

		phdr->p_type	= PT_LOAD;
		phdr->p_flags	= PF_R|PF_W|PF_X;
		phdr->p_offset	= f_offset;
		phdr->p_vaddr	= sysmem[i].vaddr;
		phdr->p_paddr	= sysmem[i].paddr;
		phdr->p_filesz	= phdr->p_memsz	= sysmem[i].size;
		phdr->p_align	= 0;//PAGE_SIZE;
		f_offset += sysmem[i].size;
	}
#if SETUP_NOTE
	/*
	 * Set up the notes in similar form to SVR4 core dumps made
	 * with info from their /proc.
	 */
	nhdr->p_offset	= offset;

	/* set up the process status */
	notes[0].name = CORE_STR;
	notes[0].type = NT_PRSTATUS;
	notes[0].datasz = sizeof(struct elf_prstatus);
	notes[0].data = &prstatus;

	memset(&prstatus, 0, sizeof(struct elf_prstatus));
	//fill_prstatus(&prstatus, current, 0);
	//if (regs)
	//	memcpy(&prstatus.pr_reg, regs, sizeof(*regs));
	//else
	//	crash_setup_regs((struct pt_regs *)&prstatus.pr_reg, NULL);

	nhdr->p_filesz	= notesize(&notes[0]);
	bufp = storenote(&notes[0], bufp);

	/* set up the process info */
	notes[1].name	= CORE_STR;
	notes[1].type	= NT_PRPSINFO;
	notes[1].datasz	= sizeof(struct elf_prpsinfo);
	notes[1].data	= &prpsinfo;

	memset(&prpsinfo, 0, sizeof(struct elf_prpsinfo));
	//fill_psinfo(&prpsinfo, current, current->mm);

	strcpy(prpsinfo.pr_fname, "vmlinux");
	//strncpy(prpsinfo.pr_psargs, saved_command_line, ELF_PRARGSZ);

	nhdr->p_filesz	+= notesize(&notes[1]);
	bufp = storenote(&notes[1], bufp);

	/* set up the task structure */
	notes[2].name	= CORE_STR;
	notes[2].type	= NT_TASKSTRUCT;
	notes[2].datasz	= sizeof(struct task_struct);
	notes[2].data	= current;

	printk("%s: data size is %d, data addr is %p",__func__,notes[2].datasz,notes[2].data);

	nhdr->p_filesz	+= notesize(&notes[2]);
	bufp = storenote(&notes[2], bufp);
#endif
	return;
} /* end elf_kcore_store_hdr() */

#if 1 /* New method to do flag checking */
int sysdump_flag_check(void) {
#ifdef DEBUG /* None user (userdebug or eng) version build mode */
	debugf("Sysdump userdebug version key detecting ... \n");
	return 1;
#else /* user mode, trigger VOLUMEUP into sysdump until screen light when power on */
	#define SCREEN_WAITING_TIME 10  //  10s
	/*change strings through command:
				sed -i 's/SYSDUMP_CONST_OFF/SYSDUMP_CONST_OFN/' u-boot.bin*/
	if (strcmp("SYSDUMP_CONST_OFN", "SYSDUMP_CONST_OFF") == 0) {
		printf("Sysdump user version key detecting ... Force return 1\n");
		return 1;
	}
	printf("Sysdump user version key detecting ...\n");
	int key_code, cnt_time = 0 ,state_key = 0;
	/*
	New check method :
		0.press VOL-
		1.press VOL+
		2.press VOL-
		3.press VOL+
		tips:
		press volume in order will trigger sysdump.
		Other key can abort what you press just now.
	*/
	key_code = board_key_scan();
	if (key_code == KEY_VOLUMEUP) {
		printf("Sysdump : KEY_VOLUMEUP is detected\n");
		display_sysdump();
		do {
			key_code = board_key_scan();
			mdelay(100);
			cnt_time ++;

			switch(state_key) {
				/*checking KEY_VOLUMEDOWN*/
				case 0: {
					if (key_code == KEY_VOLUMEDOWN )
						state_key = 1;
					break;
				}
				/*checking KEY_VOLUMEUP*/
				case 1: {
					if (key_code == KEY_VOLUMEUP)
						state_key = 2;
					break;
				}
				/*checking KEY_VOLUMEDOWN*/
				case 2: {
					if (key_code == KEY_VOLUMEDOWN )
						state_key = 3;
					break;
				}
				/*checking KEY_VOLUMEUP*/
				case 3: {
					if (key_code == KEY_VOLUMEUP) {
						state_key = 0;
						printf("out key  checking\n");
						return 1;
					}
					break;
				}
				default:
					state_key = 0;
					break;
			}
			if (cnt_time > SCREEN_WAITING_TIME*10) {
				printf("time is up\n");
				return -1;
			}
		} while(1);
		return 1;
	}
	printf("Sysdump : KEY_VOLUMEUP is not detected\n");
	return -1;
#endif
}
#else
#define PROD_PART "prodnv"
int sysdump_flag_check(void)
{
	char sysdump_buf[200]={0};

	memset(sysdump_buf,0x0,200);
	if(!do_fs_file_read(PROD_PART, "sysdump_flag", sysdump_buf,200))
		debugf("file: sysdump_flag is exist\n");
	else {
		debugf("file: sysdump_flag is not exist\n");
#ifdef SYSDUMP_BYPASS
		return -1;
#else
		return 1;
#endif
	}

	if(!strncmp(sysdump_buf, "on", 2))
		return 1;

	return -1;
}
#endif
char *rstmode[] = {
	"undefind mode",		//CMD_UNDEFINED_MODE=0,
	"power down",			//CMD_POWER_DOWN_DEVICE,
	"normal",				//CMD_NORMAL_MODE,
	"recovery",				//CMD_RECOVERY_MODE,
	"fastboot",				//CMD_FASTBOOT_MODE,
	"alarm",				//CMD_ALARM_MODE,
	"charge",				//CMD_CHARGE_MODE,
	"engtest",				//CMD_ENGTEST_MODE,
	"watchdog timeout",		//CMD_WATCHDOG_REBOOT ,
	"framework crash",		//CMD_SPECIAL_MODE,
	"manual dump",			//CMD_UNKNOW_REBOOT_MODE,
	"kernel crash",			//CMD_PANIC_REBOOT,
	"calibration",			//CMD_CALIBRATION_MODE,
	"autodloader",			//CMD_AUTODLOADER_REBOOT,
	"ext rstn reboot",		//CMD_EXT_RSTN_REBOOT_MODE,
	"iq reboot",			//CMD_IQ_REBOOT_MODE,
	"sleep",				//CMD_SLEEP_MODE,
};

#define GET_RST_MODE(x) rstmode[(x) < 17 ? (x) : 0]

#ifndef CONFIG_EMMC_BOOT
/*Copy the data saved in nand flash to ram*/
int read_nand_to_ram( struct mtd_info *mtd, loff_t paddr, unsigned int size, unsigned char *buf)
{
        int ret = 0;
        unsigned int retlen = 0;
        loff_t read_addr = 0;
        unsigned char *read_buf = NULL;
        unsigned int readsize = 0;

        debugf("%s, read 0x%.8x:0x%.8x buf: 0x%.8x\n", __func__, (unsigned int)paddr, size, buf);
        for(read_addr = paddr, read_buf = buf; read_addr < (paddr + size); read_addr += readsize, read_buf += readsize) {
                readsize = (paddr + size - read_addr) > mtd->erasesize ? mtd->erasesize : (paddr + size - read_addr);
                if(mtd->_block_isbad(mtd, read_addr) == 1) {//if met bad block, we just fill it with 0x5a
                        memset(read_buf, 0x5a, readsize);
                        continue;
                }

                ret = mtd->_read(mtd, read_addr, readsize, &retlen, read_buf);
                if(ret != 0 && retlen != readsize) {
                        printf("%s, read addr: 0x%.8x len: 0x%.8x 's value err, ret: %d, retlen: 0x%.8x\n",\
			 __func__, (unsigned int)read_addr, readsize, ret, retlen);
			lcd_printf("\nRead nand flash 0x%.8x error, you can dump it use download tools again!\n", read_addr);
			break;
                }
        }
	return ret;
}

/*dump the data saved in nand flash to sdcard when needed*/
void mtd_dump(int fs_type)
{
	int ret = 0;
	unsigned int write_len = 0, write_addr = 0;
	char *buf = NULL;
	unsigned int part_len = 0x8000000;//The size of each ubipac-part file
	int loop = 0;
	char fname[72];
	struct mtd_info *mtd = NULL;

	buf = CONFIG_SYS_SDRAM_BASE;//After dump memory to sdcard, we suppose the whole memory except u-boot used are avaliable.
	mtd = get_mtd_device_nm(UBIPAC_PART);
	if(mtd == NULL) {
		printf("Can't get the mtd part: %s\n", UBIPAC_PART);
		return;
	}

	debugf("Begin to dump 0x%.8x ubipac to sdcard!\n", mtd->size);
	for(write_addr = 0; write_addr < mtd->size; write_addr += write_len, loop++)
	{
		write_len = (mtd->size - write_addr) > part_len ? part_len : (mtd->size - write_addr);
		debugf("begin to read 0x%.8x value to ubipac%d\n", write_len, loop);
		memset(buf, 0, write_len);
		ret = read_nand_to_ram(mtd, (loff_t)(write_addr), write_len, buf);
		if(ret != 0) {
			printf("%s, read ubipac%d error, the ret is %d\n", __func__, loop, ret);
			break;
		}
		debugf("read ubipac%d end\n", loop);

		memset(fname, 0, 72);
		sprintf(fname, "ubipac%d", loop);
		sysdump_write(fs_type, fname, buf, 0, write_len);
		debugf("write ubipac%d end\n", loop);
	}
	put_mtd_device(mtd);
}
#endif

static int check_dts_sysdump_node(void)
{
	unsigned char *fdt_blob = (unsigned char*) DT_ADR;
	int nodeoffset = fdt_path_offset(fdt_blob, "/sprd_sysdump");
	debugf("nodeoffset = %d\n", nodeoffset);

	return nodeoffset;
}

static int fill_dump_mem(struct sysdump_mem *sprd_dump_mem, int nodeoffset)
{
	int sprd_dump_mem_num, lenp, offset;
	unsigned long *ptr = NULL, *ptr_end = NULL;
	unsigned char *fdt_blob = (unsigned char*) DT_ADR;
	int i = 0;

	ptr = fdt_getprop(fdt_blob, nodeoffset, "ram", &lenp);
	if (ptr == NULL)
		debugf("no ram property.\n");
	else {
		offset = fdt_path_offset(fdt_blob, ptr);
		ptr = fdt_getprop(fdt_blob, offset, "reg", &lenp);
		ptr_end = ptr + lenp / (sizeof(unsigned long));
		while (ptr < ptr_end) {
		sprd_dump_mem[i].paddr = fdt_addr_to_cpu (*ptr ++);
		sprd_dump_mem[i].vaddr = __va(sprd_dump_mem[i].paddr);
		sprd_dump_mem[i].size  = fdt_size_to_cpu (*ptr ++);
		sprd_dump_mem[i].size = ROUND_UP(sprd_dump_mem[i].size);
		sprd_dump_mem[i].soff  = 0xffffffff;
		sprd_dump_mem[i].type  = SYSDUMP_RAM;
		debugf("sprd_dump_mem[%d].paddr is %lx\n", i, sprd_dump_mem[i].paddr);
		debugf("sprd_dump_mem[%d].size  is %lx\n", i, sprd_dump_mem[i].size);
		i ++;
		}
	}
    ptr = fdt_getprop(fdt_blob, nodeoffset, "modem", &lenp);
    if (ptr == NULL)
		debugf("no modem property.\n");
    else {
		ptr_end = ptr + lenp / (sizeof(unsigned long));
		while (ptr < ptr_end) {
		sprd_dump_mem[i].paddr = fdt_addr_to_cpu (*ptr ++);
		sprd_dump_mem[i].vaddr = __va(sprd_dump_mem[i].paddr);
		sprd_dump_mem[i].size  = fdt_size_to_cpu (*ptr ++);
		sprd_dump_mem[i].soff  = 0xffffffff;
		sprd_dump_mem[i].type  = SYSDUMP_MODEM;
		debugf("sprd_dump_mem[%d].paddr is %lx\n", i, sprd_dump_mem[i].paddr);
		debugf("sprd_dump_mem[%d].size  is %lx\n", i, sprd_dump_mem[i].size);
		i ++;
		}
	}
	sprd_dump_mem_num = i;
	return sprd_dump_mem_num;
}

static unsigned long get_sprd_sysdump_info_paddr(int nodeoffset)
{
	int offset,lenp = 0, version =0;
	unsigned char *fdt_blob = (unsigned char *) DT_ADR;
	unsigned long sprd_dump_magic = 0;
	unsigned long sprd_dump_info_size = 0;
	unsigned long *ptr = NULL, *ptr_end = NULL;

	/*const char *prop = fdt_get_name(fdt_blob, nodeoffset, &lenp);
	if(prop){
		debugf("device_type is %s\n",prop);
		debugf("device_type ptr is %p\n",prop);
		debugf("lenp  is %d\n",lenp);
	}*/

	ptr = fdt_getprop(fdt_blob, nodeoffset, "version", &lenp);
	if (ptr == NULL) {
		debugf("no version property.\n");
		version = 0x1;
	} else
		version = fdt_size_to_cpu(*ptr);

	debugf("sysdump version is 0x%lx\n", version);

	ptr = fdt_getprop(fdt_blob, nodeoffset, "magic-addr", &lenp);
	if (ptr == NULL) {
		debugf("no magic-addr property.\n");
		return 0;
	} else {
		if (version == 0x1)
			sprd_dump_magic = fdt_addr_to_cpu(*ptr);
		else {
			offset = fdt_path_offset(fdt_blob, ptr);
			ptr = fdt_getprop(fdt_blob, offset, "reg", &lenp);
			ptr_end = ptr + lenp / (sizeof(unsigned long));
			while (ptr < ptr_end) {
				sprd_dump_magic = fdt_addr_to_cpu (*ptr);
				ptr ++;
				sprd_dump_info_size = fdt_size_to_cpu(*ptr);
				ptr ++;
			}
		}
	}

	debugf("sprd_dump_magic is 0x%lx\n", sprd_dump_magic);
	debugf("sprd_dump_info_size is 0x%lx\n", sprd_dump_info_size);
	return sprd_dump_magic;
}

static unsigned long long get_sprd_dump_size(int nodeoffset)
{
	unsigned char *fdt_blob = (unsigned char *) DT_ADR;
	unsigned long *ptr = NULL, *ptr_end = NULL;
	unsigned long long size = 0;
	int lenp, offset, version =0;
#if 0
	ptr = fdt_getprop(fdt_blob, nodeoffset, "version", &lenp);
	if (ptr == NULL) {
		debugf("no version property.\n");
		version = 0x1;
	} else
		version = fdt_addr_to_cpu(*ptr);

	debugf("sysdump version is 0x%lx\n", version);

	ptr = fdt_getprop(fdt_blob, nodeoffset, "magic-addr", &lenp);
	if (ptr == NULL)
		debugf("no magic-addr property.\n");
	else {
		offset = fdt_path_offset(fdt_blob, ptr);
		ptr = fdt_getprop(fdt_blob, offset, "reg", &lenp);
		ptr_end = ptr + lenp / (sizeof(unsigned long));
		while (ptr < ptr_end) {
			ptr ++;
			size += fdt_size_to_cpu(*ptr);
			ptr ++;
			debugf("magic-addr size  is %llx\n", size);
		}
	}
#endif
	ptr = fdt_getprop(fdt_blob, nodeoffset, "ram", &lenp);
	if (ptr == NULL)
		debugf("no ram property.\n");
	else {
		offset = fdt_path_offset(fdt_blob, ptr);
		ptr = fdt_getprop(fdt_blob, offset, "reg", &lenp);
		ptr_end = ptr + lenp / (sizeof(unsigned long));
		while (ptr < ptr_end) {
			ptr ++;
			size += fdt_size_to_cpu(*ptr);
			ptr ++;
			debugf("ram size is %llx\n",size);
		}
	}

	ptr = fdt_getprop(fdt_blob, nodeoffset, "modem", &lenp);
	if (ptr == NULL)
		debugf("no modem property.\n");
	else {
		ptr_end = ptr + lenp / (sizeof(unsigned long));
		while (ptr < ptr_end) {
			ptr ++;
			size += fdt_size_to_cpu(*ptr);
			ptr ++;
			debugf("modem size is %llx\n",size);
		}
	}
	return size;
}
#if 0
static int check_sysdump_integrity(struct dump_collection_crc32 *collections)
{
	debugf("collections->last_kmsg_paddr = %lx\n", collections->last_kmsg_paddr);
	debugf("collections->crc32 = %u\n", collections->crc32);
	unsigned int crc32 = 0;
	crc32 = crc32(0, &collections->last_kmsg_paddr,
					sizeof(collections->last_kmsg_paddr));
	if(crc32 != collections->crc32) {
		debugf("error: crc32 = %u\n", crc32);
		return -1;
	}
	return 0;
}

static int check_last_kmsg_integrity(struct sysdump_node *node)
{
	unsigned int crc32 = 0;
	crc32 = crc32(0, &node->paddr, sizeof(node->paddr));
	crc32 = crc32(crc32, &node->size, sizeof(node->size));
	if (crc32 != node->crc32) {
		debugf("error: crc32 = %u\n", crc32);
		return -1;
	}
	return 0;
}
#endif

//arm-unaligned-accesses
#define LOG_ALIGN	0x3
#define MAX_CPU_SUPPORTED	20
#define PREFIX_MAX	32
#define LOG_LINE_MAX	1024 - PREFIX_MAX
static int save_sysdump_last_kmsg(struct sysdump_info * dump_info)
{
	char buf[1024] = {0};
	char path[72] = {0};
	char *head = (unsigned char *)RAMDISK_ADR;
	struct sysdump_node *last_log =
			(struct sysdump_node *)dump_info->last_kmsg_paddr;
	int cnt = 0, ret = 0, fs_type = FS_INVALID;
	unsigned long long available_size;
	struct log *msg;
	unsigned long  rem_nsec, size = 0;
	unsigned long log_next_idx, log_idx, log_buf_end;

#if 0
	if ( check_last_kmsg_integrity(&last_log[0]) != 0 ||
			check_last_kmsg_integrity(&last_log[1]) != 0)
		return -1;
#endif

	ret = fs_set_blk_by_name("mmc",EMMC,"userdata",FS_TYPE_LWEXT4);
	if (ret < 0 ) {
		debugf("error fs_set_blk_by_name EMMC\n");
		return -1;
	}
	fs_type = FS_TYPE_LWEXT4;

	phys_addr_t log_buf_head = last_log[0].paddr;
	log_buf_end = last_log[0].size + log_buf_head;
	phys_addr_t log_buf_tail = last_log[0].paddr + last_log[0].size;
	log_next_idx = log_buf_head;
	log_idx = log_buf_head + *((phys_addr_t*)last_log[1].paddr);
	if (log_buf_head & LOG_ALIGN) {
		debugf("log_buf_head is %lx unaligned\n", log_buf_head);
		return -1;
	}
	msg = (struct log*)log_buf_head;
	if (msg->cpu > MAX_CPU_SUPPORTED  || msg->text_len > LOG_LINE_MAX) {
		debugf("error:log_buf_head is %lx msg->text_len is %x, msg->cpu is %u\n",
			log_buf_head, msg->text_len, msg->cpu);
		return -1;
	}

	while((log_next_idx + msg->len) <= log_idx) {
		if(!msg->len) {
			printf("msg->ts_nsec is %lx, msg->text_len is%x, msg->cpu is %x, msg->len is %u\n",
				msg->ts_nsec, msg->text_len, msg->cpu, msg->len);
			break;
		}
		memset(buf, 0, sizeof(buf));
		rem_nsec = do_div(msg->ts_nsec, 1000000000);
		sprintf(buf, "[%5lu.%06lu] c%d ",
		       (unsigned long)msg->ts_nsec, rem_nsec / 1000, msg->cpu);
		memcpy(buf + strlen(buf), (char *)msg + sizeof(struct log), msg->text_len);
		sprintf(buf + strlen(buf), "%s", "\n");
		//printf("%s",buf);
		memcpy(head, buf, strlen(buf) + 1);
		head += strlen(buf);
		size += strlen(buf);
		// debugf("log_next_idx = %lx, msg->len = %d\n", log_next_idx, msg->len);
		log_next_idx += msg->len;
		if (log_next_idx & LOG_ALIGN || log_next_idx > log_buf_end || log_next_idx < log_buf_head) {
			debugf("error log_next_idx is %lx\n", log_next_idx);
			break;
		}
		msg = (struct log *)log_next_idx;
		if (msg->cpu > MAX_CPU_SUPPORTED  || msg->text_len > LOG_LINE_MAX) {
			debugf("error:log_next_idx is %lx msg->text_len is %x, msg->cpu is %u\n",
				log_next_idx, msg->text_len, msg->cpu);
			break;
		}
		cnt ++;
	}
	debugf("cnt is %d\n",cnt);
	sprintf(path, YLOG_PATH"%s","last_kmsg");
	fs_open(fs_type);
	available_size = fs_available_size();
	if (available_size < size) {
		debugf("no space to storage %s\n",path);
		return -1;
	}
	fs_open(fs_type);
	if (fs_file_stat(path) != 0) {
		debugf("%s is not existed\n",path);
		return -1;
	}
	fs_open(fs_type);
	ret = fs_write(path, (unsigned char *)RAMDISK_ADR, 0, size);
	debugf("writing done. ret = %d\n", ret);
	return 0;
}

static int sysdump_cyclic_storage(int fs_type, unsigned long long dump_size)
{
	char path[72] = {0};
	char path_rename[72] = {0};
	int i, j, ret = 0;
	unsigned long long available_size;
	unsigned char key_code = 0;
	bool delete_flag = false;
	bool nospace_flag = false;

	/*setp 1. check available space: delete or format*/
	fs_open(fs_type);
	available_size = fs_available_size();
	debugf("dump_size = %llu\n",dump_size);
	debugf("available_size = %llu\n",available_size);
	if (available_size < dump_size) {
		nospace_flag = true;
		for (i = 1; i <= SYSDUMP_FOLDER_NUM; i++) {
			sprintf(path, "%s%d", STORAGE_PATH, i);
			fs_open(fs_type);
			ret = fs_dir_stat(path);
			if(ret > NULL)
				break;
		}
		if(i != 1 ) {
			for(j = i; j != 1;) {
				sprintf(path, "%s%d", STORAGE_PATH, --j);
				fs_open(fs_type);
				ret = fs_delete(path);
				if(ret != 0) {
					debugf("Error: delete files or folder\n");
					return -1;
				}
				fs_open(fs_type);
				available_size = fs_available_size();
				if(available_size > dump_size) {
					nospace_flag = false;
					break;
				}
			}
		}
		if(nospace_flag) {
			lcd_printf("\ndetect storage have not enough space");
			lcd_printf("\npress volumedown to format storage\notherwise press volumeup to skip sysdump\n");
			do {
				udelay(50 * 1000);
				key_code = board_key_scan();
				if (key_code == KEY_VOLUMEDOWN) {
					if(fs_type == FS_TYPE_FAT) {
						fat_format();
						fs_open(fs_type);
						ret = fs_mkdir(STORAGE_PATH);
						if (ret != 0)
							return -1;
						break;
					}else
						return -1;
				} else if(key_code == KEY_VOLUMEUP) {
					return -1;
				}
			} while (1);
		}
	}
	/*step 2. enter /ylog/sysdump*/
	fs_open(fs_type);
	ret = fs_mkdir(STORAGE_PATH);
	if (ret != 0)
		return -1;
	/*step 3. check existed folder*/
	for (i = 1; i <= SYSDUMP_FOLDER_NUM; ++i) {
		sprintf(path,"%s%d",STORAGE_PATH,i);
		fs_open(fs_type);
		ret = fs_dir_stat(path);
		if (ret > 0)
			break;
	}
	/*setp 4. delete folder greater than set number*/
	if (i > SYSDUMP_FOLDER_NUM) {
		debugf("%d history log existed !!!\n", SYSDUMP_FOLDER_NUM);
		i --;
		sprintf(path,"%s%d",STORAGE_PATH,i);
		fs_open(fs_type);
		ret = fs_delete(path);
		if(ret != 0)
			debugf("Error: delete files or folder\n");
	}

	/*setp 5. rename folder*/
	for(j = i - 1; j > 0 && i != 1; j = j - 2) {
		sprintf(path, "%s%d", STORAGE_PATH, j++);
		sprintf(path_rename, "%s%d", STORAGE_PATH, j);
		fs_open(fs_type);
		ret = fs_dir_rename(path, path_rename);
		debugf("ret = %d\n",ret);
		if (ret != 0) {
			debugf("Error: rename folder !\n");
			break;//return -1;
		}
	}
	/*setp 6. create a new folder "1"*/
	sprintf(path, STORAGE_PATH"%d", 1);
	fs_open(fs_type);
	ret = fs_mkdir(path);
	if (ret != 0)
		return -1;
	return 0;
}

#define CRC32_NUM_FILE 10
void sysdump_info_write(struct sysdump_info *infop, int fs_type, unsigned long *crc32)
{
	struct rtc_time tm;
	char fnbuf[72] = {0};
	tm = get_time_by_sec();


	debugf("time is %04d.%02d.%02d_%02d:%02d:%02d\n", tm.tm_year, tm.tm_mon, \
		tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

	sprintf(fnbuf,STORAGE_PATH"1/%04d_%02d_%02d_%02d_%02d_%02d.txt", \
		tm.tm_year, tm.tm_mon, tm.tm_mday, \
		tm.tm_hour, tm.tm_min, tm.tm_sec);

	char info[1000] = {0};
	int len = sprintf(info,"-%s%s\n-%s%s\n-%s%d\n-%s%d\n-%s0x%8p\n-%s%d\n-%s0x%lx\n",
		"time is ",
		infop->time,
		"dump_path is ",
		infop->dump_path,
		"elfhdr_size is ",
		infop->elfhdr_size,
		"mem_num is ",
		infop->mem_num,
		"dump_mem_paddr is ",
		infop->dump_mem_paddr,
		"crash_key is ",
		infop->crash_key,
		"UBOOT_START is ",
		UBOOT_START);
	char *ptr = info + len;
	unsigned long *ptr_crc32 = crc32;
	unsigned char num, len_crc32 = 0;

	for (num = 0; num < CRC32_NUM_FILE && ptr_crc32[num]; num++) {
		len_crc32 = sprintf(ptr, "%lx ",ptr_crc32[num]);
		ptr += len_crc32;
		len += len_crc32;
	}
	sysdump_write(fs_type, fnbuf, (char *)(info), 0, len);
}

void write_sysdump_before_boot(int rst_mode)
{
	int i, j, sprd_dump_mem_num, ret, fs_type = FS_INVALID;
	int auto_test_flag = 0;
	char fnbuf[72] = {0}, *path, *waddr;
	struct sysdump_mem *mem;
	struct sysdump_mem sprd_dump_mem[MAX_NUM_DUMP_MEM];
    struct sysdump_info *infop = NULL;
    unsigned long sprd_sysdump_magic = 0;
    unsigned long long dump_size = 0;
    unsigned long crc32[CRC32_NUM_FILE] = {0};
    unsigned char crc32_index = 0;
    int mod = 0, key_code = 0;
    int nodeoffset = check_dts_sysdump_node();

    if (nodeoffset < 0) {
		debugf("ERROR: device tree must have /sprd_sysdump node %s.\n", fdt_strerror(nodeoffset));
		return;
	}

    sprd_sysdump_magic = get_sprd_sysdump_info_paddr(nodeoffset);
    if (sprd_sysdump_magic <= 0)
		return;

	infop = (struct sysdump_info *)sprd_sysdump_magic;

	debugf("rst_mode:0x%x, Check if need to write sysdump info of 0x%08lx to file...\n", rst_mode,
	sprd_sysdump_magic);
	if ((rst_mode == CMD_WATCHDOG_REBOOT) || (rst_mode == CMD_UNKNOW_REBOOT_MODE) || (rst_mode == CMD_EXT_RSTN_REBOOT_MODE) || \
		((rst_mode == CMD_PANIC_REBOOT) && !memcmp(infop->magic, SYSDUMP_MAGIC, sizeof(infop->magic)))|| (rst_mode == CMD_SPECIAL_MODE) ) {
		debugf("Now enter sysdump!!!\n");
		memset(infop->magic, 0, sizeof(infop->magic));

		if(-1 == sysdump_flag_check()) {
			debugf("skip sysdump because sysdump_flag is close.\n");
			goto FINISH;
		}
		//MMU_DisableIDCM();
#ifdef CONFIG_SYSDUMP_USE_LWEXT4
		ret = fs_set_blk_by_name("mmc",EMMC,"userdata",FS_TYPE_LWEXT4);
		if (ret < 0 ) {
			debugf("error fs_set_blk_by_name EMMC\n");
			goto FINISH;
		}
		fs_type = FS_TYPE_LWEXT4;
#else
		if (SD_init(&fs_type))
		/*
		{
			mod = fat_checksys(0);
			if (mod & FSFATAL) {
				lcd_printf("\ndetect sd is demaged");
				lcd_printf("\npress volumedown to format sd\notherwise press volumeup to skip sysdump\n");
				do {
					udelay(50 * 1000);
					key_code = board_key_scan();
					if (key_code == KEY_VOLUMEDOWN) {
						fat_format();
						break;
					} else if(key_code == KEY_VOLUMEUP) {
						goto FINISH;
					}
				} while (1);
			}
			fs_type = FS_TYPE_FAT;
		} else
		*/
			goto FINISH;
#endif
		// display_crashinfo(infop, rst_mode);

		/* display on screen */
		display_writing_sysdump();
		lcd_printf("\nReset mode: %s\n\n",GET_RST_MODE(rst_mode));

		if ((rst_mode == CMD_WATCHDOG_REBOOT) || (rst_mode == CMD_UNKNOW_REBOOT_MODE) ||
			(rst_mode == CMD_EXT_RSTN_REBOOT_MODE) || (rst_mode == CMD_SPECIAL_MODE)) {
			infop->dump_path[0] = '\0';

		    sprd_dump_mem_num = fill_dump_mem(sprd_dump_mem, nodeoffset);
			infop->mem_num = sprd_dump_mem_num;
			infop->dump_mem_paddr = (unsigned long)sprd_dump_mem;
			strcpy(infop->time, "hw_watchdog");
			infop->elfhdr_size = get_elfhdr_size(infop->mem_num);
			infop->crash_key = 0;

			sysdump_fill_core_hdr(NULL,
					sprd_dump_mem,
					sprd_dump_mem_num,
					(char *)infop + sizeof(*infop),
					infop->mem_num + 1,
					infop->elfhdr_size);
		}

		if (strlen(infop->dump_path))
			path = infop->dump_path;
		else
			path = NULL;

		dump_size = get_sprd_dump_size(nodeoffset);
		dump_size = dump_size +  infop->elfhdr_size;

		if (fs_type == FS_FAT32) {
			debugf("FAT32 and need dump max space size is %llx\n",dump_size);
			mod = fat_checksys(0);
			if (mod & FSFATAL) {
				lcd_printf("\ndetect sd is demaged");
				lcd_printf("\npress volumedown to format sd\notherwise press volumeup to skip sysdump\n");
				do {
					udelay(50 * 1000);
					key_code = board_key_scan();
					if (key_code == KEY_VOLUMEDOWN) {
						fat_format();
						break;
					} else if(key_code == KEY_VOLUMEUP) {
						goto FINISH;
					}
				} while (1);
			}

			//fs_type = FS_TYPE_FAT;
			ret = sysdump_cyclic_storage(fs_type,dump_size);
			if (ret < 0)
				goto FINISH;

			//fs_type = FS_FAT32;
		}
		else if (fs_type == FS_EXFAT) {
			debugf("exFAT and need dump max space size is %llx\n",dump_size);
			mod = exfat_checksys(dump_size);
			if (mod & FSSMSIZE) {
				debugf("SD card volume size is smaller then dumped size. Skip sysdump.\n");
				goto FINISH;
			}

			if (mod &(FSFATAL | FSNOSPACE)) {
				if (mod & FSFATAL)
					lcd_printf("\ndetect sd is demaged");
				else if (mod & FSNOSPACE)
					lcd_printf("\ndetect sd have not enough space");

				lcd_printf("\npress volumedown to format sd\notherwise press volumeup to skip sysdump\n");
                        	do {
                                	udelay(50 * 1000);
                                	key_code = board_key_scan();
                                	if (key_code == KEY_VOLUMEDOWN) {
                                        	int ret;
                                        	lcd_printf("\nvolumedown key is pressed.\nSD Card format starts........\n");
                                        	ret = exfat_format();
                                        	if (ret != 0) {
							lcd_printf("\nSD Card format failed. Skip sysdump.\n");
							goto FINISH;
						}
					debugf("SD Card formatting done.\n");
                                        break;
					} else if(key_code == KEY_VOLUMEUP) {
						lcd_printf("\nvolumedup key is pressed. sysdump will be skipped.\n");
						goto FINISH;
					}
				} while (1);
			}
		}	

		if (fs_type == FS_FAT32) {
			sprintf(fnbuf, STORAGE_PATH"%s", "sysdump_auto_test.txt");
			//fs_type = FS_TYPE_FAT;
			fs_open(fs_type);
			if(fs_file_stat(fnbuf) == 0) {
				auto_test_flag = 1;
				debugf("there exited sysdump_auto_test.txt, so jump wait_for_keypress.\n");
			}

			//fs_type = FS_FAT32;
		}

		sprintf(fnbuf, STORAGE_PATH"1/"SYSDUMP_CORE_NAME_FMT, 0);
		debugf("sunting: fs_type:0x%x before writing\n", fs_type);
		crc32[crc32_index++] = sysdump_write(fs_type, fnbuf,(char *)infop + sizeof(*infop), 0, infop->elfhdr_size);

#if 1 /* TODO: jianjun.he */
		mem = (struct sysdump_mem *)infop->dump_mem_paddr;
		for (i = 0; i < infop->mem_num; i++) {
			if (0xffffffff != mem[i].soff)
				waddr = (char *)infop + sizeof(*infop) +
						infop->elfhdr_size + mem[i].soff;
			else
				waddr = mem[i].paddr;

		#ifdef CONFIG_RAMDUMP_NO_SPLIT
				sprintf(fnbuf, STORAGE_PATH"1/"SYSDUMP_CORE_NAME_FMT"_0x%08lx-0x%08lx_dump.lst", i + 1, mem[i].paddr, mem[i].paddr + mem[i].size -1);
				crc32[crc32_index++] = sysdump_write(fs_type,fnbuf, waddr, 0, mem[i].size);
		#else
			if (mem[i].size <= SZ_8M) {
				sprintf(fnbuf, SYSDUMP_CORE_NAME_FMT, i + 1);
				crc32[crc32_index++] = sysdump_write(fs_type, fnbuf, waddr, 0, mem[i].size);
			} else {
				for (j = 0; j < mem[i].size / SZ_8M; j++) {
					sprintf(fnbuf, STORAGE_PATH"/1/"SYSDUMP_CORE_NAME_FMT"_%03d",
						i + 1, j);
					crc32[crc32_index++] = sysdump_write(fs_type,fnbuf, waddr + j * SZ_8M, 0, SZ_8M);
				}

				if (mem[i].size % SZ_8M) {
					sprintf(fnbuf, STORAGE_PATH"/1/"SYSDUMP_CORE_NAME_FMT"_%03d",
						i + 1, j);
					crc32[crc32_index++] = sysdump_write(fs_type, fnbuf, waddr + j * SZ_8M, 0,(mem[i].size % SZ_8M));
				}
			}
		#endif
		}
#else
		for (i = 0; i < infop->mem_num; i++) {
			sprintf(fnbuf, SYSDUMP_CORE_NAME_FMT, /*infop->time,*/ i + 1);
			write_mem_to_mmc(path, fnbuf, fs_type, mem[i].paddr, mem[i].size);
		}
#endif
		sysdump_info_write(infop,fs_type,crc32);

#ifndef CONFIG_EMMC_BOOT
		lcd_printf("\nBegin to dump nand flash:\n");
		mtd_dump(fs_type);
#endif

#ifdef DEBUG
#if 0
        if (check_sysdump_integrity(&infop->collections) != 0)
                return;
#endif
        debugf("last_kmsg_paddr is %lx\n", infop->last_kmsg_paddr);
        if(save_sysdump_last_kmsg(infop))
                debugf("ERROR: save last kmsg failed.\n");
        else
                debugf("Success: save last_kmsg success.\n");
#endif

		debugf("\nwriting done.\nPress any key to continue...");
		lcd_printf("\nWriting done.\nPress any key (Exp power key) to continue...");
		if(!auto_test_flag)
			wait_for_keypress();
	} else
		debugf("no need.\n");

FINISH:
	return;
}



static int display_crashinfo(struct sysdump_info *sinfo, int rst_mode)
{
	struct crash_info *cinfo;
	cinfo = &sinfo->crash;

	if (!strncmp(cinfo->magic, U_MAGIC, sizeof(U_MAGIC))
	    || !strncmp(cinfo->magic, K_MAGIC, sizeof(K_MAGIC)))
	{
		lcd_printf("\n ==============================================================");
		lcd_printf("\n  Crash mode: %s ",GET_RST_MODE(rst_mode));
		lcd_printf("\n  Crash info: %s", cinfo->payload);
		lcd_printf("\n ==============================================================\n\n\n\n");
		return 0;
	}

	return 1;
}
