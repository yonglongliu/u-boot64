#include <common.h>
#include <asm/io.h>

#include <regs_adi.h>
#include "adi_hal_internal.h"
#include <asm/arch/sprd_reg.h>
#include <asm/arch/sprd_eic.h>
#include <sprd_battery.h>
#include <chipram_env.h>
#include "sprd_chg_helper.h"
#ifdef DEBUG
#define debugf(fmt, args...) do { printf("%s(): ", __func__); printf(fmt, ##args); } while (0)
#else
#define debugf(fmt, args...)
#endif
#define ADC_CAL_TYPE_NO			0
#define ADC_CAL_TYPE_NV			1
#define ADC_CAL_TYPE_EFUSE		2

extern chipram_env_t* get_chipram_env(void);
extern int charger_connected(void);
extern int read_adc_calibration_data(char *buffer, int size);
extern int sci_efuse_calibration_get(unsigned int *p_cal_data);
extern int sprd_eic_request(unsigned offset);
extern int sprd_eic_get(unsigned offset);
extern void power_down_devices(unsigned pd_cmd);


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

void sprdchg_start_charge(void)	//TODO2731 all file
{

}

void sprdchg_stop_charge(void)
{

}

static void sprdchg_set_recharge(void)
{

}

void sprdchg_set_chg_cur(uint32_t chg_current)
{

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
	int ret = 0;

	adc_cal_flag = ADC_CAL_TYPE_NO;

#ifndef FDL_CHG_SP8830
	/* get voltage values from nv */
	ret = read_adc_calibration_data((char *)adc_data, 48);
	if ((ret > 0) &&
	    ((adc_data[2] & 0xFFFF) < 4500) && ((adc_data[2] & 0xFFFF) > 3000)
	    && ((adc_data[3] & 0xFFFF) < 4500)
	    && ((adc_data[3] & 0xFFFF) > 3000)) {
		debugf("adc_para from nv is 0x%x 0x%x \n", adc_data[2],
		       adc_data[3]);
		adc_voltage_table[0][1] = adc_data[2] & 0xFFFF;
		adc_voltage_table[0][0] = (adc_data[2] >> 16) & 0xFFFF;
		adc_voltage_table[1][1] = adc_data[3] & 0xFFFF;
		adc_voltage_table[1][0] = (adc_data[3] >> 16) & 0xFFFF;
		adc_cal_flag = ADC_CAL_TYPE_NV;
	}
#endif
	/* get voltage values from efuse */
	if (adc_cal_flag == ADC_CAL_TYPE_NO) {
		ret = sci_efuse_calibration_get(adc_data);
		if (ret > 0) {
			debugf("adc_para from efuse is 0x%x 0x%x \n",
			       adc_data[0], adc_data[1]);
			adc_voltage_table[0][1] = adc_data[0] & 0xFFFF;
			adc_voltage_table[0][0] = (adc_data[0] >> 16) & 0xFFFF;
			adc_voltage_table[1][1] = adc_data[1] & 0xFFFF;
			adc_voltage_table[1][0] = (adc_data[1] >> 16) & 0xFFFF;
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
#define REGS_FGU_BASE ANA_FGU_BASE
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

#define REG_FGU_USER_AREA_SET             SCI_ADDR(REGS_FGU_BASE, 0x00A0)
#define REG_FGU_USER_AREA_CLEAR             SCI_ADDR(REGS_FGU_BASE, 0x00A4)
#define REG_FGU_USER_AREA_STATUS             SCI_ADDR(REGS_FGU_BASE, 0x00A8)

#define BITS_POWERON_TYPE_SHIFT   12
#define BITS_POWERON_TYPE(_x_)           ( (_x_) << 12 & (0xF000))
#define BITS_RTC_AREA_SHIFT     0
#define BITS_RTC_AREA(_x_)           ( (_x_) << 0 & (0xFFF) )

#define FIRST_POWERTON  0xF
#define NORMAIL_POWERTON  0x5
#define WDG_POWERTON  0xA

static void sprdfgu_rtc_reg_write(uint32_t val)
{
	sci_adi_write(REG_FGU_USER_AREA_CLEAR, BITS_RTC_AREA(~val),
		      BITS_RTC_AREA(~0));
	sci_adi_write(REG_FGU_USER_AREA_SET, BITS_RTC_AREA(val),
		      BITS_RTC_AREA(~0));
}

static uint32_t sprdfgu_rtc_reg_read(void)
{
	return (sci_adi_read(REG_FGU_USER_AREA_STATUS) & BITS_RTC_AREA(~0)) >>
	    BITS_RTC_AREA_SHIFT;
}

static void sprdfgu_poweron_type_write(uint32_t val)
{
	sci_adi_write(REG_FGU_USER_AREA_CLEAR, BITS_POWERON_TYPE(~val),
		      BITS_POWERON_TYPE(~0));
	sci_adi_write(REG_FGU_USER_AREA_SET, BITS_POWERON_TYPE(val),
		      BITS_POWERON_TYPE(~0));
}

static uint32_t sprdfgu_poweron_type_read(void)
{
	return (sci_adi_read(REG_FGU_USER_AREA_STATUS) & BITS_POWERON_TYPE(~0))
	    >> BITS_POWERON_TYPE_SHIFT;
}

#define mdelay(_ms) udelay(_ms*1000)
unsigned int fgu_vol, fgu_cur;
void sprdfgu_init(void)
{

	sci_adi_set(ANA_REG_GLB_ARM_MODULE_EN, BIT_ANA_FGU_EN);
	sci_adi_set(ANA_REG_GLB_RTC_CLK_EN, BIT_RTC_FGU_EN | BIT_RTC_FGUA_EN);

	sci_adi_clr(REG_FGU_INT_EN, 0xFFFF);	//disable int after watchdog reset
	sci_adi_set(REG_FGU_INT_CLR, 0xFFFF);
	sci_adi_write(REG_FGU_CURT_OFFSET, 0, ~0);	//init offset after watchdog reset

	//mdelay(1000);

	fgu_vol = 0;		//sci_adi_read(REG_FGU_VOLT_VAL);

	fgu_cur = 0;		//sci_adi_read(REG_FGU_CURT_VAL);
	debugf("fgu_init fgu_vol 0x%x fgu_cur 0x%x \n", fgu_vol, fgu_cur);
}

int sprdbat_is_battery_connected(void)
{
	printf("eica status bat%x\n", sprd_eic_get(EIC_BATDET));

	return ! !sprd_eic_get(EIC_BATDET);
}

void sprdbat_init(void)
{
	boot_mode_t boot_role;
	chipram_env_t* cr_env = get_chipram_env();
	boot_role = cr_env->mode;
#ifdef CONFIG_SPRD_EXT_IC_POWER
	struct sprd_ext_ic_operations *sprd_ext_ic_op = sprd_get_ext_ic_ops();
	sprd_ext_ic_op->ic_init();
#endif

	sprd_eic_request(EIC_BATDET);	//enable battery detect eic

	//set charge current 500mA(USB) or 600mA(AC)
	if (charger_connected()&&(boot_role != BOOTLOADER_MODE_DOWNLOAD)) {
		enum sprd_adapter_type adp_type = sprdchg_charger_is_adapter();
		if (adp_type == ADP_TYPE_CDP || adp_type == ADP_TYPE_DCP) {
			debugf("uboot adp AC\n");
			sprdchg_set_chg_cur(700);
		} else {
			debugf("uboot adp USB\n");
			sprdchg_set_chg_cur(450);
		}
#ifdef CONFIG_SPRD_EXT_IC_POWER
			sprd_ext_ic_op->charge_start_ext(adp_type);
#endif
	}

	sprdbat_get_vbatauxadc_caldata();
	sprdfgu_init();

	if(boot_role == BOOTLOADER_MODE_DOWNLOAD)		//if it's BOOTLOADER_MODE_DONLOAD, donot check battery is connect or not, return.
		return;
//just for tem start
	if (!sprdbat_is_battery_connected()) {
		printf("battery unconnected shutdown charge!!!!!\n");
#ifdef CONFIG_SPRD_EXT_IC_POWER
		sprd_ext_ic_op->charge_stop_ext();
#else
		sprdchg_stop_charge();
#endif
	} 
	return ;
////just for tem end
	sprdchg_start_charge();
	//if battery do NOT connect, shutdown charge,maybe system poweroff
	if (!sprdbat_is_battery_connected()) {
		printf("battery unconnected shutdown charge!!!!!\n");
#ifdef CONFIG_SPRD_EXT_IC_POWER
		sprd_ext_ic_op->charge_stop_ext();
#else
		sprdchg_stop_charge();
#endif
	} else {
		if (charger_connected()) {
			if ((FIRST_POWERTON == sprdfgu_poweron_type_read())
			    || (sprdfgu_rtc_reg_read() == 0xFFF)) {
				sprdfgu_rtc_reg_write(0xFF);
				mdelay(1);
				sprdfgu_poweron_type_write(NORMAIL_POWERTON);
				printf("charge first poweron reset!!!!!\n");
				mdelay(3);
#ifdef CONFIG_SPRD_EXT_IC_POWER
				sprd_ext_ic_op->charge_stop_ext();
#else
				sprdchg_stop_charge();
#endif
				power_down_devices(0);
			}
		}
	}
	if (sprdfgu_rtc_reg_read() == 0xFF) {
		printf("secend poweron !!!!!\n");
		sprdfgu_poweron_type_write(FIRST_POWERTON);
		mdelay(1);
		sprdfgu_rtc_reg_write(0xFFF);
	}
}
#endif
