#include "smcall.h"

uint32_t uboot_tee_call(uint32_t fnid, uint32_t param1,
			 uint32_t param2, uint32_t param3)
{
	uint32_t ret;
	smc32_args_t args;
	args.param0= fnid;
	args.param1 = param1;
	args.param2 = param2;
	args.param3 = param3;

	tee_smc_call(&args);
	ret = args.param0;
	return ret;
}
