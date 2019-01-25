/**
 * sprd_usb3_hw.c - Spreadtrum USB3 HW glue file
 *
 * Copyright (C) 2015 SPREADTRUM  - http://www.spreadtrum.com
 *
 * Authors: Miao.Zhu <miao.zhu@spreadtrum.com>,
 *
 * SPDX-License-Identifier:     GPL-2.0
 */

#include <common.h>
#include <malloc.h>
#include <asm/arch/sprd_reg.h>
#include <asm/io.h>
#include <sprd_regulator.h>
#include "linux-compat.h"
#include "core.h"

extern int regulator_enable(const char con_id[]);
extern int regulator_disable(const char con_id[]);

#define LDO_USB3 "vddusb33"

static struct dwc3_device dwc3_dev = {
	.base = SPRD_USB_BASE,
	.maximum_speed = USB_SPEED_HIGH,
	.dis_u3_susphy_quirk = 1,
	.dis_u2_susphy_quirk = 1,
};

static u32 usb3_phy_tune1 = 0x919e9dec;
static u32 usb3_phy_tune2 = 0x0f0560fe;

static void usb_clk_enable(u32 is_on)
{
	u32 val;

	val = readl(REG_AP_AHB_AHB_EB);
	if (is_on)
		writel(val | (BIT_AP_AHB_USB3_EB | BIT_AP_AHB_USB3_SUSPEND_EB
			 | BIT_AP_AHB_USB3_REF_EB), REG_AP_AHB_AHB_EB);
	else
		writel(val & ~(BIT_AP_AHB_USB3_EB | BIT_AP_AHB_USB3_SUSPEND_EB
			 | BIT_AP_AHB_USB3_REF_EB), REG_AP_AHB_AHB_EB);
}

#ifdef CONFIG_FPGA
static void usb_phy_enable(u32 is_on)
{
}
#else
static void usb_phy_enable(u32 is_on)
{
	u32 val;

	if (is_on) {
		/* regulator_set_voltage(LDO_USB3, vol); */
		regulator_enable(LDO_USB3);

		udelay(5 * 1000);

		/* Enable PipeP PowerPresent */
		val = readl(REG_ANA_APB_USB30_PHY_TEST);
		writel(val | (BIT_ANA_APB_USB30_PIPEP_POWERPRESENT_REG |
			BIT_ANA_APB_USB30_PIPEP_POWERPRESENT_CFG_SEL),
			REG_ANA_APB_USB30_PHY_TEST);

		/* Clear PHY_PD */
		val = readl(REG_ANA_APB_PWR_CTRL1);
		writel(val & ~BIT_ANA_APB_USB30_PS_PD, REG_ANA_APB_PWR_CTRL1);
		writel(usb3_phy_tune1, REG_ANA_APB_USB30_PHY_TUNE1);
		writel(usb3_phy_tune2, REG_ANA_APB_USB30_PHY_TUNE2);

		/* Enable SSP_En and Clear Ref_Clk_Div2 */
		val = readl(REG_ANA_APB_USB30_PHY_CTRL1);
		val |= BIT_ANA_APB_USB30_REF_SSP_EN;
		val &= ~BIT_ANA_APB_USB30_REF_CLKDIV2;
		writel(val, REG_ANA_APB_USB30_PHY_CTRL1);

		/* PHY Soft Reset */
		val = readl(REG_ANA_APB_RST_CTRL);
		writel(val | BIT_ANA_APB_USB30_PHY_SOFT_RST, REG_ANA_APB_RST_CTRL);
		udelay(5 * 1000);
		writel(val & ~BIT_ANA_APB_USB30_PHY_SOFT_RST, REG_ANA_APB_RST_CTRL);

		/* Core Soft Reset */
		val = readl(REG_AP_AHB_AHB_RST);
		writel(val | BIT_AP_AHB_USB3_SOFT_RST, REG_AP_AHB_AHB_RST);
		udelay(5 * 1000);
		writel(val & ~BIT_AP_AHB_USB3_SOFT_RST, REG_AP_AHB_AHB_RST);
	} else {
		regulator_disable(LDO_USB3);

		val = readl(REG_ANA_APB_USB30_PHY_TEST);
		writel(val & ~(BIT_ANA_APB_USB30_PIPEP_POWERPRESENT_REG |
			BIT_ANA_APB_USB30_PIPEP_POWERPRESENT_CFG_SEL),
			REG_ANA_APB_USB30_PHY_TEST);

		val = readl(REG_ANA_APB_PWR_CTRL1);
		writel(val | BIT_ANA_APB_USB30_PS_PD, REG_ANA_APB_PWR_CTRL1);
	}
}
#endif

