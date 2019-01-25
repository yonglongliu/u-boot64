
/*
 * Configuration for Versatile Express. Parts were derived from other ARM
 *   configurations.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __TSHARK_L_H
#define __TSHARK_L_H

//#define CONFIG_SECURE_BOOT
//#define CONFIG_ROM_VERIFY_SPL
//#define  CONFIG_SYSTEM_VERIFY
#ifdef CONFIG_SECURE_BOOT
#define PRIMPUKPATH "/dev/block/mmcblk0boot0"
#define PRIMPUKSTART 512
#define PRIMPUKLEN 260
#endif
#ifdef CONFIG_ROM_VERIFY_SPL
#define CONFIG_SPL_LOAD_LEN     (0x8000)	/* 32 KB */
#define CONFIG_BOOTINFO_LENGTH  (0x200)	/* 512 Bytes */
#define PUBKEY_BSC_BLOCK_INDEX  (CONFIG_SPL_LOAD_LEN - CONFIG_BOOTINFO_LENGTH * 2) / EMMC_SECTOR_SIZE
#define PUBKEY_VLR_BLOCK_INDEX  2
#define PUBKEY_READ_BLOCK_NUMS  1
#define CONFIG_SPL_HASH_LEN     (0x400)	/* 1KB */
#endif

#define SPRD_DEBUG_LEVEL		2

#define DFS_ON_ARM7				1
#define DFS_PARAM_SRC_ADDR		0x00101c0c
#define DFS_PARAM_TARGET_ADDR 	0x50805400
#define DFS_PARAM_LEN			0x400

/*build aarch64 u-boot.elf*/
#define CONFIG_REMAKE_ELF

//#define CONFIG_FPGA

#define CONFIG_SYS_NO_FLASH

#define CONFIG_MISC_INIT_R	/* call misc_init_r()          */
#define CONFIG_BOARD_LATE_INIT	/* call board_late_init() */

//#define CONFIG_SUPPORT_RAW_INITRD

/* Cache Definitions */
//#define CONFIG_SYS_DCACHE_OFF
//#define CONFIG_SYS_ICACHE_OFF

#define CONFIG_IDENT_STRING		" SharkLT8"

/* Link Definitions */
#define CONFIG_SYS_TEXT_BASE		0x9f000000	/*1G RAM in 0xbf700000, 512M RAM in 0x9f700000 */
#define CONFIG_SYS_INIT_SP_ADDR     \
	(CONFIG_SYS_SDRAM_END - 0x10000 - GENERATED_GBL_DATA_SIZE)

/* Flat Device Tree Definitions */
#define CONFIG_OF_LIBFDT

/* SMP Spin Table Definitions */
#define CPU_RELEASE_ADDR		(CONFIG_SYS_SDRAM_BASE + 0x7fff0)

/* Generic Timer Definitions */
#define COUNTER_FREQUENCY		(0x1800000)	/* 24MHz */

/* Generic Interrupt Controller Definitions */
#define GICD_BASE			(0x12001000)
#define GICC_BASE			(0x12002000)

#define CONFIG_SYS_MEMTEST_START	PHYS_SDRAM_1
#define CONFIG_SYS_MEMTEST_END		(PHYS_SDRAM_1 + 0x0800000)

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 3 * 1024 * 1024)

/*timer*/
#define CONFIG_SYS_HZ			1000
#define CONFIG_SPRD_TIMER_CLK		1000	/*32768 */

 /* SPREADTRUM BIGPHONE board - SoC Configuration */
//#define CONFIG_SC9836A
#define CONFIG_SC9838A
#define CONFIG_ADIE_SC2723
/*arm7 efuse*/
#define CONFIG_SECURE_EFUSE

/* Chip Driver Macro Definitions Start*/
/*sprd watchdog*/
#define CONFIG_HW_WATCHDOG
#define CONFIG_SPRD_WATCHDOG

/*sprd rtc*/
#define CONFIG_RTC_SPRD
#define CONFIG_RTC_START_YEAR 2015

/*rf board id */

//#define RF_BAND_INFO                    

#ifdef RF_BAND_INFO                     
#define LB_GPIO_NUM 			134
#define MB_GPIO_NUM 			135
//#define HB_GPIO_NUM 			0
//#define ADC_CHANNEL_FOR_NV		0
#endif


/*sprd adc*/
#define CONFIG_SPRD_ADC

/*sprd adi*/
#define CONFIG_SPRD_ADI
#define ADI_R1P0_VER
/*sprd gpio*/
#define CONFIG_SPRD_GPIO

/* SPRD Serial Configuration */
#define CONFIG_SPRD_UART		1
#define CONFIG_SYS_SC8800X_UART1	1
#define CONFIG_CONS_INDEX	1	/* use UART0 for console */
#define CONFIG_SPRD_UART_PORTS	{ (void *) (CONFIG_SYS_SERIAL0), (void *) (CONFIG_SYS_SERIAL1) }
#define CONFIG_SYS_SERIAL0		0x70000000
#define CONFIG_SYS_SERIAL1		0x70100000

/* Default baud rate */
#define CONFIG_BAUDRATE		115200
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

/* Chip Driver Macro Definitions End*/

/* Command line configuration */
#define CONFIG_MENU

/*#define CONFIG_MENU_SHOW*/
#define CONFIG_CMD_CACHE
#define CONFIG_CMD_BDI
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_PXE
#define CONFIG_CMD_ENV
#define CONFIG_CMD_FLASH
#define CONFIG_CMD_IMI
#define CONFIG_CMD_MEMORY
#define CONFIG_CMD_MII
#define CONFIG_CMD_NET
#define CONFIG_CMD_PING
#define CONFIG_CMD_SAVEENV
#define CONFIG_CMD_RUN
//#define CONFIG_CMD_BOOTD
#define CONFIG_CMD_ECHO
#define CONFIG_CMD_SOURCE
#define CONFIG_CMD_FAT
#define CONFIG_DOS_PARTITION

/* Miscellaneous configurable options */
#define CONFIG_SYS_LOAD_ADDR		(PHYS_SDRAM_1 + 0x10000000)

/* Physical Memory Map */
#define CONFIG_NR_DRAM_BANKS		1
#define PHYS_SDRAM_1			0x80000000	/* SDRAM Bank #1 */
#define PHYS_SDRAM_1_SIZE		0x20000000	/* 512M */

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1
#define CONFIG_SYS_SDRAM_SIZE     PHYS_SDRAM_1_SIZE
#define CONFIG_SYS_SDRAM_END (CONFIG_SYS_SDRAM_BASE + CONFIG_SYS_SDRAM_SIZE)

/* Initial environment variables */
#define CONFIG_BOOTCOMMAND			"cboot normal"
#define CONFIG_BOOTDELAY		0
#define	CONFIG_EXTRA_ENV_SETTINGS				"mtdparts=" MTDPARTS_DEFAULT "\0"

/* Do not preserve environment */
#define CONFIG_ENV_IS_NOWHERE		1
#define CONFIG_ENV_SIZE			0x1000

/* Monitor Command Prompt */
#define CONFIG_SYS_CBSIZE		1024	/* Console I/O Buffer Size */
#define CONFIG_SYS_PROMPT		"SharkLT8# "
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
									sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE
#define CONFIG_SYS_LONGHELP
#define CONFIG_CMDLINE_EDITING		1
#define CONFIG_SYS_MAXARGS		64	/* max command args */

#define	CONFIG_PREBOOT		"role"

#define MEM_INIT_PARA "mem=512M"
#define MTDIDS_DEFAULT "nand0=sprd-nand"
#define MTDPARTS_DEFAULT "mtdparts=sprd-nand:256k(spl),768k(2ndbl),512k(kpanic),-(ubipac)"
#define CONFIG_BOOTARGS MEM_INIT_PARA" loglevel=7 console=ttyS1,115200n8 init=/init " MTDPARTS_DEFAULT

/*auto boot with normal mode*/
//#define CONFIG_AUTOBOOT

/*boot with modem*/
#define BOOT_NATIVE_LINUX_MODEM  1

#define CONFIG_MINI_TRUSTZONE

//#define CONFIG_SUPPORT_TDLTE
#define TDDSP_ADR       0x88020000
#define CONFIG_SUPPORT_LTE
//#define CONFIG_SUPPORT_WLTE
//#define CONFIG_SUPPORT_GSM

#define LTE_GDSP_LOAD_OFFSET	0x20000

#define LTE_FIXNV_SIZE 0x0006C000
#define LTE_RUNNV_SIZE 0x00090000 // 400k the max size is 512k
#define LTE_FIXNV_ADDR 0x8AA42000
#define LTE_RUNNV_ADDR (LTE_FIXNV_ADDR + LTE_FIXNV_SIZE)
#ifdef VOLTE_SUPPORT
#define LTE_MODEM_SIZE 0x00E00000
#else
#define LTE_MODEM_SIZE 0x00C00000   //0x8f0000
#endif
#define LTE_MODEM_ADDR 0x8AB48000
#define LTE_LDSP_SIZE  0x00b00000
#define LTE_LDSP_ADDR  0x89900000
#define LTE_GDSP_SIZE 0x002E0000
#define LTE_GDSP_ADDR (0x89600000 + LTE_GDSP_LOAD_OFFSET)

#define WL_WARM_SIZE 0x00200000
#ifdef VOLTE_SUPPORT
#define WL_WARM_ADDR 0x8DA80000
#else
#define WL_WARM_ADDR 0X8CE80000
#endif


#define GSM_FIXNV_SIZE 0x20000
#define GSM_FIXNV_ADDR 0x88a90000
#define GSM_RUNNV_SIZE 0x40000
#define GSM_RUNNV_ADDR 0x88ab0000
#define GSM_MODEM_ADDR 0X88400000	//equal to WL_DSDA_WARM_ADDR
#define GSM_MODEM_SIZE 0x670000	//equal to WL_DSDA_WARM_SIZE
#define GSM_DSP_ADDR   0x88000000
#define GSM_DSP_SIZE   0x400000
#define FIXNV_SIZE     LTE_FIXNV_SIZE

#define CONFIG_DFS_ENABLE
#define DFS_ADDR    0x50800000
#define DFS_SIZE    32768	//32K

/*eMMC config*/
#define CONFIG_FAT_WRITE		1
#define CONFIG_EMMC_BOOT
#define CONFIG_MMC
#define CONFIG_SUPPORT_EMMC_BOOT
#define CONFIG_SPRD_MMC
#define CONFIG_GENERIC_MMC
#define CONFIG_MMC_SDMA
#define CONFIG_SDHCI			1
#define CONFIG_MV_SDHCI    1
#define EMMC_SECTOR_SIZE 512
#define CONFIG_EXT4_SPARSE_DOWNLOAD
#define CONFIG_FS_EXT4

/*used in usb drv*/
#define CONFIG_USB_DOWNLOAD
#define CHIP_ENDIAN_LITTLE
#define _LITTLE_ENDIAN 1

#define CONFIG_DWC_OTG
#define CONFIG_USB_DOWNLOAD
#define CONFIG_MODEM_CALIBERATE
#define CALIBRATE_ENUM_MS 15000
#define CALIBRATION_FLAG_CP0         (0x88AF0000)
#define CALIBRATION_FLAG_CP1         (0x8AA41800)
#define CONFIG_AUTODLOADER
#define CALIBRATE_IO_MS 2000
#define CONFIG_USB_GADGET_SC8800G
#define CONFIG_USB_DWC
#define CONFIG_USB_GADGET_DUALSPEED

/*use EFI partition*/
#define CONFIG_EFI_PARTITION

#define CONFIG_DDR_AUTO_DETECT
#define CONFIG_NR_DRAM_BANKS_ADDR_IN_IRAM    0x101C00   /* IRAM store ddr info */

/*for sysdump*/
#define CONFIG_FS_FAT
#define CONFIG_SPRD_SYSDUMP
#define CONFIG_RAMDUMP_NO_SPLIT 1	/* Don't split sysdump file */
#define REAL_SDRAM_SIZE 0x40000000	/*dump 1G */

#define PWR_KEY_DETECT_CNT 2
#define ALARM_LEAD_SET_MS 0	/* time set for alarm boot in advancd */
#define LOW_BAT_VOL            3400	/*phone battery voltage low than this value will not boot up */
#define LOW_BAT_VOL_CHG        3300	//3.3V charger connect

/*for device tree*/
#define DT_PLATFROM_ID 8830
#define DT_HARDWARE_ID 1
#define DT_SOC_VER     0x20000

#define SIMLOCK_ADR      (0x891FE800+0x4)	//0x4 just for data header
#define KERNEL_ADR      0x80080000
#define VLX_TAG_ADDR    0x82000100
#define DT_ADR          0x85400000
#define RAMDISK_ADR     0x85500000
#define TRUSTRAM_ADR    0x82000000
#define TRUSTZONE_ADR   0x50808000
#define TRUSTZONE_SIZE  0x10000

/*for modem entry*/
#define COOPERATE_PROCESSOR_RESET_ADDR    0x402b00b0
#define COOPERATE_PMU_CTRL0_ADDR         0x402b0044
#define COOPERATE_PMU_CTRL2_ADDR         0x402b0068
#define CORE_CP0                          BIT_0
#define CORE_CP2                          BIT_2

/*lcd config*/
#define CONFIG_LCD
#ifdef CONFIG_LCD
#define CONFIG_DSIH_VERSION_1P21A
#define CONFIG_SPLASH_SCREEN
#define CONFIG_CMD_BMP
#define CONFIG_BMP_16BPP
//#define CONFIG_FB_LCD_ILI9806E_2_MIPI
//#define CONFIG_FB_LCD_OTM8019A_MIPI
//#define CONFIG_FB_LCD_HX8394A_MIPI
#define CONFIG_FB_LCD_AMS549HQ01_MIPI
//#define CONFIG_FB_LCD_NT35596H_MIPI
//#define CONFIG_LCD_FWVGA
#define CONFIG_LCD_720P
//#define CONFIG_LCD_1080P
#endif

/*nand config*/

/*
#define CONFIG_NAND_BOOT
#define CONFIG_NAND_SC8830
#define CONFIG_CMD_NAND
#define CONFIG_CMD_UBI

#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BASE		(0X20C00000)

#define CONFIG_CMD_MTDPARTS
#define CONFIG_MTD_PARTITIONS
#define CONFIG_MTD_DEVICE
#define CONFIG_RBTREE
#define UBIPAC_PART  "ubipac"
*/

/*7S reset config*/
#define CONFIG_7S_RST_MODULE_EN		1	//0:disable module; 1:enable module

#define CONFIG_7S_RST_SW_MODE		1	//0:hw reset,1:arm reset,power keep on
#define CONFIG_7S_RST_SHORT_MODE	1	//0:long press then release key to trigger;1:press key some time to trigger
#define CONFIG_7S_RST_2KEY_MODE		0	//0:1Key--Normal mode; 1:2KEY
#define CONFIG_7S_RST_THRESHOLD		7	//7S, hold key down for this time to trigger

#define CONFIG_SPRD_EXT_IC_POWER
#define CONFIG_FAN54015_CHARGE_IC
#define CONFIG_SYS_I2C
#define CONFIG_SPRD_I2C

#endif /* __TSHARK_L_H */
