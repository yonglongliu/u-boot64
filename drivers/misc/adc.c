#include <common.h>
#include <asm/io.h>
#include "adc_reg_v3.h"
#include "adc_drvapi.h"
#include "adi_hal_internal.h"
#include <asm/arch/sprd_reg.h>
#include <linux/compat.h>
extern int sci_get_small_scale_adc_cal(unsigned int *p_cal_data);
extern int sci_efuse_calibration_get(unsigned int *p_cal_data);


struct sprd_auxadc_cal {
	uint16_t p0_vol;	//4.2V
	uint16_t p0_adc;
	uint16_t p1_vol;	//3.6V
	uint16_t p1_adc;
	uint16_t cal_type;
};
#define SPRD_AUXADC_CAL_NO         0
#define SPRD_AUXADC_CAL_CHIP      2
#define SPRDBIG_AUXADC_CAL_CHIP     3

struct sprd_auxadc_cal small_adc_cal = {
	1000, 3413,
	100, 341,
	SPRD_AUXADC_CAL_NO,
};

struct sprd_auxadc_cal big_adc_cal = {
	4200, 3310,
	3600, 2832,
	SPRD_AUXADC_CAL_NO,
};

static int __average(int a[], int N)
{
        int i, sum = 0;
        for (i = 0; i < N; i++)
                sum += a[i];
        return DIV_ROUND(sum, N);
}


void ADC_Init(void)
{
	ANA_REG_OR(ANA_REG_GLB_ARM_MODULE_EN, BIT_ANA_ADC_EN); //ADC enable
	ANA_REG_OR(ANA_REG_GLB_ARM_CLK_EN,    BIT_CLK_AUXAD_EN|BIT_CLK_AUXADC_EN); //enable auxad clock
	ANA_REG_OR(ANA_REG_GLB_XTL_WAIT_CTRL,    BIT_XTL_EN);	//enable clk
#if !(defined(CONFIG_SC9001))
	__raw_writel(__raw_readl(REG_AON_APB_SINDRV_CTRL) |BIT_SINDRV_ENA, REG_AON_APB_SINDRV_CTRL);	//enable ddie to adie clk
#endif
	ANA_REG_OR(ADC_CTRL, ADC_EN_BIT);
	ANA_REG_OR(ADC_CTRL, ADC_MODE_12B);
	sprd_get_adc_small_efuse_cal();
	sprd_get_adc_bat_efuse_cal();
}

void ADC_SetCs(adc_channel id)
{
    if(id >= ADC_MAX){
        sprd_err("adc limits to 0~%d\n", ADC_MAX);
        return;
    }

    ANA_REG_MSK_OR(ADC_CS, id, ADC_CS_BIT_MSK);
}

void ADC_SetScale(bool scale)
{
    if(ADC_SCALE_1V2 == scale)
    {
        ANA_REG_AND(ADC_CS, ~ADC_SCALE_BIT);
    }
    else if(ADC_SCALE_3V == scale)
    {
        ANA_REG_OR(ADC_CS, ADC_SCALE_BIT);
    }
    else
    {
        sprd_err("adc scale %d not support\n", scale);
    }
}

int32_t ADC_GetValues(adc_channel id, bool scale, uint8_t num, int32_t *p_buf)
{
	int32_t count,i;

	/* clear int */
	ANA_REG_OR(ADC_INT_CLR, ADC_IRQ_CLR_BIT);

	/* choose channel */
	ADC_SetCs(id);

	/* set ADC scale */
	ADC_SetScale(scale);

	/* set read numbers run ADC soft channel */
	if (num < 1) {
		return -1;
	}
	ANA_REG_MSK_OR(ADC_CTRL, BIT_SW_CH_RUN_NUM(num), SW_CH_NUM_MSK);
	ANA_REG_OR(ADC_CTRL, SW_CH_ON_BIT);

	/* wait adc complete */
	count = 1000;
	while(!(ANA_REG_GET(ADC_INT_SRC)&ADC_IRQ_RAW_BIT) && count--) {
		for (i =0; i < 0xff; i++);
	}
	if (count <= 0) {
		sprd_warning("WARNING: ADC_GetValue timeout....\n");
		return -1;
	}

	for (count = 0; count < num; count++) {
		p_buf[count] = ANA_REG_GET(ADC_DAT) & ADC_DATA_MSK;
	}

	ANA_REG_AND(ADC_CTRL, ~SW_CH_ON_BIT);			// turn off adc soft channel
	ANA_REG_MSK_OR(ADC_CTRL, BIT_SW_CH_RUN_NUM(1), SW_CH_NUM_MSK);
	ADC_SetCs(TPC_CHANNEL_X);						// set tpc channel x back
	ANA_REG_OR(ADC_INT_CLR, ADC_IRQ_CLR_BIT);		// clear irq of this time

	return 0;
}
int32_t ADC_GetValue(adc_channel id, bool scale)
{
	int32_t result;

	if (-1 == ADC_GetValues(id, scale, 1, &result)) {
		return -1;
	}

	return result;
}

static int sci_adc_set_current(u8 enable, int isen)
{
        if(enable) {
                /* BITS_AUXAD_CURRENT_IBS = (isen * 100 / 250 -1) (step from 125 to 250)*/
                isen = (isen * 100 / 250 -1);
                if(isen > BITS_AUXAD_CURRENT_IBS(-1))
                        isen = BITS_AUXAD_CURRENT_IBS(-1);
                sci_adi_write(ANA_REG_GLB_AUXAD_CTL, (BIT_AUXAD_CURRENTSEN_EN | BITS_AUXAD_CURRENT_IBS(isen)),
                        BIT_AUXAD_CURRENTSEN_EN | BITS_AUXAD_CURRENT_IBS(-1));
        } else {
                sci_adi_clr(ANA_REG_GLB_AUXAD_CTL, BIT_AUXAD_CURRENTSEN_EN);
        }

        return 0;
}

int sci_adc_get_value_by_isen(unsigned int channel, int scale, int isen)
{

        #define ADC_MESURE_NUMBER       15
        //extern int sci_adc_vol_request(int channel, int scale, int mux, int* adc);
        int results[ADC_MESURE_NUMBER + 1] = {0};
        int ret = 0, i = 0;
        int adc;
        /* Fixme: only external adc channel used */
        BUG_ON(channel > 3);

        WARN_ON(isen > 80);

        printf("isen value: %d\n", isen);
        sci_adc_set_current(1, isen);

        if(0 == ADC_GetValues(channel,scale,ADC_MESURE_NUMBER,results)) {
                ret = __average(&results[ADC_MESURE_NUMBER/5],
                        (ADC_MESURE_NUMBER - ADC_MESURE_NUMBER * 2 /5));
        }


        //ret = sci_adc_vol_request(channel , scale, 0, &adc);
        sci_adc_set_current(0, 0);

        for(i = 0; i < ARRAY_SIZE(results); i++) {
                printf("%d\t", results[i]);
        }

        printf("\n%s() adc[%d] value: %d\n", __func__, channel, ret);

        return ret;
}

int get_adc_value(int channel){
	extern uint16_t sprdbat_auxadc2vbatvol(uint16_t adcvalue);
	int vol = 0;
	uint32_t result;
	uint32_t res;
	//uint32_t channel_vol = sprd_adc_to_vol(adcvalue);
	uint32_t m, n;
	uint32_t bat_numerators, bat_denominators;
	uint32_t adc_channel_numerators, adc_channel_denominators;

	vol = sci_adc_get_value_by_isen(channel,1,20);
	vol = sprdbat_auxadc2vbatvol(vol);
	//res = (u32) sci_adc_ratio(5, 0, 0);
	bat_numerators = 7;//res >> 16;
	bat_denominators = 29;//res & 0xffff;
	//res = (u32) sci_adc_ratio(channel, 1, 0);
	adc_channel_numerators = 1;//res >> 16;
	adc_channel_denominators = 1;//res & 0xffff;

	vol = vol * bat_numerators * (adc_channel_denominators)/
		( bat_denominators * adc_channel_numerators);
	printf("result = %d\n",vol);
	if (vol <= 0 || vol >= 1200) {
		printf("vol is out of ranger [0~1200]\n");
		return -1;
	}

	if (vol >0 && vol <= 100)
		vol = 0;
	if (vol > 100 && vol <= 400 )
		vol = 1;
	if (vol > 400 && vol <= 800)
		vol = 2;
	if (vol > 800 && vol < 1200)
		vol = 3;
	return vol;

}

#if defined(CONFIG_ADIE_SC2723) || defined(CONFIG_ADIE_SC2723S)
#define RATIO(_n_, _d_) (_n_ << 16 | _d_)
static int sci_adc_ratio(int channel, int scale, int mux)
{
	switch (channel) {
	case ADC_CHANNEL_0:
		return RATIO(1, 1);
	case ADC_CHANNEL_1:
	case ADC_CHANNEL_2:
	case ADC_CHANNEL_3:
		return (scale ? RATIO(400, 1025) : RATIO(1, 1));
	case ADC_CHANNEL_VBAT:		//Vbat
	case ADC_CHANNEL_ISENSE:
		return RATIO(7, 29);
	case ADC_CHANNEL_VCHGSEN:
		return RATIO(77, 1024);
	case ADC_CHANNEL_DCDCCORE:  //dcdc core/arm/mem/gen/rf/con/wpa
		mux = mux >> 13;
		switch (mux) {
		case 1: //dcdcarm
		case 2: //dcdccore
			return (scale ? RATIO(36, 55) : RATIO(9, 11));
		case 3: //dcdcmem
		case 5: //dcdcrf
			return (scale ? RATIO(12, 25) : RATIO(3, 5));
		case 4: //dcdcgen
			return (scale ? RATIO(3, 10) : RATIO(3, 8));
		case 6: //dcdccon
			return (scale ? RATIO(9, 20) : RATIO(9, 16));
		case 7: //dcdwpa
			return (scale ? RATIO(12, 55) : RATIO(3, 11));
		default:
			return RATIO(1, 1);
		}
	case 0xE:	//LP dcxo
		return (scale ? RATIO(4, 5) : RATIO(1, 1));
	case 0x14:	//headmic
		return RATIO(1, 3);

	case ADC_CHANNEL_LDO0:		//dcdc supply LDO, vdd18/vddcamd/vddcamio/vddrf0/vddgen1/vddgen0
		return RATIO(1, 2);
	case ADC_CHANNEL_VBATBK:	//VbatBK
	case ADC_CHANNEL_LDO1:		//VbatD Domain LDO, vdd25/vddcama/vddsim2/vddsim1/vddsim0
	case ADC_CHANNEL_LDO2:		//VbatA Domain LDO,  vddwifipa/vddcammot/vddemmccore/vdddcxo/vddsdcore/vdd28
	case ADC_CHANNEL_WHTLED:	//kpled/vibr
	case ADC_CHANNEL_WHTLED_VFB://vddsdio/vddusb/vddfgu
	case ADC_CHANNEL_USBDP:		//DP from terminal
	case ADC_CHANNEL_USBDM:		//DM from terminal
		return RATIO(1, 3);

	default:
		return RATIO(1, 1);
	}
	return RATIO(1, 1);
}

void sci_adc_get_vol_ratio(unsigned int channel_id, int scale, unsigned int *div_numerators,
			   unsigned int *div_denominators)
{
	unsigned int ratio = sci_adc_ratio(channel_id, scale, 0);
	*div_numerators = ratio >> 16;
	*div_denominators = ratio << 16 >> 16;
}

unsigned int sci_adc_get_ratio(unsigned int channel_id, int scale, int mux)
{
	unsigned int ratio = (unsigned int)sci_adc_ratio(channel_id, scale, mux);

	return ratio;
}
#endif

void sprd_get_adc_small_efuse_cal(void)
{
	unsigned int efuse_cal_data[2] = { 0 };
	if (sci_get_small_scale_adc_cal(efuse_cal_data)) {
		small_adc_cal.p0_vol = efuse_cal_data[0] & 0xffff;
		small_adc_cal.p0_adc = (efuse_cal_data[0] >> 16) & 0xffff;
		small_adc_cal.p1_vol = efuse_cal_data[1] & 0xffff;
		small_adc_cal.p1_adc = (efuse_cal_data[1] >> 16) & 0xffff;
		small_adc_cal.cal_type = SPRD_AUXADC_CAL_CHIP;
	}
	printk("sprdchg_temp_adc_to_vol %d,%d,%d,%d,cal_type:%d\n",
	       small_adc_cal.p0_vol, small_adc_cal.p0_adc, small_adc_cal.p1_vol,
	       small_adc_cal.p1_adc, small_adc_cal.cal_type);
}

void sprd_get_adc_bat_efuse_cal(void)
{
	unsigned int efuse_cal_data[2] = { 0 };
	if (sci_efuse_calibration_get(efuse_cal_data)) {
        big_adc_cal.p0_vol = efuse_cal_data[0] & 0xffff;
	    big_adc_cal.p0_adc = (efuse_cal_data[0] >> 16) & 0xffff;
		big_adc_cal.p1_vol = efuse_cal_data[1] & 0xffff;
		big_adc_cal.p1_adc = (efuse_cal_data[1] >> 16) & 0xffff;
		big_adc_cal.cal_type = SPRDBIG_AUXADC_CAL_CHIP;
	}

	printk("sprd big adc cal %d,%d,%d,%d,cal_type:%d\n",
	       big_adc_cal.p0_vol, big_adc_cal.p0_adc, big_adc_cal.p1_vol,
	       big_adc_cal.p1_adc, big_adc_cal.cal_type);
}

uint16_t sprdadc_small_scale_to_vol(uint16_t adcvalue)
{
	int32_t vol;
	vol = small_adc_cal.p0_vol - small_adc_cal.p1_vol;
	vol = vol * (adcvalue - small_adc_cal.p0_adc);
	vol = vol / (small_adc_cal.p0_adc - small_adc_cal.p1_adc);
	vol = vol + small_adc_cal.p0_vol;

	return vol;
}

uint32_t sprd_bat_adc_to_vol(uint16_t adcvalue)
{
	int32_t vol;

	vol = big_adc_cal.p0_vol - big_adc_cal.p1_vol;
	vol = vol * (adcvalue - big_adc_cal.p0_adc);
	vol = vol / (big_adc_cal.p0_adc - big_adc_cal.p1_adc);
	vol = vol + big_adc_cal.p0_vol;

	return vol;
}


#if defined(CONFIG_ADIE_SC2723S) ||defined(CONFIG_ADIE_SC2723)
uint16_t sprd_chan_bat_adc_to_vol(uint16_t channel, int scale,
				   int32_t adcvalue)
{
	uint32_t result;
	uint32_t vol;
	uint32_t m, n;
	uint32_t bat_numerators, bat_denominators;
	uint32_t numerators, denominators;

	vol = sprd_bat_adc_to_vol(adcvalue);
	sci_adc_get_vol_ratio(ADC_CHANNEL_VBAT, 0, &bat_numerators,
			      &bat_denominators);

	sci_adc_get_vol_ratio(channel, scale, &numerators, &denominators);

	///v1 = vbat_vol*0.268 = vol_bat_m * r2 /(r1+r2)
	n = bat_denominators * numerators;
	m = vol * bat_numerators * (denominators);
	result = (m + n / 2) / n;
	return result;
}

uint16_t sprd_small_adc_to_vol(uint16_t channel, int scale,
				   uint16_t adcvalue)
{
	uint32_t result;
	uint32_t vol;
	uint32_t m, n;
	uint32_t bat_numerators, bat_denominators;
	uint32_t numerators, denominators;
	vol = sprdadc_small_scale_to_vol(adcvalue);
	bat_numerators = 1;
	bat_denominators = 1;
	sci_adc_get_vol_ratio(channel, scale, &numerators, &denominators);

	///v1 = vbat_vol*0.268 = vol_bat_m * r2 /(r1+r2)
	n = bat_denominators * numerators;
	m = vol * bat_numerators * (denominators);
	result = (m + n / 2) / n;
	return result;
}
#endif
