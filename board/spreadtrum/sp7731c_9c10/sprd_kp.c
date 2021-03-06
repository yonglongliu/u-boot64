#include <common.h>
#include <malloc.h>
#include "key_map.h"
#include <boot_mode.h>
#include <asm/arch/mfp.h>
#include <asm/arch/chip_drv_common_io.h>

void board_keypad_init(void)
{
	sprd_gpio_request(NULL, 199);
	sprd_gpio_direction_input(NULL, 199);

	sprd_gpio_request(NULL, 200);
	sprd_gpio_direction_input(NULL, 200);

	printf("[gpio keys] init!\n");
	return;
}

unsigned char board_key_scan(void)
{
	uint32_t key_code = KEY_RESERVED;
	int gpio_volumeup = -1;
	int gpio_volumedown = -1;

	gpio_volumedown = sprd_gpio_get(NULL, 199);
	if(gpio_volumedown < 0)
		printf("[gpio keys] volumedown : sprd_gpio_get return ERROR!\n");

	gpio_volumeup = sprd_gpio_get(NULL, 200);
	if(gpio_volumeup < 0)
		printf("[gpio keys] volumeup : sprd_gpio_get return ERROR!\n");

	if(0 == gpio_volumedown) {
		key_code = KEY_VOLUMEDOWN;
		printf("[gpio keys] volumedown pressed!\n");
		return key_code;
	}

	if(0 == gpio_volumeup) {
		key_code = KEY_VOLUMEUP;
		printf("[gpio keys] volumeup pressed!\n");
		return key_code;
	}

	printf("[gpio keys] no key pressed!\n");
	return key_code;
}

unsigned int check_key_boot(unsigned char key)
{
	if(KEY_VOLUMEUP == key)
	#ifdef CONFIG_POWER_UP_RECOVERY_DOWN_FACTORY
             return CMD_RECOVERY_MODE;
	#else
			return CMD_FACTORYTEST_MODE;
	#endif
	else if(KEY_HOME == key)
		return CMD_RECOVERY_MODE;
	else if(KEY_VOLUMEDOWN== key)
		#ifdef CONFIG_POWER_UP_RECOVERY_DOWN_FACTORY
		return CMD_FACTORYTEST_MODE;
		#else
		return CMD_RECOVERY_MODE;
		#endif
	else
		return 0;
}
