/*
 * Copyright (C) 2014 Spreadtrum Communications Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * Authors: Justin Wang <justin.wang@spreadtrum.com>
 */

#include <common.h>
#include <command.h>
#include "packet.h"
#include "dl_cmd_def.h"
#include <dl_cmd_proc.h>
#include <malloc.h>


static struct dl_cmd *cmdlist = NULL;

void dl_cmd_register(enum dl_cmd_type type,
		       int (*handle)(struct dl_packet *pkt, void *arg))
{
	printf("%s:enter\n", __FUNCTION__);
	struct dl_cmd *cmd = (void *)0;
	
	cmd = (struct dl_cmd *)malloc(sizeof(struct dl_cmd));
	//printf("%s:malloc %d to %p mem\n", __FUNCTION__ , sizeof(struct dl_cmd), cmd);
	if (cmd) {
		cmd->type = type;
		cmd->handle = handle;
		cmd->next = cmdlist;
		cmdlist = cmd;
	}
	printf("%s:leave\n", __FUNCTION__);
	return;
}
void dl_cmd_handler(void)
{
	struct dl_cmd *cmd;
	struct dl_packet *pkt;
	printf("%s:enter\n", __FUNCTION__);
	for (;;) {
		//printf("befor get_pkt\n");
		pkt = dl_get_packet();
		//printf("aftgetpk,time=%u\n ", SCI_GetTickCount());
		pkt->body.type = (pkt->body.type >> 8 | pkt->body.type << 8);
		pkt->body.size = (pkt->body.size >> 8 | pkt->body.size << 8);
		for (cmd = cmdlist; cmd; cmd = cmd->next) {
			if(cmd->type != pkt->body.type)
				continue;
			cmd->handle(pkt,NULL);
			dl_free_packet(pkt);
			break;
		}
	}
	return 0;
}

#ifdef CONFIG_DL_POWER_CONTROL
extern void dl_power_control(void);
#endif

int do_download(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
        printf("%s:enter\n", __FUNCTION__);
#ifdef CONFIG_DL_POWER_CONTROL
	dl_power_control();
#endif
        dl_packet_init ();
#ifdef CONFIG_NAND_BOOT
	fdl_ubi_dev_init();
#endif
	sprd_clean_rtc();

	/* register all cmd process functions */
	dl_cmd_register(BSL_CMD_START_DATA, dl_cmd_write_start);
	dl_cmd_register(BSL_CMD_MIDST_DATA, dl_cmd_write_midst);
	dl_cmd_register(BSL_CMD_END_DATA, dl_cmd_write_end);
	dl_cmd_register(BSL_CMD_READ_FLASH_START, dl_cmd_read_start);
	dl_cmd_register(BSL_CMD_READ_FLASH_MIDST, dl_cmd_read_midst);
	dl_cmd_register(BSL_CMD_READ_FLASH_END, dl_cmd_read_end);
	dl_cmd_register(BSL_ERASE_FLASH, dl_cmd_erase);
	dl_cmd_register(BSL_REPARTITION, dl_cmd_repartition);
	dl_cmd_register(BSL_CMD_NORMAL_RESET, dl_cmd_reboot);
	dl_cmd_register(BSL_CMD_POWER_DOWN_TYPE, dl_powerdown_device);
	//dl_cmd_register(BSL_CMD_READ_CHIP_TYPE, dl_cmd_mcu_read_chiptype);
	dl_cmd_register(BSL_CMD_READ_MCP_TYPE, dl_cmd_read_mcptype);
	dl_cmd_register(BSL_CMD_CHECK_ROOTFLAG, dl_cmd_check_rootflag);
	dl_cmd_register(BSL_CMD_READ_UID, dl_cmd_get_uid);
	dl_cmd_register(BSL_CMD_READ_REFINFO, dl_cmd_read_ref_info);

	usb_init(0);
	dl_send_ack (BSL_INCOMPATIBLE_PARTITION);
	/* enter command handler */
	dl_cmd_handler();
        return 0;
}

U_BOOT_CMD(
	download,	CONFIG_SYS_MAXARGS,	1,	do_download,
	"download mode",
	"choose to enter download mode\n"
	"it is used for downloading firmwares to storage in factory or research purpose\n"
);
