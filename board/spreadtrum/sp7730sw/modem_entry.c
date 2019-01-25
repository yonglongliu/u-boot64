#include <common.h>
#include <asm/arch/sprd_reg.h>
#include "cp_boot.h"
#include "loader_common.h"

extern void boot_cp0(void);
extern void boot_cp2(void);
extern void sipc_addr_reset();
void sipc_addr_reset()
{
        memset((void *)SIPC_WCDMA_APCP_START_ADDR, 0x0, SIPC_APCP_RESET_ADDR_SIZE);
        memset((void *)SIPC_WCN_APCP_START_ADDR, 0x0, SIPC_APCP_RESET_ADDR_SIZE);
}
void modem_entry(void)
{
#if !defined( CONFIG_KERNEL_BOOT_CP )
	sipc_addr_reset();

	/*if recovery mode and not cali mode, skip boot modem image*/
	if (strstr(getenv("bootmode"), "recovery")
			&& NULL == get_calibration_parameter()) {
		debugf("recovery mode, skip boot modem!");
		return;
	}

	boot_cp0();
#else
	/* if open CONFIG_KERNEL_BOOT_CP, in charge mode, It will not boot cp0 ,
	    so we must set  BIT_CP0_FORCE_SLEEP, other wise the cp0 can't enter sleep*/
	char *value = NULL;
	value = getenv("bootmode");
	if(NULL != strstr(value, "charger")){
		CHIP_REG_OR(REG_PMU_APB_SLEEP_CTRL, BIT_CP0_FORCE_SLEEP);
	}

#endif
}
