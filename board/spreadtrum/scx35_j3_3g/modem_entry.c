#include <common.h>
#include <asm/arch/sprd_reg.h>
#include "cp_boot.h"
#include "loader_common.h"

extern void boot_cp0(void);
extern void sipc_addr_reset(void);


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
#endif
}

