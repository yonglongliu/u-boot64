#include <common.h>
#include <malloc.h>
#include "key_map.h"
#include <boot_mode.h>
#include <asm/arch/mfp.h>
#include <asm/arch/sprd_keypad.h>
#include <asm/arch/chip_drv_common_io.h>
#include <asm/arch/sprd_eic.h>

struct key_map_info{
        unsigned int total_size;
        unsigned int keycode_size;
        unsigned char *key_map;
        unsigned int total_row;
        unsigned int total_col;
	unsigned int row;
	unsigned int col;

};

struct key_map_info * sprd_key_map = 0;

unsigned char board_key_scan(void);

void board_keypad_init(void)
{
    unsigned int key_type;

    printf("sc_keypad entry board_keypad_init\n");
    sprd_key_map = malloc(sizeof(struct key_map_info));

    if(NULL == sprd_key_map){
      printf("sc_keypad %s malloc faild\n", __FUNCTION__);
      return;
    }

    sprd_key_map->total_size = ARRAY_SIZE(board_key_map);
    sprd_key_map->keycode_size = sizeof(board_key_map[0])*2;
    sprd_key_map->key_map = board_key_map;
    sprd_key_map->total_row = CONFIG_KEYPAD_ROW_CNT;
    sprd_key_map->total_col = CONFIG_KEYPAD_COL_CNT;
    sprd_key_map->row = 0;
    sprd_key_map->col = 0;

#if 0
    if(sprd_key_map->total_size % sprd_key_map->keycode_size){
        printf("sc_keypad %s: board_key_map config error, it should be %d aligned\n", __FUNCTION__, sprd_key_map->keycode_size);
        return;
    }
#endif

    /* GPIO init */
    sprd_gpio_request(NULL, 7);
    sprd_gpio_direction_input(NULL, 7);

    sprd_gpio_request(NULL, 8);
    sprd_gpio_direction_input(NULL, 8);

    sprd_gpio_request(NULL, 9);
    sprd_gpio_direction_input(NULL, 9);

    sprd_gpio_request(NULL, 10);
    sprd_gpio_direction_input(NULL, 10);

    sprd_gpio_request(NULL, 11);
    sprd_gpio_direction_input(NULL, 11);

    sprd_gpio_request(NULL, 12);
    sprd_gpio_direction_input(NULL, 12);

    sprd_gpio_request(NULL, 13);
    sprd_gpio_direction_input(NULL, 13);

    /* init sprd keypad controller */
    REG32(REG_AON_APB_APB_EB0) |= BIT_8;
    REG32(REG_AON_APB_APB_RTC_EB) |= BIT_1;
    REG32(REG_AON_APB_APB_RST0) |= BIT_8;
    udelay(2000);
     REG32(REG_AON_APB_APB_RST0) &= ~BIT_8;

    REG_KPD_INT_CLR = KPD_INT_ALL;
    REG_KPD_POLARITY = CFG_ROW_POLARITY | CFG_COL_POLARITY;
    REG_KPD_CLK_DIV_CNT = CFG_CLK_DIV & KPDCLK0_CLK_DIV0;
    REG_KPD_LONG_KEY_CNT = CONFIG_KEYPAD_LONG_CNT;
    REG_KPD_DEBOUNCE_CNT = CONFIG_KEYPAD_DEBOUNCE_CNT;//0x8;0x13
    REG_KPD_CTRL  = (7<<8)/*Col0-Col2 Enable*/|(7<<16)/*Row0-Row2 Enable*/;
    REG_KPD_CTRL |= 1; /*Keypad Enable*/;
}

static char handle_scan_code(unsigned char scan_code)
{
    int cnt;
    int key_map_cnt;
    unsigned char * key_map;
    int pos = 0;

    if(NULL == sprd_key_map){
        printf("plase call board_keypad_init first\n");
        return 0;
    }

    key_map_cnt = sprd_key_map->total_size / sprd_key_map->keycode_size;
    key_map = sprd_key_map->key_map;
#ifdef KEYPAD_DEBUG
    printf("scan code %d\n", scan_code);
#endif
    for(cnt = 0; cnt<key_map_cnt; cnt++){
        pos = cnt * 2;
        if(key_map[pos] == scan_code)
          return key_map[pos + 1];
    }
    return 0;
}

unsigned int gpio_scan(void)
{
	int gpio_val = 0x00;
	int gpio_cnt;

	for (gpio_cnt=0; gpio_cnt<7; gpio_cnt++)
	{
		gpio_val = sprd_gpio_get(NULL, (gpio_cnt+7));
		printf("sc_keypad gpio:%d, val:%d\n", gpio_cnt+7, gpio_val);
		if (gpio_val == 0)
			return (gpio_cnt+1);
	}

	return 0;
}

//it can only handle one key now
unsigned char board_key_scan(void)
{
    uint32_t s_int_status = REG_KPD_INT_RAW_STATUS;
    uint32_t s_key_status = REG_KPD_KEY_STATUS;
    uint32_t scan_code = 0;
    uint32_t key_code =0;
    uint32_t gpio_tmp = 0;
#ifdef KEYPAD_DEBUG
	printf("key operation flags is %08x, key %08x\n", REG_KPD_INT_RAW_STATUS, REG_KPD_KEY_STATUS);
#endif
    //clear reg
    REG_KPD_INT_CLR = KPD_INT_ALL;

    if((s_int_status & KPD_PRESS_INT0) || (s_int_status & KPD_LONG_KEY_INT0)){
#if KEY_EXTEND
	scan_code = s_key_status & (KPD1_ROW_CNT | KPD1_COL_CNT);
        key_code += handle_scan_code(scan_code);
#else
	//gpio_tmp = gpio_scan();
	sprd_key_map->row = KPD_INT0_ROW(s_key_status);
	sprd_key_map->col = KPD_INT0_COL(s_key_status);

	printf("sc_keypad s_key_status:0x%x, coordinate:(%d, %d) gpio_tmp = %d\n",
						s_key_status, sprd_key_map->row, sprd_key_map->col, gpio_tmp);
	key_code = board_key_map[sprd_key_map->row][sprd_key_map->col];
	printf("sc_keypad keycode = %d,(0x%x)\n", key_code, key_code);
#endif
    } if((s_int_status & KPD_PRESS_INT1) || (s_int_status & KPD_LONG_KEY_INT1)){
        scan_code = (s_key_status & (KPD2_ROW_CNT | KPD2_COL_CNT))>>8;
        key_code += handle_scan_code(scan_code);
    }if((s_int_status & KPD_PRESS_INT2) || (s_int_status & KPD_LONG_KEY_INT2)){
        scan_code = (s_key_status & (KPD3_ROW_CNT | KPD3_COL_CNT))>>16;
        key_code += handle_scan_code(scan_code);
    }if((s_int_status & KPD_PRESS_INT3) || (s_int_status & KPD_LONG_KEY_INT3)){
        scan_code = (s_key_status & (KPD4_ROW_CNT | KPD4_COL_CNT))>>24;
        key_code += handle_scan_code(scan_code);
    }

    //clear reg
    REG_KPD_INT_CLR = KPD_INT_ALL;
    /*Adie EIC10 as Key_Volumeup*/
	int volumeup = -1;
	sprd_eic_request(EIC_KEY2_7S_RST_EXT_RSTN_ACTIVE);
	udelay(3000);
	volumeup = sprd_eic_get(EIC_KEY2_7S_RST_EXT_RSTN_ACTIVE);
	if(volumeup < 0)
		printf("[eic keys] volumeup : sprd_eic_get return ERROR!\n");

	if(volumeup > 0) {
		key_code = KEY_VOLUMEUP;
		printf("[eic keys] volumeup pressed!\n");
	}

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
