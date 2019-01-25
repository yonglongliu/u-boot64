
#include "loader_common.h"

extern unsigned int g_charger_mode;


void normal_mode(void)
{
	vibrator_hw_init();

	set_vibrator(1);
	vlx_nand_boot(BOOT_PART, BACKLIGHT_ON);
	return;
}

void calibration_mode(void)
{
	debugf("calibration_mode\n");

	setenv("bootmode", "cali");
	vlx_nand_boot(RECOVERY_PART, BACKLIGHT_OFF);
	return;
}

void autotest_mode(void)
{
	debugf("autotest_mode\n");

	setenv("bootmode", "autotest");
	vlx_nand_boot(BOOT_PART, BACKLIGHT_ON);
	return;
}

void recovery_mode(void)
{
	debugf("recovery_mode\n");
    setenv("bootmode", "recovery");
	vlx_nand_boot(RECOVERY_PART, BACKLIGHT_ON);
	return;

}


void special_mode(void)
{
	debugf("special_mode\n");

	setenv("bootmode", "special");
	vlx_nand_boot(BOOT_PART, BACKLIGHT_OFF);
	return;
}

void iq_mode(void)
{
	debugf("iq_mode\n");
	setenv("bootmode", "iq");
	vlx_nand_boot(BOOT_PART, BACKLIGHT_OFF);
	return;	

}

void watchdog_mode(void)
{
	debugf("watchdog_mode\n");
	setenv("bootmode", "wdgreboot");
	vlx_nand_boot(BOOT_PART, BACKLIGHT_OFF);
	return;

}

void unknow_reboot_mode(void)
{
	debugf("unknow_reboot_mode\n");
	setenv("bootmode", "unknowreboot");
	vlx_nand_boot(BOOT_PART, BACKLIGHT_OFF);
	return;

}

void panic_reboot_mode(void)
{
	debugf("enter\n");
	setenv("bootmode", "panic");
	vlx_nand_boot(BOOT_PART, BACKLIGHT_OFF);
	return;

}


void fastboot_mode(void)
{
	debugf("enter\n");
#ifdef CONFIG_SPLASH_SCREEN
	vibrator_hw_init();
	set_vibrator(1);

	extern void set_backlight(uint32_t value);
	lcd_printf("   fastboot mode");

	set_backlight(255);
	mdelay(400);
	set_vibrator(0);
#endif
	//MMU_DisableIDCM();
	do_fastboot();

	return;
}


void autodloader_mode(void)
{
	debugf("Enter autodloader mode\n");

	/* remap iram */
	//autodlader_remap();
	/* main handler receive and jump */
	autodloader_mainhandler();

	/*reach here means error happened*/
	return;
}


void charge_mode(void)
{
	debugf("enter\n");

	g_charger_mode = 1;
	setenv("bootmode", "charger");
	vlx_nand_boot(BOOT_PART, BACKLIGHT_ON);

}

void engtest_mode(void)
{
	printf("enter\n");
	setenv("bootmode", "engtest");
	vlx_nand_boot(BOOT_PART, BACKLIGHT_OFF);

}


void alarm_mode(void)
{
	debugf("enter\n");
	setenv("bootmode", "alarm");
	vlx_nand_boot(BOOT_PART, BACKLIGHT_OFF);

}

void factorytest_mode(void)
{
	debugf("enter\n");
    setenv("bootmode", "factorytest");
    vlx_nand_boot(RECOVERY_PART, BACKLIGHT_ON);

}

void sprdisk_mode(void)
{
	debugf("enter\n");
	setenv("bootmode", "sprdisk");
	vlx_nand_boot(BOOT_PART, BACKLIGHT_OFF);

}
