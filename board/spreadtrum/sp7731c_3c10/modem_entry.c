#include <common.h>
#include <asm/arch/sprd_reg.h>
#include "cp_boot.h"
#include "loader_common.h"

extern void boot_cp0(void);
extern void boot_cp2(void);
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
	}else {
		boot_cp0();
	}
#endif
	boot_cp2();
}
