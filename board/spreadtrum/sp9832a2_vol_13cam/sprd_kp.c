#include <common.h>
#include <malloc.h>
#include "key_map.h"
#include <boot_mode.h>
#include <asm/arch/mfp.h>
#include <asm/arch/chip_drv_common_io.h>
#include <asm/arch/sprd_eic.h>

void board_keypad_init(void)
{
	sprd_gpio_request(NULL, 124);
	sprd_gpio_direction_input(NULL, 124);

	sprd_gpio_request(NULL, 125);
	sprd_gpio_direction_input(NULL, 125);

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

    gpio_volumedown = sprd_gpio_get(NULL, 124);
    if(gpio_volumedown < 0)
        printf("[gpio keys] volumedown : sprd_gpio_get return ERROR!\n");
	if (0 == gpio_volumedown) {
        key_code = KEY_VOLUMEDOWN;
        printf("[gpio keys] volumedown pressed!\n");
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
