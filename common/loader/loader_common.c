#include "loader_common.h"
#include <mmc.h>
#include <fat.h>
#if defined(CONFIG_OF_LIBFDT)
#include <libfdt.h>
#include <fdt_support.h>
#endif

#ifdef CONFIG_MINI_TRUSTZONE
#include "trustzone_def.h"
#endif

#include "sprd_cpcmdline.h"
#include "smcall.h"

unsigned char raw_header[8192];

unsigned int g_charger_mode = 0;
char serial_number_to_transfer[SP15_MAX_SN_LEN];
unsigned int g_fb_lockflag = 0;

#ifdef CONFIG_WITH_SECURE_TOS && ifndef CONFIG_MINI_TRUSTZONE
#define KEY_ADDR 0x9d000000
#define HASH_LEN 32
uint32_t g_efuse_blk0;
uint32_t g_efuse_blk1;
#endif

extern int charger_connected(void);
extern void modem_entry(void);
extern void fixup_pmic_items(void);

unsigned short calc_checksum(unsigned char *dat, unsigned long len)
{
	unsigned short num = 0;
	unsigned long chkSum = 0;
	while (len > 1) {
		num = (unsigned short)(*dat);
		dat++;
		num |= (((unsigned short)(*dat)) << 8);
		dat++;
		chkSum += (unsigned long)num;
		len -= 2;
	}
	if (len) {
		chkSum += *dat;
	}
	chkSum = (chkSum >> 16) + (chkSum & 0xffff);
	chkSum += (chkSum >> 16);
	return (~chkSum);
}

unsigned char _chkNVEcc(uint8_t * buf, uint32_t size, uint32_t checksum)
{
	uint16_t crc;

	crc = calc_checksum(buf, size);
	debugf("_chkNVEcc calcout 0x%lx, org 0x%llx\n", crc, checksum);
	return (crc == (uint16_t) checksum);
}

/*modif to support the sp15 64 bit sn NO */
char *get_product_sn(void)
{
	SP09_PHASE_CHECK_T phase_check_sp09;
	SP15_PHASE_CHECK_T phase_check_sp15;

	uint32_t magic;
	memset(serial_number_to_transfer, 0x0, SP15_MAX_SN_LEN);

	strcpy(serial_number_to_transfer, "0123456789ABCDEF");
	if (do_raw_data_read(PRODUCTINFO_FILE_PATITION, sizeof(magic), 0, (char *)&magic)) {
		debugf("read miscdata error.\n");
		return serial_number_to_transfer;
	}
	if (magic == SP09_SPPH_MAGIC_NUMBER){
		if (do_raw_data_read(PRODUCTINFO_FILE_PATITION, sizeof(phase_check_sp09), 0, (char *)&phase_check_sp09)) {
			debugf("sp09 read miscdata error.\n");
			return serial_number_to_transfer;
		}
		if (strlen(phase_check_sp09.SN1)) {
			memcpy(serial_number_to_transfer, phase_check_sp09.SN1, SP09_MAX_SN_LEN);
		}
	} else if (magic == SP15_SPPH_MAGIC_NUMBER ){
		if (do_raw_data_read(PRODUCTINFO_FILE_PATITION, sizeof(phase_check_sp15), 0, (char *)&phase_check_sp15)) {
			debugf("sp15 read miscdata error.\n");
			return serial_number_to_transfer;
		}
		if (strlen(phase_check_sp15.SN1)) {
			memcpy(serial_number_to_transfer, phase_check_sp15.SN1, SP15_MAX_SN_LEN);
		}
	}
	return serial_number_to_transfer;
}
/*
* flag:1 means jump secure check or verify.
* flag:0 means normal check
* default: 0
*/
unsigned int get_fblockflag(void)
{
	unsigned char tmp_buffer[512];

	memset(tmp_buffer, 0, sizeof(tmp_buffer));
	if (do_raw_data_read(PRODUCTINFO_FILE_PATITION, sizeof(tmp_buffer), PDT_INFO_LOCK_FLAG_OFFSET, (char *)tmp_buffer)) {
		debugf("read miscdata error.\n");
	}

	if (!strcmp("VerifiedBoot-LOCK", tmp_buffer)) {
		g_fb_lockflag = 0;
	}
	else if (!strcmp("VerifiedBoot-UNLOCK", tmp_buffer)) {
		g_fb_lockflag = 1;
	}
	else {
		debugf("flag data invalid:%s\n", tmp_buffer);
		g_fb_lockflag = 0; /*default lock status.*/
	}
	//g_fb_lockflag = tmp_buffer[0];
	return g_fb_lockflag;
}

int set_fblockflag(unsigned int flag)
{
	unsigned char tmp_buffer[512];
	unsigned int fb_lockflag_old = 0;
	unsigned int fb_lockflag_new = 0;

	memset(tmp_buffer, 0, sizeof(tmp_buffer));
	if (flag == 1) {
		strcat(tmp_buffer, "VerifiedBoot-UNLOCK");
	}
	else if(flag == 0) {
		strcat(tmp_buffer, "VerifiedBoot-LOCK");
	}
	else {
		debugf("lockflag is invalid.\n");
		return -2;
	}

	fb_lockflag_old = get_fblockflag();

	if (do_raw_data_write(PRODUCTINFO_FILE_PATITION, sizeof(tmp_buffer), sizeof(tmp_buffer), PDT_INFO_LOCK_FLAG_OFFSET, (unsigned char *)tmp_buffer)) {
		debugf("write miscdata error.\n");
		return -1;
	}

	fb_lockflag_new = get_fblockflag();

	debugf("old value:%d, new value:%d\n", fb_lockflag_old, fb_lockflag_new);
	return 0;
}

void fdt_fixup_all(void)
{
	u8 *fdt_blob = (u8 *) DT_ADR;
	uint32_t fdt_size;
	boot_img_hdr *hdr = raw_header;
	int err;

	if (fdt_check_header(fdt_blob) != 0) {
		printf("image is not a fdt\n");
	}
	fdt_size = fdt_totalsize(fdt_blob);

	err = fdt_open_into(fdt_blob, fdt_blob, fdt_size + FDT_ADD_SIZE);
	if (err != 0) {
		printf("libfdt fdt_open_into(): %s\n", fdt_strerror(err));
	}

	fdt_initrd_norsvmem(fdt_blob, RAMDISK_ADR, RAMDISK_ADR + hdr->ramdisk_size, 1);
#ifdef CONFIG_SPLASH_SCREEN
	fdt_fixup_lcdid(fdt_blob);
	fdt_fixup_lcdbase(fdt_blob);
#endif
#ifdef CONFIG_WITH_SECURE_TOS && ifndef CONFIG_MINI_TRUSTZONE
	fdt_fixup_efuse_block(fdt_blob);
#endif
	fdt_fixup_adc_calibration_data(fdt_blob);
	fdt_fixup_dram_training(fdt_blob);
	fdt_fixup_ddr_size(fdt_blob);
	fdt_fixup_sysdump_magic(fdt_blob);
#ifdef CONFIG_SECURE_BOOT
	fdt_fixup_secureboot_param(fdt_blob);
#endif
#ifdef CONFIG_NAND_BOOT
	fdt_fixup_mtd(fdt_blob);
#endif

	/*max let cp_cmdline_fixup befor fdt_fixup_cp_boot*/
	cp_cmdline_fixup();

	fdt_fixup_cp_boot(fdt_blob);

	fdt_fixup_serialno(fdt_blob);

	fdt_fixup_chosen_bootargs_board_private(fdt_blob);

	fdt_fixup_verified_boot(fdt_blob);

	fdt_fixup_flash_lock_state(fdt_blob);

#ifdef CONFIG_MEM_LAYOUT_DECOUPLING
extern int fdt_fixup_cp_coupling_info(void *blob);
	fdt_fixup_cp_coupling_info(fdt_blob);
#endif

#ifdef CONFIG_SANSA_SECBOOT
	//fdt_fixup_socid(fdt_blob);
#endif

	return;
}

#ifdef CONFIG_WITH_VMM
/* MUST ensure dcache is disabled before this call,
 * otherwise cache need be flushed to memory after call
 */
static void fixup_before_vmm(void)
{
	memset((void *)VMM_ADDR_RT, 0, VMM_SIZE_RT);
}
#endif

#ifdef CONFIG_WITH_SECURE_TOS && ifndef CONFIG_MINI_TRUSTZONE
static void pass_hwkey_to_tos(uint8_t *hash_data)
{
	memcpy((void*)KEY_ADDR, hash_data, HASH_LEN);
	uboot_tee_call(SMC_SC_HWKEY_INFO, KEY_ADDR, HASH_LEN, 0);
	memset((void*)KEY_ADDR, 0, HASH_LEN);
}

static void pass_pubkey_to_tos()
{
	uint8_t pubkey[512];
	uint8_t *lockptr;
	int pubkey_len;
	int dev_lock;
	if (loader_binding_key_get(pubkey, &pubkey_len) < 0) {
		printf("has not gotten pubkey!\n");
		return;
	}
	dev_lock = loader_binding_state_get();
	if (dev_lock < 0) {
		printf("has not gotten dev state!\n");
		return;
	}
	memcpy((void*)KEY_ADDR, pubkey, pubkey_len);
	lockptr = (uint8_t *)KEY_ADDR;
	lockptr += pubkey_len;
	memcpy((void*)lockptr, &dev_lock, sizeof(int));
	uboot_tee_call(SMC_SC_PUBKEY_INFO, KEY_ADDR, pubkey_len+sizeof(int), 0);
	memset((void*)KEY_ADDR, 0, pubkey_len+sizeof(int));
}

/*
* add this __attribute__ to avoid memset to be optimized by
* compiler
*/
__attribute__((optimize("O0")))
static void pass_info_to_tos()
{
	uint64_t hwkey = 0;
	uint8_t hash_data[HASH_LEN];

	g_efuse_blk0 &= ~(0x80000000);
	g_efuse_blk1 &= ~(0x80000000);
	hwkey = ((uint64_t)(g_efuse_blk0<<32))| g_efuse_blk1;
	sha256_csum_wd((uint8_t*)&hwkey, sizeof(uint64_t), hash_data, NULL);
	pass_hwkey_to_tos(hash_data);
	/* clear temporary buffer to avoid exploitation */
	memset((void*)hash_data, 0, sizeof(hash_data));
	pass_pubkey_to_tos();
}
#endif

static int start_linux()
{
#ifdef CONFIG_OF_LIBFDT
	void (*theKernel) (int zero, int arch, u32 params);
	theKernel = (void (*)(void *, int, int, int))KERNEL_ADR;

	fdt_fixup_all();

	/*disable all caches,flush cache*/
	cleanup_before_linux();
	/*start modem CP */
	modem_entry();
#ifdef CONFIG_MINI_TRUSTZONE
	trustzone_entry(TRUSTZONE_ADR + 0x200);
#endif

#ifdef CONFIG_WITH_VMM
	fixup_before_vmm();
#endif

#ifdef CONFIG_WITH_SECURE_TOS && ifndef CONFIG_MINI_TRUSTZONE
	secure_sp_entry(SECURE_TOS_ADDR);
	pass_info_to_tos();
#endif
	/* jump to kernel with register set */
	theKernel(0, machine_arch_type, (u32)DT_ADR);
#else
	void (*theKernel) (int zero, int arch, u32 params);
	u32 machine_type = 0;

	machine_type = machine_arch_type;	/* get machine type */
	theKernel = (void (*)(int, int, u32))KERNEL_ADR;	/* set the kernel address */

	/*start modem CP */
	cp_cmdline_fixup();
	modem_entry();

	/* jump to kernel with register set */
	theKernel(0, machine_type, VLX_TAG_ADDR);
#endif /*CONFIG_OF_LIBFDT */
	while (1) ;
	return 0;
}

static int start_linux_armv8()
{
	void (*theKernel) (void *dtb_addr, int zero, int arch, int reserved);
	theKernel = (void (*)(void *, int, int, int))KERNEL_ADR;

	fdt_fixup_all();

	/*before switch to el2,flush all cache */
	/*FIXME: cleanup_before_linux() will cause panic here, we need to find the solution*/
	flush_dcache_range(CONFIG_SYS_SDRAM_BASE, CONFIG_SYS_SDRAM_END);
#ifdef CONFIG_DUAL_DDR
		flush_dcache_range(CONFIG_SYS_SDRAM_BASE2, CONFIG_SYS_SDRAM_END2);
#endif

	/*start modem CP */
	modem_entry();

#ifdef CONFIG_MINI_TRUSTZONE
	trustzone_entry(TRUSTZONE_ADR + 0x200);
#endif

	/*kernel must run in el2, so here switch to el2 */
	armv8_switch_to_el2();
	theKernel(DT_ADR, 0, 0, 0);

	/*never enter here*/
	while (1) ;
	return 0;
}


void vlx_entry(void)
{
#if 0
	/*down the device if charger disconnect during calibration detect. */
	if (g_charger_mode && !charger_connected()) {
		g_charger_mode = 0;
		power_down_devices(0);
		while (1) ;
	}

	MMU_DisableIDCM();
#endif

#ifdef DFS_ON_ARM7
	cp_dfs_param_for_arm7();
#endif
	/*shutdown usb ldo, can't shutdown it in the ldo_sleep.c because download mode must use usb */
	fixup_pmic_items();

#ifdef CONFIG_ARM64
	smp_kick_all_cpus();
	start_linux_armv8();
#else
	start_linux();
#endif

}
