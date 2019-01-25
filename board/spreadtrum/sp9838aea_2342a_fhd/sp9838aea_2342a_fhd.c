/*
 * (C) Copyright 2014
 * David Feng <fenghua@phytium.com.cn>
 * Sharma Bhupesh <bhupesh.sharma@freescale.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <malloc.h>
#include <boot_mode.h>
#include "adi_hal_internal.h"
#include <chipram_env.h>
#include <sprd_regulator.h>

DECLARE_GLOBAL_DATA_PTR;

phys_size_t real_ram_size = 0x40000000;


int board_init(void)
{
	/*get env transfer from chipram*/
	setup_chipram_env();

	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;
	ADI_init();
	/*FPGA forbiden*/
	misc_init();
	LDO_Init();
	ADC_Init();
	/*FPGA forbiden*/
	pin_init();
	sprd_eic_init();
	sprd_intc_enable();
	sprd_gpio_init();
	/*FPGA forbiden*/
	init_ldo_sleep_gr();
	TDPllRefConfig(1);

	boot_pwr_check();
	return 0;
}


phys_size_t get_real_ram_size(void)
{
        return real_ram_size;
}

int dram_init(void)
{
#ifdef CONFIG_DDR_AUTO_DETECT
	ulong sdram_base = CONFIG_SYS_SDRAM_BASE;
	ulong sdram_size = 0;
	int i;

	real_ram_size = 0;
	uint32_t bank_cnt = *(volatile uint32_t *)CONFIG_NR_DRAM_BANKS_ADDR_IN_IRAM;

	for (i = 1; i <= bank_cnt; i++) {
		real_ram_size += *(volatile uint32_t *)((volatile uint32_t *)CONFIG_NR_DRAM_BANKS_ADDR_IN_IRAM + i);
	}
	real_ram_size = get_ram_size((volatile void *)sdram_base, real_ram_size);

#else
	real_ram_size = REAL_SDRAM_SIZE;
#endif

	/*
	 * Clear spin table so that secondary processors
	 * observe the correct value after waken up from wfe.
	 */
	*(unsigned long *)CPU_RELEASE_ADDR = 0;

        gd->ram_size = get_ram_size((volatile void *)PHYS_SDRAM_1,
                        PHYS_SDRAM_1_SIZE);

        return 0;
}



int misc_init_r(void)
{
	boot_pwr_check();
	/*reserver for future use*/
	return 0;
}

int board_late_init(void)
{
        boot_mode_t boot_role;
        extern chipram_env_t* get_chipram_env(void);
        chipram_env_t* cr_env = get_chipram_env();
        boot_role = cr_env->mode;

	boot_pwr_check();

#if !defined(CONFIG_FPGA)
#ifdef CONFIG_NAND_BOOT
	//extern int nand_ubi_dev_init(void);
	nand_ubi_dev_init();
	debugf("nand ubi init OK!\n");
#endif
	if(boot_role == BOOTLOADER_MODE_LOAD)           //if it's BOOTLOADER_MODE_LOAD, goto sprdbat_init.	
		sprdbat_init();
	debugf("CHG init OK!\n");
	if (sprdbat_get_vbatauxadc_caltype() != 0) {
		/*fix me*/
		DCDC_Cal_ArmCore();
	}
#endif
	board_keypad_init();
	return 0;
}


CBOOT_FUNC s_boot_func_array[CHECK_BOOTMODE_FUN_NUM] = {

	get_mode_from_bat_low,
	write_sysdump_before_boot_extend,
	/* 1 get mode from file*/
	get_mode_from_file_extend,
	/* 2 get mode from watch dog*/
	get_mode_from_watchdog,
	/*3 get mode from alarm register*/
	get_mode_from_alarm_register,
	/*0 get mode from pc tool*/
    get_mode_from_pctool,
	/*4 get mode from charger*/
	get_mode_from_charger,
	/*5 get mode from keypad*/
	get_mode_from_keypad,
	/*6 get mode from gpio*/
	//get_mode_from_gpio_extend,

	/*shutdown device*/
	//get_mode_from_shutdown,

	0
};



void board_boot_mode_regist(CBOOT_MODE_ENTRY *array)
{
	MODE_REGIST(CMD_NORMAL_MODE, normal_mode);
	MODE_REGIST(CMD_RECOVERY_MODE, recovery_mode);
	MODE_REGIST(CMD_FASTBOOT_MODE, fastboot_mode);
	MODE_REGIST(CMD_WATCHDOG_REBOOT, watchdog_mode);
	MODE_REGIST(CMD_UNKNOW_REBOOT_MODE, unknow_reboot_mode);
	MODE_REGIST(CMD_PANIC_REBOOT, panic_reboot_mode);
	MODE_REGIST(CMD_AUTODLOADER_REBOOT, autodloader_mode);
	MODE_REGIST(CMD_SPECIAL_MODE, special_mode);
	MODE_REGIST(CMD_CHARGE_MODE, charge_mode);
	MODE_REGIST(CMD_ENGTEST_MODE,engtest_mode);
	MODE_REGIST(CMD_FACTORYTEST_MODE,factorytest_mode);
	MODE_REGIST(CMD_CALIBRATION_MODE, calibration_mode);
	MODE_REGIST(CMD_AUTOTEST_MODE, autotest_mode);
	MODE_REGIST(CMD_EXT_RSTN_REBOOT_MODE, normal_mode);
	MODE_REGIST(CMD_IQ_REBOOT_MODE, iq_mode);
	MODE_REGIST(CMD_ALARM_MODE, alarm_mode);

	return ;
}

