/*
 * Copyright (C) 2014 Spreadtrum Communications Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "secure_efuse.h"

#include <mmc.h>
#include <secureboot/sec_common.h>

#include "uboot_sec_drv.h"

#define  SOCID_SIZE_IN_WORDS 8 

imgToVerifyInfo g_imgToVerInfo = {0};

int secure_efuse_program_sansa(void)
{
  //unsigned char t_splImageBuf[SPL_IMAGE_SIZE];
  printf("###### Emmc_Read ######\n"); 
  /**read spl image**/
  Emmc_Read(PARTITION_BOOT1, 0, SPL_EMMC_SECTOR_NUM, (unsigned char *)SAVE_SPL_IMAGE);

  g_imgToVerInfo.img_addr = SAVE_SPL_IMAGE;
  g_imgToVerInfo.img_len = SPL_IMAGE_SIZE;

  printf("addr = %x,len = %x \n",g_imgToVerInfo.img_addr,g_imgToVerInfo.img_len);

  uboot_program_efuse(&g_imgToVerInfo,sizeof(imgToVerifyInfo));

  return PROG_OK;
  
}

unsigned int sansa_compute_socid(void *socid)
{
	int ret = 0;
	int i;
	uint32_t socIdBuff[SOCID_SIZE_IN_WORDS] = {0};

	ret = uboot_get_socid(socIdBuff, sizeof(socIdBuff));
	if (ret == 0) {
		memcpy((uint8_t *)socid, (uint8_t *)socIdBuff, sizeof(socIdBuff));
	}

	return ret;
}
