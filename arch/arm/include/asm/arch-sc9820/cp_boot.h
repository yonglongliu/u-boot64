#include <common.h>
#include <asm/arch/sprd_reg.h>




#define msleep(cnt) udelay(cnt*1000)
/*****************************************************************************/
//  Description:    Gets the current reset mode.
//  Author:         Andrew.Yang
//  Note:
/*****************************************************************************/
void pmic_arm7_RAM_active(void)
{
    u32 state;
    *((volatile u32*)REG_AON_APB_ARM7_SYS_SOFT_RST) |= BIT(0);   /* 0x402e0114*/
    msleep(50);
    *((volatile u32*)REG_PMU_APB_CP_SOFT_RST)|= BIT(8);   /* reset arm7*/
    msleep(50);
    *((volatile u32*)REG_PMU_APB_CP_SOFT_RST) &= ~BIT(8); /* clear arm7*/
    while(1)
    {
        state = *((volatile u32*)REG_PMU_APB_CP_SOFT_RST);
        if(!(state & BIT(8)))
          break;
    }
}

