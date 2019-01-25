/******************************************************************************
 ** File Name:    sdio_reg_v0.h                                            *
 ** Author:       Steve.Zhan                                                 *
 ** DATE:         06/05/2010                                                  *
 ** Copyright:    2010 Spreatrum, Incoporated. All Rights Reserved.           *
 ** Description:                                                              *
 ******************************************************************************/
/******************************************************************************
 **                   Edit    History                                         *
 **---------------------------------------------------------------------------*
 ** DATE          NAME            DESCRIPTION                                 *
 ** 06/05/2010    Steve.Zhan      Create.                                     *
 ******************************************************************************/
#ifndef _SDIO_REG_V0_H_
#define _SDIO_REG_V0_H_
/*----------------------------------------------------------------------------*
 **                         Dependencies                                      *
 **------------------------------------------------------------------------- */

/**---------------------------------------------------------------------------*
 **                             Compiler Flag                                 *
 **--------------------------------------------------------------------------*/
#ifdef   __cplusplus
extern   "C"
{
#endif
/**---------------------------------------------------------------------------*
**                               Micro Define                                **
**---------------------------------------------------------------------------*/
#include <sprd_regulator.h>
#include <sdhci.h>

#define	 EMMC 	0
#define 	 SD 		1

typedef struct
{
	uint32_t 	pin_base;
	uint32_t 	pin_offset;	
	uint32_t 	pin_value;		
}SDIO_PING_REG;

typedef struct
{
	uint32_t  dev_type;
	uint32_t  regbase;
	uint32_t  baseclk_reg;
	uint32_t  baseclk_mask;

	uint32_t  maxclk;
	uint32_t  minclk;

	uint32_t  ahb_enable_reg;
	uint32_t  ahb_enable_bit;
	uint32_t  ahb_reset_reg;
	uint32_t  ahb_reset_bit;

	LDO_ID_E  ldo_io;
	LDO_ID_E  ldo_core;
}SDIO_BASE_INFO;

SDIO_BASE_INFO*  get_sdcontrol_info(uint32_t device_type);
void  sdio_dump(uint32_t device_type);

#ifdef   __cplusplus
}
#endif
/**---------------------------------------------------------------------------*/
#endif
// End
