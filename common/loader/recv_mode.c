#include <common.h>
#include <boot_mode.h>
#include <part_efi.h>
#include "loader_common.h"


#ifdef CONFIG_EMMC_BOOT
#include <mmc.h>

char misc_buf[8192];

/* Recovery Message */
struct recovery_message {
	char command[32];
	char status[32];
	char recovery[1024];
};

int get_recovery_message(struct recovery_message *out)
{
	block_dev_desc_t *p_block_dev = NULL;
	disk_partition_t info;
	int size = 8192;
	p_block_dev = get_dev("mmc", 0);
	if (NULL == p_block_dev)
		return -1;
        if (0 != get_partition_info_by_name(p_block_dev, "misc", &info)){
            return -1;}
#ifdef OTA_BACKUP_MISC_RECOVERY
	int ret = 0;
	debugf("info.attributes.power_off_protection=%d\n", info.attributes.fields.power_off_protection);
	if (1 == info.attributes.fields.power_off_protection) {
		debugf("get the power-off protection flag, need to check the misc file in sd card\n");
		ret = get_recovery_msg_in_sd((void*)out,sizeof(struct recovery_message));
		if (ret > 0) {
			debugf("get recovery image from sd card\n");
			return 0;
		} else {
			debugf("no recovery image in sd card\n");
		}
	}
#endif
	if(! Emmc_Read(PARTITION_USER, info.start, size/EMMC_SECTOR_SIZE, (void *)misc_buf)){
		debugf("emcc read error\n");
		return -1;
	}

	memcpy(out, misc_buf, sizeof(*out));
	return 0;

}

int set_recovery_message(const struct recovery_message *in)
{
	block_dev_desc_t *p_block_dev = NULL;
	disk_partition_t info;
	int size = 8192;
	p_block_dev = get_dev("mmc", 0);
	if (NULL == p_block_dev)
		return -1;

	if (!get_partition_info_by_name(p_block_dev, "misc", &info)) {
		memset(misc_buf, 0, sizeof(misc_buf));
		if (!Emmc_Read(PARTITION_USER, info.start, size / EMMC_SECTOR_SIZE, (void *)misc_buf)) {
			debugf("emcc read error\n");
			return -1;
		}
		memcpy((void *)misc_buf, in, sizeof(*in));
		if (!Emmc_Write(PARTITION_USER, info.start, size / EMMC_SECTOR_SIZE, (void *)misc_buf)) {
			debugf("emcc write error\n");
			return -1;
		}
	}
	return 0;
}

#endif

int get_mode_from_file(void)
{

	struct recovery_message msg;
	char partition_name[32];
	unsigned valid_command = 0;

	/*get recovery message */
	if (get_recovery_message(&msg))
		return CMD_UNKNOW_REBOOT_MODE;
	if (msg.command[0] != 0 && msg.command[0] != 255) {
		debugf("Recovery command: %.*s\n", sizeof(msg.command), msg.command);
	}
	/*Ensure termination */
	msg.command[sizeof(msg.command) - 1] = '\0';

	if (!strcmp("boot-recovery", msg.command)) {
		debugf("%s:Message in misc indicate the RECOVERY MODE\n", __FUNCTION__);
		return CMD_RECOVERY_MODE;
	} else if (!strcmp("update-radio", msg.command)) {
		strcpy(msg.status, "OKAY");
		strcpy(msg.command, "boot-recovery");
		/*send recovery message */
		set_recovery_message(&msg);
		reboot_devices(0);
		return CMD_UNKNOW_REBOOT_MODE;
	} else {
		return 0;
	}

}
