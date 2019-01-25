/* drivers/video/sc8810/lcd_gc9305_spi.c
 *
 *
 *
 *
 * Copyright (C) 2010 Spreadtrum
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

#include <asm/arch/sprd_lcd.h>
#include <spi.h>
#define printk printf

#define  LCD_DEBUG
#ifdef LCD_DEBUG
#define LCD_PRINT printk
#else
#define LCD_PRINT(...)
#endif
#define REG32(x)              (*((volatile uint32 *)(x)))
#define GC9305_SpiWriteCmd(cmd) \
{ \
	spi_send_cmd((cmd & 0xFF));\
}

#define  GC9305_SpiWriteData(data)\
{ \
	spi_send_data((data & 0xFF));\
}

#define  GC9305_SpiRead(data,len)\
{ \
	spi_read(data,len);\
}
#define BGRA32toRBG565(b,g,r)		((((r>>3)&0x1f)<<11)|(((g>>2)&0x3f)<<5)|(((b>>3)&0x1f)<<0))
#define LCM_GPIO_RSTN	(50)
#define GC9305_SPI_SPEED 		(48*1000*1000UL)
static int32_t gc9305_reset(void)
{
	static int32_t is_first_run = 1;
	if(is_first_run)
	{
		sprd_gpio_request(NULL,LCM_GPIO_RSTN);
		sprd_gpio_direction_output(NULL,LCM_GPIO_RSTN,1);
		is_first_run = 0;
	}
	sprd_gpio_set(NULL,LCM_GPIO_RSTN,1);
	mdelay(10);
	sprd_gpio_set(NULL,LCM_GPIO_RSTN,0);
	mdelay(10);
	sprd_gpio_set(NULL,LCM_GPIO_RSTN,1);
	mdelay(20);
}
static int32_t gc9305_refresh(struct panel_spec *self,void *base)
{
	int i = 0;
	uint16_t *prgb = (uint16_t *)base;
	uint16_t rgb;
	LCD_PRINT("gc9305_freshbuffer\r\n");
	spi_send_cmd_t spi_send_cmd = self->info.spi->ops->spi_send_cmd;
	spi_send_data_t spi_send_data = self->info.spi->ops->spi_send_data;
	spi_read_t spi_read = self->info.spi->ops->spi_read;

	for(i = 0 ; i < 240*320;i++)
	{
		rgb = *prgb++;
		GC9305_SpiWriteData(rgb>>8);
		GC9305_SpiWriteData(rgb&0xff);
	}
	GC9305_SpiWriteCmd(0x29);
	return 0;
}

static int32_t gc9305_init(struct panel_spec *self)
{
	uint32_t data = 0;
	spi_send_cmd_t spi_send_cmd = self->info.spi->ops->spi_send_cmd;
	spi_send_data_t spi_send_data = self->info.spi->ops->spi_send_data;
	spi_read_t spi_read = self->info.spi->ops->spi_read;
	gc9305_reset();
	GC9305_SpiWriteCmd(0xfe);
	GC9305_SpiWriteCmd(0xef);


	GC9305_SpiWriteCmd(0x36);
	GC9305_SpiWriteData(0x48);

	GC9305_SpiWriteCmd(0x3a);
	GC9305_SpiWriteData(0x05);

	GC9305_SpiWriteCmd(0xa4);
	GC9305_SpiWriteData(0x44);
	GC9305_SpiWriteData(0x44);

	GC9305_SpiWriteCmd(0xa5);
	GC9305_SpiWriteData(0x42);
	GC9305_SpiWriteData(0x42);

	GC9305_SpiWriteCmd(0xaa);
	GC9305_SpiWriteData(0x88);
	GC9305_SpiWriteData(0x88);

	GC9305_SpiWriteCmd(0xe8);
	GC9305_SpiWriteData(0x11);//16
	GC9305_SpiWriteData(0x0b);

	GC9305_SpiWriteCmd(0xe3);
	GC9305_SpiWriteData(0x01);
	GC9305_SpiWriteData(0x10);

//	GC9305_SpiWriteCmd(0xe1);
	//GC9305_SpiWriteData(0x10);
	//GC9305_SpiWriteData(0x0a);

	GC9305_SpiWriteCmd(0xff);
	GC9305_SpiWriteData(0x61);


	GC9305_SpiWriteCmd(0xAC);
	GC9305_SpiWriteData(0x00);

	GC9305_SpiWriteCmd(0xAf);
	GC9305_SpiWriteData(0x55);

	GC9305_SpiWriteCmd(0xAE);
	GC9305_SpiWriteData(0x2B);

	GC9305_SpiWriteCmd(0xAD);
	GC9305_SpiWriteData(0x33);

	//GC9305_SpiWriteCmd(0xFD);
	//GC9305_SpiWriteData(0x00);

	GC9305_SpiWriteCmd(0xa6);
	GC9305_SpiWriteData(0x2a);//29
	GC9305_SpiWriteData(0x2a);

	GC9305_SpiWriteCmd(0xa7);
	GC9305_SpiWriteData(0x2b);//27
	GC9305_SpiWriteData(0x2b);

	GC9305_SpiWriteCmd(0xa8);
	GC9305_SpiWriteData(0x18);//17
	GC9305_SpiWriteData(0x18);

	GC9305_SpiWriteCmd(0xa9);
	GC9305_SpiWriteData(0x2a);//26
	GC9305_SpiWriteData(0x2a);
	//----------------------------display window 240X320------------------------------------//
	GC9305_SpiWriteCmd(0x2a);
	GC9305_SpiWriteData(0x00);
	GC9305_SpiWriteData(0x00);
	GC9305_SpiWriteData(0x00);
	GC9305_SpiWriteData(0xef);
	
	GC9305_SpiWriteCmd(0x2b);
	GC9305_SpiWriteData(0x00);	
	GC9305_SpiWriteData(0x00);
	GC9305_SpiWriteData(0x01);
	GC9305_SpiWriteData(0x3f);
	GC9305_SpiWriteCmd(0x2c);

	//----gamma setting---------//
	GC9305_SpiWriteCmd(0xf0);
	GC9305_SpiWriteData(0x02);
	GC9305_SpiWriteData(0x01);
	GC9305_SpiWriteData(0x00);
	GC9305_SpiWriteData(0x01);
	GC9305_SpiWriteData(0x05);
	GC9305_SpiWriteData(0x09);

	GC9305_SpiWriteCmd(0xf1);
	GC9305_SpiWriteData(0x01);
	GC9305_SpiWriteData(0x01);
	GC9305_SpiWriteData(0x00);
	GC9305_SpiWriteData(0x05);
	GC9305_SpiWriteData(0x09);
	GC9305_SpiWriteData(0x0f);

	GC9305_SpiWriteCmd(0xf2);
	GC9305_SpiWriteData(0x0b);
	GC9305_SpiWriteData(0x07);
	GC9305_SpiWriteData(0x35);
	GC9305_SpiWriteData(0x03);
	GC9305_SpiWriteData(0x03);
	GC9305_SpiWriteData(0x49);

	GC9305_SpiWriteCmd(0xf3);
	GC9305_SpiWriteData(0x11);
	GC9305_SpiWriteData(0x0d);
	GC9305_SpiWriteData(0x3e);
	GC9305_SpiWriteData(0x07);
	GC9305_SpiWriteData(0x06);
	GC9305_SpiWriteData(0x4e);

	GC9305_SpiWriteCmd(0xf4);
	GC9305_SpiWriteData(0x09);
	GC9305_SpiWriteData(0x15);
	GC9305_SpiWriteData(0x14);
	GC9305_SpiWriteData(0x22);
	GC9305_SpiWriteData(0x3a);
	GC9305_SpiWriteData(0x0F);

	GC9305_SpiWriteCmd(0xf5);
	GC9305_SpiWriteData(0x09);
	GC9305_SpiWriteData(0x11);
	GC9305_SpiWriteData(0xf);
	GC9305_SpiWriteData(0x1d);
	GC9305_SpiWriteData(0x3a);
	GC9305_SpiWriteData(0x0F);
	//---end gamma setting-----//


	GC9305_SpiWriteCmd(0x35);
	GC9305_SpiWriteData(0x00);
	
	GC9305_SpiWriteCmd(0x44);
	GC9305_SpiWriteData(0x00);
	GC9305_SpiWriteData(0x40);//48 10 18 58
	
	
	GC9305_SpiWriteCmd(0x11);
	mdelay(120);
	GC9305_SpiWriteCmd(0x29);
	GC9305_SpiWriteCmd(0x2c);

//	GC9305_SpiWriteCmd(0x29);
//	GC9305_SpiWriteCmd(0x2c);

	LCD_PRINT("gc9305_init\n");
}

static int32_t gc9305_enter_sleep(struct panel_spec *self, uint8_t is_sleep)
{
	spi_send_cmd_t spi_send_cmd = self->info.spi->ops->spi_send_cmd;
	spi_send_data_t spi_send_data = self->info.spi->ops->spi_send_data;
	spi_read_t spi_read = self->info.spi->ops->spi_read;

	GC9305_SpiWriteCmd(0xfe);
	GC9305_SpiWriteCmd(0xef);

	if(is_sleep==1){
		//Sleep In
		GC9305_SpiWriteCmd(0x28);
		mdelay(120);
		GC9305_SpiWriteCmd(0x10);
		mdelay(15);
	}else{
		//Sleep Out
		GC9305_SpiWriteCmd(0x11);
		mdelay(120);
		GC9305_SpiWriteCmd(0x29);
		mdelay(10);
	}
	return 0;
}



static int32_t gc9305_set_window(struct panel_spec *self,
		uint16_t left, uint16_t top, uint16_t right, uint16_t bottom)
{
	uint32_t *test_data[4] = {0};
	spi_send_cmd_t spi_send_cmd = self->info.spi->ops->spi_send_cmd;
	spi_send_data_t spi_send_data = self->info.spi->ops->spi_send_data;
	spi_read_t spi_read = self->info.spi->ops->spi_read;


	GC9305_SpiWriteCmd(0x2A);
	GC9305_SpiWriteData((left>>8));// set left address
	GC9305_SpiWriteData((left&0xff));
	GC9305_SpiWriteData((right>>8));// set right address
	GC9305_SpiWriteData((right&0xff));

	GC9305_SpiWriteCmd(0x2B);
	GC9305_SpiWriteData((top>>8));// set left address
	GC9305_SpiWriteData((top&0xff));
	GC9305_SpiWriteData((bottom>>8));// set bottom address
	GC9305_SpiWriteData((bottom&0xff));
	GC9305_SpiWriteCmd(0x2C);

	return 0;
}
static int32_t gc9305_invalidate(struct panel_spec *self)
{
	LCD_PRINT("gc9305_invalidate\n");

	return self->ops->panel_set_window(self, 0, 0,
		self->width - 1, self->height - 1);
}



static int32_t gc9305_invalidate_rect(struct panel_spec *self,
				uint16_t left, uint16_t top,
				uint16_t right, uint16_t bottom)
{
	LCD_PRINT("gc9305_invalidate_rect \n");

	return self->ops->panel_set_window(self, left, top,
			right, bottom);
}
static int32_t gc9305_read_id(struct panel_spec *self)
{
	LCD_PRINT("sprdfb:gc9305_read_id\n");
	spi_send_cmd_t spi_send_cmd = self->info.spi->ops->spi_send_cmd;
	spi_send_data_t spi_send_data = self->info.spi->ops->spi_send_data;
	spi_read_t spi_read = self->info.spi->ops->spi_read;
	uint32_t lcm_id[4]={0};
	gc9305_reset();
	GC9305_SpiWriteCmd(0xFE);
	GC9305_SpiWriteCmd(0xEF);
	GC9305_SpiWriteCmd(0x04);
	GC9305_SpiRead(lcm_id,4);
	LCD_PRINT("sprdfb:gc9305_read_id lcm id[0-3] = 0x%x %x %x %x\n",lcm_id[0],lcm_id[1],lcm_id[2],lcm_id[3]);
	if(lcm_id[1] == 0x93 && lcm_id[2] == 0x05)
		return 0x93051;
	return 0x00;
}

static struct panel_operations lcd_gc9305_spi_operations = {
	.panel_init = gc9305_init,
	.panel_set_window = gc9305_set_window,
	.panel_invalidate_rect= gc9305_invalidate_rect,
	.panel_invalidate = gc9305_invalidate,
	.panel_enter_sleep = gc9305_enter_sleep,
	.panel_readid          = gc9305_read_id,
	.panel_refresh		=gc9305_refresh,
};

static struct info_spi lcd_gc9305_spi_info = {
	.bus_num = 0,
	.cs = 0,
	.cd_gpio = 53,
	.spi_mode = 1,
	.spi_pol_mode = SPI_MODE_0,
	.speed = GC9305_SPI_SPEED,
};

struct panel_spec lcd_gc9305_1_spi_spec = {
	.width = 240,
	.height = 320,
	.fps = 35,
	.type = SPRDFB_PANEL_TYPE_SPI,
	.direction = LCD_DIRECT_NORMAL,
	.info = {
		.spi = &lcd_gc9305_spi_info
	},
	.ops = &lcd_gc9305_spi_operations,
};
