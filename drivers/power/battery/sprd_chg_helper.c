#include "sprd_chg_helper.h"

struct sprd_ext_ic_operations *sprd_ext_ic_op = NULL;
static int count = 0;
void sprd_ext_charger_init(void)
{
	int i = 0;
	enum sprd_adapter_type adp_type = sprdchg_charger_is_adapter();
	sprd_ext_ic_op = sprd_get_ext_ic_ops();
	if(sprd_ext_ic_op  == NULL){
		printf("sprd_ext_ic_op == NULL,return\n");
		return;
	}
	sprd_ext_ic_op->ic_init();
	sprd_ext_ic_op->charge_start_ext(adp_type);

	return;
}
void chg_low_bat_chg(void)
{
	count ++;
	if(count == 50){
		sprd_ext_ic_op = sprd_get_ext_ic_ops();
		count = 0;
		sprd_ext_ic_op->timer_callback_ext();
	}
}
