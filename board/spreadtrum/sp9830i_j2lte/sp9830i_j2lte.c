#include <common.h>
#include <asm/io.h>
#include <sprd_regulator.h>
//#include <asm/arch/sprd_reg_ahb.h>
//#include <asm/arch/regs_ahb.h>
#include <asm/arch/common.h>
#include <asm/arch/chip_drv_common_io.h>

#include <asm/u-boot.h>
#include <part.h>
#include <sdhci.h>
#include <asm/arch/mfp.h>
//#include <linux/gpio.h>
//#include <asm/arch/gpio.h>

#include <asm/arch/pinmap.h>
#include "asm/arch/sprd_module_config.h"
#include "adi_hal_internal.h"
#include <boot_mode.h>
#include <chipram_env.h>


DECLARE_GLOBAL_DATA_PTR;

extern void sprd_gpio_init(void);
extern void ADI_init (void);
extern int LDO_Init(void);
extern void ADC_Init(void);
extern void init_ldo_sleep_gr(void);

phys_size_t real_ram_size = REAL_SDRAM_SIZE;


//extern struct eic_gpio_resource sprd_gpio_resource[];

#ifdef CONFIG_HARDWARE_VERSION
#define GPIO_BOARD_ID_VALUE 0x40280000
void PinMap_Init()
{
    REG32(CTL_PIN_BASE+REG_PIN_RFCTL17) =  BIT_PIN_SLP_AP|BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(3)|BIT_PIN_NUL|BIT_PIN_SLP_NUL|BIT_PIN_SLP_IE;//HW_REV_MOD_0
    REG32(CTL_PIN_BASE+REG_PIN_RFCTL20) =  BIT_PIN_SLP_AP|BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(3)|BIT_PIN_NUL|BIT_PIN_SLP_NUL|BIT_PIN_SLP_IE;//HW_REV_MOD_1
    REG32(CTL_PIN_BASE+REG_PIN_RFCTL12) =  BIT_PIN_SLP_AP|BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(3)|BIT_PIN_NUL|BIT_PIN_SLP_NUL|BIT_PIN_SLP_IE;//HW_REV_MOD_2
}
void GPIO_Init()
{
    REG32(REG_AON_APB_APB_EB0) |= BIT_GPIO_EB;
}
void hw_gpio_init()
{
	PinMap_Init();
	GPIO_Init();
}
unsigned int get_hw_version()
{
	//HW_REV_MOD:[0, 1, 2]; RFCTL:17, 20, 12; gpio:9, 12, 31; GPIO_BASE:gpio0-gpio15;
	//GPIO_BASE+0x80:gpio16-gpio31
	int gpio_states0 = 0;
	int gpio_states1 = 1;
	unsigned int hw_version = 0;

	//mask
	REG32(GPIO_BOARD_ID_VALUE+0x4) |= (1<<9) | (1<<12); 
	REG32(GPIO_BOARD_ID_VALUE+0x80+0x4) |= (1<<15);
	//data
	gpio_states0 = REG32(GPIO_BOARD_ID_VALUE);
	gpio_states1 = REG32(GPIO_BOARD_ID_VALUE+0x80);

	//HW Ver0.0 [0, 1, 2] values is [0, 0, 0]
	//HW Ver0.1 [0, 1, 2] values is [0, 0, 1]
	hw_version = (
				((gpio_states1 & (1<<15)) >> 13) |
				((gpio_states0 & (1<<12)) >> 11) |
				((gpio_states0 & (1<<9)) >> 9)
				);

	printf("%s HW board ID values=%d\n", __FUNCTION__, hw_version);

	return hw_version;
}
#endif
int board_init()
{
	//gd->bd->bi_arch_number = MACH_TYPE_OPENPHONE;
	/*get env transfer from chipram*/
	setup_chipram_env();
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;
	ADI_init();
	hw_gpio_init();
	misc_init();
	LDO_Init();
	ADC_Init();
	pin_init();
	sprd_eic_init();
	sprd_gpio_init();
	init_ldo_sleep_gr();
//	TDPllRefConfig(1);

	return 0;
}

phys_size_t get_real_ram_size(void)
{
		printf("real_ram_size=0x%x\n", real_ram_size);
        return real_ram_size;
}

int dram_init(void)
{
#ifdef CONFIG_DDR_AUTO_DETECT
        ulong sdram_base = CONFIG_SYS_SDRAM_BASE;
        ulong sdram_size = 0;
        int i;

        real_ram_size = 0;
        ulong bank_cnt = CONFIG_NR_DRAM_BANKS_ADDR_IN_IRAM;

        for (i = 1; i <= *(volatile uint32 *)CONFIG_NR_DRAM_BANKS_ADDR_IN_IRAM; i++) {
                real_ram_size += *(volatile ulong *)((volatile ulong *)CONFIG_NR_DRAM_BANKS_ADDR_IN_IRAM + i);
        }

//        real_ram_size = get_ram_size((volatile void *)sdram_base, real_ram_size);
#else
	real_ram_size = REAL_SDRAM_SIZE;
#endif
        gd->ram_size = get_ram_size((volatile void *)PHYS_SDRAM_1,
                        PHYS_SDRAM_1_SIZE);

        return 0;
}


CBOOT_FUNC s_boot_func_array[CHECK_BOOTMODE_FUN_NUM] = {

	get_mode_from_bat_low,
	write_sysdump_before_boot_extend,
	/* 1 get mode from file*/
	get_mode_from_file_extend,
	/* 2 get mode from watch dog*/
	get_mode_from_watchdog,
	/*3 get mode from alarm register*/
	get_mode_from_alarm_register,
	/*0 get mode from calibration detect*/
	get_mode_from_pctool,
	/*4 get mode from charger*/
	get_mode_from_charger,
	/*5 get mode from keypad*/
	get_mode_from_keypad,
	/*6 get mode from gpio*/
	get_mode_from_gpio_extend,

	/*shutdown device*/
//	get_mode_from_shutdown,

	0
};



void board_boot_mode_regist(CBOOT_MODE_ENTRY *array)
{
	MODE_REGIST(CMD_NORMAL_MODE, normal_mode);
	MODE_REGIST(CMD_RECOVERY_MODE, recovery_mode);
	MODE_REGIST(CMD_FASTBOOT_MODE, fastboot_mode);
	MODE_REGIST(CMD_WATCHDOG_REBOOT, watchdog_mode);
	MODE_REGIST(CMD_UNKNOW_REBOOT_MODE, unknow_reboot_mode);
	MODE_REGIST(CMD_PANIC_REBOOT, panic_reboot_mode);
	MODE_REGIST(CMD_AUTODLOADER_REBOOT, autodloader_mode);
	MODE_REGIST(CMD_SPECIAL_MODE, special_mode);
	MODE_REGIST(CMD_CHARGE_MODE, charge_mode);
	MODE_REGIST(CMD_ENGTEST_MODE,engtest_mode);
	MODE_REGIST(CMD_CALIBRATION_MODE, calibration_mode);
	MODE_REGIST(CMD_EXT_RSTN_REBOOT_MODE, normal_mode);
	MODE_REGIST(CMD_IQ_REBOOT_MODE, iq_mode);
	MODE_REGIST(CMD_ALARM_MODE, alarm_mode);
       MODE_REGIST(CMD_AUTOTEST_MODE, autotest_mode);

	return ;
}

int board_late_init(void)
{
	boot_mode_t boot_role;
	extern chipram_env_t* get_chipram_env(void);
	chipram_env_t* cr_env = get_chipram_env();
	boot_role = cr_env->mode;

	boot_pwr_check();

#if !defined(CONFIG_FPGA)
#ifdef CONFIG_NAND_BOOT
	//extern int nand_ubi_dev_init(void);
	nand_ubi_dev_init();
	debugf("nand ubi init OK!\n");
#endif

#endif
	extern void sprdbat_init(void);
	sprdbat_init();
	debugf("CHG init OK!\n");
	if (sprdbat_get_vbatauxadc_caltype() != 0) {
		/*fix me*/
		DCDC_Cal_ArmCore();
	}
	board_keypad_init();
	return 0;
}

void fdt_fixup_chosen_bootargs_board(char *buf, int calibration_mode)
{
	char *p = buf;
	/*
	 * Because of in u-boot, we can't find FDT chosen remove function
	 * and samsung only uses uart to do calibration,
	 * so in samsung board .dts, we remove the "console=ttyS1,115200n8" in chosen node by default.
	 * so in normal mode, we need to append console
	 */
#ifdef NORMAL_UART_MODE
	if (!calibration_mode) {
		p += sprintf(p, " eng_device=0 console=null no_console_suspend ");
	}
#else
	if (!calibration_mode) {
		p += sprintf(p, " console=ttyS1,115200n8 no_console_suspend ");
	}
#endif
}
