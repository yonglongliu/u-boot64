#include <common.h>
#include <boot_mode.h>
#include <asm/arch/sprd_reg.h>
#include <asm/arch/chip_drv_common_io.h>
#include "adi_hal_internal.h"
#include <asm/arch/check_reboot.h>


#if defined(CONFIG_ADIE_SC2723S)||defined(CONFIG_ADIE_SC2723)
#define BIT_AON_GPIO_EB BIT_GPIO_EB
#endif
extern int hw_watchdog_rst_pending(void);
extern int is_7s_reset(void);
extern int is_7s_reset_for_systemdump(void);
extern int is_hw_smpl_enable(void);
extern int is_smpl_bootup(void);

static void rtc_domain_reg_write(uint32_t val)
{
#if defined(CONFIG_ADIE_SC2723S)||defined(CONFIG_ADIE_SC2723)
	sci_adi_write(ANA_REG_GLB_RTC_RST1, (~val),(~0));  //clear status reg
	sci_adi_write(ANA_REG_GLB_RTC_RST0, (val),(~0));   //set status reg
#else
	val = val;
#endif
}

static uint32_t rtc_domain_reg_read(void)
{
#if defined(CONFIG_ADIE_SC2723S)||defined(CONFIG_ADIE_SC2723)
	return sci_adi_read(ANA_REG_GLB_RTC_RST2);  //read
#else
	return 0;
#endif
}

unsigned check_reboot_mode(void)
{
	unsigned rst_mode= 0;
	unsigned hw_rst_mode = ANA_REG_GET(ANA_REG_GLB_POR_SRC_FLAG);
	sprd_dbg("hw_rst_mode==%x\n", hw_rst_mode);

/*for download mode reboot system */
	if(HWRST_RTCSTATUS_DOWNLOAD_BOOT == rtc_domain_reg_read()) {
		rtc_domain_reg_write(HWRST_RTCSTATUS_DEFAULT);
		return CMD_NORMAL_MODE;
	}
/*for download mode reboot system ---- end*/

	rst_mode = ANA_REG_GET(ANA_REG_GLB_POR_RST_MONITOR);
	rst_mode &= 0x7FFF;
	ANA_REG_SET(ANA_REG_GLB_POR_RST_MONITOR, 0); //clear flag
	/* clear the flag which used by modem side in any scenes , prevent some errors */
	ANA_REG_SET(ANA_REG_GLB_WDG_RST_MONITOR, 0);

	sprd_dbg("rst_mode==%x\n",rst_mode);
	if(hw_watchdog_rst_pending()){
		sprd_dbg("hw watchdog rst int pending\n");
		if(rst_mode == HWRST_STATUS_RECOVERY)
			return CMD_RECOVERY_MODE;
		else if(rst_mode == HWRST_STATUS_FASTBOOT)
			return CMD_FASTBOOT_MODE;
		else if(rst_mode == HWRST_STATUS_NORMAL)
			return CMD_NORMAL_MODE;
		else if(rst_mode == HWRST_STATUS_NORMAL2)
			return CMD_WATCHDOG_REBOOT;
		else if(rst_mode == HWRST_STATUS_ALARM)
			return CMD_ALARM_MODE;
		else if(rst_mode == HWRST_STATUS_SLEEP)
			return CMD_SLEEP_MODE;
		else if(rst_mode == HWRST_STATUS_CALIBRATION)
			return CMD_CALIBRATION_MODE;
		else if(rst_mode == HWRST_STATUS_PANIC)
			return CMD_PANIC_REBOOT;
		else if(rst_mode == HWRST_STATUS_SPECIAL)
			return CMD_SPECIAL_MODE;
		else if(rst_mode == HWRST_STATUS_AUTODLOADER)
			return CMD_AUTODLOADER_REBOOT;
		else if(rst_mode == HWRST_STATUS_IQMODE)
			return CMD_IQ_REBOOT_MODE;
		else if(rst_mode == HWRST_STATUS_SPRDISK)
			return CMD_SPRDISK_MODE;
		else{
			sprd_dbg(" a boot mode not supported\n");
			return 0;
		}
	}else{
		sprd_dbg("is_7s_reset 0x%x, systemdump 0x%x\n", is_7s_reset(), is_7s_reset_for_systemdump());
		sprd_dbg("is_hw_smpl_enable %d\n", is_hw_smpl_enable());
		sprd_dbg("no hw watchdog rst int pending\n");
		if(rst_mode == HWRST_STATUS_NORMAL2)
			return CMD_UNKNOW_REBOOT_MODE;
#if 0 /* SC2711 & SC2723 change */
		else if(hw_rst_mode & HW_7SRST_STATUS)
		{
			return CMD_UNKNOW_REBOOT_MODE;
		}
#endif
		else if(is_7s_reset_for_systemdump())
		{
			return CMD_UNKNOW_REBOOT_MODE;
		}
		else if(is_7s_reset())
		{
			return CMD_NORMAL_MODE;
		}
		else if(hw_rst_mode & SW_EXT_RSTN_STATUS)
		{
			return CMD_EXT_RSTN_REBOOT_MODE;
		}
#ifndef CONFIG_SS_FUNCTION
		else if(is_smpl_bootup())
		{
			debugf("SMPL bootup!\n");
			return CMD_NORMAL_MODE;
		}
#endif
		else
			return 0;
	}

}

int get_mode_from_gpio(void)
{
	int ret = 0;
	unsigned hw_rst_mode;

	hw_rst_mode = ANA_REG_GET(ANA_REG_GLB_POR_SRC_FLAG);
	ret = (hw_rst_mode & HW_PBINT2_STATUS) && !charger_connected();

	return ret;
}

void reset_to_normal(unsigned reboot_mode)
{
	unsigned rst_mode = 0;

#if defined(CONFIG_ADIE_SC2723S)||defined(CONFIG_ADIE_SC2723)
	if (CMD_NORMAL_MODE == reboot_mode) {
		rtc_domain_reg_write(HWRST_RTCSTATUS_DOWNLOAD_BOOT);
		udelay(300);
	}
#endif
	if (reboot_mode ==  CMD_NORMAL_MODE) {
		rst_mode = HWRST_STATUS_NORMAL;
	}

	ANA_REG_SET(ANA_REG_GLB_POR_RST_MONITOR, rst_mode);

	reset_cpu(0);
}

void reboot_devices(unsigned reboot_mode)
{
	unsigned rst_mode = 0;

	if(reboot_mode == CMD_RECOVERY_MODE){
		rst_mode = HWRST_STATUS_RECOVERY;
	}else if(reboot_mode == CMD_FASTBOOT_MODE){
		rst_mode = HWRST_STATUS_FASTBOOT;
	}else if(reboot_mode == CMD_NORMAL_MODE){
		rst_mode = HWRST_STATUS_NORMAL;
	}else{
		rst_mode = 0;
	}

	ANA_REG_SET(ANA_REG_GLB_POR_RST_MONITOR, rst_mode);

	reset_cpu(0);
}
void power_down_devices(unsigned pd_cmd)
{
	power_down_cpu(0);
}
#ifdef CONFIG_SMPL_MODE
int is_real_battery(void)
{
	sprd_eic_request(EIC_BATDET);
	udelay(3000);
	return !!sprd_eic_get(EIC_BATDET);
}
#endif
int power_button_pressed(void)
{
#if defined(CONFIG_ADIE_SC2723S)||defined(CONFIG_ADIE_SC2723)
	sci_glb_set(REG_AON_APB_APB_EB0,BIT_AON_GPIO_EB | BIT_EIC_EB);
	sci_glb_set(REG_AON_APB_APB_RTC_EB,BIT_EIC_RTC_EB);
	sci_adi_set(ANA_REG_GLB_ARM_MODULE_EN, BIT_ANA_EIC_EN);
#else
	sci_glb_set(REG_AON_APB_APB_EB0,BIT_GPIO_EB | BIT_EIC_EB);
	sci_glb_set(REG_AON_APB_APB_RTC_EB,BIT_EIC_RTC_EB);
	sci_adi_set(ANA_REG_GLB_ARM_MODULE_EN, BIT_ANA_EIC_EN | BIT_ANA_GPIO_EN);
#endif
	sci_adi_set(ANA_REG_GLB_RTC_CLK_EN,BIT_RTC_EIC_EN);

	ANA_REG_SET(ADI_EIC_MASK, 0xff);

	udelay(3000);

	int status = ANA_REG_GET(ADI_EIC_DATA);
	status = status & (1 << 2);

	sprd_dbg("power_button_pressed eica status 0x%x\n", status );
	
	return !status;//low level if pb hold
}

int charger_connected(void)
{
	sprd_eic_request(EIC_CHG_INT);
	udelay(3000);
	sprd_dbg("eica status %x\n", sprd_eic_get(EIC_CHG_INT));
#ifdef  CONFIG_SHARK_PAD_HW_V102
	sprd_eic_request(EIC_USB_DETECT);
	udelay(3000);
	sprd_dbg("eica status %x\n", sprd_eic_get(EIC_USB_DETECT));
	return (!!sprd_eic_get(EIC_USB_DETECT)) ||(!!sprd_eic_get(EIC_CHG_INT));
#else
	return !!sprd_eic_get(EIC_CHG_INT);
#endif
}

int alarm_triggered(void)
{
	//printf("ANA_RTC_INT_RSTS is 0x%x\n", ANA_RTC_INT_RSTS);
	sprd_dbg("value of it 0x%x\n", ANA_REG_GET(ANA_RTC_INT_RSTS));
	return ANA_REG_GET(ANA_RTC_INT_RSTS) & BIT_4;
}

