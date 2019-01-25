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
#define DDR_PHY_IND_ADDR  0x30010184
#define DDR_PHY_IND_ADDR1 0x30040000
#define DDR_PHY_IND_ADDR2 0x402e3048
#define DDR_PHY_IND_PHY_ALIVE 0x00000001
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

	start_watchdog(5);
    
    while (i--);    

    wdt_reset_MCU();
    
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
#if !defined(CONFIG_ARCH_SCX35L)
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
#endif
    return 0;
}

void pmu_commom_config(void)
{
#if defined(CONFIG_ARCH_SCX35L)
#if defined(CONFIG_ARCH_SCX20L)
	//reserved register,need not config
        //CHIP_REG_SET(REG_PMU_APB_PD_CP0_SYS_CFG,
	//	0
	//);
        //removed ARM9_0 sys
	CHIP_REG_SET(REG_PMU_APB_PD_CP0_ARM9_0_CFG,
		BIT_PD_CP0_ARM9_0_FORCE_SHUTDOWN |
		//BIT_PD_CP0_ARM9_0_AUTO_SHUTDOWN_EN |
		BITS_PD_CP0_ARM9_0_PWR_ON_DLY(0x08) |
		BITS_PD_CP0_ARM9_0_PWR_ON_SEQ_DLY(0x06) |
		BITS_PD_CP0_ARM9_0_ISO_ON_DLY(0x02) |
		0
	);
	//removed ARM9_1 sys
	CHIP_REG_SET(REG_PMU_APB_PD_CP0_ARM9_1_CFG,
		BIT_PD_CP0_ARM9_1_FORCE_SHUTDOWN |
		//BIT_PD_CP0_ARM9_1_AUTO_SHUTDOWN_EN |
		BITS_PD_CP0_ARM9_1_PWR_ON_DLY(0x08) |
		BITS_PD_CP0_ARM9_1_PWR_ON_SEQ_DLY(0x06) |
		BITS_PD_CP0_ARM9_1_ISO_ON_DLY(0x02) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PD_CP0_GSM_0_CFG,
		//BIT_PD_CP0_GSM_0_FORCE_SHUTDOWN |
		BIT_PD_CP0_GSM_0_AUTO_SHUTDOWN_EN |
		BITS_PD_CP0_GSM_0_PWR_ON_DLY(0x08) |
		BITS_PD_CP0_GSM_0_PWR_ON_SEQ_DLY(0x06) |
		BITS_PD_CP0_GSM_0_ISO_ON_DLY(0x02) |
		0
	);
        //removed GSM_1 sys
	CHIP_REG_SET(REG_PMU_APB_PD_CP0_GSM_1_CFG,
		BIT_PD_CP0_GSM_1_FORCE_SHUTDOWN |
		//BIT_PD_CP0_GSM_1_AUTO_SHUTDOWN_EN |
		BITS_PD_CP0_GSM_1_PWR_ON_DLY(0x08) |
		BITS_PD_CP0_GSM_1_PWR_ON_SEQ_DLY(0x04) |
		BITS_PD_CP0_GSM_1_ISO_ON_DLY(0x04) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PD_CP0_CEVA_0_CFG,
		//BIT_PD_CP0_CEVA_0_FORCE_SHUTDOWN |
		BIT_PD_CP0_CEVA_0_AUTO_SHUTDOWN_EN |
		BITS_PD_CP0_CEVA_0_PWR_ON_DLY(0x08) |
		BITS_PD_CP0_CEVA_0_PWR_ON_SEQ_DLY(0x06) |
		BITS_PD_CP0_CEVA_0_ISO_ON_DLY(0x02) |
		0
	);
        //removed CEVA_1 sys
	CHIP_REG_SET(REG_PMU_APB_PD_CP0_CEVA_1_CFG,
		BIT_PD_CP0_CEVA_1_FORCE_SHUTDOWN |
		//BIT_PD_CP0_CEVA_1_AUTO_SHUTDOWN_EN |
		BITS_PD_CP0_CEVA_1_PWR_ON_DLY(0x08) |
		BITS_PD_CP0_CEVA_1_PWR_ON_SEQ_DLY(0x06) |
		BITS_PD_CP0_CEVA_1_ISO_ON_DLY(0x02) |
		0
	);
        //remove HU3GE sys
	CHIP_REG_SET(REG_PMU_APB_PD_CP0_HU3GE_CFG,
		BIT_PD_CP0_HU3GE_FORCE_SHUTDOWN |
		//BIT_PD_CP0_HU3GE_AUTO_SHUTDOWN_EN |
		BITS_PD_CP0_HU3GE_PWR_ON_DLY(0x08) |
		BITS_PD_CP0_HU3GE_PWR_ON_SEQ_DLY(0x02) |
		BITS_PD_CP0_HU3GE_ISO_ON_DLY(0x04) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PD_CP0_TD_CFG,
		//BIT_PD_CP0_TD_FORCE_SHUTDOWN |
		BIT_PD_CP0_TD_AUTO_SHUTDOWN_EN |
		BITS_PD_CP0_TD_PWR_ON_DLY(0x08) |
		BITS_PD_CP0_TD_PWR_ON_SEQ_DLY(0x02) |
		BITS_PD_CP0_TD_ISO_ON_DLY(0x04) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PD_CA7_C0_CFG,
		//BIT_PD_CA7_C0_WFI_SHUTDOWN_EN |
		//BIT_PD_CA7_C0_DBG_SHUTDOWN_EN |
		//BIT_PD_CA7_C0_FORCE_SHUTDOWN |
		BIT_PD_CA7_C0_AUTO_SHUTDOWN_EN |
		BITS_PD_CA7_C0_PWR_ON_DLY(0x08) |
		BITS_PD_CA7_C0_PWR_ON_SEQ_DLY(0x06) |
		BITS_PD_CA7_C0_ISO_ON_DLY(0x02) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PD_CA7_C1_CFG,
		//BIT_PD_CA7_C1_WFI_SHUTDOWN_EN |
		//BIT_PD_CA7_C1_DBG_SHUTDOWN_EN |
		BIT_PD_CA7_C1_FORCE_SHUTDOWN |
		//BIT_PD_CA7_C1_AUTO_SHUTDOWN_EN |
		BITS_PD_CA7_C1_PWR_ON_DLY(0x08) |
		BITS_PD_CA7_C1_PWR_ON_SEQ_DLY(0x04) |
		BITS_PD_CA7_C1_ISO_ON_DLY(0x02) |
		0
	);
        //removed CA7_C2
	CHIP_REG_SET(REG_PMU_APB_PD_CA7_C2_CFG,
		//BIT_PD_CA7_C2_WFI_SHUTDOWN_EN |
		//BIT_PD_CA7_C2_DBG_SHUTDOWN_EN |
		BIT_PD_CA7_C2_FORCE_SHUTDOWN |
		//BIT_PD_CA7_C2_AUTO_SHUTDOWN_EN |
		BITS_PD_CA7_C2_PWR_ON_DLY(0x08) |
		BITS_PD_CA7_C2_PWR_ON_SEQ_DLY(0x04) |
		BITS_PD_CA7_C2_ISO_ON_DLY(0x02) |
		0
	);
        //removed CA7_C2
	CHIP_REG_SET(REG_PMU_APB_PD_CA7_C3_CFG,
		//BIT_PD_CA7_C3_WFI_SHUTDOWN_EN |
		//BIT_PD_CA7_C3_DBG_SHUTDOWN_EN |
		BIT_PD_CA7_C3_FORCE_SHUTDOWN |
		//BIT_PD_CA7_C3_AUTO_SHUTDOWN_EN |
		BITS_PD_CA7_C3_PWR_ON_DLY(0x08) |
		BITS_PD_CA7_C3_PWR_ON_SEQ_DLY(0x04) |
		BITS_PD_CA7_C3_ISO_ON_DLY(0x02) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PD_AP_SYS_CFG,
		//BIT_PD_AP_SYS_FORCE_SHUTDOWN |
		BIT_PD_AP_SYS_AUTO_SHUTDOWN_EN |
		BITS_PD_AP_SYS_PWR_ON_DLY(0x08) |
		BITS_PD_AP_SYS_PWR_ON_SEQ_DLY(0x00) |
		BITS_PD_AP_SYS_ISO_ON_DLY(0x06) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PD_GPU_TOP_CFG ,
		BIT_PD_GPU_TOP_FORCE_SHUTDOWN |
		//BIT_PD_GPU_TOP_AUTO_SHUTDOWN_EN |
		BITS_PD_GPU_TOP_PWR_ON_DLY(0x08) |
		BITS_PD_GPU_TOP_PWR_ON_SEQ_DLY(0x00) |
		BITS_PD_GPU_TOP_ISO_ON_DLY(0x04) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PD_MM_TOP_CFG,
		BIT_PD_MM_TOP_FORCE_SHUTDOWN |
		//BIT_PD_MM_TOP_AUTO_SHUTDOWN_EN |
		BITS_PD_MM_TOP_PWR_ON_DLY(0x08) |
		BITS_PD_MM_TOP_PWR_ON_SEQ_DLY(0x00) |
		BITS_PD_MM_TOP_ISO_ON_DLY(0x04) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PD_CP1_CA5_CFG,
		//BIT_PD_CP1_CA5_FORCE_SHUTDOWN |
		BIT_PD_CP1_CA5_AUTO_SHUTDOWN_EN |
		BITS_PD_CP1_CA5_PWR_ON_DLY(0x08) |
		BITS_PD_CP1_CA5_PWR_ON_SEQ_DLY(0x02) |
		BITS_PD_CP1_CA5_ISO_ON_DLY(0x04) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PD_CP1_LTE_P1_CFG,
		//BIT_PD_CP1_LTE_P1_FORCE_SHUTDOWN |
		BIT_PD_CP1_LTE_P1_AUTO_SHUTDOWN_EN |
		BITS_PD_CP1_LTE_P1_PWR_ON_DLY(0x08) |
		BITS_PD_CP1_LTE_P1_PWR_ON_SEQ_DLY(0x02) |
		BITS_PD_CP1_LTE_P1_ISO_ON_DLY(0x04) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PD_CP1_LTE_P2_CFG,
		//BIT_PD_CP1_LTE_P2_FORCE_SHUTDOWN |
		BIT_PD_CP1_LTE_P2_AUTO_SHUTDOWN_EN |
		BITS_PD_CP1_LTE_P2_PWR_ON_DLY(0x08) |
		BITS_PD_CP1_LTE_P2_PWR_ON_SEQ_DLY(0x02) |
		BITS_PD_CP1_LTE_P2_ISO_ON_DLY(0x04) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PD_CP1_CEVA_CFG,
		//BIT_PD_CP1_CEVA_FORCE_SHUTDOWN |
		BIT_PD_CP1_CEVA_AUTO_SHUTDOWN_EN |
		BITS_PD_CP1_CEVA_PWR_ON_DLY(0x08) |
		BITS_PD_CP1_CEVA_PWR_ON_SEQ_DLY(0x02) |
		BITS_PD_CP1_CEVA_ISO_ON_DLY(0x04) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PD_CP1_COMWRAP_CFG,
		//BIT_PD_CP1_COMWRAP_FORCE_SHUTDOWN |
		BIT_PD_CP1_COMWRAP_AUTO_SHUTDOWN_EN |
		BITS_PD_CP1_COMWRAP_PWR_ON_DLY(0x08) |
		BITS_PD_CP1_COMWRAP_PWR_ON_SEQ_DLY(0x02) |
		BITS_PD_CP1_COMWRAP_ISO_ON_DLY(0x04) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PD_PUB_SYS_CFG,
		//BIT_PD_PUB_SYS_FORCE_SHUTDOWN |
		BIT_PD_PUB_SYS_AUTO_SHUTDOWN_EN |
		BITS_PD_PUB_SYS_PWR_ON_DLY(0x08) |
		BITS_PD_PUB_SYS_PWR_ON_SEQ_DLY(0x00) |
		BITS_PD_PUB_SYS_ISO_ON_DLY(0x06) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PD_CA7_TOP_CFG,
		//BIT_PD_CA7_TOP_DBG_SHUTDOWN_EN |
		//BIT_PD_CA7_TOP_FORCE_SHUTDOWN |
		BIT_PD_CA7_TOP_AUTO_SHUTDOWN_EN |
		BITS_PD_CA7_TOP_PWR_ON_DLY(0x08) |
		BITS_PD_CA7_TOP_PWR_ON_SEQ_DLY(0x02) |
		BITS_PD_CA7_TOP_ISO_ON_DLY(0x04) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_XTL_WAIT_CNT,
		BITS_XTL1_WAIT_CNT(0x39) |
		BITS_XTL0_WAIT_CNT(0x39) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_XTLBUF_WAIT_CNT,
		BITS_XTLBUF1_WAIT_CNT(7) |
		BITS_XTLBUF0_WAIT_CNT(7) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PLL_WAIT_CNT1,
		BITS_LTEPLL_WAIT_CNT(7) |
		BITS_TWPLL_WAIT_CNT(7) |
		BITS_DPLL_WAIT_CNT(7) |
		BITS_MPLL_WAIT_CNT(7) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PLL_WAIT_CNT2,
		BITS_LVDSDIS_PLL_WAIT_CNT(7) |
		0
	);

	CHIP_REG_SET(REG_PMU_APB_PWR_CNT_WAIT_CFG0,
		BITS_VCP0_PWR_WAIT_CNT(0x17) |
		BITS_CP1_PWR_WAIT_CNT(0x17) |
		BITS_CP0_PWR_WAIT_CNT(0x17) |
		BITS_AP_PWR_WAIT_CNT(0x17) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PWR_CNT_WAIT_CFG1,
		BITS_ARM7_PWR_WAIT_CNT(0x17) |
		BITS_VCP1_PWR_WAIT_CNT(0x17) |
		0
	);
#else
	CHIP_REG_SET(REG_PMU_APB_PD_CP0_SYS_CFG,
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
		//BIT_PD_CP0_ARM9_1_FORCE_SHUTDOWN |
		BIT_PD_CP0_ARM9_1_AUTO_SHUTDOWN_EN |
		BITS_PD_CP0_ARM9_1_PWR_ON_DLY(0x08) |
		BITS_PD_CP0_ARM9_1_PWR_ON_SEQ_DLY(0x06) |
		BITS_PD_CP0_ARM9_1_ISO_ON_DLY(0x02) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PD_CP0_GSM_0_CFG,
		//BIT_PD_CP0_GSM_0_FORCE_SHUTDOWN |
		BIT_PD_CP0_GSM_0_AUTO_SHUTDOWN_EN |
		BITS_PD_CP0_GSM_0_PWR_ON_DLY(0x08) |
		BITS_PD_CP0_GSM_0_PWR_ON_SEQ_DLY(0x06) |
		BITS_PD_CP0_GSM_0_ISO_ON_DLY(0x02) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PD_CP0_GSM_1_CFG,
		//BIT_PD_CP0_GSM_1_FORCE_SHUTDOWN |
		BIT_PD_CP0_GSM_1_AUTO_SHUTDOWN_EN |
		BITS_PD_CP0_GSM_1_PWR_ON_DLY(0x08) |
		BITS_PD_CP0_GSM_1_PWR_ON_SEQ_DLY(0x04) |
		BITS_PD_CP0_GSM_1_ISO_ON_DLY(0x04) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PD_CP0_CEVA_0_CFG,
		//BIT_PD_CP0_CEVA_0_FORCE_SHUTDOWN |
		BIT_PD_CP0_CEVA_0_AUTO_SHUTDOWN_EN |
		BITS_PD_CP0_CEVA_0_PWR_ON_DLY(0x08) |
		BITS_PD_CP0_CEVA_0_PWR_ON_SEQ_DLY(0x06) |
		BITS_PD_CP0_CEVA_0_ISO_ON_DLY(0x02) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PD_CP0_CEVA_1_CFG,
		//BIT_PD_CP0_CEVA_1_FORCE_SHUTDOWN |
		BIT_PD_CP0_CEVA_1_AUTO_SHUTDOWN_EN |
		BITS_PD_CP0_CEVA_1_PWR_ON_DLY(0x08) |
		BITS_PD_CP0_CEVA_1_PWR_ON_SEQ_DLY(0x06) |
		BITS_PD_CP0_CEVA_1_ISO_ON_DLY(0x02) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PD_CP0_HU3GE_CFG,
		//BIT_PD_CP0_HU3GE_FORCE_SHUTDOWN |
		BIT_PD_CP0_HU3GE_AUTO_SHUTDOWN_EN |
		BITS_PD_CP0_HU3GE_PWR_ON_DLY(0x08) |
		BITS_PD_CP0_HU3GE_PWR_ON_SEQ_DLY(0x02) |
		BITS_PD_CP0_HU3GE_ISO_ON_DLY(0x04) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PD_CP0_TD_CFG,
		//BIT_PD_CP0_TD_FORCE_SHUTDOWN |
		BIT_PD_CP0_TD_AUTO_SHUTDOWN_EN |
		BITS_PD_CP0_TD_PWR_ON_DLY(0x08) |
		BITS_PD_CP0_TD_PWR_ON_SEQ_DLY(0x02) |
		BITS_PD_CP0_TD_ISO_ON_DLY(0x04) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PD_CA7_C0_CFG,
		//BIT_PD_CA7_C0_WFI_SHUTDOWN_EN |
		//BIT_PD_CA7_C0_DBG_SHUTDOWN_EN |
		//BIT_PD_CA7_C0_FORCE_SHUTDOWN |
		BIT_PD_CA7_C0_AUTO_SHUTDOWN_EN |
		BITS_PD_CA7_C0_PWR_ON_DLY(0x08) |
		BITS_PD_CA7_C0_PWR_ON_SEQ_DLY(0x06) |
		BITS_PD_CA7_C0_ISO_ON_DLY(0x02) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PD_CA7_C1_CFG,
		//BIT_PD_CA7_C1_WFI_SHUTDOWN_EN |
		//BIT_PD_CA7_C1_DBG_SHUTDOWN_EN |
		BIT_PD_CA7_C1_FORCE_SHUTDOWN |
		//BIT_PD_CA7_C1_AUTO_SHUTDOWN_EN |
		BITS_PD_CA7_C1_PWR_ON_DLY(0x08) |
		BITS_PD_CA7_C1_PWR_ON_SEQ_DLY(0x04) |
		BITS_PD_CA7_C1_ISO_ON_DLY(0x02) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PD_CA7_C2_CFG,
		//BIT_PD_CA7_C2_WFI_SHUTDOWN_EN |
		//BIT_PD_CA7_C2_DBG_SHUTDOWN_EN |
		BIT_PD_CA7_C2_FORCE_SHUTDOWN |
		//BIT_PD_CA7_C2_AUTO_SHUTDOWN_EN |
		BITS_PD_CA7_C2_PWR_ON_DLY(0x08) |
		BITS_PD_CA7_C2_PWR_ON_SEQ_DLY(0x04) |
		BITS_PD_CA7_C2_ISO_ON_DLY(0x02) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PD_CA7_C3_CFG,
		//BIT_PD_CA7_C3_WFI_SHUTDOWN_EN |
		//BIT_PD_CA7_C3_DBG_SHUTDOWN_EN |
		BIT_PD_CA7_C3_FORCE_SHUTDOWN |
		//BIT_PD_CA7_C3_AUTO_SHUTDOWN_EN |
		BITS_PD_CA7_C3_PWR_ON_DLY(0x08) |
		BITS_PD_CA7_C3_PWR_ON_SEQ_DLY(0x04) |
		BITS_PD_CA7_C3_ISO_ON_DLY(0x02) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PD_AP_SYS_CFG,
		//BIT_PD_AP_SYS_FORCE_SHUTDOWN |
		BIT_PD_AP_SYS_AUTO_SHUTDOWN_EN |
		BITS_PD_AP_SYS_PWR_ON_DLY(0x08) |
		BITS_PD_AP_SYS_PWR_ON_SEQ_DLY(0x00) |
		BITS_PD_AP_SYS_ISO_ON_DLY(0x06) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PD_GPU_TOP_CFG ,
		//BIT_PD_GPU_TOP_FORCE_SHUTDOWN |
		//BIT_PD_GPU_TOP_AUTO_SHUTDOWN_EN |
		BITS_PD_GPU_TOP_PWR_ON_DLY(0x08) |
		BITS_PD_GPU_TOP_PWR_ON_SEQ_DLY(0x00) |
		BITS_PD_GPU_TOP_ISO_ON_DLY(0x04) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PD_MM_TOP_CFG,
		//BIT_PD_MM_TOP_FORCE_SHUTDOWN |
		//BIT_PD_MM_TOP_AUTO_SHUTDOWN_EN |
		BITS_PD_MM_TOP_PWR_ON_DLY(0x08) |
		BITS_PD_MM_TOP_PWR_ON_SEQ_DLY(0x00) |
		BITS_PD_MM_TOP_ISO_ON_DLY(0x04) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PD_CP1_CA5_CFG,
		//BIT_PD_CP1_CA5_FORCE_SHUTDOWN |
		BIT_PD_CP1_CA5_AUTO_SHUTDOWN_EN |
		BITS_PD_CP1_CA5_PWR_ON_DLY(0x08) |
		BITS_PD_CP1_CA5_PWR_ON_SEQ_DLY(0x02) |
		BITS_PD_CP1_CA5_ISO_ON_DLY(0x04) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PD_CP1_LTE_P1_CFG,
		//BIT_PD_CP1_LTE_P1_FORCE_SHUTDOWN |
		BIT_PD_CP1_LTE_P1_AUTO_SHUTDOWN_EN |
		BITS_PD_CP1_LTE_P1_PWR_ON_DLY(0x08) |
		BITS_PD_CP1_LTE_P1_PWR_ON_SEQ_DLY(0x02) |
		BITS_PD_CP1_LTE_P1_ISO_ON_DLY(0x04) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PD_CP1_LTE_P2_CFG,
		//BIT_PD_CP1_LTE_P2_FORCE_SHUTDOWN |
		BIT_PD_CP1_LTE_P2_AUTO_SHUTDOWN_EN |
		BITS_PD_CP1_LTE_P2_PWR_ON_DLY(0x08) |
		BITS_PD_CP1_LTE_P2_PWR_ON_SEQ_DLY(0x02) |
		BITS_PD_CP1_LTE_P2_ISO_ON_DLY(0x04) |
		0
	);

	CHIP_REG_SET(REG_PMU_APB_PD_CP1_CEVA_CFG,
		//BIT_PD_CP1_CEVA_FORCE_SHUTDOWN |
		BIT_PD_CP1_CEVA_AUTO_SHUTDOWN_EN |
		BITS_PD_CP1_CEVA_PWR_ON_DLY(0x08) |
		BITS_PD_CP1_CEVA_PWR_ON_SEQ_DLY(0x02) |
		BITS_PD_CP1_CEVA_ISO_ON_DLY(0x04) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PD_CP1_COMWRAP_CFG,
		//BIT_PD_CP1_COMWRAP_FORCE_SHUTDOWN |
		BIT_PD_CP1_COMWRAP_AUTO_SHUTDOWN_EN |
		BITS_PD_CP1_COMWRAP_PWR_ON_DLY(0x08) |
		BITS_PD_CP1_COMWRAP_PWR_ON_SEQ_DLY(0x02) |
		BITS_PD_CP1_COMWRAP_ISO_ON_DLY(0x04) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PD_PUB_SYS_CFG,
		//BIT_PD_PUB_SYS_FORCE_SHUTDOWN |
		BIT_PD_PUB_SYS_AUTO_SHUTDOWN_EN |
		BITS_PD_PUB_SYS_PWR_ON_DLY(0x08) |
		BITS_PD_PUB_SYS_PWR_ON_SEQ_DLY(0x00) |
		BITS_PD_PUB_SYS_ISO_ON_DLY(0x06) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PD_CA7_TOP_CFG,
		//BIT_PD_CA7_TOP_DBG_SHUTDOWN_EN |
		//BIT_PD_CA7_TOP_FORCE_SHUTDOWN |
		BIT_PD_CA7_TOP_AUTO_SHUTDOWN_EN |
		BITS_PD_CA7_TOP_PWR_ON_DLY(0x08) |
		BITS_PD_CA7_TOP_PWR_ON_SEQ_DLY(0x02) |
		BITS_PD_CA7_TOP_ISO_ON_DLY(0x04) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_XTL_WAIT_CNT,
		BITS_XTL1_WAIT_CNT(0x39) |
		BITS_XTL0_WAIT_CNT(0x39) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_XTLBUF_WAIT_CNT,
		BITS_XTLBUF1_WAIT_CNT(7) |
		BITS_XTLBUF0_WAIT_CNT(7) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PLL_WAIT_CNT1,
		BITS_LTEPLL_WAIT_CNT(7) |
		BITS_TWPLL_WAIT_CNT(7) |
		BITS_DPLL_WAIT_CNT(7) |
		BITS_MPLL_WAIT_CNT(7) |
		0
	);
	CHIP_REG_SET(REG_PMU_APB_PLL_WAIT_CNT2,
		BITS_LVDSDIS_PLL_WAIT_CNT(7) |
		0
	);

	CHIP_REG_SET(REG_PMU_APB_PWR_CNT_WAIT_CFG0,
		BITS_VCP0_PWR_WAIT_CNT(8) |
		BITS_CP1_PWR_WAIT_CNT(8) |
		BITS_CP0_PWR_WAIT_CNT(8) |
		BITS_AP_PWR_WAIT_CNT(0x17) |
		0
	);

	CHIP_REG_SET(REG_PMU_APB_PWR_CNT_WAIT_CFG1,
		BITS_ARM7_PWR_WAIT_CNT(8) |
		BITS_VCP1_PWR_WAIT_CNT(8) |
		0
	);
#endif
#endif
}


static void setup_autopd_mode(void)
{
	/*enable the emc auto gate en*/
	CHIP_REG_SET(REG_AON_APB_EMC_AUTO_GATE_EN,
		BIT_CP1_PUB_AUTO_GATE_EN |
		BIT_CP0_PUB_AUTO_GATE_EN |
		BIT_AP_PUB_AUTO_GATE_EN  |
		BIT_AON_APB_PUB_AUTO_GATE_EN |
		BIT_CP1_EMC_AUTO_GATE_EN |
		BIT_CP0_EMC_AUTO_GATE_EN |
		//BIT_AP_EMC_AUTO_GATE_EN |
		BIT_CA7_EMC_AUTO_GATE_EN |
		0
	);
}

#if defined(CONFIG_SP9830I)
/*
 * switch dcdc rf pwm mode
 * */
static int dcdc_rf_pwm_mode()
{
	ANA_REG_OR(ANA_REG_GLB_DCDC_CTRL5,BIT_DCDC_RF_PFM);
	ANA_REG_BIC(ANA_REG_GLB_DCDC_CTRL5,BIT_DCDC_RF_DCM);
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
		//0x3008 /*wpa*/
	};
	unsigned int i;
	for(i=0;i<sizeof(dcdc_ctrl)/sizeof(dcdc_ctrl[0]);i++)
	{
		sci_adi_write(ANA_REG_GLB_DCDC_CTRL0 + (i << 2),dcdc_ctrl[i],0xffff);
	}
#if defined(CONFIG_SP9830I)
	dcdc_rf_pwm_mode();
#endif

#endif
}

struct dcdc_sleep_reg_ctrl{
	volatile unsigned int magic_header;
	volatile unsigned int is_auto_ds;
	volatile unsigned int magic_ender;
};
struct dcdc_sleep_reg_ctrl ds_cfg=
{
	0x11119999,
	0x00000007,
	0x99991111
};

struct dcdc_core_volt_reg {
	unsigned int dcdc_core_ctl_bits;
	unsigned int dcdc_core_cali_bits;
};

#define SHARKLC_CHIP_ID          (0x96301000)
#define SHARKLC_CHIP_ID_MASK     (0xFFF)
#define BLK_INDEX 15
#define BIT_INDEX 7
static void dcdc_core_ds_config(unsigned int para)
{
#if defined(CONFIG_ADIE_SC2723S) || defined(CONFIG_ADIE_SC2723)
	uint32 i = 0;
	uint32 chip_id = 0;
	uint32 reg_ctl_cal = 0;
	uint32 efuse_value =0;
	struct dcdc_core_volt_reg dcdc_core_step_volt[5];
	struct dcdc_core_ds_step_info step_info[5]={
		{ANA_REG_GLB_DCDC_SLP_CTRL2, 0,		ANA_REG_GLB_DCDC_SLP_CTRL4, 0},
		{ANA_REG_GLB_DCDC_SLP_CTRL2, 5,		ANA_REG_GLB_DCDC_SLP_CTRL4, 5},
		{ANA_REG_GLB_DCDC_SLP_CTRL2, 10,	ANA_REG_GLB_DCDC_SLP_CTRL4,10},
		{ANA_REG_GLB_DCDC_SLP_CTRL3, 0,		ANA_REG_GLB_DCDC_SLP_CTRL5, 0},
		{ANA_REG_GLB_DCDC_SLP_CTRL3, 5,		ANA_REG_GLB_DCDC_SLP_CTRL5, 5}
	};

	chip_id = __raw_readl(REG_AON_APB_AON_CHIP_ID);
	if (SHARKLC_CHIP_ID == (chip_id & ~SHARKLC_CHIP_ID_MASK)) {    //if is sharklc only 2723G2 used
		//config normal 1.05V, sleep 0.8V
		struct dcdc_core_volt_reg dcdc_core_slep_volt_tmp = {0x00, 0x10};
		struct dcdc_core_volt_reg dcdc_core_step_volt_tmp[5] = {{0x02, 0x00}, {0x02, 0x08}, {0x02, 0x10}, {0x02, 0x14}, {0x02, 0x18}};

		reg_ctl_cal =   BITS_DCDC_CORE_CAL_DS_SW(dcdc_core_slep_volt_tmp.dcdc_core_cali_bits)
			          | BITS_DCDC_CORE_CTRL_DS_SW(dcdc_core_slep_volt_tmp.dcdc_core_ctl_bits);

		for (i=0; i<5; i++) {
			dcdc_core_step_volt[i].dcdc_core_cali_bits = dcdc_core_step_volt_tmp[i].dcdc_core_cali_bits;
			dcdc_core_step_volt[i].dcdc_core_ctl_bits  = dcdc_core_step_volt_tmp[i].dcdc_core_ctl_bits;
		}

	} else {    //sharkls configuration, use both 2723G1/2723G2PMIC
		//sharkls config noraml 0.9V, sleep 0.81V
		efuse_value = __adie_efuse_read(BLK_INDEX);

		if ((efuse_value >> BIT_INDEX) & BIT(0)) {  //2723g2 1.05V dcdccore PMIC,have 150mV offset
			struct dcdc_core_volt_reg dcdc_core_slep_volt_tmp = {0x00, 0x13};
			struct dcdc_core_volt_reg dcdc_core_step_volt_tmp[5] = {{0x00, 0x18}, {0x01, 0x00}, {0x01, 0x04}, {0x01, 0x08}, {0x01, 0x0C}};

			reg_ctl_cal = BITS_DCDC_CORE_CAL_DS_SW(dcdc_core_slep_volt_tmp.dcdc_core_cali_bits)
						| BITS_DCDC_CORE_CTRL_DS_SW(dcdc_core_slep_volt_tmp.dcdc_core_ctl_bits);

			for (i=0; i<5; i++) {
				dcdc_core_step_volt[i].dcdc_core_cali_bits = dcdc_core_step_volt_tmp[i].dcdc_core_cali_bits;
				dcdc_core_step_volt[i].dcdc_core_ctl_bits  = dcdc_core_step_volt_tmp[i].dcdc_core_ctl_bits;
			}
		} else {      //2723g1 0.9V dcdccore PMIC
			struct dcdc_core_volt_reg dcdc_core_slep_volt_tmp = {0x02, 0x03};
			struct dcdc_core_volt_reg dcdc_core_step_volt_tmp[5] = {{0x02, 0x08}, {0x02, 0x10}, {0x02, 0x14}, {0x02, 0x18}, {0x02, 0x1C}};

			reg_ctl_cal = BITS_DCDC_CORE_CAL_DS_SW(dcdc_core_slep_volt_tmp.dcdc_core_cali_bits)
						| BITS_DCDC_CORE_CTRL_DS_SW(dcdc_core_slep_volt_tmp.dcdc_core_ctl_bits);

			for (i=0; i<5; i++) {
				dcdc_core_step_volt[i].dcdc_core_cali_bits = dcdc_core_step_volt_tmp[i].dcdc_core_cali_bits;
				dcdc_core_step_volt[i].dcdc_core_ctl_bits  = dcdc_core_step_volt_tmp[i].dcdc_core_ctl_bits;
			}
		}
	}

	ANA_REG_SET(ANA_REG_GLB_DCDC_SLP_CTRL1, reg_ctl_cal);   //set dcdccore sleep voltage

	for (i=0; i<5; i++) {    //set dcdccore step voltage form sleep to wakeup
		sci_adi_write(step_info[i].ctl_reg, dcdc_core_step_volt[i].dcdc_core_ctl_bits<<step_info[i].ctl_sht, 0x1F << step_info[i].ctl_sht);
		sci_adi_write(step_info[i].cal_reg, dcdc_core_step_volt[i].dcdc_core_cali_bits<<step_info[i].cal_sht, 0x1F << step_info[i].cal_sht);
	}

	if(0x00000007 == ds_cfg.is_auto_ds) {
		/*enable the DCDC_CORE_SLEEP_OUT_STEP for dcdc core step down */
		ANA_REG_OR(ANA_REG_GLB_DCDC_SLP_CTRL0, 0x3);
	} else {
		/*disable the DCDC_CORE_SLEEP_OUT_STEP */
		ANA_REG_AND(ANA_REG_GLB_DCDC_SLP_CTRL0, ~(0x3));
	}

#endif
}
#if 0
static void dcdc_core_ds_config(unsigned int para)
{
#if defined(CONFIG_ADIE_SC2723S) || defined(CONFIG_ADIE_SC2723)
	uint32 reg_val = 0;
	uint32 dcdc_core_ctl_ds = -1;
	uint32 dcdc_core_cal_ds = -1;
	uint32 dcdc_core_cal_adi = 0;
	uint32 dcdc_core_ctl_adi = -1;
	uint32 reg_val_cal = 0;
	static uint32 step_cal = 3;
	uint32 step_cal_flag = 0;

	static struct dcdc_core_ds_step_info step_info[5]={
		{ANA_REG_GLB_DCDC_SLP_CTRL2, 0,		ANA_REG_GLB_DCDC_SLP_CTRL4, 0},
		{ANA_REG_GLB_DCDC_SLP_CTRL2, 5,		ANA_REG_GLB_DCDC_SLP_CTRL4, 5},
		{ANA_REG_GLB_DCDC_SLP_CTRL2, 10,	ANA_REG_GLB_DCDC_SLP_CTRL4,10},
		{ANA_REG_GLB_DCDC_SLP_CTRL3, 0,		ANA_REG_GLB_DCDC_SLP_CTRL5, 0},
		{ANA_REG_GLB_DCDC_SLP_CTRL3, 5,		ANA_REG_GLB_DCDC_SLP_CTRL5, 5}
	};

	static char dcdc_core_down_volt[]={4,1,1,2,3,5,0,6};
	static char dcdc_core_up_volt[]={6,2,3,4,0,1,7,7};
	uint32 i;
	/*1100,700,800,900,1000,650,1200,1300*/
	static uint32 step_ratio[]={8,8,4,4,4};

	/*sleep dcdc cal transformer for sc2723*/
	dcdc_core_cal_adi = (ANA_REG_GET(ANA_REG_GLB_DCDC_CORE_ADI)) & 0x1F;
	/*step_cal = 3, so 3*3mV=9mV ~ 0.01V*/
	dcdc_core_cal_ds  = dcdc_core_cal_adi+step_cal;

	if(dcdc_core_cal_ds >= 0x1F) {
		/*if cal > 1,set step_cal_flag = 1,carry bit*/
		dcdc_core_cal_ds = dcdc_core_cal_ds - 0x1F;
		step_cal_flag = 0x1;
	} else {
		step_cal_flag = 0;
	}

	reg_val_cal = ANA_REG_GET(ANA_REG_GLB_DCDC_SLP_CTRL1);
	reg_val_cal &= ~ ( 0x1F << 5);
	reg_val_cal |= dcdc_core_cal_ds << 5;

	ANA_REG_SET(ANA_REG_GLB_DCDC_SLP_CTRL1, reg_val_cal);

	/*sleep dcdc ctl transformer for sc2723*/
	dcdc_core_ctl_adi = (ANA_REG_GET(ANA_REG_GLB_DCDC_CORE_ADI) >> 5) & 0x1F;
	if(0x1 == step_cal_flag) {
		/*if step_cal_flag = 1,the ctl will first down and after up, so it no change*/
		dcdc_core_ctl_ds  = dcdc_core_ctl_adi;
		step_cal_flag = 0;
	} else {
		dcdc_core_ctl_ds  = dcdc_core_down_volt[dcdc_core_ctl_adi];
	}

	reg_val = ANA_REG_GET(ANA_REG_GLB_DCDC_SLP_CTRL1);
	reg_val &= ~0x1F;
	reg_val |= dcdc_core_ctl_ds;

	ANA_REG_SET(ANA_REG_GLB_DCDC_SLP_CTRL1, reg_val);

	dcdc_core_ctl_ds = ANA_REG_GET(ANA_REG_GLB_DCDC_SLP_CTRL1) & 0x1F;
	dcdc_core_ctl_adi = (ANA_REG_GET(ANA_REG_GLB_DCDC_CORE_ADI) >> 5) & 0x1F;
	dcdc_core_cal_adi = (ANA_REG_GET(ANA_REG_GLB_DCDC_CORE_ADI)) & 0x1F;

	 if(dcdc_core_ctl_ds < dcdc_core_ctl_adi){
                for(i=0;i<5;i++) {
                        reg_val = dcdc_core_cal_adi + step_ratio[i];
                        if(reg_val <= 0x1F) {
                                sci_adi_write(step_info[i].ctl_reg,dcdc_core_ctl_ds<<step_info[i].ctl_sht,0x1F <<step_info[i].ctl_sht);
                                sci_adi_write(step_info[i].cal_reg,reg_val<<step_info[i].cal_sht,0x1F << step_info[i].cal_sht);
                                dcdc_core_cal_adi = reg_val;
                        } else {
                                sci_adi_write(step_info[i].ctl_reg,dcdc_core_up_volt[dcdc_core_ctl_ds]<<step_info[i].ctl_sht,
                                                                                                0x1F << step_info[i].ctl_sht);
                                sci_adi_write(step_info[i].cal_reg,(reg_val-0x1F)<<step_info[i].cal_sht,0x1F << step_info[i].cal_sht);
                                dcdc_core_ctl_ds = dcdc_core_up_volt[dcdc_core_ctl_ds];
				dcdc_core_cal_adi = reg_val - 0x1F;
                        }
                }
        } else {
                for(i=0;i<5;i++) {
                        /*every step should equal function mode*/
                        sci_adi_write(step_info[i].ctl_reg,dcdc_core_ctl_adi<<step_info[i].ctl_sht,0x1F << step_info[i].ctl_sht);
                        sci_adi_write(step_info[i].cal_reg,dcdc_core_cal_adi<<step_info[i].cal_sht,0x1F << step_info[i].cal_sht);
                }
        }

		reg_val =(ANA_REG_GET(ANA_REG_GLB_CHIP_ID_HIGH) & 0xFFFF) << 16;
		reg_val |= (ANA_REG_GET(ANA_REG_GLB_CHIP_ID_LOW) & 0xFFFF);

	if(reg_val == 0x2723a000) {
		/*disable the DCDC_CORE_SLEEP_OUT_STEP for sc2723a000 otp no value*/
		ANA_REG_AND(ANA_REG_GLB_DCDC_SLP_CTRL0,~(0x3));
	} else {
		if(0x00000007 == ds_cfg.is_auto_ds) {
			/*enable the DCDC_CORE_SLEEP_OUT_STEP for dcdc core step down */
			ANA_REG_OR(ANA_REG_GLB_DCDC_SLP_CTRL0, 0x3);
		} else {
			/*disable the DCDC_CORE_SLEEP_OUT_STEP */
			ANA_REG_AND(ANA_REG_GLB_DCDC_SLP_CTRL0, ~(0x3));
		}
	}
#endif
}
#endif
static void setup_ap_cp_sync_sleep_code(unsigned int start_addr)
{
	unsigned int *sa = (unsigned int*)start_addr;
	/*always on the pub sys power*/
	CHIP_REG_AND(REG_PMU_APB_PD_PUB_SYS_CFG,~(0x3 << 24));
	/*set for indcate phy is alive*/
	CHIP_REG_OR(DDR_PHY_IND_ADDR1,DDR_PHY_IND_PHY_ALIVE);
	/** set flag for indicate phy is alive*/
	CHIP_REG_OR(DDR_PHY_IND_ADDR,DDR_PHY_IND_PHY_ALIVE);
	/* set flag for indicate phy is alive*/
	*(volatile unsigned int*)DDR_PHY_IND_ADDR2 = DDR_PHY_IND_PHY_ALIVE;
	/*close umctl and phy and publ auto retention*/
	*(volatile unsigned int*)REG_PMU_APB_DDR_OP_MODE_CFG &= ~((0x3 << 27)|(0x1 << 25));
}

void CSP_Init(unsigned int gen_para)
{
	unsigned int reg_val;
	setup_autopd_mode();
	pmu_commom_config();
	//calibrate_register_callback((void*)dcdc_core_ds_config);
	dcdc_core_ds_config(0x00000000);
	setup_ap_cp_sync_sleep_code(0x50001800);
	dcdc_optimize_config(0x00000000);
	/*open adi clock auto gate for power consume*/
//	reg_val = readl(ADI_GSSI_CTL0);
//	reg_val &= ~(0x1 << 30);
//	writel(reg_val,ADI_GSSI_CTL0);
	/*disable int ana dcd otp interrupt*/
//	ANA_REG_AND(REG_ANA_INTC_INT_EN,~BIT_DCDCOTP_INT_EN);
}
/**---------------------------------------------------------------------------*
 **                         Compiler Flag                                     *
 **---------------------------------------------------------------------------*/
#ifdef __cplusplus
}
#endif
