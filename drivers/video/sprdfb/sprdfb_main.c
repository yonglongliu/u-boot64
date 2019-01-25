/******************************************************************************
 ** File Name:    sprdfb_main.h                                            *
 ** Author:                                                           *
 ** DATE:                                                           *
 ** Copyright:    2005 Spreatrum, Incoporated. All Rights Reserved.           *
 ** Description:                                                            *
 ******************************************************************************/
/******************************************************************************
 **                   Edit    History                                         *
 **---------------------------------------------------------------------------*
 ** DATE          NAME            DESCRIPTION                                 *
 **
 ******************************************************************************/

#include "sprdfb_chip_common.h"
#include "sprdfb.h"
#include "sprdfb_panel.h"
#include <malloc.h>


#define BMP_RESERVED_ADDR(p, sz) ((p) + ALIGN((sz), PAGE_SIZE))

extern int sprdfb_panel_probe(struct sprdfb_device *dev);
extern void sprdfb_panel_remove(struct sprdfb_device *dev);
#ifdef CONFIG_FB_SWDISPC
extern struct display_ctrl sprdfb_swdispc_ctrl;
#else
extern struct display_ctrl sprdfb_dispc_ctrl;
#endif
static struct sprdfb_device s_sprdfb_dev = {0};
static uint32_t lcd_id_to_kernel = 0;


void save_lcd_id_to_kernel(uint32_t id)
{
	lcd_id_to_kernel = id;
}

uint32_t load_lcd_id_to_kernel(void)
{
	return lcd_id_to_kernel;
}

void *lcd_get_base_addr(void *lcd_base)
{
	static void *base = NULL;

	if (!base)
		base = lcd_base;

	return base;
}

static int real_refresh(struct sprdfb_device *dev)
{
	int32_t ret;

	FB_PRINT("sprdfb: [%s]\n", __FUNCTION__);

	if(NULL == dev->panel){
		printf("sprdfb: [%s] fail (no panel!)\n", __FUNCTION__);
		return -1;
	}
	if(dev->ctrl->refresh)
		ret = dev->ctrl->refresh(dev);
	if (ret) {
		printf("sprdfb: failed to refresh !!!!\n");
		return -1;
	}

	return 0;
}

static int sprdfb_probe(void *lcdbase)
{
	struct sprdfb_device *dev = &s_sprdfb_dev;

	printf("sprdfb:[%s]\n", __FUNCTION__);
#ifdef CONFIG_SC9001
	misc_noc_ctrl(REG_AON_APB_DISP_NOC_CTRL, NORMAL_MODE, SIZE_64);
#endif
	set_backlight(0);

#ifdef CONFIG_GRANDPRIME3G_VE
	sprd_gpio_request(NULL, 214);
	sprd_gpio_direction_output(NULL, 214, 0);
	sprd_gpio_set(NULL, 214, 1);
	sprd_gpio_request(NULL, 167);
	sprd_gpio_direction_output(NULL, 167, 0);
	sprd_gpio_set(NULL, 167, 1);
#endif

#ifdef CONFIG_FB_SWDISPC
	dev->ctrl = &sprdfb_swdispc_ctrl;
#else
	dev->ctrl = &sprdfb_dispc_ctrl;
#endif
	if(dev->ctrl->early_init)
		dev->ctrl->early_init(dev);

	if (0 != sprdfb_panel_probe(dev)) {
		sprdfb_panel_remove(dev);
		if(dev->ctrl->uninit)
			dev->ctrl->uninit(dev);
		printf("sprdfb: failed to probe\n");
		return -EFAULT;
	}

#ifdef CONFIG_FB_LOW_RES_SIMU
#if (defined LCD_DISPLAY_WIDTH) && (defined LCD_DISPLAY_HEIGHT)
	dev->display_width = LCD_DISPLAY_WIDTH;
	dev->display_height = LCD_DISPLAY_HEIGHT;
#else
	dev->display_width = dev->panel->width;
	dev->display_height = dev->panel->height;
#endif
#endif

	lcd_get_base_addr(lcdbase);
	dev->smem_start = lcdbase;
	if(dev->ctrl->init)
		dev->ctrl->init(dev);
	printf("sprdfb:[%s]---\n", __FUNCTION__);
	return 0;
}

void lcd_disable(void)
{
	printf("sprdfb: [%s]\n", __FUNCTION__);
	sprdfb_panel_remove(&s_sprdfb_dev);
	if(s_sprdfb_dev.ctrl->uninit)
		s_sprdfb_dev.ctrl->uninit(&s_sprdfb_dev);
}

void lcd_enable(void)
{
	printf("sprdfb: [%s]\n", __FUNCTION__);
	real_refresh(&s_sprdfb_dev);
}

void lcd_ctrl_init(void *lcdbase)
{
	int size;
	void *bmp;
	char *s;
	ulong addr;
	ulong all_sz;

	printf("sprdfb: %s: lcdbase is 0x%p\n", __FUNCTION__, lcdbase);
	all_sz = CONFIG_SYS_MALLOC_LEN >> 1;
	size = lcd_get_size(&lcd_line_length);
	bmp = malloc(all_sz < SZ_1M ? all_sz : SZ_1M);
	if (!bmp) {
		printf("sprdfb: %s: failed to alloc bmp space\n", __FUNCTION__);
		return;
	}
	setenv_hex("splashimage", (ulong)bmp);
	s = getenv("splashimage");
	addr = simple_strtoul(s, NULL, 16);
	printf("sprdfb: splashimage addr is 0x%p, fb size is 0x%x\n",
			(void *)addr, size);
	lcd_set_flush_dcache(TRUE);
	sprdfb_probe(lcdbase);
	printf("sprdfb:[%s]---\n", __FUNCTION__);
}

#ifdef CONFIG_LCD_INFO
#include <nand.h>
extern nand_info_t nand_info[];

void lcd_show_board_info(void)
{
    ulong dram_size, nand_size;
    int i;
    char temp[32];

    dram_size = 0;
    for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++)
      dram_size += gd->bd->bi_dram[i].size;
    nand_size = 0;
    for (i = 0; i < CONFIG_SYS_MAX_NAND_DEVICE; i++)
      nand_size += nand_info[i].size;

    lcd_printf("\n%s\n", U_BOOT_VERSION);
    lcd_printf("  %ld MB SDRAM, %ld MB NAND\n",
                dram_size >> 20,
                nand_size >> 20 );
    lcd_printf("  Board            : esd ARM9 \n");
    lcd_printf("  Mach-type        : %lu\n", gd->bd->bi_arch_number);
}
#endif /* CONFIG_LCD_INFO */

/* dummy function */
void lcd_setcolreg(ushort regno, ushort red, ushort green, ushort blue)
{
	return;
}
int32_t sprdfb_spi_refresh(void *base)
{
	struct sprdfb_device * dev = &s_sprdfb_dev;
	if(!dev->panel)
	{
		printf("sprdfb:sprdfb_refresh dev->panel is null\n");
		return -1;
	}
	dev->panel->ops->panel_invalidate(dev->panel);
	dev->panel->ops->panel_refresh(dev->panel,base);
	return 0;
}
