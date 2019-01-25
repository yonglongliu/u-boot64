#include <common.h>
#include <asm/arch/hardware.h>
#include <asm/io.h>
#include <asm/arch/sprd_reg.h>

#define PWM_INDEX 2

#define PWM_PRESCALE (0x0000)
#define PWM_CNT (0x0004)
#define PWM_PAT_LOW (0x000C)
#define PWM_PAT_HIG (0x0010)

#define PWM_ENABLE (1 << 8)
#define PWM2_SCALE 0x0
#define PWM_REG_MSK 0xffff
#define PWM_MOD_MAX 0xff
#ifdef CONFIG_BACKLIGHT_WHTLED
#define DIMMING_PWD_BASE       (SPRD_MISC_BASE + 0x8020)
#define PD_PWM_BASE		DIMMING_PWD_BASE
#endif

static inline uint32_t pwm_read(int index, uint32_t reg)
{
	return __raw_readl(CTL_BASE_PWM + index * 0x20 + reg);
}

static void pwm_write(int index, uint32_t value, uint32_t reg)
{
	__raw_writel(value, CTL_BASE_PWM + index * 0x20 + reg);
}

void set_backlight(uint32_t brightness)
{

#ifndef CONFIG_BACKLIGHT_WHTLED
	int index = PWM_INDEX;

	__raw_bits_or((0x1 << 0), REG_AON_CLK_PWM0_CFG + index * 4);//ext_26m select

	if (0 == brightness) {
		pwm_write(index, 0, PWM_PRESCALE);
		printf("sprd backlight power off. pwm_index=%d  brightness=%d\n", index, brightness);
	} else {
		__raw_bits_or((0x1 << (index+4)), REG_AON_APB_APB_EB0); //PWMx EN

		pwm_write(index, PWM2_SCALE, PWM_PRESCALE);
		pwm_write(index, (brightness << 8) | PWM_MOD_MAX, PWM_CNT);
		pwm_write(index, PWM_REG_MSK, PWM_PAT_LOW);
		pwm_write(index, PWM_REG_MSK, PWM_PAT_HIG);
		pwm_write(index, PWM_ENABLE, PWM_PRESCALE);
		printf("sprd backlight power on. pwm_index=%d  brightness=%d\n", index, brightness);
	}

	return;
#else

	u32 bl_brightness, led_level;
	u32 reg_val;
	__raw_writel(0x9600,0x400388f0);
	if(0==brightness){
	__raw_writel(0x9601,0x400388f0);
	}else{
		bl_brightness = brightness& PWM_MOD_MAX;
		/*parallel mode*/
		led_level = (bl_brightness >> 2) & 0x3f;
		reg_val = __raw_readl((unsigned long)0x400388f0);
		reg_val &= ~(0x3f << 1);
		reg_val |= led_level << 1;
		__raw_writel(reg_val,0x400388f0);
		udelay(100);
		//__raw_writel(0x0,0x400388e4);
		//udelay(100);
		//__raw_writel(0x1f,0x40038990);
		//udelay(100);
		__raw_writel(0x332,0x400388e8);
		udelay(100);
		__raw_writel(0xc005,0x400388ec);
		udelay(100);
		//__raw_writel(0x8000,0x40038020);
		//udelay(100);
		__raw_writel(0x100,0x40038024);
		udelay(100);
		__raw_writel(0x190,0x40038028);
		udelay(100);
		__raw_writel(0xffff,0x4003802c);
		udelay(100);
		__raw_writel(0xffff,0x40038030);
		udelay(100);
		__raw_writel(0x8100,0x40038020);
		udelay(100);
	}
#endif
}
