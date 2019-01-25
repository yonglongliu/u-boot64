/*
 * (C) Copyright 2009 DENX Software Engineering
 * Author: John Rigby <jrigby@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include "sprd_pikel_modem_volte.h"

/* Flat Device Tree Definitions */
#define CONFIG_OF_LIBFDT


#define DT_PLATFROM_ID 8830
#define DT_HARDWARE_ID 1
#define DT_SOC_VER     0x20000

//only used in fdl2 .in uart download, the debug infors  from  serial will break the download process.

#define BOOT_NATIVE_LINUX_MODEM  1
#define CONFIG_SILENT_CONSOLE
#define CONFIG_SRAM_REPAIR


/*
 * SPREADTRUM BIGPHONE board - SoC Configuration
 */
#define CONFIG_AUTODLOADER
#define CONFIG_SP9820EA
#define CONFIG_SC9630
#define CONFIG_ADIE_SC2723
#define CONFIG_SPL_32K




#define CONFIG_PMIC_ARM7_BOOT
#define CONFIG_CP1_BOOT
#define CHIP_ENDIAN_LITTLE
#define _LITTLE_ENDIAN 1

#define CONFIG_RAM512M

#define CONFIG_EMMC_BOOT
#define CONFIG_ARCH_SCX35L
#define CONFIG_SPRD_MMC
#define CONFIG_SUPPORT_EMMC_BOOT
#define CONFIG_ARCH_SCX20L

#ifdef  CONFIG_EMMC_BOOT
#define EMMC_SECTOR_SIZE 512

#define CONFIG_FS_EXT4
#define CONFIG_EXT4_WRITE
#define CONFIG_CMD_EXT4
#define CONFIG_CMD_EXT4_WRITE


#define CONFIG_UEFI_PARTITION
#define CONFIG_EFI_PARTITION
#define CONFIG_EXT4_SPARSE_DOWNLOAD
#endif

/*
 * MMC definition
 */
#define CONFIG_CMD_MMC
#ifdef  CONFIG_CMD_MMC
#define CONFIG_CMD_FAT			1
#define CONFIG_FAT_WRITE		1
#define CONFIG_MMC			1
#define CONFIG_GENERIC_MMC		1
#define CONFIG_SDHCI			1
#define CONFIG_SDHCI_CTRL_NO_HISPD 	1 /* disable high speed control */
#define CONFIG_SYS_MMC_MAX_BLK_COUNT	0x1000
#define CONFIG_MMC_SDMA			1
#define CONFIG_MV_SDHCI			1
#define CONFIG_DOS_PARTITION		1
#define CONFIG_EFI_PARTITION		1
#define CONFIG_SYS_MMC_NUM		1
#endif



#define CONFIG_SYS_HZ			1000
#define CONFIG_SPRD_TIMER_CLK		1000 /*32768*/

#define CP0_ZERO_MAP_ADR    0x50000000
#define CP0_ARM0_EXEC_ADR        0x88400000 

#define CP1_ZERO_MAP_ADR    0x50001000
#define CP1_EXEC_ADR        LTE_MODEM_ADDR


#define MODEM_SIZE		(0x800000)
#define DSP_SIZE		(0x2E0000)
#define RUNTIMENV_SIZE		(3*128 * 1024)
#define CONFIG_SPL_LOAD_LEN	(0x6000)


/*#define CMDLINE_NEED_CONV */


#define CONFIG_SYS_SDRAM_BASE 0x80000000
#define CONFIG_SYS_SDRAM_END (CONFIG_SYS_SDRAM_BASE + PHYS_SDRAM_1_SIZE)

#define CONFIG_SYS_INIT_SP_ADDR     \
	(CONFIG_SYS_SDRAM_END - 0x10000 - GENERATED_GBL_DATA_SIZE)

#define CONFIG_SKIP_LOWLEVEL_INIT





#define CONFIG_CMDLINE_TAG		1	/* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS	1
#define CONFIG_INITRD_TAG		1

/*
 * Memory Info
 */
/* malloc() len */
#define CONFIG_SYS_MALLOC_LEN           (CONFIG_ENV_SIZE + 5 * 1024 * 1024)
/*
 * Board has 2 32MB banks of DRAM but there is a bug when using
 * both so only the first is configured
 */
#define CONFIG_NR_DRAM_BANKS	1

#define PHYS_SDRAM_1		0x80000000
#define PHYS_SDRAM_1_SIZE	0x20000000
#if (CONFIG_NR_DRAM_BANKS == 2)
#define PHYS_SDRAM_2		0x90000000
#define PHYS_SDRAM_2_SIZE	0x10000000
#endif
/* 8MB DRAM test */
#define CONFIG_SYS_MEMTEST_START	PHYS_SDRAM_1
#define CONFIG_SYS_MEMTEST_END		(PHYS_SDRAM_1+0x0800000)
#define CONFIG_STACKSIZE	(256 * 1024)	/* regular stack */

/*
 * Flash & Environment
 */
/* No NOR flash present */
#define CONFIG_SYS_NO_FLASH	1
#define CONFIG_ENV_IS_NOWHERE
#define CONFIG_ENV_SIZE		(128 * 1024)	
/*
#define	CONFIG_ENV_IS_IN_NAND
#define	CONFIG_ENV_OFFSET	CONFIG_SYS_MONITOR_LEN
#define CONFIG_ENV_OFFSET_REDUND	(CONFIG_ENV_OFFSET + CONFIG_ENV_SIZE)
*/

/* DDR */
//#define DDR_CLK 464
//---these three macro below,only one can be open
//#define DDR_LPDDR1
//#define DDR_LPDDR2
//#define DDR_DDR3

#define CONFIG_DDR_AUTO_DETECT
#define CONFIG_NR_DRAM_BANKS_ADDR_IN_IRAM    0x1C00   /* IRAM store ddr info */

//#define DDR_TYPE DRAM_LPDDR2_2CS_8G_X32
//#define DDR_TYPE DRAM_LPDDR2_1CS_4G_X32
//#define DDR_TYPE DRAM_LPDDR2_1CS_8G_X32
//#define DDR_TYPE DRAM_LPDDR2_2CS_16G_X32
//#define DDR_TYPE DRAM_DDR3_1CS_2G_X8_4P
//#define DDR_TYPE DRAM_DDR3_1CS_4G_X16_2P

//#define DDR3_DLL_ON TRUE
//#define DLL_BYPASS
//#define DDR_APB_CLK 128
//#define DDR_DFS_SUPPORT
//#define DDR_DFS_VAL_BASE 0X1c00

//#define DDR_SCAN_SUPPORT
//#define MEM_IO_DS LPDDR2_DS_40R

//#define PUBL_LPDDR1_DS PUBL_LPDDR1_DS_48OHM
//#define PUBL_LPDDR2_DS PUBL_LPDDR2_DS_40OHM
//#define PUBL_DDR3_DS   PUBL_DDR3_DS_34OHM

/* NAND */
#define CONFIG_NAND_SC9630
//#define CONFIG_SPRD_NAND_REGS_BASE	(0x21100000)
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BASE		(0x21100000)


#define CONFIG_CMD_MTDPARTS
#define CONFIG_MTD_PARTITIONS
#define CONFIG_MTD_DEVICE
#define CONFIG_CMD_UBI
#define CONFIG_RBTREE

/* U-Boot general configuration */
#define CONFIG_SYS_PROMPT	"=> "	/* Monitor Command Prompt */
#define CONFIG_SYS_CBSIZE	1024	/* Console I/O Buffer Size  */
/* Print buffer sz */
#define CONFIG_SYS_PBSIZE	(CONFIG_SYS_CBSIZE + \
		sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS	32	/* max number of command args */
/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE
#define CONFIG_SYS_LONGHELP




#define CONFIG_BOOTDELAY	0


#define CONFIG_LOADADDR		(CONFIG_SYS_TEXT_BASE - CONFIG_SYS_MALLOC_LEN - 4*1024*1024)	/* loadaddr env var */
#define CONFIG_SYS_LOAD_ADDR	CONFIG_LOADADDR


#define MTDIDS_DEFAULT "nand0=sprd-nand"
#define MTDPARTS_DEFAULT "mtdparts=sprd-nand:256k(spl),512k(2ndbl),256k(params),512k(vmjaluna),10m(modem),3840k(fixnv),3840k(backupfixnv),5120k(dsp),3840k(runtimenv),10m(boot),10m(recovery),250m(system),180m(userdata),20m(cache),256k(misc),1m(boot_logo),1m(fastboot_logo),3840k(productinfo),512k(kpanic)"
#define CONFIG_BOOTARGS "mem=512M loglevel=1 console=ttyS1,115200n8 init=/init " MTDPARTS_DEFAULT

#define COPY_LINUX_KERNEL_SIZE	(0x600000)

#define CONFIG_BOOTCOMMAND "cboot normal"
#define	CONFIG_EXTRA_ENV_SETTINGS				""	



#define CONFIG_USB_GADGET_SC8800G
#define CONFIG_USB_DWC
#define CONFIG_USB_GADGET_DUALSPEED

#define CONFIG_MODEM_CALIBERATE

#define CONFIG_LCD
#ifdef  CONFIG_LCD
#define CONFIG_DSIH_VERSION_1P21A
#define CONFIG_SPLASH_SCREEN
#define LCD_BPP LCD_COLOR16
#define CONFIG_LCD_WVGA
#define CONFIG_LCD_QVGA
//#define CONFIG_LCD_720P
#define CONFIG_CMD_BMP
#define CONFIG_SPI
#define CONFIG_SPRD_SPI
#define CONFIG_FB_SWDISPC
#define CONFIG_FB_LCD_RM68180_MIPI
//#define CONFIG_FB_LCD_OTM8019A_MIPI
#define CONFIG_FB_LCD_NT35516_MIPI
#define CONFIG_FB_LCD_ILI9806E_MIPI
#define CONFIG_FB_LCD_GC9304_SPI
#define CONFIG_FB_LCD_GC9305_SPI
#define CONFIG_SYS_WHITE_ON_BLACK
#ifdef  LCD_TEST_PATTERN
#define CONSOLE_COLOR_RED 0xf800
#define CONSOLE_COLOR_GREEN 0x07e0
#define CONSOLE_COLOR_YELLOW 0x07e0
#define CONSOLE_COLOR_BLUE 0x001f
#define CONSOLE_COLOR_MAGENTA 0x001f
#define CONSOLE_COLOR_CYAN 0x001f
#endif
#endif // CONFIG_LCD

#define CONFIG_BACKLIGHT_WHTLED
/*for sysdump*/
#define CONFIG_FS_FAT
#define CONFIG_SPRD_SYSDUMP
#define CONFIG_RAMDUMP_NO_SPLIT 1	/* Don't split sysdump file */
#define REAL_SDRAM_SIZE 0x40000000	/*dump 1G */


#define CALIBRATE_ENUM_MS 3000
#define CALIBRATE_IO_MS 2000

//#define LOW_BAT_ADC_LEVEL 782 /*phone battery adc value low than this value will not boot up*/
#define LOW_BAT_VOL            3400 /*phone battery voltage low than this value will not boot up*/
#define LOW_BAT_VOL_CHG        3300    //3.3V charger connect

#define PWR_KEY_DETECT_CNT 2 /*this should match the count of boot_pwr_check() function */
#define ALARM_LEAD_SET_MS 0 /* time set for alarm boot in advancd */


/*rf board id */
//#define RF_BAND_INFO
#ifdef RF_BAND_INFO
#define LB_GPIO_NUM   134
#define MB_GPIO_NUM   135
//#define HB_GPIO_NUM  0
//#define ADC_CHANNEL_FOR_NV  1
#endif

/* 32K LESS SUPPORT */
#define CONFIG_32K_LESS_COMPATIBLE
#ifdef CONFIG_32K_LESS_COMPATIBLE
#define CONFIG_32K_LESS
#endif

/* use gpio89 separate the TSX from the DCXO area */
#define CONFIG_TSX
#ifdef CONFIG_TSX
#define TSX_GPIO_NUM   89
#endif

/* #define CONFIG_SPRD_AUDIO_DEBUG */


#define CONFIG_PBINT_7S_RESET_V1
/*7S reset config*/
#define CONFIG_7S_RST_MODULE_EN		1	//0:disable module; 1:enable module

#define CONFIG_7S_RST_SW_MODE		1	//0:hw reset,1:arm reset,power keep on
#define CONFIG_7S_RST_SHORT_MODE	1	//0:long press then release key to trigger;1:press key some time to trigger
#define CONFIG_7S_RST_2KEY_MODE		0	//0:1Key--Normal mode; 1:2KEY
#define CONFIG_7S_RST_THRESHOLD		7	//7S, hold key down for this time to trigger

#define DT_ADR          0x85400000
#define KERNEL_ADR      0x80008000
#define VLX_TAG_ADDR    0x82000100
#define RAMDISK_ADR     0x85500000
#define CONFIG_SPRD_VBAT_CHEK
#define CONFIG_USB_DOWNLOAD
#define CONFIG_DWC_OTG
#define CONFIG_ARM32
#define CONFIG_ARCH_SHARK
#define CONFIG_PREBOOT "role"
#define CONFIG_BOARD_LATE_INIT



/*rf board id */

#define RF_BAND_INFO                    

#ifdef RF_BAND_INFO                     
#define LB_GPIO_NUM 			134
#define MB_GPIO_NUM 			135
//#define HB_GPIO_NUM 			0
//#define ADC_CHANNEL_FOR_NV		0
#endif


/* Chip Driver Macro Definitions Start*/
/*sprd adc*/
#define CONFIG_SPRD_ADC

/*sprd adi*/
#define CONFIG_SPRD_ADI

/*sprd gpio*/
#define CONFIG_SPRD_GPIO

/*sprd rtc*/
#define CONFIG_RTC_SPRD

/*sprd watchdog*/
#define CONFIG_HW_WATCHDOG
#define CONFIG_SPRD_WATCHDOG

 /*Serial Info*/
#define CONFIG_SPRD_UART		1
#define CONFIG_SYS_SC8800X_UART1	1
#define CONFIG_CONS_INDEX	1	/* use UART0 for console */
#define CONFIG_BAUDRATE		115200	/* Default baud rate */
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

#define CONFIG_SPRD_UART_PORTS	{ (void *) (CONFIG_SYS_SERIAL0), (void *) (CONFIG_SYS_SERIAL1) }
#define CONFIG_SYS_SERIAL0		0x70000000
#define CONFIG_SYS_SERIAL1		0x70100000

/* Chip Driver Macro Definitions End*/
/*active arm7 ram before access to it*/
#define CONFIG_ARM7_RAM_ACTIVE

#define CONFIG_SYS_I2C
#define CONFIG_SPRD_I2C

#endif /* __CONFIG_H */

