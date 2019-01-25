/******************************************************************************
 ** File Name:      pmu.c                                             *
 ** Author:         Chris                                              *
 ** DATE:           04/11/2014                                                *
 ** Copyright:      2014 Spreatrum, Incoporated. All Rights Reserved.         *
 ** Description:    This file defines the basic information on chip.          *
 ******************************************************************************

 ******************************************************************************
 **                        Edit History                                       *
 ** ------------------------------------------------------------------------- *
 ** DATE           NAME             DESCRIPTION                               *
 ** 04/11/2014     Chris               Create.
 ** 07/23/2015     Chris               Modify for new chip.
 ******************************************************************************/

/**---------------------------------------------------------------------------*
 **                         Dependencies                                      *
 **---------------------------------------------------------------------------*/
#include <asm/io.h>
#include "asm/arch/sc_reg.h"
#include "adi_hal_internal.h"
//#include "asm/arch/wdg_drvapi.h"
#include "asm/arch/sprd_reg.h"
#include "asm/arch/boot_drvapi.h"
/**---------------------------------------------------------------------------*
 **                         Compiler Flag                                     *
 **---------------------------------------------------------------------------*/
#ifdef   __cplusplus
extern   "C"
{
#endif

/**---------------------------------------------------------------------------*
 **                         Macro defines.
 **---------------------------------------------------------------------------*/


/**---------------------------------------------------------------------------*
 **                         Struct defines.
 **---------------------------------------------------------------------------*/
/**---------------------------------------------------------------------------*
 **                         Global variables                                  *
 **---------------------------------------------------------------------------*/

/**---------------------------------------------------------------------------*
 **                         Function Definitions                              *
 **---------------------------------------------------------------------------*/
/**---------------------------------------------------------------------------*
 **                         Struct defines.
 **---------------------------------------------------------------------------*/
struct dcdc_core_ds_step_info{
	u32 ctl_reg;
	u32 ctl_sht;
	u32 cal_reg;
	u32 cal_sht;
};

void pmu_commom_config(void)
{
	CHIP_REG_SET(REG_PMU_APB_CGM_PMU_SEL,
		BITS_PMU_APB_CGM_PMU_SEL(0x3) | /*Set pmu_clock=4M for whale to improve latency of pmu*/
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PD_AP_SYS_CFG,
		//BIT_PMU_APB_PD_AP_SYS_FORCE_SHUTDOWN |
		BIT_PMU_APB_PD_AP_SYS_AUTO_SHUTDOWN_EN |
		BITS_PMU_APB_PD_AP_SYS_PWR_ON_DLY(0x01) |
		BITS_PMU_APB_PD_AP_SYS_PWR_ON_SEQ_DLY(0x01) |
		BITS_PMU_APB_PD_AP_SYS_ISO_ON_DLY(0x07) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PD_CA53_TOP_CFG,
		//BIT_PMU_APB_PD_CA53_TOP_DBG_SHUTDOWN_EN |
		//BIT_PMU_APB_PD_CA53_TOP_PD_SEL |
		//BIT_PMU_APB_PD_CA53_TOP_FORCE_SHUTDOWN |
		BIT_PMU_APB_PD_CA53_TOP_AUTO_SHUTDOWN_EN |
		BITS_PMU_APB_PD_CA53_TOP_PWR_ON_DLY(0x01) |
		BITS_PMU_APB_PD_CA53_TOP_PWR_ON_SEQ_DLY(0x02) |
		BITS_PMU_APB_PD_CA53_TOP_ISO_ON_DLY(0x06) |
		0
	);

	CHIP_REG_SET(REG_PMU_APB_PD_CA53_LIT_MP4_CFG,
		//BIT_PMU_APB_PD_CA53_LIT_MP4_DBG_SHUTDOWN_EN |
		//BIT_PMU_APB_PD_CA53_LIT_MP4_PD_SEL |
		//BIT_PMU_APB_PD_CA53_LIT_MP4_FORCE_SHUTDOWN |
		BIT_PMU_APB_PD_CA53_LIT_MP4_AUTO_SHUTDOWN_EN |
		BITS_PMU_APB_PD_CA53_LIT_MP4_PWR_ON_DLY(0x7E) |
		BITS_PMU_APB_PD_CA53_LIT_MP4_PWR_ON_SEQ_DLY(0x06) |
		BITS_PMU_APB_PD_CA53_LIT_MP4_ISO_ON_DLY(0x05) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PD_CA53_LIT_C0_CFG,
		//BIT_PMU_APB_PD_CA53_LIT_C0_DBG_SHUTDOWN_EN |
		//BIT_PMU_APB_PD_CA53_LIT_C0_PD_SEL |
		BIT_PMU_APB_CA53_LIT_C0_WAKEUP_EN |
		//BIT_PMU_APB_PD_CA53_LIT_C0_FORCE_SHUTDOWN |
		BIT_PMU_APB_PD_CA53_LIT_C0_AUTO_SHUTDOWN_EN |
		BITS_PMU_APB_PD_CA53_LIT_C0_PWR_ON_DLY(0x01) |
		BITS_PMU_APB_PD_CA53_LIT_C0_PWR_ON_SEQ_DLY(0x87) |
		BITS_PMU_APB_PD_CA53_LIT_C0_ISO_ON_DLY(0x01) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PD_CA53_LIT_C1_CFG,
		//BIT_PMU_APB_PD_CA53_LIT_C1_DBG_SHUTDOWN_EN |
		//BIT_PMU_APB_PD_CA53_LIT_C1_PD_SEL |
		//BIT_PMU_APB_CA53_LIT_C1_WAKEUP_EN |
		BIT_PMU_APB_PD_CA53_LIT_C1_FORCE_SHUTDOWN |
		//BIT_PMU_APB_PD_CA53_LIT_C1_AUTO_SHUTDOWN_EN |
		BITS_PMU_APB_PD_CA53_LIT_C1_PWR_ON_DLY(0x01) |
		BITS_PMU_APB_PD_CA53_LIT_C1_PWR_ON_SEQ_DLY(0x86) |
		BITS_PMU_APB_PD_CA53_LIT_C1_ISO_ON_DLY(0x02) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PD_CA53_LIT_C2_CFG,
		//BIT_PMU_APB_PD_CA53_LIT_C2_DBG_SHUTDOWN_EN |
		//BIT_PMU_APB_PD_CA53_LIT_C2_PD_SEL |
		//BIT_PMU_APB_CA53_LIT_C2_WAKEUP_EN |
		BIT_PMU_APB_PD_CA53_LIT_C2_FORCE_SHUTDOWN |
		//BIT_PMU_APB_PD_CA53_LIT_C2_AUTO_SHUTDOWN_EN |
		BITS_PMU_APB_PD_CA53_LIT_C2_PWR_ON_DLY(0x01) |
		BITS_PMU_APB_PD_CA53_LIT_C2_PWR_ON_SEQ_DLY(0x85) |
		BITS_PMU_APB_PD_CA53_LIT_C2_ISO_ON_DLY(0x03) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PD_CA53_LIT_C3_CFG,
		//BIT_PMU_APB_PD_CA53_LIT_C3_DBG_SHUTDOWN_EN |
		//BIT_PMU_APB_PD_CA53_LIT_C3_PD_SEL |
		//BIT_PMU_APB_CA53_LIT_C3_WAKEUP_EN |
		BIT_PMU_APB_PD_CA53_LIT_C3_FORCE_SHUTDOWN |
		//BIT_PMU_APB_PD_CA53_LIT_C3_AUTO_SHUTDOWN_EN |
		BITS_PMU_APB_PD_CA53_LIT_C3_PWR_ON_DLY(0x01) |
		BITS_PMU_APB_PD_CA53_LIT_C3_PWR_ON_SEQ_DLY(0x84) |
		BITS_PMU_APB_PD_CA53_LIT_C3_ISO_ON_DLY(0x04) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PD_CA53_BIG_MP4_CFG,
		//BIT_PMU_APB_PD_CA53_BIG_MP4_DBG_SHUTDOWN_EN |
		//BIT_PMU_APB_PD_CA53_BIG_MP4_PD_SEL |
		BIT_PMU_APB_PD_CA53_BIG_MP4_FORCE_SHUTDOWN |
		//BIT_PMU_APB_PD_CA53_BIG_MP4_AUTO_SHUTDOWN_EN |
		BITS_PMU_APB_PD_CA53_BIG_MP4_PWR_ON_DLY(0x7E) |
		BITS_PMU_APB_PD_CA53_BIG_MP4_PWR_ON_SEQ_DLY(0x07) |
		BITS_PMU_APB_PD_CA53_BIG_MP4_ISO_ON_DLY(0x05) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PD_CA53_BIG_C0_CFG,
		//BIT_PMU_APB_PD_CA53_BIG_C0_DBG_SHUTDOWN_EN |
		//BIT_PMU_APB_PD_CA53_BIG_C0_PD_SEL |
		//BIT_PMU_APB_CA53_BIG_C0_WAKEUP_EN |
		BIT_PMU_APB_PD_CA53_BIG_C0_FORCE_SHUTDOWN |
		//BIT_PMU_APB_PD_CA53_BIG_C0_AUTO_SHUTDOWN_EN |
		BITS_PMU_APB_PD_CA53_BIG_C0_PWR_ON_DLY(0x01) |
		BITS_PMU_APB_PD_CA53_BIG_C0_PWR_ON_SEQ_DLY(0x8B) |
		BITS_PMU_APB_PD_CA53_BIG_C0_ISO_ON_DLY(0x01) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PD_CA53_BIG_C1_CFG,
		//BIT_PMU_APB_PD_CA53_BIG_C1_DBG_SHUTDOWN_EN |
		//BIT_PMU_APB_PD_CA53_BIG_C1_PD_SEL |
		//BIT_PMU_APB_CA53_BIG_C1_WAKEUP_EN |
		BIT_PMU_APB_PD_CA53_BIG_C1_FORCE_SHUTDOWN |
		//BIT_PMU_APB_PD_CA53_BIG_C1_AUTO_SHUTDOWN_EN |
		BITS_PMU_APB_PD_CA53_BIG_C1_PWR_ON_DLY(0x01) |
		BITS_PMU_APB_PD_CA53_BIG_C1_PWR_ON_SEQ_DLY(0x8A) |
		BITS_PMU_APB_PD_CA53_BIG_C1_ISO_ON_DLY(0x02) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PD_CA53_BIG_C2_CFG,
		//BIT_PMU_APB_PD_CA53_BIG_C2_DBG_SHUTDOWN_EN |
		//BIT_PMU_APB_PD_CA53_BIG_C2_PD_SEL |
		//BIT_PMU_APB_CA53_BIG_C2_WAKEUP_EN |
		BIT_PMU_APB_PD_CA53_BIG_C2_FORCE_SHUTDOWN |
		//BIT_PMU_APB_PD_CA53_BIG_C2_AUTO_SHUTDOWN_EN |
		BITS_PMU_APB_PD_CA53_BIG_C2_PWR_ON_DLY(0x01) |
		BITS_PMU_APB_PD_CA53_BIG_C2_PWR_ON_SEQ_DLY(0x89) |
		BITS_PMU_APB_PD_CA53_BIG_C2_ISO_ON_DLY(0x03) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PD_CA53_BIG_C3_CFG,
		//BIT_PMU_APB_PD_CA53_BIG_C3_DBG_SHUTDOWN_EN |
		//BIT_PMU_APB_PD_CA53_BIG_C3_PD_SEL |
		//BIT_PMU_APB_CA53_BIG_C3_WAKEUP_EN |
		BIT_PMU_APB_PD_CA53_BIG_C3_FORCE_SHUTDOWN |
		//BIT_PMU_APB_PD_CA53_BIG_C3_AUTO_SHUTDOWN_EN |
		BITS_PMU_APB_PD_CA53_BIG_C3_PWR_ON_DLY(0x01) |
		BITS_PMU_APB_PD_CA53_BIG_C3_PWR_ON_SEQ_DLY(0x88) |
		BITS_PMU_APB_PD_CA53_BIG_C3_ISO_ON_DLY(0x04) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PD_VSP_SYS_CFG ,
		//BIT_PMU_APB_PD_VSP_SYS_FORCE_SHUTDOWN |
		//BIT_PMU_APB_PD_VSP_SYS_AUTO_SHUTDOWN_EN |
		BITS_PMU_APB_PD_VSP_SYS_PWR_ON_DLY(0x01) |
		BITS_PMU_APB_PD_VSP_SYS_PWR_ON_SEQ_DLY(0x01) |
		BITS_PMU_APB_PD_VSP_SYS_ISO_ON_DLY(0x07) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PD_DBG_SYS_CFG ,
		//BIT_PMU_APB_PD_DBG_SYS_FORCE_SHUTDOWN |
		//BIT_PMU_APB_PD_DBG_SYS_AUTO_SHUTDOWN_EN |
		BITS_PMU_APB_PD_DBG_SYS_PWR_ON_DLY(0x01) |
		BITS_PMU_APB_PD_DBG_SYS_PWR_ON_SEQ_DLY(0x01) |
		BITS_PMU_APB_PD_DBG_SYS_ISO_ON_DLY(0x07) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PD_CAM_SYS_CFG ,
		//BIT_PMU_APB_PD_CAM_SYS_FORCE_SHUTDOWN |
		//BIT_PMU_APB_PD_CAM_SYS_AUTO_SHUTDOWN_EN |
		BITS_PMU_APB_PD_CAM_SYS_PWR_ON_DLY(0x01) |
		BITS_PMU_APB_PD_CAM_SYS_PWR_ON_SEQ_DLY(0x01) |
		BITS_PMU_APB_PD_CAM_SYS_ISO_ON_DLY(0x07) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PD_DISP_SYS_CFG ,
		//BIT_PMU_APB_PD_DISP_SYS_FORCE_SHUTDOWN |
		//BIT_PMU_APB_PD_DISP_SYS_AUTO_SHUTDOWN_EN |
		BITS_PMU_APB_PD_DISP_SYS_PWR_ON_DLY(0x01) |
		BITS_PMU_APB_PD_DISP_SYS_PWR_ON_SEQ_DLY(0x01) |
		BITS_PMU_APB_PD_DISP_SYS_ISO_ON_DLY(0x07) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PD_GPU_TOP_CFG ,
		//BIT_PMU_APB_PD_GPU_TOP_FORCE_SHUTDOWN |
		//BIT_PMU_APB_PD_GPU_TOP_AUTO_SHUTDOWN_EN |
		BITS_PMU_APB_PD_GPU_TOP_PWR_ON_DLY(0x01) |
		BITS_PMU_APB_PD_GPU_TOP_PWR_ON_SEQ_DLY(0x01) |
		BITS_PMU_APB_PD_GPU_TOP_ISO_ON_DLY(0x07) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PD_GPU_C0_CFG ,
		//BIT_PMU_APB_PD_GPU_C0_FORCE_SHUTDOWN |
		//BIT_PMU_APB_PD_GPU_C0_AUTO_SHUTDOWN_EN |
		BITS_PMU_APB_PD_GPU_C0_PWR_ON_DLY(0x01) |
		BITS_PMU_APB_PD_GPU_C0_PWR_ON_SEQ_DLY(0x01) |
		BITS_PMU_APB_PD_GPU_C0_ISO_ON_DLY(0x06) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PD_GPU_C1_CFG ,
		//BIT_PMU_APB_PD_GPU_C1_FORCE_SHUTDOWN |
		//BIT_PMU_APB_PD_GPU_C1_AUTO_SHUTDOWN_EN |
		BITS_PMU_APB_PD_GPU_C1_PWR_ON_DLY(0x01) |
		BITS_PMU_APB_PD_GPU_C1_PWR_ON_SEQ_DLY(0x02) |
		BITS_PMU_APB_PD_GPU_C1_ISO_ON_DLY(0x05) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PD_PUB0_SYS_CFG,
		//BIT_PMU_APB_PD_PUB0_SYS_FORCE_SHUTDOWN |
		//BIT_PMU_APB_PD_PUB0_SYS_AUTO_SHUTDOWN_EN |
		BITS_PMU_APB_PD_PUB0_SYS_PWR_ON_DLY(0x01) |
		BITS_PMU_APB_PD_PUB0_SYS_PWR_ON_SEQ_DLY(0x01) |
		BITS_PMU_APB_PD_PUB0_SYS_ISO_ON_DLY(0x07) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PD_PUB1_SYS_CFG,
		//BIT_PMU_APB_PD_PUB1_SYS_FORCE_SHUTDOWN |
		//BIT_PMU_APB_PD_PUB1_SYS_AUTO_SHUTDOWN_EN |
		BITS_PMU_APB_PD_PUB1_SYS_PWR_ON_DLY(0x01) |
		BITS_PMU_APB_PD_PUB1_SYS_PWR_ON_SEQ_DLY(0x0) |
		BITS_PMU_APB_PD_PUB1_SYS_ISO_ON_DLY(0x07) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PD_WTLCP_SYS_CFG,
		//BIT_PMU_APB_PD_WTLCP_SYS_FORCE_SHUTDOWN |
		//BIT_PMU_APB_PD_WTLCP_SYS_AUTO_SHUTDOWN_EN |
		BITS_PMU_APB_PD_WTLCP_SYS_PWR_ON_DLY(0x01) |
		BITS_PMU_APB_PD_WTLCP_SYS_PWR_ON_SEQ_DLY(0x01) |
		BITS_PMU_APB_PD_WTLCP_SYS_ISO_ON_DLY(0x0B) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PD_WTLCP_TGDSP_CFG,
		//BIT_PMU_APB_PD_WTLCP_TGDSP_FORCE_SHUTDOWN |
		//BIT_PMU_APB_PD_WTLCP_TGDSP_AUTO_SHUTDOWN_EN |
		BITS_PMU_APB_PD_WTLCP_TGDSP_PWR_ON_DLY(0x01) |
		BITS_PMU_APB_PD_WTLCP_TGDSP_PWR_ON_SEQ_DLY(0x0B) |
		BITS_PMU_APB_PD_WTLCP_TGDSP_ISO_ON_DLY(0x01) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PD_WTLCP_LDSP_CFG,
		//BIT_PMU_APB_PD_WTLCP_LDSP_FORCE_SHUTDOWN |
		//BIT_PMU_APB_PD_WTLCP_LDSP_AUTO_SHUTDOWN_EN |
		BITS_PMU_APB_PD_WTLCP_LDSP_PWR_ON_DLY(0x01) |
		BITS_PMU_APB_PD_WTLCP_LDSP_PWR_ON_SEQ_DLY(0x0A) |
		BITS_PMU_APB_PD_WTLCP_LDSP_ISO_ON_DLY(0x02) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PD_WTLCP_HU3GE_A_CFG,
		//BIT_PMU_APB_PD_WTLCP_HU3GE_A_FORCE_SHUTDOWN |
		//BIT_PMU_APB_PD_WTLCP_HU3GE_A_AUTO_SHUTDOWN_EN |
		BITS_PMU_APB_PD_WTLCP_HU3GE_A_PWR_ON_DLY(0x01) |
		BITS_PMU_APB_PD_WTLCP_HU3GE_A_PWR_ON_SEQ_DLY(0x08) |
		BITS_PMU_APB_PD_WTLCP_HU3GE_A_ISO_ON_DLY(0x04) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PD_WTLCP_GSM_CFG,
		//BIT_PMU_APB_PD_WTLCP_GSM_FORCE_SHUTDOWN |
		//BIT_PMU_APB_PD_WTLCP_GSM_AUTO_SHUTDOWN_EN |
		BITS_PMU_APB_PD_WTLCP_GSM_PWR_ON_DLY(0x01) |
		BITS_PMU_APB_PD_WTLCP_GSM_PWR_ON_SEQ_DLY(0x07) |
		BITS_PMU_APB_PD_WTLCP_GSM_ISO_ON_DLY(0x05) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PD_WTLCP_TD_CFG,
		//BIT_PMU_APB_PD_WTLCP_TD_FORCE_SHUTDOWN |
		//BIT_PMU_APB_PD_WTLCP_TD_AUTO_SHUTDOWN_EN |
		BITS_PMU_APB_PD_WTLCP_TD_PWR_ON_DLY(0x01) |
		BITS_PMU_APB_PD_WTLCP_TD_PWR_ON_SEQ_DLY(0x06) |
		BITS_PMU_APB_PD_WTLCP_TD_ISO_ON_DLY(0x06) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PD_WTLCP_LTE_P1_CFG,
		//BIT_PMU_APB_PD_WTLCP_LTE_P1_FORCE_SHUTDOWN |
		//BIT_PMU_APB_PD_WTLCP_LTE_P1_AUTO_SHUTDOWN_EN |
		BITS_PMU_APB_PD_WTLCP_LTE_P1_PWR_ON_DLY(0x01) |
		BITS_PMU_APB_PD_WTLCP_LTE_P1_PWR_ON_SEQ_DLY(0x02) |
		BITS_PMU_APB_PD_WTLCP_LTE_P1_ISO_ON_DLY(0x0A) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PD_WTLCP_LTE_P2_CFG,
		//BIT_PMU_APB_PD_WTLCP_LTE_P2_FORCE_SHUTDOWN |
		//BIT_PMU_APB_PD_WTLCP_LTE_P2_AUTO_SHUTDOWN_EN |
		BITS_PMU_APB_PD_WTLCP_LTE_P2_PWR_ON_DLY(0x01) |
		BITS_PMU_APB_PD_WTLCP_LTE_P2_PWR_ON_SEQ_DLY(0x03) |
		BITS_PMU_APB_PD_WTLCP_LTE_P2_ISO_ON_DLY(0x09) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PD_AGCP_SYS_CFG,
		//BIT_PMU_APB_PD_AGCP_SYS_FORCE_SHUTDOWN |
		//BIT_PMU_APB_PD_AGCP_SYS_AUTO_SHUTDOWN_EN |
		BITS_PMU_APB_PD_AGCP_SYS_PWR_ON_DLY(0x01) |
		BITS_PMU_APB_PD_AGCP_SYS_PWR_ON_SEQ_DLY(0x01) |
		BITS_PMU_APB_PD_AGCP_SYS_ISO_ON_DLY(0x03) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PD_AGCP_DSP_CFG,
		//BIT_PMU_APB_PD_AGCP_DSP_FORCE_SHUTDOWN |
		//BIT_PMU_APB_PD_AGCP_DSP_AUTO_SHUTDOWN_EN |
		BITS_PMU_APB_PD_AGCP_DSP_PWR_ON_DLY(0x01) |
		BITS_PMU_APB_PD_AGCP_DSP_PWR_ON_SEQ_DLY(0x03) |
		BITS_PMU_APB_PD_AGCP_DSP_ISO_ON_DLY(0x01) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PD_AGCP_GSM_CFG,
		//BIT_PMU_APB_PD_AGCP_GSM_FORCE_SHUTDOWN |
		//BIT_PMU_APB_PD_AGCP_GSM_AUTO_SHUTDOWN_EN |
		BITS_PMU_APB_PD_AGCP_GSM_PWR_ON_DLY(0x01) |
		BITS_PMU_APB_PD_AGCP_GSM_PWR_ON_SEQ_DLY(0x02) |
		BITS_PMU_APB_PD_AGCP_GSM_ISO_ON_DLY(0x02) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PD_PUBCP_SYS_CFG,
		//BIT_PMU_APB_PD_PUBCP_SYS_FORCE_SHUTDOWN |
		//BIT_PMU_APB_PD_PUBCP_SYS_AUTO_SHUTDOWN_EN |
		BITS_PMU_APB_PD_PUBCP_SYS_PWR_ON_DLY(0x01) |
		BITS_PMU_APB_PD_PUBCP_SYS_PWR_ON_SEQ_DLY(0x01) |
		BITS_PMU_APB_PD_PUBCP_SYS_ISO_ON_DLY(0x01) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PD_WTLCP_LTE_P3_CFG,
		//BIT_PMU_APB_PD_WTLCP_LTE_P3_FORCE_SHUTDOWN |
		//BIT_PMU_APB_PD_WTLCP_LTE_P3_AUTO_SHUTDOWN_EN |
		BITS_PMU_APB_PD_WTLCP_LTE_P3_PWR_ON_DLY(0x01) |
		BITS_PMU_APB_PD_WTLCP_LTE_P3_PWR_ON_SEQ_DLY(0x04) |
		BITS_PMU_APB_PD_WTLCP_LTE_P3_ISO_ON_DLY(0x08) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PD_WTLCP_LTE_P4_CFG,
		//BIT_PMU_APB_PD_WTLCP_LTE_P4_FORCE_SHUTDOWN |
		//BIT_PMU_APB_PD_WTLCP_LTE_P4_AUTO_SHUTDOWN_EN |
		BITS_PMU_APB_PD_WTLCP_LTE_P4_PWR_ON_DLY(0x01) |
		BITS_PMU_APB_PD_WTLCP_LTE_P4_PWR_ON_SEQ_DLY(0x05) |
		BITS_PMU_APB_PD_WTLCP_LTE_P4_ISO_ON_DLY(0x07) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PD_WTLCP_HU3GE_B_CFG,
		//BIT_PMU_APB_PD_WTLCP_HU3GE_B_FORCE_SHUTDOWN |
		//BIT_PMU_APB_PD_WTLCP_HU3GE_B_AUTO_SHUTDOWN_EN |
		BITS_PMU_APB_PD_WTLCP_HU3GE_B_PWR_ON_DLY(0x01) |
		BITS_PMU_APB_PD_WTLCP_HU3GE_B_PWR_ON_SEQ_DLY(0x07) |
		BITS_PMU_APB_PD_WTLCP_HU3GE_B_ISO_ON_DLY(0x03) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_RC_WAIT_CNT,
		BITS_PMU_APB_RC0_WAIT_CNT(0x03) |
		BITS_PMU_APB_RC1_WAIT_CNT(0x03) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_XTL_WAIT_CNT,
		BITS_PMU_APB_XTL0_WAIT_CNT(0x46) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_XTLBUF_WAIT_CNT,
		BITS_PMU_APB_XTLBUF2_WAIT_CNT(0x01) |
		BITS_PMU_APB_XTLBUF1_WAIT_CNT(0x01) |
		BITS_PMU_APB_XTLBUF0_WAIT_CNT(0x47) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PLL_WAIT_CNT1,
		BITS_PMU_APB_DPLL1_WAIT_CNT(0x08) |
		BITS_PMU_APB_DPLL0_WAIT_CNT(0x08) |
		BITS_PMU_APB_MPLL1_WAIT_CNT(0x08) |
		BITS_PMU_APB_MPLL0_WAIT_CNT(0x08) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PLL_WAIT_CNT2,
		BITS_PMU_APB_LTEPLL1_WAIT_CNT(0x08) |
		BITS_PMU_APB_LTEPLL_WAIT_CNT(0x08) |
		BITS_PMU_APB_TWPLL_WAIT_CNT(0x08) |
		BITS_PMU_APB_LVDSDIS_PLL_WAIT_CNT(0x08) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PLL_WAIT_CNT3,
		BITS_PMU_APB_GPLL_WAIT_CNT(0x08) |
		BITS_PMU_APB_RPLL1_WAIT_CNT(0x08) |
		BITS_PMU_APB_RPLL0_WAIT_CNT(0x08) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PWR_CNT_WAIT_CFG0,
		BITS_PMU_APB_PUBCP_PWR_WAIT_CNT(0x01) |
		BITS_PMU_APB_AGCP_PWR_WAIT_CNT(0x01) |
		BITS_PMU_APB_WTLCP_PWR_WAIT_CNT(0x01) |
		BITS_PMU_APB_AP_PWR_WAIT_CNT(0x09) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PWR_CNT_WAIT_CFG1,
		BITS_PMU_APB_SLP_CTRL_CLK_DIV_CFG(0x7C) |
		BITS_PMU_APB_CM3_PWR_WAIT_CNT(0x0) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PWR_ST_DEBUG_DLY0,
		BITS_PMU_APB_CGM_OFF_DLY(0x02) |
		BITS_PMU_APB_CGM_ON_DLY(0x02) |
		BITS_PMU_APB_ISO_OFF_DLY(0x02) |
		BITS_PMU_APB_RST_DEASSERT_DLY(0x02) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PWR_ST_DEBUG_DLY1,
		BITS_PMU_APB_SHUTDOWN_M_D_DLY(0x04) |
		BITS_PMU_APB_PWR_ST_CLK_DIV_CFG(0x07) |
		BITS_PMU_APB_RST_ASSERT_DLY(0x02) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PD_WAIT_CNT1,
		BITS_PMU_APB_PUBCP_PWR_PD_WAIT_CNT(0x07) |
		BITS_PMU_APB_AGCP_PWR_PD_WAIT_CNT(0x07) |
		BITS_PMU_APB_WTLCP_PWR_PD_WAIT_CNT(0x07) |
		BITS_PMU_APB_AP_PWR_PD_WAIT_CNT(0x00) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PD_WAIT_CNT2,
		BITS_PMU_APB_CM3_PWR_PD_WAIT_CNT(0x00) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PD_CA53_LIT_MP4_SHUTDOWN_DLY,
		BITS_PMU_APB_PD_CA53_LIT_MP4_SHUTDOWN_DLY(0x7D) |
		0
	);
	CHIP_REG_AND(REG_PMU_APB_GPLL_PWR_CTRL,
		~BIT_PMU_APB_GPLL_PD	|
		0
	);
}

static void setup_autopd_mode(void)
{
	/*enable the emc auto gate en*/
	CHIP_REG_SET(REG_AON_APB_EMC_AUTO_GATE_EN,
		BIT_AON_APB_PUBCP_AON_AUTO_GATE_EN |
		BIT_AON_APB_AGCP_AON_AUTO_GATE_EN |
		BIT_AON_APB_WTLCP_AON_AUTO_GATE_EN |
		BIT_AON_APB_AP_AON_AUTO_GATE_EN |
		BIT_AON_APB_PUB_AON_AUTO_GATE_EN |
		BIT_AON_APB_CAM_DMC_AUTO_GATE_EN |
		BIT_AON_APB_DISP_DMC_AUTO_GATE_EN |
		BIT_AON_APB_VSP_DMC_AUTO_GATE_EN |
		BIT_AON_APB_PUBCP_DMC_AUTO_GATE_EN |
		BIT_AON_APB_AGCP_DMC_AUTO_GATE_EN |
		BIT_AON_APB_WTLCP_DMC_AUTO_GATE_EN |
		BIT_AON_APB_AP_DMC_AUTO_GATE_EN |
		BIT_AON_APB_CA53_DMC_AUTO_GATE_EN |
		0
	);
}

static void dcdc_ldo_optimize_config(uint32_t para)
{
#if defined(CONFIG_ADIE_SC2723S) || defined(CONFIG_ADIE_SC2723)
	unsigned short dcdc_ctrl[]={
		0x1a80,/*core*/
		0x1a80,/*mem*/
		0x1c80,/*gen*/
		0x1a80,/*arm*/
		0x1a80,/*wpa*/
		0x1a80,/*rf*/
		0x1a80,/*con*/
		0x0808,/*mem & core*/
		0x0a08,/*arm & gen*/
		0x0808,/*con & rf*/
		//0x3008 /*wpa*/
	};
	uint32_t i;
	uint32_t reg_val = 0;
	for(i=0;i<sizeof(dcdc_ctrl)/sizeof(dcdc_ctrl[0]);i++)
	{
		sci_adi_write(ANA_REG_GLB_DCDC_CTRL0 + (i << 2),dcdc_ctrl[i],0xffff);
	}

	reg_val =(ANA_REG_GET(ANA_REG_GLB_CHIP_ID_HIGH) & 0xFFFF) << 16;
		reg_val |= (ANA_REG_GET(ANA_REG_GLB_CHIP_ID_LOW) & 0xFFFF);

	if((reg_val != 0x2723a000) && (reg_val != 0x2723a001)) {
			/*enable the vdd28 low power mode for sc2723AD*/
		ANA_REG_OR(ANA_REG_GLB_PWR_SLP_CTRL2, 0x2);
	}
#elif defined(CONFIG_ADIE_SC2731)
	return;
#endif
}

struct dcdc_sleep_reg_ctrl{
	volatile uint32_t magic_header;
	volatile uint32_t is_auto_ds;
	volatile uint32_t magic_ender;
};

struct dcdc_sleep_reg_ctrl ds_cfg=
{
	0x11119999,
#if  defined(CONFIG_SC9836A) || defined(CONFIG_SC9838A)
	0x00000007,
#else
	0x00000000,
#endif
	0x99991111
};

static void dcdc_core_ds_config(uint32_t para)
{
#if defined(CONFIG_ADIE_SC2723S) || defined(CONFIG_ADIE_SC2723)
	uint32_t core_adi_val = 0;
        uint32_t reg_chip_id = 0;
        uint32_t reg_val = 0;
        uint32_t reg_val_cal = 0;

        reg_chip_id =(ANA_REG_GET(ANA_REG_GLB_CHIP_ID_HIGH) & 0xFFFF) << 16;
        reg_chip_id |= (ANA_REG_GET(ANA_REG_GLB_CHIP_ID_LOW) & 0xFFFF);
        if(reg_chip_id == 0x2723a000) {
                reg_val = ANA_REG_GET(ANA_REG_GLB_DCDC_CORE_ADI);
                reg_val &= 0x1F;
                core_adi_val |= reg_val << 5;
                reg_val_cal = ANA_REG_GET(ANA_REG_GLB_DCDC_CORE_ADI);
                reg_val_cal &= 0x1F << 5;
                core_adi_val |= reg_val_cal >> 5;
                ANA_REG_SET(ANA_REG_GLB_DCDC_SLP_CTRL1, core_adi_val);
                /*disable the DCDC_CORE_SLEEP_OUT_STEP for dcdc core step down */
                ANA_REG_AND(ANA_REG_GLB_DCDC_SLP_CTRL0, ~(0x3));
        } else {
                if(0x00000007 == ds_cfg.is_auto_ds) {
                        /*enable the DCDC_CORE_SLEEP_OUT_STEP for dcdc core step down */
                        ANA_REG_OR(ANA_REG_GLB_DCDC_SLP_CTRL0, 0x3);
                } else {
                        /*SLEEP_VDDCORE equal NOMAL_VDDCORE*/
                        reg_val = ANA_REG_GET(ANA_REG_GLB_DCDC_CORE_ADI);
                        reg_val &= 0x1F;
                        core_adi_val |= reg_val << 5;
                        reg_val_cal = ANA_REG_GET(ANA_REG_GLB_DCDC_CORE_ADI);
                        reg_val_cal &= 0x1F << 5;
                        core_adi_val |= reg_val_cal >> 5;
                        ANA_REG_SET(ANA_REG_GLB_DCDC_SLP_CTRL1, core_adi_val);
                        /*enable the DCDC_CORE_SLEEP_OUT_STEP for dcdc core step down */
                        //ANA_REG_OR(ANA_REG_GLB_DCDC_SLP_CTRL0, 0x3);
                        /*disable the DCDC_CORE_SLEEP_OUT_STEP */
                        ANA_REG_AND(ANA_REG_GLB_DCDC_SLP_CTRL0, ~(0x3));
                }
        }
#elif defined(CONFIG_ADIE_SC2731)
	uint32_t core_adi_val = 0;
        uint32_t reg_chip_id = 0;
        uint32_t reg_val = 0;
        uint32_t reg_val_cal = 0;

	//return;

	reg_chip_id =(ANA_REG_GET(ANA_REG_GLB_CHIP_ID_HIGH) & 0xFFFF) << 16;
        reg_chip_id |= (ANA_REG_GET(ANA_REG_GLB_CHIP_ID_LOW) & 0xFFFF);
	if(0x00000007 == ds_cfg.is_auto_ds) {
		/*enable the DCDC_CORE_SLEEP_OUT_STEP for dcdc core step down */
		ANA_REG_OR(ANA_REG_GLB_DCDC_CORE_SLP_CTRL0, 0x3);
	} else {
		/*SLEEP_VDDCORE equal NOMAL_VDDCORE*/
		reg_val = ANA_REG_GET(ANA_REG_GLB_DCDC_CORE_VOL);
		reg_val &= 0x1F;
		core_adi_val |= reg_val << 5;
		reg_val_cal = ANA_REG_GET(ANA_REG_GLB_DCDC_CORE_VOL);
		reg_val_cal &= 0x1F << 5;
		core_adi_val |= reg_val_cal >> 5;
		ANA_REG_SET(ANA_REG_GLB_DCDC_CORE_SLP_CTRL1, core_adi_val);
		/*enable the DCDC_CORE_SLEEP_OUT_STEP for dcdc core step down */
		//ANA_REG_OR(ANA_REG_GLB_DCDC_SLP_CTRL0, 0x3);
		/*disable the DCDC_CORE_SLEEP_OUT_STEP */
		ANA_REG_AND(ANA_REG_GLB_DCDC_CORE_SLP_CTRL0, ~(0x3));
	}
#endif
}

void fixup_pmic_items(void)
{
	/*set ldo_usb pd for tsharkl uboot64*/
	//ANA_REG_OR(ANA_REG_GLB_LDO_PD_CTRL,BIT_LDO_USB_PD);
}

void CSP_Init(uint32_t gen_para)
{
	uint32_t reg_val;
	//setup_autopd_mode();
	pmu_commom_config();
	dcdc_core_ds_config(0x00000000);
	//setup_ap_cp_sync_sleep_code(0x50001800);
	//dcdc_ldo_optimize_config(0x00000000);
}

/**---------------------------------------------------------------------------*
 **                         Compiler Flag                                     *
 **---------------------------------------------------------------------------*/
#ifdef __cplusplus
}
#endif


