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

/*
 *Version SP9830A-2_LA_SCH_V1.1.0_V1.0
 */

#include <asm/io.h>
#include <asm/arch/pinmap.h>
#include "pinmap_hw00.h"
#include "pinmap_hw01.h"
#ifdef CONFIG_HARDWARE_VERSION
extern unsigned int get_hw_version();
#endif
/*here is the adie pinmap such as 2723*/
static pinmap_t adie_pinmap[]={
{REG_PIN_ANA_EXT_XTL_EN0,		BITS_ANA_PIN_DS(1)|BIT_ANA_PIN_WPD|BIT_ANA_PIN_SLP_IE},
};

int  pin_init(void)
{
	int i;
	unsigned int hw_version = 0;
	unsigned int pinmap_cnt = 0;
	static pinmap_t *pinmap;
#ifdef CONFIG_HARDWARE_VERSION
	hw_version = get_hw_version();
#endif
	switch (hw_version){
		case 0:
			pinmap = pinmap_hw00;
			pinmap_cnt = sizeof(pinmap_hw00)/sizeof(pinmap_t);
			break;
		case 1:
		default:
			pinmap = pinmap_hw01;
			pinmap_cnt = sizeof(pinmap_hw01)/sizeof(pinmap_t);
			break;
	}
	for (i = 0; i < pinmap_cnt ; i++){
		__raw_writel((pinmap+i)->val,CTL_PIN_BASE + (pinmap+i)->reg);
	}

	for (i = 0; i < sizeof(adie_pinmap)/sizeof(adie_pinmap[0]); i++) {
		sci_adi_set(CTL_ANA_PIN_BASE + adie_pinmap[i].reg, adie_pinmap[i].val);
	}

	return 0;
}

