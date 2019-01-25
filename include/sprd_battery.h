/*
 * Copyright (C) 2012 Spreadtrum Communications Inc.
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

#ifndef _SPRD_BATTERY_H_
#define _SPRD_BATTERY_H_
enum sprd_adapter_type {
	ADP_TYPE_UNKNOW = 0,	//unknow adapter type
	ADP_TYPE_CDP = 1,	//Charging Downstream Port,USB&standard charger
	ADP_TYPE_DCP = 2,	//Dedicated Charging Port, standard charger
	ADP_TYPE_SDP = 4,	//Standard Downstream Port,USB and nonstandard charge
};

void sprdchg_start_charge(void);
void sprdchg_stop_charge(void);
void sprdchg_set_chg_cur(uint32_t chg_current);
void sprdchg_lowbat_charge(void);
int sprdchg_charger_is_adapter(void);
uint16_t sprdbat_auxadc2vbatvol(uint16_t adcvalue);
uint32_t sprdbat_get_vbatauxadc_caltype(void);
void sprdbat_lateinit(void);
void sprdbat_get_vbatauxadc_caldata(void);
void sprdbat_init(void);
int sprdbat_is_battery_connected(void);
void sprdfgu_init(void);

#endif

