#include <asm/arch/sci_types.h>
#include <asm/arch/os_api.h>
#include <sprd_battery.h>
#include <common.h>
#include <i2c.h>
typedef unsigned char BYTE;

struct sprd_ext_ic_operations {
	void (*ic_init) (void);
	void (*charge_start_ext) (int);
	void (*charge_stop_ext) (void);
	void (*timer_callback_ext) (void);
};
extern int sprdchg_charger_is_adapter(void);
extern void sprd_ext_charger_init(void);
extern void chg_low_bat_chg(void);
extern struct sprd_ext_ic_operations *sprd_get_ext_ic_ops(void);