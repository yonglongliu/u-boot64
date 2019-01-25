/*
 * Copyright (C) 2014 Spreadtrum Communications Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

/* IMPORTANT:
 *
 * TODO:
 */

#include <common.h>
#include <otp_helper.h>

#define BLK_WIDTH_OTP_EMEMORY			( 8 ) /* bit counts */
#define BLK_ADC_DETA_ABC_OTP			( 7 ) /* start block for ADC otp delta */

#if defined(CONFIG_ARCH_SCX35L)
#define BLK_UID_DETA_START			( 0 )
#define BLK_UID_DETA_END			( 1)
#endif

#define BLK_ADC_DETA                    ( 7 )
#define BASE_ADC_P0				711	//3.6V
#define BASE_ADC_P1				830	//4.2V
#define VOL_P0					3600
#define VOL_P1					4200
#define ADC_DATA_OFFSET			128

#ifndef BIT
#define BIT(x) (1<<x)
#endif

u32 efuse_read(int id,int blk_index)
{
    if(id == DDIE_EFUSE_ID){
        return __ddie_efuse_read(blk_index);
	}else if(id == ARM7_EFUSE_ID){
#if defined(CONFIG_SECURE_EFUSE)
		return __ddie_arm7_efuse_read( blk_index);
#else
		return 0;
#endif
    }else{
#if defined(CONFIG_ADIE_SC2723) || defined(CONFIG_ADIE_SC2723S)
        return __adie_efuse_read(blk_index);
#else
        return 0;
#endif
    }
}


int efuse_prog(int id,int blk_index,u32 val)
{
	if(id == DDIE_EFUSE_ID){
	return __ddie_efuse_prog(blk_index,val);
	}else if(id == ARM7_EFUSE_ID){
#if defined(CONFIG_SECURE_EFUSE)
		return __ddie_arm7_efuse_prog( blk_index, val);
#else
		return 0;
#endif
	}else{
#if defined(CONFIG_SECURE_EFUSE)
		return __ddie_arm7_efuse_prog( blk_index, val);
#else
		return 0;
#endif
	}
}
#if defined(CONFIG_ARCH_SCX35L)
void get_efuse_uid(void)
{
	unsigned int block0,block1;
	block0 = __ddie_efuse_read(BLK_UID_DETA_START);
	block1 = __ddie_efuse_read(BLK_UID_DETA_END);
    printf("efuse uid is %08x%08x\n", __func__, block0 ,block1);
}
#endif
int sci_efuse_calibration_get(unsigned int *p_cal_data)
{
	unsigned int deta;
	unsigned short adc_temp;

#if defined(CONFIG_ADIE_SC2723) || defined(CONFIG_ADIE_SC2723S)
	__adie_efuse_block_dump(); /* dump a-die efuse */

	/* verify the otp data of ememory written or not */
	adc_temp = (__adie_efuse_read(0) & (1 << 7));
	if (adc_temp)
		return 0;

	deta = __adie_efuse_read_bits(BLK_ADC_DETA_ABC_OTP * BLK_WIDTH_OTP_EMEMORY, 16);
#elif defined(CONFIG_SPX30G) || defined(CONFIG_ARCH_SCX35L)
    	__ddie_efuse_block_dump(); /* dump d-die efuse */

	deta = __ddie_efuse_read(BLK_ADC_DETA);
#else
	#warning "AuxADC CAL DETA need fixing"
#endif

	printf("%s() get efuse block %u, deta: 0x%08x\n", __func__, BLK_ADC_DETA, deta);

	deta &= 0xFFFFFF;

	if ((!deta) || (p_cal_data == NULL)) {
		return 0;
	}

	//adc 3.6V
	adc_temp = ((deta >> 8) & 0x00FF) + BASE_ADC_P0 - ADC_DATA_OFFSET;
	p_cal_data[1] = (VOL_P0) | ((adc_temp << 2) << 16);

	//adc 4.2V
	adc_temp = (deta & 0x00FF) + BASE_ADC_P1 - ADC_DATA_OFFSET;
	p_cal_data[0] = (VOL_P1) | ((adc_temp << 2) << 16);

	return 1;
}

#ifdef CONFIG_SPX20
#define BLK_USB_PHY_TUNE 12
int sci_efuse_usb_phy_tune_get(unsigned int *p_cal_data)
{
	unsigned int data, ret;

	data = __ddie_efuse_read(BLK_USB_PHY_TUNE);


	*p_cal_data = (data >> 16) & 0x1f;
	ret = !!(data & 1 << 30);
	printf("%s: usb phy tune is %s, tfhres: 0x%x\n",
		__func__, ret ? "OK" : "NOT OK", *p_cal_data);

	return ret;
}
#endif

#if defined(CONFIG_ARCH_SCX35L)
#define BLK_DHR_DETA                                    ( 7 )
#define BLK_DHR_DETA_SHARKLC		( 13 )
#define BLK_DHR_DETA_SHARKLS		( 12 )

int  sci_efuse_Dhryst_binning_get(int *cal)
{
	u32 data = 0;
	int Dhry_binning = 0;
	 /*sharklc chipid auto adapt*/
	 if(soc_is_sharklc()){
		data=__ddie_efuse_read(BLK_DHR_DETA_SHARKLC);
		Dhry_binning = (data >> 10) & 0x003F;
		printf("sharlc block %u, deta: 0x%08x\n", BLK_DHR_DETA_SHARKLC, Dhry_binning);
	}else if(soc_is_sharkls()){
		data=__ddie_efuse_read(BLK_DHR_DETA_SHARKLS);
		Dhry_binning = (data >> 4) & 0x003F;
		printf("sharkls block %u, deta: 0x%08x\n", BLK_DHR_DETA_SHARKLS, Dhry_binning);
	}else{
		data=__ddie_efuse_read(BLK_DHR_DETA);
		Dhry_binning = (data >> 4) & 0x003F;
		printf("sharkl block %u, deta: 0x%08x\n", BLK_DHR_DETA, Dhry_binning);
	}
	*cal = Dhry_binning;
	return 0;
}

#endif

#define BLK_TEMP_ADC_DETA		(9)
#define BASE_TEMP_ADC_P0				819	//1.0V
#define BASE_TEMP_ADC_P1				82	//0.1V
#define VOL_TEMP_P0					1000
#define VOL_TEMP_P1					100
#define ADC_DATA_OFFSET			128
int sci_get_small_scale_adc_cal(unsigned int *p_cal_data)
{
#if defined(CONFIG_ADIE_SC2723) || defined(CONFIG_ADIE_SC2723S)
	unsigned int deta = 0;
	unsigned short adc_temp = 0;

	/* verify the otp data of ememory written or not */
	adc_temp = (__adie_efuse_read(0) & BIT(7));
	if (adc_temp)
		return 0;

	deta = __adie_efuse_read_bits(BLK_TEMP_ADC_DETA * BLK_WIDTH_OTP_EMEMORY, 16);
	printf("%s() get efuse block %u, deta: 0x%08x\n", __func__, BLK_TEMP_ADC_DETA, deta);

	deta &= 0xFFFFFF; /* get BIT0 ~ BIT23) in block 7 */

	if ((!deta) || (p_cal_data == NULL)) {
		return 0;
	}
	//adc 0.1V
	adc_temp = ((deta & 0x00FF) + BASE_TEMP_ADC_P1 - ADC_DATA_OFFSET) * 4;
	printf("0.1V adc_temp =%d/0x%x\n",adc_temp,adc_temp);
	p_cal_data[1] = (VOL_TEMP_P1) | (adc_temp << 16);

	//adc 1.0V
	adc_temp =(( (deta >> 8) & 0x00FF) + BASE_TEMP_ADC_P0 - ADC_DATA_OFFSET ) * 4;
	printf("1.0V adc_temp =%d/0x%x\n",adc_temp,adc_temp);
	p_cal_data[0] = (VOL_TEMP_P0) | (adc_temp << 16);

	return 1;
#else
	return 0;
#endif
}

#ifdef CONFIG_ARCH_SCX20
#define BLK_BINNING_DETA	13
#define BINNING_CAL_BIT			(15)

int sci_efuse_binning_result_get(u32 *p_binning_data)
{
	u32 data=__ddie_efuse_read(BLK_BINNING_DETA);
	u32 binning = (data >> BINNING_CAL_BIT) & 0xF;
	if (binning ==0) {
		*p_binning_data  = 0;
		return -1;
	}
	*p_binning_data = binning ;
	return 0;
}
#endif

#if defined(CONFIG_ADIE_SC2723) || defined(CONFIG_ADIE_SC2723S)
#define BITSINDEX(b, o)	( (b) * 8 + (o) )
int sci_efuse_ib_trim_get(unsigned int *p_cal_data)
{
	unsigned int data,blk0;

	blk0 = __adie_efuse_read(0);
	if (blk0 & (1 << 7)) {
		return 0;
	}

	data = __adie_efuse_read_bits(BITSINDEX(15, 0), 7);
	*p_cal_data = data;
	return 1;
}
#endif
