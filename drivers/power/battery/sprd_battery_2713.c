#include <common.h>
#include <asm/io.h>

#include <regs_adi.h>
#include "adi_hal_internal.h"
#include <asm/arch/sprd_reg.h>
#include <sprd_battery.h>
#include <chipram_env.h>

#ifdef DEBUG
#define debugf(fmt, args...) do { printf("%s(): ", __func__); printf(fmt, ##args); } while (0)
#else
#define debugf(fmt, args...)
#endif
#define ADC_CAL_TYPE_NO			0
#define ADC_CAL_TYPE_NV			1
#define ADC_CAL_TYPE_EFUSE		2

extern int charger_connected(void);
extern int read_adc_calibration_data(char *buffer,int size);
extern int sci_efuse_calibration_get(unsigned int * p_cal_data);
uint16_t adc_voltage_table[2][2] = {
	{3310, 4200},
	{2832, 3600},
};

uint32_t adc_cal_flag = 0;

uint16_t sprdbat_auxadc2vbatvol(uint16_t adcvalue)
{
	int32_t temp;
	temp = adc_voltage_table[0][1] - adc_voltage_table[1][1];
	temp = temp * (adcvalue - adc_voltage_table[0][0]);
	temp = temp / (adc_voltage_table[0][0] - adc_voltage_table[1][0]);

	debugf("uboot battery voltage:%d,adc4200:%d,adc3600:%d\n",
	       temp + adc_voltage_table[0][1], adc_voltage_table[0][0],
	       adc_voltage_table[1][0]);

	return temp + adc_voltage_table[0][1];
}

void sprdchg_start_charge(void)
{
#if defined(CONFIG_ADIE_SC2723S) || defined(CONFIG_ADIE_SC2723)
	ANA_REG_MSK_OR(ANA_REG_GLB_CHGR_CTRL0, 0, BIT_CHGR_PD);
#else
	ANA_REG_MSK_OR(ANA_REG_GLB_CHGR_CTRL0, BIT_CHGR_PD_RTCCLR, (BIT_CHGR_PD_RTCCLR | BIT_CHGR_PD_RTCSET));
#endif
}

void sprdchg_stop_charge(void)
{
#if defined(CONFIG_ADIE_SC2723S) || defined(CONFIG_ADIE_SC2723)
	ANA_REG_MSK_OR(ANA_REG_GLB_CHGR_CTRL0, BIT_CHGR_PD, BIT_CHGR_PD);
#else
	ANA_REG_MSK_OR(ANA_REG_GLB_CHGR_CTRL0, BIT_CHGR_PD_RTCSET, (BIT_CHGR_PD_RTCCLR | BIT_CHGR_PD_RTCSET));
#endif
}

static void sprdchg_set_recharge(void)
{
	ANA_REG_OR(ANA_REG_GLB_CHGR_CTRL2, BIT_RECHG);
}
void sprdchg_set_chg_cur(uint32_t chg_current)
{
	uint32_t temp;

	if (chg_current < 1400) {
		temp = ((chg_current - 300) / 50);
	} else {
		temp = ((chg_current - 1400) / 100);
		temp += 0x16;
	}

	sci_adi_clr(ANA_REG_GLB_CHGR_CTRL2, BIT_CHGR_CC_EN);

	sci_adi_write(ANA_REG_GLB_CHGR_CTRL1,
		      BITS_CHGR_CC_I(temp), BITS_CHGR_CC_I(~0));

	sci_adi_set(ANA_REG_GLB_CHGR_CTRL2, BIT_CHGR_CC_EN);
}

uint32_t sprdbat_get_vbatauxadc_caltype(void)
{
	return adc_cal_flag;
}

#ifndef CONFIG_FDL2_BUILD
extern void sprd_ext_charger_init(void);
extern void sprd_ext_charger_uninit(void);
void sprdchg_lowbat_charge(void)
{
#ifdef CONFIG_SPRD_EXT_IC_POWER
	sprd_ext_charger_init();
#else
	return;
#endif
}

void sprdbat_lateinit(void)
{
#ifdef CONFIG_SPRD_EXT_IC_POWER
	sprd_ext_charger_uninit();
#else
	return;
#endif
}

#endif


#ifndef CONFIG_FDL1
/* used to get adc calibration data from nv or efuse */
void sprdbat_get_vbatauxadc_caldata(void)
{
	unsigned int adc_data[64];
	int ret=0;

	adc_cal_flag = ADC_CAL_TYPE_NO;

#ifndef FDL_CHG_SP8830
	/* get voltage values from nv */
	ret = read_adc_calibration_data((char *)adc_data,48);
	if((ret > 0) &&
			((adc_data[2] & 0xFFFF) < 4500 ) && ((adc_data[2] & 0xFFFF) > 3000) &&
			((adc_data[3] & 0xFFFF) < 4500 ) && ((adc_data[3] & 0xFFFF) > 3000)){
		debugf("adc_para from nv is 0x%x 0x%x \n",adc_data[2],adc_data[3]);
		adc_voltage_table[0][1]=adc_data[2] & 0xFFFF;
		adc_voltage_table[0][0]=(adc_data[2] >> 16) & 0xFFFF;
		adc_voltage_table[1][1]=adc_data[3] & 0xFFFF;
		adc_voltage_table[1][0]=(adc_data[3] >> 16) & 0xFFFF;
		adc_cal_flag = ADC_CAL_TYPE_NV;
	}
#endif
	/* get voltage values from efuse */
	if (adc_cal_flag == ADC_CAL_TYPE_NO){
		ret = sci_efuse_calibration_get(adc_data);
		if (ret > 0) {
			debugf("adc_para from efuse is 0x%x 0x%x \n",adc_data[0],adc_data[1]);
			adc_voltage_table[0][1]=adc_data[0] & 0xFFFF;
			adc_voltage_table[0][0]=(adc_data[0]>>16) & 0xFFFF;
			adc_voltage_table[1][1]=adc_data[1] & 0xFFFF;
			adc_voltage_table[1][0]=(adc_data[1] >> 16) & 0xFFFF;
			adc_cal_flag = ADC_CAL_TYPE_EFUSE;
		}
	}
}
#endif

#ifndef FDL_CHG_SP8830
int sprdchg_charger_is_adapter(void)
{
	int ret = ADP_TYPE_UNKNOW;
	int charger_status;
	charger_status = sci_adi_read(ANA_REG_GLB_CHGR_STATUS)
	    & (BIT_CDP_INT | BIT_DCP_INT | BIT_SDP_INT);

	switch (charger_status) {
	case BIT_CDP_INT:
		ret = ADP_TYPE_CDP;
		break;
	case BIT_DCP_INT:
		ret = ADP_TYPE_DCP;
		break;
	case BIT_SDP_INT:
		ret = ADP_TYPE_SDP;
		break;
	default:
		break;
	}
	return ret;
}

#define REGS_FGU_BASE SPRD_ANA_FPU_PHYS
#define REG_FGU_START                   SCI_ADDR(REGS_FGU_BASE, 0x0000)
#define REG_FGU_CONFIG                  SCI_ADDR(REGS_FGU_BASE, 0x0004)
#define REG_FGU_INT_EN                  SCI_ADDR(REGS_FGU_BASE, 0x0010)
#define REG_FGU_INT_CLR                 SCI_ADDR(REGS_FGU_BASE, 0x0014)
#define REG_FGU_VOLT_VAL                SCI_ADDR(REGS_FGU_BASE, 0x0020)
#define REG_FGU_OCV_VAL                 SCI_ADDR(REGS_FGU_BASE, 0x0024)
#define REG_FGU_POCV_VAL                SCI_ADDR(REGS_FGU_BASE, 0x0028)
#define REG_FGU_CURT_VAL                SCI_ADDR(REGS_FGU_BASE, 0x002c)
#define REG_FGU_CURT_OFFSET             SCI_ADDR(REGS_FGU_BASE, 0x0090)

#define BIT_VOLT_H_VALID                ( BIT(12) )
#define BITS_VOLT_DUTY(_x_)             ( (_x_) << 5 & (BIT(5)|BIT(6)) )

#define mdelay(_ms) udelay(_ms*1000)
unsigned int fgu_vol, fgu_cur;
void sprdfgu_init(void)
{
#if defined(CONFIG_ADIE_SC2723S) || defined(CONFIG_ADIE_SC2723)
	//sci_adi_clr(ANA_REG_GLB_CHGR_DET_FGU_CTRL, (BIT_SD_CHOP_CAP_EN|BIT_CHOP_EN));
#else
	//sci_adi_set(ANA_REG_GLB_MP_MISC_CTRL, (BIT(1)));
	sci_adi_write(ANA_REG_GLB_MP_MISC_CTRL, BIT(1), (BIT(1) | BIT(2)));
	sci_adi_write(ANA_REG_GLB_DCDC_CTRL2, (4 << 8), (7 << 8));
#endif
	sci_adi_set(ANA_REG_GLB_ARM_MODULE_EN, BIT_ANA_FGU_EN);
	sci_adi_set(ANA_REG_GLB_RTC_CLK_EN, BIT_RTC_FGU_EN | BIT_RTC_FGUA_EN);

#if 1
	{
		int i = 0;
		sci_adi_set(ANA_REG_GLB_ARM_RST, BIT_ANA_FGU_SOFT_RST);
		for (i = 0; i < 1000; i++) {;}
		sci_adi_clr(ANA_REG_GLB_ARM_RST, BIT_ANA_FGU_SOFT_RST);
	}
#endif
	//sci_adi_clr(REG_FGU_INT_EN, 0xFFFF);    //disable int after watchdog reset
	//sci_adi_set(REG_FGU_INT_CLR, 0xFFFF);
	//sci_adi_write(REG_FGU_CURT_OFFSET, 0, ~0);  //init offset after watchdog reset

	//sci_adi_clr(REG_FGU_CONFIG, BIT_VOLT_H_VALID);
	//sci_adi_clr(REG_FGU_CONFIG, BIT_AD1_ENABLE);
#if defined(CONFIG_ADIE_SC2723S) || defined(CONFIG_ADIE_SC2723)
	if(sci_get_adie_chip_id() < 0x2711A100) {
		sci_adi_write(REG_FGU_CONFIG, BITS_VOLT_DUTY(3), BITS_VOLT_DUTY(3)|BIT_VOLT_H_VALID);
	}
#else
	sci_adi_write(REG_FGU_CONFIG, BITS_VOLT_DUTY(3), BITS_VOLT_DUTY(3)|BIT_VOLT_H_VALID);
#endif
	//mdelay(1000);

	fgu_vol = 0; //sci_adi_read(REG_FGU_VOLT_VAL);

	fgu_cur = 0; //sci_adi_read(REG_FGU_CURT_VAL);
	debugf("fgu_init fgu_vol 0x%x fgu_cur 0x%x \n", fgu_vol, fgu_cur);
}

void sprdbat_init(void)
{
	boot_mode_t boot_role;
	chipram_env_t* cr_env = get_chipram_env();
	boot_role = cr_env->mode;

#ifdef  CONFIG_SHARK_PAD_HW_V102
	sprd_gpio_request(NULL,USB_CHG_EN);
	sprd_gpio_direction_output(NULL, USB_CHG_EN, 1);
	sprd_gpio_set(NULL, USB_CHG_EN, 1);
	mdelay(20);
	sprd_gpio_set(NULL, USB_CHG_EN, 0);
#endif
	//set charge current 500mA(USB) or 600mA(AC)
	if (charger_connected()) {
		enum sprd_adapter_type adp_type = sprdchg_charger_is_adapter();
		if (adp_type == ADP_TYPE_CDP || adp_type == ADP_TYPE_DCP) {
			debugf("uboot adp AC\n");
			sprdchg_set_chg_cur(700);
		} else {
			debugf("uboot adp USB\n");
			sprdchg_set_chg_cur(450);
		}
	}

	sprdchg_set_recharge();
	ANA_REG_OR(ANA_REG_GLB_CHGR_CTRL2, BIT_CHGR_CC_EN);

	sprdbat_get_vbatauxadc_caldata();
	sprdfgu_init();

	if(boot_role == BOOTLOADER_MODE_DOWNLOAD){
		return;
	} else {
		sprdchg_start_charge();
	}

}
#endif
