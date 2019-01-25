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


#include <common.h>
#include <linux/types.h>
#include <linux/err.h>
#include <asm/io.h>
//#include <asm/arch/hardware.h>
#include <asm/arch/sprd_reg.h>
#include <asm/arch/sci.h>
#include <regs_efuse.h>
#include <regs_ana_efuse.h>


#define EFUSE_MAGIC 				(0x8810)
#define REG_ARM7_BASE			(0x50800000)
#define REG_ARM7_AHB_BASE		(REG_ARM7_BASE + 0x20000)
#define REG_ARM7_EFUSE_EB		(REG_ARM7_AHB_BASE + 0X0000)
#define REG_ARM7_SOFT_RST		(REG_ARM7_AHB_BASE + 0X0004)
#define BIT_ARM7_EFUSE_EB		(BIT(3))
#define BIT_ARM7_EFUSE_SOFT_RST	(BIT(3))

#define REGS_ARM7_EFUSE_BASE	(REG_ARM7_BASE + 0xf0000)
#define REGS_EFUSE_BASE                 	REGS_ARM7_EFUSE_BASE

#define pr_debug(args...)  printf(args)
#define pr_info() printf

#define WARN_ON(a)
u32 SCI_GetTickCount(void);
#define jiffies (SCI_GetTickCount()) /* return msec count */
#define msecs_to_jiffies(a) (a)
#define time_after(a,b)	((int)(b) - (int)(a) < 0)
#define cpu_relax()

static void msleep(u32 ms)
{
	u32 timeout = jiffies + ms;
	while(!time_after(jiffies, timeout)) {
		;
	}
}



#define EFUSE_BLOCK_MAX                 ( 16 )
#define EFUSE_BLOCK_WIDTH               ( 32 )	/* bit counts */

static void efuse_lookat(void)
{
	volatile u32 *reg = (volatile u32 *)REGS_EFUSE_BASE;
	int cnt = 16;
	while(cnt--) {
		printf("[%p] = %08x\n", reg, *reg);
		reg++;
	}
	printf("\n");
}

static void efuse_lock(void)
{
}

static void efuse_unlock(void)
{
}

 static void efuse_reset(void)
{
	/* should enable module before soft reset efuse */
	WARN_ON(!sci_glb_read(REG_ARM7_EFUSE_EB, BIT_ARM7_EFUSE_EB));
	sci_glb_set(REG_ARM7_SOFT_RST, BIT_ARM7_EFUSE_SOFT_RST);
	udelay(5);
	sci_glb_clr(REG_ARM7_SOFT_RST, BIT_ARM7_EFUSE_SOFT_RST);
}


static void __efuse_power_on(void)
{
	u32 cfg0;
	sci_glb_set(REG_ARM7_EFUSE_EB, BIT_ARM7_EFUSE_EB);
	efuse_reset();
	cfg0 = __raw_readl((void *)REG_EFUSE_CFG0);
	cfg0 &= ~BIT_EFS_VDDQ_K1_ON;
	cfg0 |= BIT_EFS_VDD_ON | BIT_EFS_VDDQ_K2_ON;
	__raw_writel(cfg0, (void *)REG_EFUSE_CFG0);
	msleep(1);
}

static void __efuse_power_off(void)
{
	u32 cfg0 = __raw_readl((void *)REG_EFUSE_CFG0);
	if (cfg0 & BIT_EFS_VDDQ_K1_ON) {
		cfg0 &= ~BIT_EFS_VDDQ_K1_ON;
		__raw_writel(cfg0, (void *)REG_EFUSE_CFG0);
		msleep(1);
	}

	cfg0 |= BIT_EFS_VDDQ_K2_ON;
	cfg0 &= ~BIT_EFS_VDD_ON;
	__raw_writel(cfg0, (void *)REG_EFUSE_CFG0);
	msleep(1);

	sci_glb_clr(REG_ARM7_EFUSE_EB, BIT_ARM7_EFUSE_EB);
}

static __inline int __efuse_wait_clear(u32 bits)
{
	int ret = 0;
	unsigned long timeout;

	pr_debug("wait %x\n", __raw_readl((void *)REG_EFUSE_STATUS));

	/* wait for maximum of 300 msec */
	timeout = jiffies + msecs_to_jiffies(300);
	while (__raw_readl((void *)REG_EFUSE_STATUS) & bits) {
		if (time_after(jiffies, timeout)) {
			WARN_ON(1);
			ret = -ETIMEDOUT;
			break;
		}
		cpu_relax();
	}
	return ret;
}

static u32 __efuse_read(int blk)
{
	u32 val = 0;

	/* enable efuse module clk and power before */

	__raw_writel(BITS_READ_WRITE_INDEX(blk),
		     (void *)REG_EFUSE_READ_WRITE_INDEX);
	__raw_writel(BIT_RD_START, (void *)REG_EFUSE_MODE_CTRL);

	if (IS_ERR_VALUE(__efuse_wait_clear(BIT_READ_BUSY)))
		goto out;

	val = __raw_readl((void *)REG_EFUSE_DATA_RD);

out:
	return val;
}

static u32 efuse_read(int blk_index)
{
	u32 val;
	pr_debug("efuse read %d\n", blk_index);
	efuse_lock();

	__efuse_power_on();
	val = __efuse_read(blk_index);
	__efuse_power_off();

	efuse_unlock();
	return val;
}

u32 __ddie_arm7_efuse_read(int blk_index)
{
	return efuse_read(blk_index);
}



static void __efuse_prog_power_on(void)
{
	u32 cfg0;
	sci_glb_set(REG_ARM7_EFUSE_EB, BIT_ARM7_EFUSE_EB);
	cfg0 = __raw_readl((void *)REG_EFUSE_CFG0);
	cfg0 &= ~(BIT_EFS_VDDQ_K2_ON | BIT_EFS_VDDQ_K1_ON);
	cfg0 |= BIT_EFS_VDD_ON;
	__raw_writel(cfg0, (void *)REG_EFUSE_CFG0);
	msleep(1);

	cfg0 |= BIT_EFS_VDDQ_K1_ON;
	__raw_writel(cfg0, (void *)REG_EFUSE_CFG0);
	msleep(1);
}


static int __efuse_prog(int blk, u32 val)
{
	u32 cfg0 = __raw_readl((void *)REG_EFUSE_CFG0);


	if (blk < 0 || blk >= EFUSE_BLOCK_MAX)	/* debug purpose */
		goto out;

	/* enable pgm mode and setup magic number before programming */
	cfg0 |= BIT_PGM_EN;
	__raw_writel(cfg0, (void *)REG_EFUSE_CFG0);
	__raw_writel(BITS_MAGIC_NUMBER(EFUSE_MAGIC),
		     (void *)REG_EFUSE_MAGIC_NUMBER);

	__raw_writel(val, (void *)REG_EFUSE_DATA_WR);
	__raw_writel(BITS_READ_WRITE_INDEX(blk),
		     (void *)REG_EFUSE_READ_WRITE_INDEX);
	pr_debug("cfg0 %x\n", __raw_readl((void *)REG_EFUSE_CFG0));
	__raw_writel(BIT_PG_START, (void *)REG_EFUSE_MODE_CTRL);
	if (IS_ERR_VALUE(__efuse_wait_clear(BIT_PGM_BUSY)))
		goto out;

out:
	__raw_writel(0, (void *)REG_EFUSE_MAGIC_NUMBER);
	cfg0 &= ~BIT_PGM_EN;
	__raw_writel(cfg0, (void *)REG_EFUSE_CFG0);
	return 0;
}


static int efuse_prog(int blk_index, u32 val)
{
	int ret;
	pr_debug("efuse prog %d %08x\n", blk_index, val);

	efuse_lock();

	/* enable vddon && vddq */
	__efuse_prog_power_on();
	ret = __efuse_prog(blk_index, val);
	__efuse_power_off();

	efuse_unlock();
	return ret;
}


int __ddie_arm7_efuse_prog(int blk_index, u32 val)
{
        return efuse_prog(blk_index, val);
}
 
void __ddie_arm7_efuse_block_dump(void)
{
	int idx;
	printf("arm7 efuse base %08x\n", REGS_EFUSE_BASE);
	printf("arm7 efuse blocks dump:\n");
	for (idx = 0; idx < EFUSE_BLOCK_MAX; idx++) {
		printf("[%02d] %08x\n", idx, __ddie_arm7_efuse_read(idx));
	}
}

