/* drivers/video/sprdfb/lcd_s6e8aa5x01_mipi.c
 *
 * Support for s6e8aa5x01 mipi LCD device
 *
 * Copyright (C) 2015 Spreadtrum
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
//#include <asm/arch/sprd_lcd.h>
#include "../sprdfb_chip_common.h"                                                                                                                                             LCM_SLEEP
#include "../sprdfb_panel.h"                                                                                                                                                   LCM_TAG_MASK
#include "../sprdfb.h"
#define printk printf

#define  LCD_DEBUG
#ifdef LCD_DEBUG
#define LCD_PRINT printk
#else
#define LCD_PRINT(...)
#endif

#define MAX_DATA   150

#define PIKEA_J1_NBW_PANEL


typedef struct LCM_Init_Code_tag {
	unsigned int tag;
	unsigned char data[MAX_DATA];
}LCM_Init_Code;

typedef struct LCM_force_cmd_code_tag{
	unsigned int datatype;
	LCM_Init_Code real_cmd_code;
}LCM_Force_Cmd_Code;

#define LCM_TAG_SHIFT 24
#define LCM_TAG_MASK  ((1 << 24) -1)
#define LCM_SEND(len) ((1 << LCM_TAG_SHIFT)| len)
#define LCM_SLEEP(ms) ((2 << LCM_TAG_SHIFT)| ms)
//#define ARRAY_SIZE(array) ( sizeof(array) / sizeof(array[0]))

#define LCM_TAG_SEND  (1<< 0)
#define LCM_TAG_SLEEP (1 << 1)

#if 1  //copy Z3
static LCM_Init_Code init_data[] = {
	{LCM_SEND(5),		{3, 0x00, 0xF0, 0x5A, 0x5A} },
	{LCM_SEND(2),		{0xCC, 0x4C} },
	{LCM_SEND(1),		{0x11} },
	{LCM_SLEEP(20)},
	{LCM_SEND(36),		{34, 0x00, 0xCA,
						0x01, 0x00, 0x01,
						0x00, 0x01, 0x00,
						0x80, 0x80, 0x80,
						0x80, 0x80, 0x80,
						0x80, 0x80, 0x80,
						0x80, 0x80, 0x80,
						0x80, 0x80, 0x80,
						0x80, 0x80, 0x80,
						0x80, 0x80, 0x80,
						0x80, 0x80, 0x80,
						0x00, 0x00, 0x00, } },
	{LCM_SEND(7),		{5, 0x00, 0xB2, 0x00, 0x0F, 0x00, 0x0F} },
	{LCM_SEND(5),		{3, 0x00, 0xB6, 0xBC, 0x0F} },
	{LCM_SEND(2),		{0xf7, 0x03} },
	{LCM_SEND(2),		{0xf7, 0x00} },
	{LCM_SEND(6),		{4, 0x00, 0xC0, 0xD8, 0xD8, 0x40} },
	{LCM_SEND(10),		{8, 0x00, 0xB8, 0x38, 0x00, 0x00, 0x60,
							0x44, 0x00, 0xA8} },
	{LCM_SEND(5),		{3, 0x00, 0xF0, 0xA5, 0xA5} },
	{LCM_SLEEP(120)},
	{LCM_SEND(1),		{0x29} },
};
#else //copy samsung dts
static LCM_Init_Code init_data[] = {
	{LCM_SEND(5),		{3, 0x00, 0xF0, 0x5A, 0x5A} },
	{LCM_SEND(2),		{0xCC, 0x4C} },
	{LCM_SEND(1),		{0x11}},
	{LCM_SLEEP(120)},

	
	{LCM_SEND(5),		{3, 0x00, 0xF1, 0x5A, 0x5A} },
	{LCM_SEND(36),		{34, 0x00, 0xCA, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
							0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
							0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00, } },
	{LCM_SEND(7),		{5, 0x00, 0xB2, 0x00, 0x0F, 0x00, 0x0F} },
	{LCM_SEND(5),		{3, 0x00, 0xB6, 0xB3, 0x0F} },
	{LCM_SEND(2),		{0xf7, 0x03} },
	{LCM_SEND(2),		{0xf7, 0x00} },
	{LCM_SEND(6),		{4, 0x00, 0xC0, 0xD8, 0xD8, 0x40} },
	{LCM_SEND(10),		{8, 0x00, 0xb8, 0x38, 0x00,0x00,0x60,0x44,0x00,0xa8} },

	{LCM_SEND(5),		{3, 0x00, 0xF0, 0xA5, 0xA5} },
	//{LCM_SEND(2),		{0xB0, 0x06} },
	//{LCM_SEND(2),		{0xB8, 0xA8} },
	{LCM_SEND(1),		{0x11}},
	{LCM_SLEEP(120)},
	{LCM_SEND(1),		{0x29}},
};

#endif


static LCM_Init_Code disp_on =  {LCM_SEND(1), {0x29}};

static LCM_Init_Code sleep_in[] =  {
	{LCM_SEND(1), {0x28}},
	{LCM_SLEEP(150)}, 	//>150ms
	{LCM_SEND(1), {0x10}},
	{LCM_SLEEP(150)},	//>150ms
};

static LCM_Init_Code sleep_out[] =  {
	{LCM_SEND(1), {0x11}},
	{LCM_SLEEP(120)},//>120ms
	{LCM_SEND(1), {0x29}},
	{LCM_SLEEP(20)}, //>20ms
};
static int32_t s6e8aa5x01_mipi_init(struct panel_spec *self)
{
	int32_t i = 0;
	LCM_Init_Code *init = init_data;
	unsigned int tag;

	mipi_set_cmd_mode_t mipi_set_cmd_mode = self->info.mipi->ops->mipi_set_cmd_mode;
	mipi_dcs_write_t mipi_dcs_write = self->info.mipi->ops->mipi_dcs_write;
	mipi_eotp_set_t mipi_eotp_set = self->info.mipi->ops->mipi_eotp_set;

	printk("kernel s6e8aa5x01_mipi_init\n");

	mipi_set_cmd_mode();
	mipi_eotp_set(0,0);

	for(i = 0; i < ARRAY_SIZE(init_data); i++){
		tag = (init->tag >>24);
		if(tag & LCM_TAG_SEND){
			mipi_dcs_write(init->data, (init->tag & LCM_TAG_MASK));
			udelay(20);
		}else if(tag & LCM_TAG_SLEEP){
			mdelay(init->tag & LCM_TAG_MASK);
		}
		init++;
	}
	mipi_eotp_set(0,0);

	return 0;
}

static uint32_t s6e8aa5x01_readid(struct panel_spec *self)
	{
	uint8_t j =0;
	uint8_t read_data[4] = {0};
	int32_t read_rtn = 0;
	uint8_t param[2] = {0};
	mipi_set_cmd_mode_t mipi_set_cmd_mode = self->info.mipi->ops->mipi_set_cmd_mode;
	mipi_force_write_t mipi_force_write = self->info.mipi->ops->mipi_force_write;
	mipi_force_read_t mipi_force_read = self->info.mipi->ops->mipi_force_read;
	mipi_eotp_set_t mipi_eotp_set = self->info.mipi->ops->mipi_eotp_set;

	LCD_PRINT("lcd_s6e8aa5x01_mipi read id!\n");

	mipi_set_cmd_mode();
	mipi_eotp_set(0,0);

	for(j = 0; j < 4; j++){
		param[0] = 0x03;
		param[1] = 0x00;
		mipi_force_write(0x37, param, 2);
		read_rtn = mipi_force_read(0x04, 3, read_data);
		LCD_PRINT("lcd_s6e8aa5x01_mipi read id 0xda, 0xdb,0xdc is 0x%x,0x%x,0x%x!\n",
				read_data[0], read_data[1], read_data[2]);
		if ((0x40 == read_data[0]) && (0x00 == read_data[1])
				&& (0x02 == read_data[2])) {
			LCD_PRINT("lcd_s6e8aa5x01_mipi read 0x02 id success!\n");
			return 0x400002;
		} else if ((0x40 == read_data[0]) && (0x00 == read_data[1])
				&& (0x03 == read_data[2])) {
			LCD_PRINT("lcd_s6e8aa5x01_mipi read 0x03 id success!\n");
			return 0x400002;
		} else if ((0x41 == read_data[0]) && (0x00 == read_data[1])
				&& (0x03 == read_data[2])) {
			LCD_PRINT("lcd_s6e8aa5x01_mipi read 0x03 id success!\n");
			return 0x400002;
		}
	}

	mipi_eotp_set(0,0);

	LCD_PRINT("lcd_s6e8aa5x01_mipi read id failed!\n");
	return 0;

}


static int32_t s6e8aa5x01_enter_sleep(struct panel_spec *self, uint8_t is_sleep)
{
	int32_t i = 0;
	LCM_Init_Code *sleep_in_out = NULL;
	unsigned int tag;
	int32_t size = 0;

	mipi_set_cmd_mode_t mipi_set_cmd_mode = self->info.mipi->ops->mipi_set_cmd_mode;
	mipi_dcs_write_t mipi_dcs_write = self->info.mipi->ops->mipi_dcs_write;
	mipi_eotp_set_t mipi_eotp_set = self->info.mipi->ops->mipi_eotp_set;

	printk("kernel s6e8aa5x01_enter_sleep, is_sleep = %d\n", is_sleep);

	if(is_sleep){
		sleep_in_out = sleep_in;
		size = ARRAY_SIZE(sleep_in);
	}else{
		sleep_in_out = sleep_out;
		size = ARRAY_SIZE(sleep_out);
	}

	mipi_set_cmd_mode();
	mipi_eotp_set(0,0);

	for(i = 0; i <size ; i++){
		tag = (sleep_in_out->tag >>24);
		if(tag & LCM_TAG_SEND){
			mipi_dcs_write(sleep_in_out->data, (sleep_in_out->tag & LCM_TAG_MASK));
		}else if(tag & LCM_TAG_SLEEP){
			mdelay(sleep_in_out->tag & LCM_TAG_MASK);
		}
		sleep_in_out++;
	}
	mipi_eotp_set(0,0);

	return 0;
}

static struct panel_operations lcd_s6e8aa5x01_mipi_operations = {
	.panel_init = s6e8aa5x01_mipi_init,
	.panel_readid = s6e8aa5x01_readid,
	.panel_enter_sleep = s6e8aa5x01_enter_sleep,
};

#if 1
static struct timing_rgb lcd_s6e8aa5x01_mipi_timing = {
	.hfp = 226,  /* unit: pixel */
	.hbp = 100,
	.hsync = 20,
	.vfp = 14, /* unit: line */
	.vbp = 8,
	.vsync = 2,
};
#else
static struct timing_rgb lcd_s6e8aa5x01_mipi_timing = {
	.hfp = 84,  /* unit: pixel */
	.hbp = 90,
	.hsync = 40,
	.vfp = 14, /* unit: line */
	.vbp = 10,
	.vsync = 4,
};
#endif

static struct info_mipi lcd_s6e8aa5x01_mipi_info = {
	.work_mode  = SPRDFB_MIPI_MODE_VIDEO,
	.video_bus_width = 24, /*18,16*/
	.lan_number = 	4,
	.phy_feq =500*1000,
	.h_sync_pol = SPRDFB_POLARITY_POS,
	.v_sync_pol = SPRDFB_POLARITY_POS,
	.de_pol = SPRDFB_POLARITY_POS,
	.te_pol = SPRDFB_POLARITY_POS,
	.color_mode_pol = SPRDFB_POLARITY_NEG,
	.shut_down_pol = SPRDFB_POLARITY_NEG,
	.timing = &lcd_s6e8aa5x01_mipi_timing,
	.ops = NULL,
};

struct panel_spec lcd_s6e8aa5x01_mipi_spec = {
	.width = 720,
	.height = 1280,
//	.width_mm = 62,
//	.height_mm = 110,
	.fps	= 57,
	.type = LCD_MODE_DSI,
	.direction = LCD_DIRECT_NORMAL,
	.info = {
		.mipi = &lcd_s6e8aa5x01_mipi_info
	},
	.ops = &lcd_s6e8aa5x01_mipi_operations,
};

