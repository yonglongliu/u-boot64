#include <common.h>
#include "adc_drvapi.h"
#include <boot_mode.h>

int charger_connected(void);

#define _BUF_SIZE 10
uint32_t vbat_buf[_BUF_SIZE];
void put_vbat_value(uint32_t vbat)
{
	int i;
	for (i = 0; i < _BUF_SIZE - 1; i++) {
		vbat_buf[i] = vbat_buf[i + 1];
	}

	vbat_buf[_BUF_SIZE - 1] = vbat;
}

uint32_t get_vbat_value(void)
{
	unsigned long sum = 0;
	int i;
	for (i = 0; i < _BUF_SIZE; i++)
		sum += vbat_buf[i];

	return sum / _BUF_SIZE;
}

int is_bat_low(void)
{
	int adc_value = 0;
	unsigned int comp_vbat = 0;
	int i;

	if (charger_connected()) {
		comp_vbat = LOW_BAT_VOL_CHG;
	} else {
		comp_vbat = LOW_BAT_VOL;
	}

	ADC_Init();

	for (i = 0; i < _BUF_SIZE; i++) {
retry_adc:
		adc_value = ADC_GetValue(ADC_CHANNEL_VBAT, false);
		if (adc_value < 0) {
			printf("ADC read error\n");
			udelay(10);
			goto retry_adc;
		} else {
			put_vbat_value(adc_value);
		}
	}

	adc_value = get_vbat_value();

	printf("is_bat_low adc_value:%d,comp_vbat:%d\n", adc_value, comp_vbat);

	if (sprdbat_auxadc2vbatvol(adc_value) < comp_vbat)
		return 1;
	else
		return 0;

}
