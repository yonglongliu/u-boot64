#include <common.h>
#include <malloc.h>
#include "key_map.h"
#include <boot_mode.h>
#include <asm/arch/mfp.h>
#include <asm/arch/chip_drv_common_io.h>
#include <asm/arch/sprd_eic.h>

void board_keypad_init(void)
{
#if 0
	sprd_gpio_request(NULL, 124);
	sprd_gpio_direction_input(NULL, 124);

	sprd_gpio_request(NULL, 125);
	sprd_gpio_direction_input(NULL, 125);
#endif
	printf("[gpio keys] init!\n");
	return;
}

unsigned char board_key_scan(void)
{
	uint32_t key_code = KEY_RESERVED;
	int gpio_volumeup = -1;
	int gpio_volumedown = -1;

	sprd_eic_request(EIC_KEY2_7S_RST_EXT_RSTN_ACTIVE);
	udelay(3000);
	gpio_volumeup = sprd_eic_get(EIC_KEY2_7S_RST_EXT_RSTN_ACTIVE);
	if(gpio_volumeup < 0)
		printf("[eic keys] volumeup : sprd_eic_get return ERROR!\n");
	if(gpio_volumeup > 0) {
		key_code = KEY_VOLUMEUP;
		printf("[eic keys] volumeup pressed!\n");
	}

	sprd_eic_request(SPRD_DDIE_EIC_EXTINT1);
	udelay(3000);
	gpio_volumedown = sprd_eic_get(SPRD_DDIE_EIC_EXTINT1);
	if(gpio_volumedown < 0)
		printf("[eic keys] volumedown : sprd_eic_get return ERROR!\n");
	/*T8 volumedown connected to KEYIN0 and default is high level*/
	if (0 == gpio_volumedown) {
		key_code = KEY_VOLUMEDOWN;
		printf("[eic keys] volumedown pressed!\n");
	}

	if (KEY_RESERVED == key_code)
		printf("[gpio keys] no key pressed!\n");

	return key_code;
}

unsigned int check_key_boot(unsigned char key)
{
    if(KEY_VOLUMEUP == key)
      return CMD_FACTORYTEST_MODE;
    else if(KEY_HOME == key)
      return CMD_FASTBOOT_MODE;
    else if(KEY_VOLUMEDOWN== key)
      return CMD_RECOVERY_MODE;
    else 
      return 0;
}

