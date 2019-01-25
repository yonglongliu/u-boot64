#ifndef _KEY_MAP_H_
#define _KEY_MAP_H_

#include <linux/input.h>

#define KEY_EXTEND (0)

#if KEY_EXTEND
static unsigned char board_key_map[]={
#if 0
    0x27, KEY_HOME,
    0x42, KEY_BACK,
    0x41, KEY_VOLUMEUP,
#else
    //0x00, KEY_BACK,//DOWN
    //0x01, KEY_HOME,//CAM
    //0x10, KEY_MENU,//UP
   0x10, KEY_VOLUMEDOWNMEUP,//DOWN
   0x01, KEY_HOME,//CAM
   0x00, KEY_VOLUMEDOWN,//UP
#endif
};
#else
static unsigned char board_key_map[3][10] = {

	{KEY_VOLUMEDOWN, KEY_RIGHT, KEY_3, KEY_7, KEY_9, 0xe4, 0xa9, 0x00, 0x00, 0x00}, //00 ~ 09
	{KEY_VOLUMEUP, KEY_ENTER, KEY_2, KEY_5, KEY_8, KEY_0, KEY_VOLUMEDOWN, 0x00, 0x00, 0x00}, //10 ~ 19
	{KEY_HOME, KEY_LEFT, KEY_1, KEY_4, KEY_7, 0xe3, 0x00, 0x00, 0x00, 0x00}  //20 ~ 29
};
#endif

#define CONFIG_KEYPAD_ROW_CNT 3
#define CONFIG_KEYPAD_COL_CNT 3
#define CONFIG_KEYPAD_LONG_CNT 0xc
#define CONFIG_KEYPAD_DEBOUNCE_CNT 0x48
#define KPD_INT0_ROW(x) (((x)>>4) & 0x7)
#define KPD_INT0_COL(x) (((x)>>0) & 0x7)
#endif //_KEY_MAP_H_
