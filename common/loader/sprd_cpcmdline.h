#ifndef _MODEM_COMMON_H_
#define _MODEM_COMMON_H_

#include <config.h>
#include <common.h>
#include <linux/string.h>


#define MAX_CP_CMDLINE_LEN  (200)

/*cp cmd define */

#define BOOT_MODE        "androidboot.mode"
#define CALIBRATION_MODE "calibration"
#define LTE_MODE         "ltemode"
#define AP_VERSION       "apv"
#define RF_BOARD_ID      "rfboard.id"
#define SL_KEY_N	"N"
#define SL_KEY_E	"E"
#define RF_HW_INFO       "hardware.version"
#define K32_LESS         "32k.less"
#define CRYSTAL_TYPE	"crystal"
#define AUTO_TEST        "autotest"

#define INVALID_BOARD_ID 0xFF

void cmdline_add_cp_cmdline(char *cmd, char* value);
void cp_cmdline_fixup(void);
char *cp_getcmdline(void);
char *cp_get_rf_hardware_info(void);
#endif // _MODEM_COMMON_H_
