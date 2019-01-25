/******************************************************************************
 ** File Name:      chip_phy_v3.c                                             *
 ** Author:         Richard Yang                                              *
 ** DATE:           08/14/2002                                                *
 ** Copyright:      2002 Spreatrum, Incoporated. All Rights Reserved.         *
 ** Description:    This file defines the basic information on chip.          *
 ******************************************************************************

 ******************************************************************************
 **                        Edit History                                       *
 ** ------------------------------------------------------------------------- *
 ** DATE           NAME             DESCRIPTION                               *
 ** 08/14/2002     Richard.Yang     Create.                                   *
 ** 09/16/2003     Xueliang.Wang    Modify CR4013                             *
 ** 08/23/2004     JImmy.Jia        Modify for SC6600D                        *
 ******************************************************************************/

/**---------------------------------------------------------------------------*
 **                         Dependencies                                      *
 **---------------------------------------------------------------------------*/
#include <asm/io.h>
#include "asm/arch/sc_reg.h"
#include "adi_hal_internal.h"
#include "asm/arch/sprd_reg.h"
#include "asm/arch/boot_drvapi.h"
#include "regs_adi.h"
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
#define REG_ANA_INTC_BASE		0x40038380
#define REG_ANA_INTC_INT_EN		(REG_ANA_INTC_BASE + 0x8)
#define BIT_DCDCOTP_INT_EN		(0x1 << 10)

/**---------------------------------------------------------------------------*
 **                         Struct defines.
 **---------------------------------------------------------------------------*/
struct dcdc_core_ds_step_info{
	u32 ctl_reg;
	u32 ctl_sht;
	u32 cal_reg;
	u32 cal_sht;
};
/**---------------------------------------------------------------------------*
 **                         Global variables                                  *
 **---------------------------------------------------------------------------*/

/**---------------------------------------------------------------------------*
 **                         Function Definitions                              *
 **---------------------------------------------------------------------------*/
/*****************************************************************************/
// Description :    This function is used to reset MCU.
// Global resource dependence :
// Author :         Xueliang.Wang
// Note :
/*****************************************************************************/
void CHIP_ResetMCU (void)  //reset interrupt disable??
{
    // This loop is very important to let the reset process work well on V3 board
    // @Richard
    uint32 i = 10000;

	WDG_ClockOn ();
    WDG_TimerInit ();
    
    while (i--);    

    WDG_ResetMCU ();
    
    {
        volatile uint32 tick1 = SCI_GetTickCount();
        volatile uint32 tick2 = SCI_GetTickCount();

        while ( (tick2 - tick1) < 500)
        {
            tick2 = SCI_GetTickCount();
        }
    }
}

/*****************************************************************************/
//  Description:    Returns the HW_RST register address.
//  Author:         Jeff.Li
//  Note :          Because there is no register which can restore information
//                  when watchdog resets the system, so we choose IRAM.
/*****************************************************************************/
LOCAL uint32 CHIP_PHY_GetHwRstAddr (void)
{
    // Returns a DWORD of IRAM shared with DCAM
    return ANA_REG_GLB_WDG_RST_MONITOR;
}

/*****************************************************************************/
//  Description:    Returns the reset mode register address.
//  Author:         Jeff.Li
//  Note:
/*****************************************************************************/
LOCAL uint32 CHIP_PHY_GetRstModeAddr (void)
{
    return ANA_REG_GLB_POR_RST_MONITOR;
}

/*****************************************************************************/
//  Description:    Gets the register in analog die to judge the reset mode. 
//  Author:         Jeff.Li
//  Note:           !It is called before __main now, so it can not call the adi
//                  interface because it contains SCI_DisableIRQ inside, below 
//                  writes the adi read interface individually. Because the la-
//                  ckless of SCI_DisableIRQ, so this function must be called 
//                  before system interrupt is turnned on!
/*****************************************************************************/
LOCAL uint32 CHIP_PHY_GetANAReg (void)
{
    return ANA_REG_GET(ANA_REG_GLB_POR_RST_MONITOR);
}

/*****************************************************************************/
//  Description:    This fuction returns the HW_RST value written before reset.
//  Author:         Jeff.Li
//  Note:           
/*****************************************************************************/
LOCAL uint32 CHIP_PHY_GetHWFlag (void)
{
    // Switch IRAM from DCAM to ARM
    return ANA_REG_GET (CHIP_PHY_GetHwRstAddr ());
}

/*****************************************************************************/
//  Description:    PHY layer realization of BOOT_SetRstMode.
//  Author:         Jeff.Li
//  Note:           The valid bit filed is from bit15 to bit0
/*****************************************************************************/
PUBLIC void CHIP_PHY_SetRstMode (uint32 val)
{
    ANA_REG_AND (CHIP_PHY_GetRstModeAddr (), ~0xFFFF);
    ANA_REG_OR (CHIP_PHY_GetRstModeAddr (), (val&0xFFFF));
}

/*****************************************************************************/
//  Description:    This fuction returns the reset mode value.
//  Author:         Jeff.Li
//  Note:
/*****************************************************************************/
PUBLIC uint32 CHIP_PHY_GetRstMode (void)
{
    return (ANA_REG_GET (CHIP_PHY_GetRstModeAddr ()) & 0xFFFF);
}

/*****************************************************************************/
//  Description:    PHY layer realization of BOOT_ResetHWFlag. It resets the HW
//                  reset register after system initialization.
//  Author:         Jeff.Li
//  Note:           The valid bit filed of analog register is from bit11 to bit0.
//                  | 11   10   9   8 |  7   6   5   4  |  3   2   1   0   |
//                  |ALL_HRST_MONITOR | POR_HRST_MONITOR| WDG_HRST_MONITOR |
//
//                  The valid bit filed of HW_RST is from bit11 to bit0.
/*****************************************************************************/
PUBLIC void CHIP_PHY_ResetHWFlag (uint32 val)
{
    // Reset the analog die register
    ANA_REG_AND(ANA_REG_GLB_POR_RST_MONITOR, ~0xFFF);
    ANA_REG_OR (ANA_REG_GLB_POR_RST_MONITOR, (val&0xFFF));

    // Reset the HW_RST
    ANA_REG_AND(CHIP_PHY_GetHwRstAddr (), ~0xFFFF);
    ANA_REG_OR (CHIP_PHY_GetHwRstAddr (), (val&0xFFFF));
}

/*****************************************************************************/
//  Description:    PHY layer realization of BOOT_SetWDGHWFlag. It Writes flag
//                  to the register which would not be reset by watchdog reset.
//  Author:         Jeff.Li
//  Note:           The valid bit filed is from bit15 to bit0
/*****************************************************************************/
PUBLIC void CHIP_PHY_SetWDGHWFlag (WDG_HW_FLAG_T type, uint32 val)
{
    if(TYPE_RESET == type)
    {        
        ANA_REG_AND(CHIP_PHY_GetHwRstAddr (), ~0xFFFF);
        ANA_REG_OR (CHIP_PHY_GetHwRstAddr (), (val&0xFFFF));
    }
    else
    {
        //wrong type, TODO
    }
}


/*****************************************************************************/
//  Description:    PHY layer realization of __BOOT_IRAM_EN.
//  Author:         Jeff.Li
//  Note:           Do nothing. There are 32KB internal ram dedicated for ARM.
/*****************************************************************************/
PUBLIC void CHIP_PHY_BootIramEn ()
{
}

/*****************************************************************************/
// Description :    This function returns whether the watchdog reset is caused
//                  by software reset or system halted.
// Author :         Jeff.Li
// Note :           The valid bit filed is from bit15 to bit0
/*****************************************************************************/
PUBLIC BOOLEAN CHIP_PHY_IsWDGRstByMCU (uint32 flag)
{
    // Copy the value of HW_RST register to the register specific to reset mode
    ANA_REG_SET (CHIP_PHY_GetRstModeAddr (),
                  (CHIP_PHY_GetHWFlag () & 0xFFFF));

    if ((CHIP_PHY_GetHWFlag () & 0xFFFF) == (flag & 0xFFFF))
    {
        return SCI_FALSE;
    }
    else
    {
        return SCI_TRUE;
    }
}

/*****************************************************************************/
// Description :    This function returns whether the reset is caused by power
//                  up.
// Author :         Jeff.Li
// Note :           | 11   10   9   8 |  7   6   5   4  |  3   2   1   0   |
//                  |ALL_HRST_MONITOR | POR_HRST_MONITOR| WDG_HRST_MONITOR |
/*****************************************************************************/
PUBLIC BOOLEAN CHIP_PHY_IsResetByPowerUp()
{
    if ((CHIP_PHY_GetANAReg () & 0xF0) == 0x0)
    {
        return SCI_TRUE;
    }
    else
    {
        return SCI_FALSE;
    }
}

/*****************************************************************************/
// Description :    This function returns whether the reset is caused by watch-
//                  dog reset.
// Author :         Jeff.Li
// Note :           | 11   10   9   8 |  7   6   5   4  |  3   2   1   0   |
//                  |ALL_HRST_MONITOR | POR_HRST_MONITOR| WDG_HRST_MONITOR |
/*****************************************************************************/
PUBLIC BOOLEAN CHIP_PHY_IsResetByWatchDog()
{
    if ((CHIP_PHY_GetANAReg () & 0xF) == 0x0)
    {
        return SCI_TRUE;
    }
    else
    {
        return SCI_FALSE;
    }
}

/************************************************************
*select TDPLL's reference crystal,
*(1)--RF0---------xtlbuf0-----------
*                               -?-tdpll_ref_sel-----TDPLL
*(2)--RF1---------xtlbuf1-----------
1)rf_id = 0,TDPLL will select (1), or select (2)
************************************************************/
PUBLIC uint32 TDPllRefConfig(TDPLL_REF_T rf_id)
{
    uint32 pll_reg;
/* before switch reference crystal, it must be sure that no module is using TDPLL */
    pll_reg = readl(REG_AP_CLK_AP_AHB_CFG);
    pll_reg &= ~AP_AHB_CLK_SEL_MASK;
    writel(pll_reg, REG_AP_CLK_AP_AHB_CFG);

    pll_reg = readl(REG_AON_CLK_PUB_AHB_CFG);
    pll_reg &= ~PUB_AHB_CLK_SEL_MASK;
    writel(pll_reg, REG_AON_CLK_PUB_AHB_CFG);

    pll_reg = readl(REG_AP_CLK_AP_APB_CFG);
    pll_reg &= ~AP_APB_CLK_SEL_MASK;
    writel(pll_reg, REG_AP_CLK_AP_APB_CFG);

    pll_reg = readl(REG_AON_CLK_AON_APB_CFG);
    pll_reg &= ~PUB_APB_CLK_SEL_MASK;
    writel(pll_reg, REG_AON_CLK_AON_APB_CFG);

    pll_reg = readl(REG_AON_APB_PLL_SOFT_CNT_DONE);
    pll_reg &= ~(BIT_TDPLL_SOFT_CNT_DONE);
    writel(pll_reg, REG_AON_APB_PLL_SOFT_CNT_DONE);
    udelay(1);

/* switch TDPLL reference crystal */
    if (rf_id == TDPLL_REF0)
    {
        pll_reg = readl(REG_PMU_APB_TDPLL_REL_CFG);
        pll_reg &= ~(0x1 << 4);
        writel(pll_reg, REG_PMU_APB_TDPLL_REL_CFG);

        pll_reg = readl(REG_PMU_APB_XTL0_REL_CFG);
        pll_reg |= BIT_XTL1_AP_SEL;
        writel(pll_reg, REG_PMU_APB_XTL0_REL_CFG);

        pll_reg = readl(REG_PMU_APB_XTLBUF0_REL_CFG);
        pll_reg |= BIT_XTLBUF1_AP_SEL;
        writel(pll_reg, REG_PMU_APB_XTLBUF0_REL_CFG);
    }
    else if(rf_id == TDPLL_REF1)
    {
        pll_reg = readl(REG_PMU_APB_TDPLL_REL_CFG);
        pll_reg |= (0x1 << 4);
        writel(pll_reg, REG_PMU_APB_TDPLL_REL_CFG);

        pll_reg = readl(REG_PMU_APB_XTL1_REL_CFG);
        pll_reg |= BIT_XTL1_AP_SEL;
        writel(pll_reg, REG_PMU_APB_XTL1_REL_CFG);

        pll_reg = readl(REG_PMU_APB_XTLBUF1_REL_CFG);
        pll_reg |= BIT_XTLBUF1_AP_SEL;
        writel(pll_reg, REG_PMU_APB_XTLBUF1_REL_CFG);
    }
    else
        return 1;

    pll_reg = readl(REG_AON_APB_PLL_SOFT_CNT_DONE);
    pll_reg |= (BIT_TDPLL_SOFT_CNT_DONE);
    writel(pll_reg, REG_AON_APB_PLL_SOFT_CNT_DONE);

    udelay(120);

/* after switch, up ahb clock to 128M, APB to 64M */
    pll_reg = readl(REG_AP_CLK_AP_AHB_CFG);
    pll_reg |= 0x3;
    writel(pll_reg, REG_AP_CLK_AP_AHB_CFG);

    pll_reg = readl(REG_AON_CLK_PUB_AHB_CFG);
    pll_reg |= 0x3;
    writel(pll_reg, REG_AON_CLK_PUB_AHB_CFG);
    
    pll_reg = readl(REG_AP_CLK_AP_APB_CFG);
    pll_reg |= 0x1;
    writel(pll_reg, REG_AP_CLK_AP_APB_CFG);

    pll_reg = readl(REG_AON_CLK_AON_APB_CFG);
    pll_reg |= 0x3;
    writel(pll_reg, REG_AON_CLK_AON_APB_CFG);
    return 0;
}

typedef enum{
	MODEM_WCDMA = 0,
	MODEM_TD,
	MODEM_CON,
	MODEM_MAX
}SPRD_MODEM_E;

void pmu_cp_modem_config(SPRD_MODEM_E modem)
{

	if((modem == MODEM_WCDMA)||(modem == MODEM_TD)) {
			CHIP_REG_SET(REG_PMU_APB_PD_CP0_SYS_CFG,
				BIT_CP0_FORCE_DEEP_SLEEP |
				BIT_PD_CP0_SYS_FORCE_SHUTDOWN |
				BITS_PD_CP0_SYS_PWR_ON_DLY(0x08) |
				BITS_PD_CP0_SYS_PWR_ON_SEQ_DLY(0x00) |
				BITS_PD_CP0_SYS_ISO_ON_DLY(0x06) |
				0
			);	
			CHIP_REG_SET(REG_PMU_APB_PD_CP0_ARM9_0_CFG,
				//BIT_PD_CP0_ARM9_0_FORCE_SHUTDOWN |
				BIT_PD_CP0_ARM9_0_AUTO_SHUTDOWN_EN |
				BITS_PD_CP0_ARM9_0_PWR_ON_DLY(0x08) |
				BITS_PD_CP0_ARM9_0_PWR_ON_SEQ_DLY(0x06) |
				BITS_PD_CP0_ARM9_0_ISO_ON_DLY(0x02) |
				0
			);
			CHIP_REG_SET(REG_PMU_APB_PD_CP0_ARM9_1_CFG,
				BIT_PD_CP0_ARM9_1_FORCE_SHUTDOWN |
				//BIT_PD_CP0_ARM9_1_AUTO_SHUTDOWN_EN |
				BITS_PD_CP0_ARM9_1_PWR_ON_DLY(0x08) |
				BITS_PD_CP0_ARM9_1_PWR_ON_SEQ_DLY(0x06) |
				BITS_PD_CP0_ARM9_1_ISO_ON_DLY(0x02) |
				0
			);
			CHIP_REG_SET(REG_PMU_APB_PD_CP0_ARM9_2_CFG,
				BIT_PD_CP0_ARM9_2_FORCE_SHUTDOWN |
				//BIT_PD_CP0_ARM9_2_AUTO_SHUTDOWN_EN |
				BITS_PD_CP0_ARM9_2_PWR_ON_DLY(0x08) |
				BITS_PD_CP0_ARM9_2_PWR_ON_SEQ_DLY(0x06) |
				BITS_PD_CP0_ARM9_2_ISO_ON_DLY(0x02) |
				0
			);
			CHIP_REG_SET(REG_PMU_APB_PD_CP0_GSM_CFG,
				//BIT_PD_CP0_GSM_FORCE_SHUTDOWN |
				//BIT_PD_CP0_GSM_AUTO_SHUTDOWN_EN |
				BITS_PD_CP0_GSM_PWR_ON_DLY(0x08) |
				BITS_PD_CP0_GSM_PWR_ON_SEQ_DLY(0x04) |
				BITS_PD_CP0_GSM_ISO_ON_DLY(0x04) |
				0
			);
			CHIP_REG_SET(REG_PMU_APB_PD_CP0_CEVA_CFG,
				//BIT_PD_CP0_CEVA_FORCE_SHUTDOWN |
				//BIT_PD_CP0_CEVA_AUTO_SHUTDOWN_EN |
				BITS_PD_CP0_CEVA_PWR_ON_DLY(0x08) |
				BITS_PD_CP0_CEVA_PWR_ON_SEQ_DLY(0x06) |
				BITS_PD_CP0_CEVA_ISO_ON_DLY(0x02) |
				0
			);
			CHIP_REG_SET(REG_PMU_APB_PD_CP0_HU3GE_CFG,
				BIT_PD_CP0_HU3GE_FORCE_SHUTDOWN |
				//BIT_PD_CP0_HU3GE_AUTO_SHUTDOWN_EN |
				BITS_PD_CP0_HU3GE_PWR_ON_DLY(0x08) |
				BITS_PD_CP0_HU3GE_PWR_ON_SEQ_DLY(0x02) |
				BITS_PD_CP0_HU3GE_ISO_ON_DLY(0x04) |
				0
			);
			CHIP_REG_SET(REG_PMU_APB_PD_CP0_TD_CFG,
				BIT_PD_CP0_TD_FORCE_SHUTDOWN |
				//BIT_PD_CP0_TD_AUTO_SHUTDOWN_EN |
				BITS_PD_CP0_TD_PWR_ON_DLY(0x08) |
				BITS_PD_CP0_TD_PWR_ON_SEQ_DLY(0x02) |
				BITS_PD_CP0_TD_ISO_ON_DLY(0x04) |
				0
			);			
	}

	switch(modem)
	{
		case MODEM_WCDMA:
			CHIP_REG_SET(REG_PMU_APB_PD_CP0_HU3GE_CFG,
				//BIT_PD_CP0_HU3GE_FORCE_SHUTDOWN |
				//BIT_PD_CP0_HU3GE_AUTO_SHUTDOWN_EN |
				BITS_PD_CP0_HU3GE_PWR_ON_DLY(0x08) |
				BITS_PD_CP0_HU3GE_PWR_ON_SEQ_DLY(0x02) |
				BITS_PD_CP0_HU3GE_ISO_ON_DLY(0x04) |
				0
			);
			break;
		case MODEM_TD:
			CHIP_REG_SET(REG_PMU_APB_PD_CP0_TD_CFG,
				//BIT_PD_CP0_TD_FORCE_SHUTDOWN |
				//BIT_PD_CP0_TD_AUTO_SHUTDOWN_EN |
				BITS_PD_CP0_TD_PWR_ON_DLY(0x08) |
				BITS_PD_CP0_TD_PWR_ON_SEQ_DLY(0x02) |
				BITS_PD_CP0_TD_ISO_ON_DLY(0x04) |
				0
			);
			break;
		case MODEM_CON:
			CHIP_REG_SET(REG_PMU_APB_PD_CP2_SYS_CFG,
				BIT_CP2_FORCE_DEEP_SLEEP |
				BIT_PD_CP2_SYS_FORCE_SHUTDOWN |
				BITS_PD_CP2_SYS_PWR_ON_DLY(0x08) |
				BITS_PD_CP2_SYS_PWR_ON_SEQ_DLY(0x00) |
				BITS_PD_CP2_SYS_ISO_ON_DLY(0x06) |
				0
			);
			CHIP_REG_SET(REG_PMU_APB_PD_CP2_ARM9_CFG,
				//BIT_PD_CP2_ARM9_FORCE_SHUTDOWN |
				BIT_PD_CP2_ARM9_AUTO_SHUTDOWN_EN |
				BITS_PD_CP2_ARM9_PWR_ON_DLY(0x08) |
				BITS_PD_CP2_ARM9_PWR_ON_SEQ_DLY(0x02) |
				BITS_PD_CP2_ARM9_ISO_ON_DLY(0x04) |
				0
			);
			CHIP_REG_SET(REG_PMU_APB_PD_CP2_WIFI_CFG,
				//BIT_PD_CP2_WIFI_FORCE_SHUTDOWN |
				BIT_PD_CP2_WIFI_AUTO_SHUTDOWN_EN |
				BITS_PD_CP2_WIFI_PWR_ON_DLY(0x08) |
				BITS_PD_CP2_WIFI_PWR_ON_SEQ_DLY(0x02) |
				BITS_PD_CP2_WIFI_ISO_ON_DLY(0x04) |
				0
			);
			break;
		default:
			break;
	}

}
#ifdef CONFIG_SPX20
void pmu_common_config(void)
{
	unsigned int reg_val;

       CHIP_REG_SET(REG_PMU_APB_PD_CP0_SYS_CFG,
                //BIT_CP0_FORCE_DEEP_SLEEP |
                //BIT_PD_CP0_SYS_FORCE_SHUTDOWN |
                //BIT_PD_CP0_SYS_AUTO_SHUTDOWN_EN |
                BITS_PD_CP0_SYS_PWR_ON_DLY(0x08) |
                BITS_PD_CP0_SYS_PWR_ON_SEQ_DLY(0x00) |
                BITS_PD_CP0_SYS_ISO_ON_DLY(0x06) |
                0
        );
        CHIP_REG_SET(REG_PMU_APB_PD_CP0_ARM9_0_CFG,
                //BIT_PD_CP0_ARM9_0_FORCE_SHUTDOWN |
                //BIT_PD_CP0_ARM9_0_AUTO_SHUTDOWN_EN |
                BITS_PD_CP0_ARM9_0_PWR_ON_DLY(0x08) |
                BITS_PD_CP0_ARM9_0_PWR_ON_SEQ_DLY(0x06) |
                BITS_PD_CP0_ARM9_0_ISO_ON_DLY(0x02) |
                0
        );
        CHIP_REG_SET(REG_PMU_APB_PD_CP0_ARM9_1_CFG,
                //BIT_PD_CP0_ARM9_1_FORCE_SHUTDOWN |
                //BIT_PD_CP0_ARM9_1_AUTO_SHUTDOWN_EN |
                BITS_PD_CP0_ARM9_1_PWR_ON_DLY(0x08) |
                BITS_PD_CP0_ARM9_1_PWR_ON_SEQ_DLY(0x06) |
                BITS_PD_CP0_ARM9_1_ISO_ON_DLY(0x02) |
                0
        );
        CHIP_REG_SET(REG_PMU_APB_PD_CP0_GSM_CFG,
                //BIT_PD_CP0_GSM_FORCE_SHUTDOWN |
                //BIT_PD_CP0_GSM_AUTO_SHUTDOWN_EN |
                BITS_PD_CP0_GSM_PWR_ON_DLY(0x08) |
                BITS_PD_CP0_GSM_PWR_ON_SEQ_DLY(0x04) |
                BITS_PD_CP0_GSM_ISO_ON_DLY(0x04) |
                0
        );
        CHIP_REG_SET(REG_PMU_APB_PD_CP0_CEVA_CFG,
                //BIT_PD_CP0_CEVA_FORCE_SHUTDOWN |
                //BIT_PD_CP0_CEVA_AUTO_SHUTDOWN_EN |
                BITS_PD_CP0_CEVA_PWR_ON_DLY(0x08) |
                BITS_PD_CP0_CEVA_PWR_ON_SEQ_DLY(0x06) |
                BITS_PD_CP0_CEVA_ISO_ON_DLY(0x02) |
                0
        );
        CHIP_REG_SET(REG_PMU_APB_PD_CP0_HU3GE_CFG,
                //BIT_PD_CP0_HU3GE_FORCE_SHUTDOWN |
                //BIT_PD_CP0_HU3GE_AUTO_SHUTDOWN_EN |
                BITS_PD_CP0_HU3GE_PWR_ON_DLY(0x08) |
                BITS_PD_CP0_HU3GE_PWR_ON_SEQ_DLY(0x02) |
                BITS_PD_CP0_HU3GE_ISO_ON_DLY(0x04) |
                0
        );
        CHIP_REG_SET(REG_PMU_APB_PD_CP0_HARQ_CFG,
                //BIT_PD_CP0_HARQ_FORCE_SHUTDOWN |
                //BIT_PD_CP0_HARQ_AUTO_SHUTDOWN_EN |
                BITS_PD_CP0_HARQ_PWR_ON_DLY(0x08) |
                BITS_PD_CP0_HARQ_PWR_ON_SEQ_DLY(0x02) |
                BITS_PD_CP0_HARQ_ISO_ON_DLY(0x04) |
                0
        );
        CHIP_REG_SET(REG_PMU_APB_PD_CP0_TD_CFG,
                //BIT_PD_CP0_TD_FORCE_SHUTDOWN |
                //BIT_PD_CP0_TD_AUTO_SHUTDOWN_EN |
                BITS_PD_CP0_TD_PWR_ON_DLY(0x08) |
                BITS_PD_CP0_TD_PWR_ON_SEQ_DLY(0x02) |
                BITS_PD_CP0_TD_ISO_ON_DLY(0x04) |
                0
        );
        CHIP_REG_SET(REG_PMU_APB_PD_CP2_SYS_CFG,
                //BIT_CP2_FORCE_DEEP_SLEEP |
                //BIT_PD_CP2_SYS_FORCE_SHUTDOWN |
                //BIT_PD_CP2_SYS_AUTO_SHUTDOWN_EN |
                BITS_PD_CP2_SYS_PWR_ON_DLY(0x08) |
                BITS_PD_CP2_SYS_PWR_ON_SEQ_DLY(0x00) |
                BITS_PD_CP2_SYS_ISO_ON_DLY(0x06) |
                0
        );
        CHIP_REG_SET(REG_PMU_APB_PD_CP2_ARM9_CFG,
                //BIT_PD_CP2_ARM9_FORCE_SHUTDOWN |
                //BIT_PD_CP2_ARM9_AUTO_SHUTDOWN_EN |
                BITS_PD_CP2_ARM9_PWR_ON_DLY(0x08) |
                BITS_PD_CP2_ARM9_PWR_ON_SEQ_DLY(0x02) |
                BITS_PD_CP2_ARM9_ISO_ON_DLY(0x04) |
                0
        );
        CHIP_REG_SET(REG_PMU_APB_PD_CP2_WIFI_CFG,
                //BIT_PD_CP2_WIFI_FORCE_SHUTDOWN |
                //BIT_PD_CP2_WIFI_AUTO_SHUTDOWN_EN |
                BITS_PD_CP2_WIFI_PWR_ON_DLY(0x08) |
                BITS_PD_CP2_WIFI_PWR_ON_SEQ_DLY(0x02) |
                BITS_PD_CP2_WIFI_ISO_ON_DLY(0x04) |
                0
        );
        CHIP_REG_SET(REG_PMU_APB_PD_CA7_TOP_CFG,
                //BIT_PD_CA7_TOP_FORCE_SHUTDOWN	        |
                //BIT_PD_CA7_TOP_AUTO_SHUTDOWN_EN       |
                BITS_PD_CA7_TOP_PWR_ON_DLY(8)           |
                BITS_PD_CA7_TOP_PWR_ON_SEQ_DLY(2)       |
                BITS_PD_CA7_TOP_ISO_ON_DLY(4)           |
                0
        );
        CHIP_REG_SET(REG_PMU_APB_PD_CA7_C0_CFG,
                //BIT_PD_CA7_C0_FORCE_SHUTDOWN |
                //BIT_PD_CA7_C0_AUTO_SHUTDOWN_EN        |
                BITS_PD_CA7_C0_PWR_ON_DLY(8)            |
                BITS_PD_CA7_C0_PWR_ON_SEQ_DLY(6)        |
                BITS_PD_CA7_C0_ISO_ON_DLY(2)            |
                0
        );

        CHIP_REG_SET(REG_PMU_APB_PD_CA7_C1_CFG,
                //BIT_PD_CA7_C1_FORCE_SHUTDOWN          |
                //BIT_PD_CA7_C1_AUTO_SHUTDOWN_EN        |
                BITS_PD_CA7_C1_PWR_ON_DLY(8)            |
                BITS_PD_CA7_C1_PWR_ON_SEQ_DLY(4)        |
                BITS_PD_CA7_C1_ISO_ON_DLY(2)            |
                0
        );

        CHIP_REG_SET(REG_PMU_APB_PD_CA7_C2_CFG,
                //BIT_PD_CA7_C2_FORCE_SHUTDOWN          |
                //BIT_PD_CA7_C2_AUTO_SHUTDOWN_EN        |
                BITS_PD_CA7_C2_PWR_ON_DLY(8)            |
                BITS_PD_CA7_C2_PWR_ON_SEQ_DLY(4)        |
                BITS_PD_CA7_C2_ISO_ON_DLY(2)            |
                0
        );

        CHIP_REG_SET(REG_PMU_APB_PD_CA7_C3_CFG,
                //BIT_PD_CA7_C3_FORCE_SHUTDOWN          |
                //BIT_PD_CA7_C3_AUTO_SHUTDOWN_EN	|
                BITS_PD_CA7_C3_PWR_ON_DLY(8)            |
                BITS_PD_CA7_C3_PWR_ON_SEQ_DLY(4)        |
                BITS_PD_CA7_C3_ISO_ON_DLY(2)            |
                0
        );

        CHIP_REG_SET(REG_PMU_APB_PD_AP_SYS_CFG,
                //BIT_PD_AP_SYS_FORCE_SHUTDOWN          |
                BIT_PD_AP_SYS_AUTO_SHUTDOWN_EN        |
                BITS_PD_AP_SYS_PWR_ON_DLY(8)            |
                BITS_PD_AP_SYS_PWR_ON_SEQ_DLY(0)        |
                BITS_PD_AP_SYS_ISO_ON_DLY(6)            |
                0
        );

        CHIP_REG_SET(REG_PMU_APB_PD_MM_TOP_CFG,
                //BIT_PD_MM_TOP_FORCE_SHUTDOWN          |
                //BIT_PD_MM_TOP_AUTO_SHUTDOWN_EN        |
                BITS_PD_MM_TOP_PWR_ON_DLY(8)            |
                BITS_PD_MM_TOP_PWR_ON_SEQ_DLY(0)        |
                BITS_PD_MM_TOP_ISO_ON_DLY(4)            |
                0
        );

        CHIP_REG_SET(REG_PMU_APB_PD_GPU_TOP_CFG,
                //BIT_PD_GPU_TOP_FORCE_SHUTDOWN           |
                //BIT_PD_GPU_TOP_AUTO_SHUTDOWN_EN           |
                BITS_PD_GPU_TOP_PWR_ON_DLY(8)   |
                BITS_PD_GPU_TOP_PWR_ON_SEQ_DLY(0)       |
                BITS_PD_GPU_TOP_ISO_ON_DLY(4)           |
                0
        );

	reg_val = CHIP_REG_GET(REG_PMU_APB_PD_PUB_SYS_CFG);
	reg_val &= ~0xffffff;
	reg_val |= (
		BITS_PD_PUB_SYS_PWR_ON_DLY(8)           |
		BITS_PD_PUB_SYS_PWR_ON_SEQ_DLY(0)       |
		BITS_PD_PUB_SYS_ISO_ON_DLY(6)           |
		0
		);

        CHIP_REG_SET(REG_PMU_APB_PD_PUB_SYS_CFG,reg_val);

		reg_val = CHIP_REG_GET(REG_PMU_APB_PD_DDR_PHY_CFG);
		reg_val &= ~0xffffff;
		reg_val |= (
		BITS_PD_PUB_SYS_PWR_ON_DLY(8)           |
		BITS_PD_PUB_SYS_PWR_ON_SEQ_DLY(0)       |
		BITS_PD_PUB_SYS_ISO_ON_DLY(6)           |
		0
		);

        CHIP_REG_SET(REG_PMU_APB_PD_DDR_PHY_CFG,reg_val);

#ifdef CONFIG_SP7731C  //pike
	CHIP_REG_SET(REG_PMU_APB_MEM_PD_CFG0, 0x02a80000);
	CHIP_REG_SET(REG_PMU_APB_MEM_PD_CFG1, 0x0202aaa2);
#endif


	CHIP_REG_SET(REG_PMU_APB_XTL_WAIT_CNT,
                BITS_XTL1_WAIT_CNT(0x39)                |
                BITS_XTL0_WAIT_CNT(0x39)                |
                0
        );

        CHIP_REG_SET(REG_PMU_APB_XTLBUF_WAIT_CNT,
                BITS_XTLBUF1_WAIT_CNT(7)                |
                BITS_XTLBUF0_WAIT_CNT(7)                |
                0
        );

        CHIP_REG_SET(REG_PMU_APB_PLL_WAIT_CNT1,
                BITS_WPLL_WAIT_CNT(7)                   |
                BITS_TDPLL_WAIT_CNT(7)                  |
                BITS_DPLL_WAIT_CNT(7)                   |
                BITS_MPLL_WAIT_CNT(7)                   |
                0
        );

        CHIP_REG_SET(REG_PMU_APB_PLL_WAIT_CNT2,
                BITS_WIFIPLL2_WAIT_CNT(7)               |
                BITS_WIFIPLL1_WAIT_CNT(7)               |
                BITS_CPLL_WAIT_CNT(7)                   |
                0
        );

        ANA_REG_SET(ANA_REG_GLB_SLP_WAIT_DCDCARM,
                BITS_SLP_IN_WAIT_DCDCARM(7)             |
                BITS_SLP_OUT_WAIT_DCDCARM(8)            |
                0
        );

}

static void clk_auto_gate_config(unsigned int para)
{
	CHIP_REG_SET(REG_PMU_APB_CGM_AP_AUTO_GATE_EN,
		BIT_CGM_307M2_AP_AUTO_GATE_EN	|
		BIT_CGM_208M_AP_AUTO_GATE_EN	|
		BIT_CGM_12M_AP_AUTO_GATE_EN	|
		BIT_CGM_24M_AP_AUTO_GATE_EN	|
		BIT_CGM_48M_AP_AUTO_GATE_EN	|
		BIT_CGM_51M2_AP_AUTO_GATE_EN	|
		BIT_CGM_64M_AP_AUTO_GATE_EN	|
		BIT_CGM_76M8_AP_AUTO_GATE_EN	|
		BIT_CGM_96M_AP_AUTO_GATE_EN	|
		BIT_CGM_128M_AP_AUTO_GATE_EN	|
		BIT_CGM_153M6_AP_AUTO_GATE_EN	|
		BIT_CGM_192M_AP_AUTO_GATE_EN	|
		BIT_CGM_256M_AP_AUTO_GATE_EN	|
		BIT_CGM_384M_AP_AUTO_GATE_EN	|
		BIT_CGM_312M_AP_AUTO_GATE_EN	|
		BIT_CGM_MPLL_AP_AUTO_GATE_EN	|
		//BIT_CGM_WPLL_AP_AUTO_GATE_EN	|
		//BIT_CGM_WIFIPLL1_AP_AUTO_GATE_EN	|
		BIT_CGM_TDPLL_AP_AUTO_GATE_EN	|
		//BIT_CGM_CPLL_AP_AUTO_GATE_EN	|
		BIT_CGM_DPLL_AP_AUTO_GATE_EN	|
		BIT_CGM_26M_AP_AUTO_GATE_EN	|
		0
		);

	CHIP_REG_SET(REG_PMU_APB_CGM_AP_EN,
		BIT_CGM_307M2_AP_AUTO_GATE_EN	|
		BIT_CGM_208M_AP_AUTO_GATE_EN	|
		BIT_CGM_12M_AP_AUTO_GATE_EN	|
		BIT_CGM_24M_AP_AUTO_GATE_EN	|
		BIT_CGM_48M_AP_AUTO_GATE_EN	|
		BIT_CGM_51M2_AP_AUTO_GATE_EN	|
		BIT_CGM_64M_AP_AUTO_GATE_EN	|
		BIT_CGM_76M8_AP_AUTO_GATE_EN	|
		BIT_CGM_96M_AP_AUTO_GATE_EN	|
		BIT_CGM_128M_AP_AUTO_GATE_EN	|
		BIT_CGM_153M6_AP_AUTO_GATE_EN	|
		BIT_CGM_192M_AP_AUTO_GATE_EN	|
		BIT_CGM_256M_AP_AUTO_GATE_EN	|
		BIT_CGM_384M_AP_AUTO_GATE_EN	|
		BIT_CGM_312M_AP_AUTO_GATE_EN	|
		BIT_CGM_MPLL_AP_AUTO_GATE_EN	|
		//BIT_CGM_WPLL_AP_AUTO_GATE_EN	|
		//BIT_CGM_WIFIPLL1_AP_AUTO_GATE_EN	|
		BIT_CGM_TDPLL_AP_AUTO_GATE_EN	|
		//BIT_CGM_CPLL_AP_AUTO_GATE_EN	|
		BIT_CGM_DPLL_AP_AUTO_GATE_EN	|
		BIT_CGM_26M_AP_AUTO_GATE_EN	|
		0
		);
}
#elif defined(CONFIG_SPX30G3)
void pmu_common_config(void)
{
	unsigned int reg_val;

	CHIP_REG_SET(REG_PMU_APB_PD_CP0_ARM9_0_CFG,
        //BIT_PD_CP0_ARM9_0_FORCE_SHUTDOWN |
        BIT_PD_CP0_ARM9_0_AUTO_SHUTDOWN_EN |
        BITS_PD_CP0_ARM9_0_PWR_ON_DLY(0x08) |
        BITS_PD_CP0_ARM9_0_PWR_ON_SEQ_DLY(0x06) |
        BITS_PD_CP0_ARM9_0_ISO_ON_DLY(0x02) |
        0
	);

	CHIP_REG_AND(REG_PMU_APB_PD_CP0_ARM9_1_CFG, ~BIT_PD_CP0_ARM9_1_AUTO_SHUTDOWN_EN);
	CHIP_REG_OR(REG_PMU_APB_PD_CP0_ARM9_1_CFG, BIT_PD_CP0_ARM9_1_FORCE_SHUTDOWN);

	CHIP_REG_AND(REG_PMU_APB_PD_CP0_ARM9_2_CFG, ~BIT_PD_CP0_ARM9_2_AUTO_SHUTDOWN_EN);
	CHIP_REG_OR(REG_PMU_APB_PD_CP0_ARM9_2_CFG, BIT_PD_CP0_ARM9_2_FORCE_SHUTDOWN);

	CHIP_REG_AND(REG_PMU_APB_PD_CP0_HU3GE_CFG, ~BIT_PD_CP0_HU3GE_AUTO_SHUTDOWN_EN); //the sequence is configured by CP
	CHIP_REG_OR(REG_PMU_APB_PD_CP0_HU3GE_CFG, BIT_PD_CP0_HU3GE_FORCE_SHUTDOWN);

	CHIP_REG_AND(REG_PMU_APB_PD_CP0_TD_CFG, ~BIT_PD_CP0_TD_AUTO_SHUTDOWN_EN); //keep the default sequence as tshark2
	CHIP_REG_OR(REG_PMU_APB_PD_CP0_TD_CFG, BIT_PD_CP0_TD_FORCE_SHUTDOWN);

	CHIP_REG_SET(REG_PMU_APB_PD_CP0_GSM_CFG,
        //BIT_PD_CP0_GSM_FORCE_SHUTDOWN |
        //BIT_PD_CP0_GSM_AUTO_SHUTDOWN_EN |
        BITS_PD_CP0_GSM_PWR_ON_DLY(0x08) |
        BITS_PD_CP0_GSM_PWR_ON_SEQ_DLY(0x04) |
        BITS_PD_CP0_GSM_ISO_ON_DLY(0x04) |
        0
	);
	CHIP_REG_SET(REG_PMU_APB_PD_CP0_CEVA_CFG,
        //BIT_PD_CP0_CEVA_FORCE_SHUTDOWN |
        //BIT_PD_CP0_CEVA_AUTO_SHUTDOWN_EN |
        BITS_PD_CP0_CEVA_PWR_ON_DLY(0x08) |
        BITS_PD_CP0_CEVA_PWR_ON_SEQ_DLY(0x06) |
        BITS_PD_CP0_CEVA_ISO_ON_DLY(0x02) |
        0
	);
	CHIP_REG_SET(REG_PMU_APB_PD_CP0_SYS_CFG,
        //BIT_CP0_FORCE_DEEP_SLEEP |
        //BIT_PD_CP0_SYS_FORCE_SHUTDOWN |
        //BIT_PD_CP0_SYS_AUTO_SHUTDOWN_EN |
        BITS_PD_CP0_SYS_PWR_ON_DLY(0x08) |
        BITS_PD_CP0_SYS_PWR_ON_SEQ_DLY(0x00) |
        BITS_PD_CP0_SYS_ISO_ON_DLY(0x06) |
        0
	);

	//there's no CP2 subsystem
	CHIP_REG_OR(REG_PMU_APB_PD_CP2_SYS_CFG, BIT_PD_CP2_WIFI_FORCE_SHUTDOWN); //force shutdown,no auto shutdown bit

	CHIP_REG_AND(REG_PMU_APB_PD_CP2_ARM9_CFG, ~BIT_PD_CP2_ARM9_AUTO_SHUTDOWN_EN);
	CHIP_REG_OR(REG_PMU_APB_PD_CP2_ARM9_CFG, BIT_PD_CP2_ARM9_FORCE_SHUTDOWN);

	CHIP_REG_AND(REG_PMU_APB_PD_CP2_WIFI_CFG, ~BIT_PD_CP2_WIFI_AUTO_SHUTDOWN_EN);
	CHIP_REG_OR(REG_PMU_APB_PD_CP2_WIFI_CFG, BIT_PD_CP2_WIFI_FORCE_SHUTDOWN);

	CHIP_REG_SET(REG_PMU_APB_PD_CA7_TOP_CFG,
        //BIT_PD_CA7_TOP_FORCE_SHUTDOWN	        |
        BIT_PD_CA7_TOP_AUTO_SHUTDOWN_EN       |
        BITS_PD_CA7_TOP_PWR_ON_DLY(8)           |
        BITS_PD_CA7_TOP_PWR_ON_SEQ_DLY(2)       |
        BITS_PD_CA7_TOP_ISO_ON_DLY(4)           |
        0
	);
	CHIP_REG_SET(REG_PMU_APB_PD_CA7_C0_CFG,
        //BIT_PD_CA7_C0_FORCE_SHUTDOWN |
        BIT_PD_CA7_C0_AUTO_SHUTDOWN_EN        |
        BITS_PD_CA7_C0_PWR_ON_DLY(8)            |
        BITS_PD_CA7_C0_PWR_ON_SEQ_DLY(6)        |
        BITS_PD_CA7_C0_ISO_ON_DLY(2)            |
        0
	);

	CHIP_REG_SET(REG_PMU_APB_PD_CA7_C1_CFG,
        BIT_PD_CA7_C1_FORCE_SHUTDOWN          |
        //BIT_PD_CA7_C1_AUTO_SHUTDOWN_EN        |
        BITS_PD_CA7_C1_PWR_ON_DLY(8)            |
        BITS_PD_CA7_C1_PWR_ON_SEQ_DLY(4)        |
        BITS_PD_CA7_C1_ISO_ON_DLY(2)            |
        0
	);

	CHIP_REG_SET(REG_PMU_APB_PD_CA7_C2_CFG,
        BIT_PD_CA7_C2_FORCE_SHUTDOWN          |
        //BIT_PD_CA7_C2_AUTO_SHUTDOWN_EN        |
        BITS_PD_CA7_C2_PWR_ON_DLY(8)            |
        BITS_PD_CA7_C2_PWR_ON_SEQ_DLY(4)        |
        BITS_PD_CA7_C2_ISO_ON_DLY(2)            |
        0
	);

	CHIP_REG_SET(REG_PMU_APB_PD_CA7_C3_CFG,
        BIT_PD_CA7_C3_FORCE_SHUTDOWN          |
        //BIT_PD_CA7_C3_AUTO_SHUTDOWN_EN	|
        BITS_PD_CA7_C3_PWR_ON_DLY(8)            |
        BITS_PD_CA7_C3_PWR_ON_SEQ_DLY(4)        |
        BITS_PD_CA7_C3_ISO_ON_DLY(2)            |
        0
	);

	CHIP_REG_SET(REG_PMU_APB_PD_AP_SYS_CFG,
        //BIT_PD_AP_SYS_FORCE_SHUTDOWN          |
        BIT_PD_AP_SYS_AUTO_SHUTDOWN_EN        |
        BITS_PD_AP_SYS_PWR_ON_DLY(8)            |
        BITS_PD_AP_SYS_PWR_ON_SEQ_DLY(0)        |
        BITS_PD_AP_SYS_ISO_ON_DLY(6)            |
        0
	);

	CHIP_REG_SET(REG_PMU_APB_PD_MM_TOP_CFG,
        BIT_PD_MM_TOP_FORCE_SHUTDOWN          |
        //BIT_PD_MM_TOP_AUTO_SHUTDOWN_EN        |
        BITS_PD_MM_TOP_PWR_ON_DLY(8)            |
        BITS_PD_MM_TOP_PWR_ON_SEQ_DLY(0)        |
        BITS_PD_MM_TOP_ISO_ON_DLY(4)            |
        0
	);

	CHIP_REG_SET(REG_PMU_APB_PD_GPU_TOP_CFG,
        BIT_PD_GPU_TOP_FORCE_SHUTDOWN           |
        //BIT_PD_GPU_TOP_AUTO_SHUTDOWN_EN           |
        BITS_PD_GPU_TOP_PWR_ON_DLY(8)   |
        BITS_PD_GPU_TOP_PWR_ON_SEQ_DLY(0)       |
        BITS_PD_GPU_TOP_ISO_ON_DLY(4)           |
        0
	);

	CHIP_REG_SET(REG_PMU_APB_PD_CRYPTO_TOP_CFG,
		//BIT_PD_CRYPTO_TOP_FORCE_SHUTDOWN	|
		BIT_PD_CRYPTO_TOP_AUTO_SHUTDOWN_EN  |
		BITS_PD_CRYPTO_TOP_PWR_ON_DLY(8)		|
		BITS_PD_CRYPTO_TOP_PWR_ON_SEQ_DLY(0)	|
		BITS_PD_CRYPTO_TOP_ISO_ON_DLY(6)		|
		0
	);

	CHIP_REG_SET(REG_PMU_APB_PD_CP0_ARM_IRAM_CFG,
		//BIT_PD_CP0_ARM_IRAM_FORCE_SHUTDOWN	|
		BIT_PD_CP0_ARM_IRAM_AUTO_SHUTDOWN_EN  |
		BITS_PD_CP0_ARM_IRAM_PWR_ON_DLY(8)		|
		BITS_PD_CP0_ARM_IRAM_PWR_ON_SEQ_DLY(6)	|
		BITS_PD_CP0_ARM_IRAM_ISO_ON_DLY(2)		|
		0
	);

	CHIP_REG_SET(REG_PMU_APB_PD_CODEC_TOP_CFG,
		//BIT_PD_CODEC_TOP_FORCE_SHUTDOWN
		//BIT_PD_CODEC_TOP_AUTO_SHUTDOWN_EN	   |
		BITS_PD_CODEC_TOP_PWR_ON_DLY(8)            |
		BITS_PD_CODEC_TOP_PWR_ON_SEQ_DLY(0)        |
		BITS_PD_CODEC_TOP_ISO_ON_DLY(4)            |
		0
	);

	reg_val = CHIP_REG_GET(REG_PMU_APB_PD_PUB_SYS_CFG);
	reg_val &= ~0xffffff;
	reg_val |= (
		BITS_PD_PUB_SYS_PWR_ON_DLY(8)           |
		BITS_PD_PUB_SYS_PWR_ON_SEQ_DLY(0)       |
		BITS_PD_PUB_SYS_ISO_ON_DLY(6)           |
		0
	);
    CHIP_REG_SET(REG_PMU_APB_PD_PUB_SYS_CFG,reg_val);

	CHIP_REG_SET(REG_PMU_APB_PD_DDR_PUBL_CFG,
		BIT_PD_DDR_PUBL_AUTO_SHUTDOWN_EN	|
		BITS_PD_DDR_PUBL_PWR_ON_DLY(8)		|
		BITS_PD_DDR_PUBL_PWR_ON_SEQ_DLY(0)	|
		BITS_PD_DDR_PUBL_ISO_ON_DLY(6)		|
		0
	);

	reg_val = CHIP_REG_GET(REG_PMU_APB_PD_DDR_PHY_CFG);
	reg_val &= ~0xffffff;
	reg_val |= (
		BITS_PD_PUB_SYS_PWR_ON_DLY(8)           |
		BITS_PD_PUB_SYS_PWR_ON_SEQ_DLY(0)       |
		BITS_PD_PUB_SYS_ISO_ON_DLY(6)           |
		0
	);
    CHIP_REG_SET(REG_PMU_APB_PD_DDR_PHY_CFG,reg_val);

	CHIP_REG_SET(REG_PMU_APB_MEM_PD_CFG2,
		BITS_CP0_ARM_MEM_PD_CFG_IRAM15(1)      |
		BITS_CP0_ARM_MEM_PD_CFG_IRAM14(1)      |
		BITS_CP0_ARM_MEM_PD_CFG_IRAM13(1)      |
		BITS_CP0_ARM_MEM_PD_CFG_IRAM12(1)      |
		BITS_CP0_ARM_MEM_PD_CFG_IRAM11(1)      |
		BITS_CP0_ARM_MEM_PD_CFG_IRAM10(1)      |
		BITS_CP0_ARM_MEM_PD_CFG_IRAM9(1)       |
		BITS_CP0_ARM_MEM_PD_CFG_IRAM8(1)       |
		BITS_CP0_ARM_MEM_PD_CFG_IRAM7(1)       |
		BITS_CP0_ARM_MEM_PD_CFG_IRAM6(1)       |
		BITS_CP0_ARM_MEM_PD_CFG_IRAM5(1)       |
		BITS_CP0_ARM_MEM_PD_CFG_IRAM4(1)       |
		BITS_CP0_ARM_MEM_PD_CFG_IRAM3(1)       |
		BITS_CP0_ARM_MEM_PD_CFG_IRAM2(1)       |
		BITS_CP0_ARM_MEM_PD_CFG_IRAM1(1)       |
		BITS_CP0_ARM_MEM_PD_CFG_IRAM0(1)       |
		0
	);

	CHIP_REG_SET(REG_PMU_APB_MEM_PD_CFG3,
		BITS_CP0_DSP_MEM_PD_CFG_IRAM1(1)       |
		BITS_CP0_DSP_MEM_PD_CFG_IRAM0(1)       |
		0
	);

	CHIP_REG_SET(REG_PMU_APB_MEM_PD_FRC,
		BITS_CP0_W_MEM_PD_CFG_ICI(1)         |
		0
	);

	CHIP_REG_SET(REG_PMU_APB_XTL_WAIT_CNT,
        BITS_XTL1_WAIT_CNT(0x39)                |
        BITS_XTL0_WAIT_CNT(0x39)                |
        0
    );

    CHIP_REG_SET(REG_PMU_APB_XTLBUF_WAIT_CNT,
        BITS_XTLBUF1_WAIT_CNT(7)                |
        BITS_XTLBUF0_WAIT_CNT(7)                |
        0
    );

    CHIP_REG_SET(REG_PMU_APB_PLL_WAIT_CNT1,
        BITS_WPLL_WAIT_CNT(7)                   |
        BITS_TDPLL_WAIT_CNT(7)                  |
        BITS_DPLL_WAIT_CNT(7)                   |
        BITS_MPLL_WAIT_CNT(7)                   |
        0
    );

    CHIP_REG_SET(REG_PMU_APB_PLL_WAIT_CNT2,
        BITS_WIFIPLL2_WAIT_CNT(7)               |
        BITS_WIFIPLL1_WAIT_CNT(7)               |
        BITS_CPLL_WAIT_CNT(7)                   |
        0
    );

	CHIP_REG_SET(REG_PMU_APB_PLL_WAIT_CNT3,
		BITS_RPLL_WAIT_CNT(7)            |
		0
	);

    ANA_REG_SET(ANA_REG_GLB_SLP_WAIT_DCDCARM,
        BITS_SLP_IN_WAIT_DCDCARM(7)             |
        BITS_SLP_OUT_WAIT_DCDCARM(8)            |
        0
    );

}

static void clk_auto_gate_config(unsigned int para)
{
	return;
#if 0
	CHIP_REG_SET(REG_PMU_APB_CGM_AP_AUTO_GATE_EN,
		BIT_CGM_307M2_AP_AUTO_GATE_EN	|
		BIT_CGM_208M_AP_AUTO_GATE_EN	|
		BIT_CGM_12M_AP_AUTO_GATE_EN	|
		BIT_CGM_24M_AP_AUTO_GATE_EN	|
		BIT_CGM_48M_AP_AUTO_GATE_EN	|
		BIT_CGM_51M2_AP_AUTO_GATE_EN	|
		BIT_CGM_64M_AP_AUTO_GATE_EN	|
		BIT_CGM_76M8_AP_AUTO_GATE_EN	|
		BIT_CGM_96M_AP_AUTO_GATE_EN	|
		BIT_CGM_128M_AP_AUTO_GATE_EN	|
		BIT_CGM_153M6_AP_AUTO_GATE_EN	|
		BIT_CGM_192M_AP_AUTO_GATE_EN	|
		BIT_CGM_256M_AP_AUTO_GATE_EN	|
		BIT_CGM_384M_AP_AUTO_GATE_EN	|
		BIT_CGM_312M_AP_AUTO_GATE_EN	|
		BIT_CGM_MPLL_AP_AUTO_GATE_EN	|
		//BIT_CGM_WPLL_AP_AUTO_GATE_EN	|
		//BIT_CGM_WIFIPLL1_AP_AUTO_GATE_EN	|
		BIT_CGM_TDPLL_AP_AUTO_GATE_EN	|
		//BIT_CGM_CPLL_AP_AUTO_GATE_EN	|
		BIT_CGM_DPLL_AP_AUTO_GATE_EN	|
		BIT_CGM_26M_AP_AUTO_GATE_EN	|
		0
		);

	CHIP_REG_SET(REG_PMU_APB_CGM_AP_EN,
		BIT_CGM_307M2_AP_AUTO_GATE_EN	|
		BIT_CGM_208M_AP_AUTO_GATE_EN	|
		BIT_CGM_12M_AP_AUTO_GATE_EN	|
		BIT_CGM_24M_AP_AUTO_GATE_EN	|
		BIT_CGM_48M_AP_AUTO_GATE_EN	|
		BIT_CGM_51M2_AP_AUTO_GATE_EN	|
		BIT_CGM_64M_AP_AUTO_GATE_EN	|
		BIT_CGM_76M8_AP_AUTO_GATE_EN	|
		BIT_CGM_96M_AP_AUTO_GATE_EN	|
		BIT_CGM_128M_AP_AUTO_GATE_EN	|
		BIT_CGM_153M6_AP_AUTO_GATE_EN	|
		BIT_CGM_192M_AP_AUTO_GATE_EN	|
		BIT_CGM_256M_AP_AUTO_GATE_EN	|
		BIT_CGM_384M_AP_AUTO_GATE_EN	|
		BIT_CGM_312M_AP_AUTO_GATE_EN	|
		BIT_CGM_MPLL_AP_AUTO_GATE_EN	|
		//BIT_CGM_WPLL_AP_AUTO_GATE_EN	|
		//BIT_CGM_WIFIPLL1_AP_AUTO_GATE_EN	|
		BIT_CGM_TDPLL_AP_AUTO_GATE_EN	|
		//BIT_CGM_CPLL_AP_AUTO_GATE_EN	|
		BIT_CGM_DPLL_AP_AUTO_GATE_EN	|
		BIT_CGM_26M_AP_AUTO_GATE_EN	|
		0
		);
#endif
}
#endif

static void dcdc_optimize_config(unsigned int para)
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
                0x3008 /*wpa*/
        };
        unsigned int i;
        for(i=0;i<sizeof(dcdc_ctrl)/sizeof(dcdc_ctrl[0]);i++)
        {
                sci_adi_write(ANA_REG_GLB_DCDC_CTRL0 + (i << 2),dcdc_ctrl[i],0xffff);
        }
#endif
}

struct dcdc_sleep_reg_ctrl{
	volatile unsigned int magic_header;
	volatile unsigned int is_auto_ds;
	volatile unsigned int vddcore;
	volatile unsigned int magic_ender;
};

struct dcdc_sleep_reg_ctrl ds_cfg =
{
	0x11119999,
#if defined(CONFIG_SPX20)
	0x00000007,
	0x00000360,
#else
	0x00000007,
	0x00000000,
#endif
	0x99991111
};


static void dcdc_core_ds_config(unsigned int para)
{
#if defined(CONFIG_ADIE_SC2723S) || defined(CONFIG_ADIE_SC2723)
#if defined(CONFIG_SPX20)
	uint32 reg_val = 0;
	uint32 dcdc_core_ctl_ds = -1;
	uint32 dcdc_core_cal_ds = -1;
	uint32 dcdc_core_cal_adi = 0;
	uint32 dcdc_core_ctl_adi = -1;
	uint32 reg_val_cal = 0;
	static uint32 step_cal = 3;
	uint32 step_cal_flag = 0;
	uint32 core_adi_val = 0;
	uint32 reg_chip_id = 0;
	uint32 reg_test = 0;

	reg_chip_id =(ANA_REG_GET(ANA_REG_GLB_CHIP_ID_HIGH) & 0xFFFF) << 16;
	reg_chip_id |= (ANA_REG_GET(ANA_REG_GLB_CHIP_ID_LOW) & 0xFFFF);

	if(reg_chip_id == 0x2723a000) {
                /*disable the DCDC_CORE_SLEEP_OUT_STEP for sc2723a000 otp no value*/
		reg_val = ANA_REG_GET(ANA_REG_GLB_DCDC_CORE_ADI);
		reg_val &= 0x1F;
		core_adi_val |= reg_val << 5;
		reg_val_cal = ANA_REG_GET(ANA_REG_GLB_DCDC_CORE_ADI);
		reg_val_cal &= 0x1F << 5;
		core_adi_val |= reg_val_cal >> 5;
		ANA_REG_SET(ANA_REG_GLB_DCDC_SLP_CTRL1, core_adi_val);
		/*disable the DCDC_CORE_SLEEP_OUT_STEP for dcdc core step down */
		ANA_REG_AND(ANA_REG_GLB_DCDC_SLP_CTRL0,~(0x3));
	} else {
                if(0x00000007 == ds_cfg.is_auto_ds) {
			ANA_REG_SET(ANA_REG_GLB_DCDC_SLP_CTRL1, ds_cfg.vddcore);
                        /*enable the DCDC_CORE_SLEEP_OUT_STEP for dcdc core step down */
			ANA_REG_AND(ANA_REG_GLB_DCDC_SLP_CTRL0, ~(0x3));
			reg_test = ANA_REG_GET(ANA_REG_GLB_DCDC_SLP_CTRL1);

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
#else
	uint32 dcdc_core_ctl_adi = 0;
	uint32 reg_val = 0;
	reg_val = ANA_REG_GET(ANA_REG_GLB_DCDC_SLP_CTRL0);
	reg_val |= 0x1 << 1;
	ANA_REG_OR(ANA_REG_GLB_DCDC_SLP_CTRL0, reg_val);
#endif
#else
	uint32 dcdc_core_ctl_adi = 0;
	uint32 reg_val = 0;
	uint32 dcdc_core_ctl_ds = -1;

	static struct dcdc_core_ds_step_info step_info[5]={
		{ANA_REG_GLB_MP_PWR_CTRL1, 0,ANA_REG_GLB_MP_PWR_CTRL2, 0},
		{ANA_REG_GLB_MP_PWR_CTRL1, 3,ANA_REG_GLB_MP_PWR_CTRL2, 5},
		{ANA_REG_GLB_MP_PWR_CTRL1, 6,ANA_REG_GLB_MP_PWR_CTRL2,10},
		{ANA_REG_GLB_MP_PWR_CTRL1, 9,ANA_REG_GLB_MP_PWR_CTRL3, 0},
		{ANA_REG_GLB_MP_PWR_CTRL1,12,ANA_REG_GLB_MP_PWR_CTRL3, 5}
	};
	static char dcdc_core_down_volt[]={4,1,1,2,3,5,0,6};
	static char dcdc_core_up_volt[]={6,2,3,4,0,1,7,7};
	uint32 dcdc_core_cal_adi,i;
	/*1100,700,800,900,1000,650,1200,1300*/
	static uint32 step_ratio[]={10,10,6,3,3};
	dcdc_core_ctl_adi = (sci_adi_read(ANA_REG_GLB_MP_MISC_CTRL) >> 3) & 0x7;
	dcdc_core_ctl_ds  = dcdc_core_down_volt[dcdc_core_ctl_adi];
	dcdc_core_ctl_ds  = dcdc_core_ctl_adi;

	reg_val = sci_adi_read(ANA_REG_GLB_DCDC_SLP_CTRL);
	reg_val &= ~0x7;
	reg_val |= dcdc_core_ctl_ds;
	sci_adi_write(ANA_REG_GLB_DCDC_SLP_CTRL, reg_val, 0xffff);

	dcdc_core_cal_adi = sci_adi_read(ANA_REG_GLB_DCDC_CORE_ADI) & 0x1F;
	if(dcdc_core_ctl_ds < dcdc_core_ctl_adi){
		/*last step must equel function mode */
		sci_adi_write(step_info[4].ctl_reg,dcdc_core_ctl_adi<<step_info[4].ctl_sht,0x07 << step_info[4].ctl_sht);
		sci_adi_write(step_info[4].cal_reg,dcdc_core_cal_adi<<step_info[4].cal_sht,0x1F << step_info[4].cal_sht);

		for(i=0;i<4;i++) {
			reg_val = dcdc_core_cal_adi + step_ratio[i];
			if(reg_val <= 0x1F) {
				sci_adi_write(step_info[i].ctl_reg,dcdc_core_ctl_ds<<step_info[i].ctl_sht,0x07<<step_info[i].ctl_sht);
				sci_adi_write(step_info[i].cal_reg,reg_val<<step_info[i].cal_sht,0x1F << step_info[i].cal_sht);
				dcdc_core_cal_adi = reg_val;
			} else {
				sci_adi_write(step_info[i].ctl_reg,dcdc_core_up_volt[dcdc_core_ctl_ds]<<step_info[i].ctl_sht,
												0x07 << step_info[i].ctl_sht);
				sci_adi_write(step_info[i].cal_reg,(reg_val-0x1F)<<step_info[i].cal_sht,0x1F << step_info[i].cal_sht);
				dcdc_core_ctl_ds = dcdc_core_up_volt[dcdc_core_ctl_ds];
				dcdc_core_cal_adi = reg_val - 0x1F;
			}
		}
	} else {
		for(i=0;i<5;i++) {
			/*every step should equal function mode*/
			sci_adi_write(step_info[i].ctl_reg,dcdc_core_ctl_adi<<step_info[i].ctl_sht,0x07 << step_info[i].ctl_sht);
			sci_adi_write(step_info[i].cal_reg,dcdc_core_cal_adi<<step_info[i].cal_sht,0x1F << step_info[i].cal_sht);
		}
	}
#endif
}

struct ddr_phy_lp_reg_ctrl{
	volatile unsigned int magic_header;
	volatile unsigned int is_auto_pd;
	volatile unsigned int magic_ender;
	volatile unsigned int reg[3];
};
struct ddr_phy_lp_reg_ctrl lp_cfg=
{
	0x88889999,
#if defined(CONFIG_SP8730SEA)
	0x00000000,
#else
#ifdef CONFIG_WITH_VMM
	0x00000000,
#else
	0x00000007,
#endif
#endif
	0x99998888,
	0x402b006c,
	0x402b0140,
	0x402b0144
};
static unsigned int dmc_phy_is_auto_pd(unsigned int is_auto_pd)
{
	unsigned int i;
	for(i=0;i<3;i++)
	{
		*(volatile unsigned int*)lp_cfg.reg[i] &= ~(0x1 << 25);
		if(is_auto_pd & (0x1 << i)){
			*(volatile unsigned int*)(lp_cfg.reg[i]) |= (0x1 << 24);
		} else {
			*(volatile unsigned int*)(lp_cfg.reg[i]) &= ~(0x1 << 24);
		}
	}
	return is_auto_pd;
}

static void setup_ap_cp_sync_sleep_code(unsigned int start_addr)
{
	#ifndef CONFIG_SPX20
	unsigned int *sa = (unsigned int*)start_addr;

	dmc_phy_is_auto_pd(lp_cfg.is_auto_pd);
	#endif
	/*enable cp2 can access 0x50001800*/
	*(volatile unsigned int*)0x402e3038 &= ~(0x7 << 7);
	/*enable cp0 can access 0x50001800*/
	*(volatile unsigned int*)0x402e3028 &= ~(0x7 << 7);
	#ifndef CONFIG_SPX20
	/*set for indcate phy is alive*/
	*(volatile unsigned int*)0x30040000 |= (0x1 << 0);
	*(volatile unsigned int*)0x30010184 |= (0x1 << 0);
	/*close umctl and phy and publ auto retention*/
	*(volatile unsigned int*)0x402b012c &= ~((0x3 << 27)|(0x1 << 25));
	#endif
}

#ifdef CONFIG_SRAM_REPAIR
static void sram_repair(unsigned int para)
{
#ifdef CONFIG_SP7731C //sram repair for sp7731c(pike)
	CHIP_REG_AND(REG_PMU_APB_BISR_CFG, ~(BIT_PD_MM_TOP_BISR_FORCE_BYP |\
				BIT_PD_CP0_HU3GE_BISR_FORCE_BYP |\
				BIT_PD_CP0_CEVA_BISR_FORCE_BYP));
	CHIP_REG_AND(REG_PMU_APB_BISR_CFG2, ~BIT_PD_CP0_HARQ_BISR_FORCE_BYP);
#endif
}
#endif

void CSP_Init(unsigned int gen_para)
{
	unsigned int reg_val;
#ifdef CONFIG_SRAM_REPAIR
	sram_repair(0x0);
#endif
	calibrate_register_callback((void*)dcdc_core_ds_config);
	setup_ap_cp_sync_sleep_code(0x50001800);
	/*open adi clock auto gate for power consume*/
	reg_val = readl(ADI_GSSI_CTL0);
	reg_val &= ~(0x1 << 30);
	writel(reg_val,ADI_GSSI_CTL0);
	/*disable int ana dcd otp interrupt*/
	ANA_REG_AND(REG_ANA_INTC_INT_EN,~BIT_DCDCOTP_INT_EN);
	#if defined(CONFIG_SPX20) || defined(CONFIG_SPX30G3)
	pmu_common_config();
	clk_auto_gate_config(0x00000000);
	#else
	pmu_cp_modem_config(MODEM_CON);
	#endif
	dcdc_optimize_config(0x00000000);
	reg_val = readl(REG_AON_APB_BB_BG_CTRL);
	reg_val &= ~BITS_BB_LDO_V(0xF);
	reg_val |= BITS_BB_LDO_V(0x4);
	writel(reg_val,REG_AON_APB_BB_BG_CTRL);
}

/**---------------------------------------------------------------------------*
 **                         Compiler Flag                                     *
 **---------------------------------------------------------------------------*/
#ifdef __cplusplus
}
#endif
