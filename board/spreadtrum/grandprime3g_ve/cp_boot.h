#include <common.h>
#include <asm/arch/sprd_reg.h>

#define COOPERATE_PROCESSOR_RESET_ADDR    0x402b00b0
#define COOPERATE_PMU_CTRL0_ADDR         0x402b0044
#define COOPERATE_PMU_CTRL2_ADDR         0x402b0068
#define CORE_CP0                          BIT_0
#define CORE_CP2                          BIT_2
#define msleep(cnt) udelay(cnt*1000)
static u32 state;

void boot_cp0(void)
{
	u32 m_data[3] = {0xe59f0000, 0xe12fff10, MODEM_ADR};
	memcpy(CP0_CODE_COPY_ADR, m_data, sizeof(m_data));	  /* copy cp0 source code */

	*((volatile u32*)COOPERATE_PMU_CTRL0_ADDR) &= ~BIT_25;  /* open cp0 pmu controller*/
	msleep(50);
	*((volatile u32*)COOPERATE_PMU_CTRL0_ADDR) &= ~BIT_28;
	msleep(50);

	*((volatile u32*)COOPERATE_PROCESSOR_RESET_ADDR)|= CORE_CP0;	/* reset cp0 */
	*((volatile u32*)COOPERATE_PROCESSOR_RESET_ADDR) &= ~CORE_CP0; /* clear cp0 force shutdown */

	while(1)
	{
		state = *((volatile u32*)COOPERATE_PROCESSOR_RESET_ADDR);

		if(!(state & CORE_CP0))
			break;
	}


}



