/*
 * This file is produced by tools!!
 *
 * Copyright (C) 2012 Spreadtrum Communications Inc.
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

#include <common.h>
#include <asm/arch/sprd_reg.h>
#include <asm/io.h>
#include <asm/arch/sdio_cfg.h>
#include <asm/arch/sdio_reg.h>
#include <asm/arch/chip_drv_common_io.h>


const  SDIO_BASE_INFO  sdio_ctrl_info[2] =
{
	{
		EMMC,
		//SPRD_EMMC_BASE,
		0X20600000,
#ifdef CONFIG_FPGA
             NULL,
             NULL,
	      36000000,
	      100000,
#else
		REG_AP_CLK_EMMC_CFG,
		BIT_0|BIT_1,
 	       384000000,
		400000,
#endif
		//REG_AP_AHB_AHB_EB,
		0X20E00000,
		BIT_11,
		//REG_AP_AHB_AHB_RST,
		0X20E00004,
		BIT_14,

		LDO_LDO_EMMCIO,
		LDO_LDO_EMMCCORE,
	},
	{
		SD,
		0X20300000,
#ifdef CONFIG_FPGA
             NULL,
             NULL,
	      36000000,
	      100000,
#else
		REG_AP_CLK_SDIO0_CFG,
		BIT_0|BIT_1,
		384000000,
		400000,
#endif

		//REG_AP_AHB_AHB_EB,
		0X20E00000,
		BIT_8,
		//REG_AP_AHB_AHB_RST,
		0X20E00004,
		BIT_11,

		LDO_LDO_SDIO1,
		LDO_LDO_SDIO0,
	}
};

SDIO_BASE_INFO*  get_sdcontrol_info(uint32_t device_type)
{
	return  &sdio_ctrl_info[device_type];
}

void  sdio_dump(uint32_t regbase)
{
	uint32_t  i;
	printf("*****SDIO REGISTER DUMP*****\n");
    for(i=0;i<8;i++){
		printf("0x%08x,0x%08x,0x%08x,0x%08x\n",CHIP_REG_GET(regbase + i*16 +0),CHIP_REG_GET(regbase + i*16 +4),CHIP_REG_GET(regbase + i*16 +8),CHIP_REG_GET(regbase + i*16+0xc));
	}
}


