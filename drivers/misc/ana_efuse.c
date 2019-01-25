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

/* IMPORTANT:
 *
 * TODO:
 */
#include <common.h>
#include <linux/types.h>
#include <linux/err.h>
#include <asm/io.h>
//#include <asm/arch/hardware.h>
#include <asm/arch/sprd_reg.h>
#include <asm/arch/sci.h>
#include "adi.h"
#include <regs_efuse.h>
#include <regs_ana_efuse.h>

#define pr_debug(args...)  printf(args)

#define WARN_ON(a)
u32 SCI_GetTickCount(void);
#define jiffies (SCI_GetTickCount())	/* return msec count */
#define msecs_to_jiffies(a) (a)
#define time_after(a,b)	((int)(b) - (int)(a) < 0)
#define cpu_relax()

//#define SPRD_ADISLAVE_BASE (SPRD_ADI_PHYS + 0x8000)


#define ANA_REGS_EFUSE_BASE ANA_EFS_BASE
#define EFUSE_BLOCK_MAX                 ( 32 )
#ifdef CONFIG_ADIE_SC2731
//#define ANA_REGS_EFUSE_BASE             ( SPRD_ADISLAVE_BASE + 0x380 )
#define EFUSE_BLOCK_WIDTH               ( 16 )	/* bit counts */
#define VALUE_TYPE	u16
#else
//#define ANA_REGS_EFUSE_BASE             ( SPRD_ADISLAVE_BASE + 0x200 )
#define EFUSE_BLOCK_WIDTH               ( 8 )	/* bit counts */
#define VALUE_TYPE	u8

#endif


static VALUE_TYPE dummy_cached_otp[EFUSE_BLOCK_MAX] = {
	0x9f, 0xdf, 0xef, 0x37, 0x91, 0x27, 0xfc, 0xfd,
	0x7f, 0x7f, 0x7f, 0x7f, 0xff, 0xfe, 0x7d, 0xbf,
	0x7f, 0xff, 0x6c, 0x00, 0xe7, 0x71, 0x5f, 0xb7,
	0x81, 0x5b, 0xe7, 0x3c, 0xa1, 0x27, 0x8a, 0xa1,
};

void adie_efuse_workaround(void)
{
#ifdef CONFIG_ADIE_SC2731
#define REG_ADI_GSSI_CFG0					(SPRD_ADI_PHYS + 0x20)
#else
#define REG_ADI_GSSI_CFG0					(SPRD_ADI_PHYS + 0x1c)

#endif
/** BIT_RF_GSSI_SCK_ALL_IN
   * 0: sclk auto gate
   * 1: sclk always on
*/
#define BIT_RF_GSSI_SCK_ALL_IN		BIT(30)
	sci_glb_write(REG_ADI_GSSI_CFG0,
		      BIT_RF_GSSI_SCK_ALL_IN, BIT_RF_GSSI_SCK_ALL_IN);
}

void adie_efuse_lookat(void)
{
	volatile u32 *reg = (volatile u32 *)ANA_REGS_EFUSE_BASE;
	int cnt = 16;
	while (cnt--) {
		printf("[%p] = %08x\n", reg, sci_adi_read((u32) reg));
		reg++;
	}
	printf("\n");
}

static void adie_efuse_lock(void)
{
}

static void adie_efuse_unlock(void)
{
}

static void adie_efuse_reset(void)
{
}

static void __adie_efuse_power_on(void)
{
	sci_adi_set(ANA_REG_GLB_ARM_MODULE_EN, BIT_ANA_EFS_EN);
	/* FIXME: rtc_efs only for prog
	   sci_adi_set(ANA_REG_GLB_RTC_CLK_EN, BIT_RTC_EFS_EN);
	 */

	/* FIXME: sclk always on or not ? */
	/* adie_efuse_workaround(); */
}

static void __adie_efuse_power_off(void)
{
	/* FIXME: rtc_efs only for prog
	   sci_adi_clr(ANA_REG_GLB_RTC_CLK_EN, BIT_RTC_EFS_EN);
	 */
	sci_adi_clr(ANA_REG_GLB_ARM_MODULE_EN, BIT_ANA_EFS_EN);
}

static __inline int __adie_efuse_wait_clear(u32 bits)
{
	int ret = 0;
	unsigned long timeout;
	u32 bit_status = 1;
        u32 val;
	printk("wait %x\n", sci_adi_read(ANA_REG_EFUSE_STATUS));
	udelay(1000);

	/* wait for maximum of 3000 msec */
	timeout = jiffies + msecs_to_jiffies(3000);
	  while (bit_status) {
		val = sci_adi_read(ANA_REG_EFUSE_STATUS);
#ifdef CONFIG_ADIE_SC2731
		bit_status = (~val)& bits;
#else
        bit_status = val & bits;
#endif
		if (time_after(jiffies, timeout)) {
			WARN_ON(1);
			ret = -ETIMEDOUT;
			break;
		}
		cpu_relax();
	}
	return ret;
}

static u32 adie_efuse_read(int blk_index)
{
	u32 val = 0;

	pr_debug("adie efuse read %d\n", blk_index);
	adie_efuse_lock();
	__adie_efuse_power_on();
	/* enable adie_efuse module clk and power before */

	/* FIXME: set read timing, why 0x20 (default value)
	   sci_adi_raw_write(ANA_REG_EFUSE_RD_TIMING_CTRL,
	   BITS_EFUSE_RD_TIMING(0x20));
	 */
#ifdef CONFIG_ADIE_SC2731
	if (IS_ERR_VALUE(__adie_efuse_wait_clear(BIT_STANDBY_BUSY)))
		goto out;
#endif
	sci_adi_raw_write(ANA_REG_EFUSE_BLOCK_INDEX,
			  BITS_READ_WRITE_INDEX(blk_index));
	sci_adi_raw_write(ANA_REG_EFUSE_MODE_CTRL, BIT_RD_START);
#ifdef CONFIG_ADIE_SC2731
	if (IS_ERR_VALUE(__adie_efuse_wait_clear(BIT_NORMAL_RD_DONE)))
		goto out;
#else
	if (IS_ERR_VALUE(__adie_efuse_wait_clear(BIT_READ_BUSY)))
		goto out;
#endif
	val = sci_adi_read(ANA_REG_EFUSE_DATA_RD);
#ifndef CONFIG_ADIE_SC2731
	/* FIXME: reverse the otp value */
	val = BITS_EFUSE_DATA_RD(~val);
#endif
#ifdef CONFIG_ADIE_SC2731
	sci_adi_clr(ANA_REG_EFUSE_STATUS, BIT_NORMAL_RD_DONE);
#endif
out:
	__adie_efuse_power_off();
	adie_efuse_unlock();
	return val;
}

u32 __adie_efuse_read(int blk_index)
{
	return adie_efuse_read(blk_index);
}

//EXPORT_SYMBOL_GPL(__adie_efuse_read);

u32 __adie_efuse_read_bits(int bit_index, int length)
{
	int i, blk_index = (int)bit_index / EFUSE_BLOCK_WIDTH;
	int blk_max = DIV_ROUND_UP(bit_index + length, EFUSE_BLOCK_WIDTH);
	u32 val = 0;
	pr_debug("otp read blk %d - %d\n", blk_index, blk_max);
	/* FIXME: length should not bigger than 8 */
	for (i = blk_index; i < blk_max; i++) {
		val |= __adie_efuse_read(i)
		    << ((i - blk_index) * EFUSE_BLOCK_WIDTH);
	}
	//pr_debug("val=%08x\n", val);
	val >>= (bit_index & (EFUSE_BLOCK_WIDTH - 1));
	val &= BIT(length) - 1;
	pr_debug("otp read bits %d ++ %d 0x%08x\n\n", bit_index, length, val);
	return val;
}

void __adie_efuse_block_dump(void)
{
	int i, idx;
	printf("adie efuse blocks dump:\n");
#ifdef CONFIG_ADIE_SC2731
	printf("--  15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0");
#else
	printf("--  7 6 5 4 3 2 1 0");
#endif
	for (idx = 0; idx < EFUSE_BLOCK_MAX; idx++) {
		u32 val = 0;
		printf("\n");
		val = __adie_efuse_read(idx);
		dummy_cached_otp[idx] = (VALUE_TYPE) val;
		printf("%02d  ", idx);
		for (i = EFUSE_BLOCK_WIDTH - 1; i >= 0; --i)
			printf("%s ", (val & BIT(i)) ? "1" : "0");
		printf("\n");
	}
	printf("\n");
}
