/*
 * Copyright (C) 2012 Spreadtrum Communications Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

/*
 *Version SP9001-1_SCH_V1.1.0_Pinmap_V1.0
 */

#include <asm/io.h>
#include <asm/arch/pinmap.h>
#define BIT_PIN_SLP_ALL  (BIT_PIN_SLP_AP|BIT_PIN_SLP_PUBCP|BIT_PIN_SLP_WTLCP|BIT_PIN_SLP_AGCP)

#define BIT_PIN_SLP_ALL_CP  (BIT_PIN_SLP_PUBCP|BIT_PIN_SLP_WTLCP|BIT_PIN_SLP_AGCP)

static pinmap_t pinmap[] = {
	{REG_PIN_CTRL0, 0x08000000},
	{REG_PIN_CTRL1, 0x0},
	{REG_PIN_CTRL2, 0x0040a200},	// uart0->bt'uart;  uart1->ap'uart0; uart2->cp1'uart0; uart3->ltedsp uart1; uart4->agdsp uart0
{REG_PIN_CTRL3,0x38000},
	{REG_PIN_CTRL4, 0x10},
	{REG_PIN_CTRL5, 0x0},
	{REG_PIN_SIMCLK0, BIT_PIN_SLP_PUBCP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//SIM0_CLK
	{REG_PIN_SIMDA0, BIT_PIN_SLP_PUBCP | BIT_PIN_WPUS | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_WPU | BIT_PIN_SLP_IE},	//SIM0_DA
	{REG_PIN_SIMRST0, BIT_PIN_SLP_PUBCP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//SIM0_RST
	{REG_PIN_SIMCLK1, BIT_PIN_SLP_PUBCP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//SIM1_CLK
	{REG_PIN_SIMDA1, BIT_PIN_SLP_PUBCP | BIT_PIN_WPUS | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_WPU | BIT_PIN_SLP_IE},	//SIM1_DA
	{REG_PIN_SIMRST1, BIT_PIN_SLP_PUBCP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//SIM1_RST
	{REG_PIN_SIMCLK2, BIT_PIN_SLP_AP | BIT_PIN_WPUS | BITS_PIN_DS(1) | BITS_PIN_AF(1) | BIT_PIN_WPU | BIT_PIN_SLP_WPU | BIT_PIN_SLP_Z},	//I2C4_SCL
	{REG_PIN_SIMDA2, BIT_PIN_SLP_AP | BIT_PIN_WPUS | BITS_PIN_DS(1) | BITS_PIN_AF(1) | BIT_PIN_WPU | BIT_PIN_SLP_WPU | BIT_PIN_SLP_Z},	//I2C4_SDA
	{REG_PIN_SIMRST2, BIT_PIN_SLP_AP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(1) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//CODEC_CLKIN
	{REG_PIN_PCC_RF_LVDS_ADC_ON, BIT_PIN_SLP_ALL_CP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//LVDS_RF_ADC_ON_RFA
	{REG_PIN_PCC_RF_LVDS_DAC_ON, BIT_PIN_SLP_ALL_CP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//LVDS_RF_DAC_ON_RFA
	{REG_PIN_PCC_RFCTL0, BIT_PIN_SLP_AP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(3) | BIT_PIN_WPU | BIT_PIN_SLP_WPU | BIT_PIN_SLP_IE},	//CHG_INT
	{REG_PIN_PCC_RFCTL1, BIT_PIN_SLP_ALL_CP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//NC
	{REG_PIN_PCC_RFCTL2, BIT_PIN_SLP_AP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(3) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//CODEC_U0RXD  T2100   For LVDS Stress test
	{REG_PIN_PCC_RFCTL3, BIT_PIN_SLP_ALL_CP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//T2101     For LVDS Stress test
	{REG_PIN_PCC_RFCTL4, BIT_PIN_SLP_ALL_CP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//NC
	{REG_PIN_PCC_RFCTL5, BIT_PIN_SLP_ALL_CP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//NC
	{REG_PIN_PCC_RFCTL6, BIT_PIN_SLP_ALL_CP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//NC
	{REG_PIN_PCC_RFCTL7, BIT_PIN_SLP_ALL_CP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//NC
	{REG_PIN_PCC_RFCTL8, BIT_PIN_SLP_ALL_CP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//NC
	{REG_PIN_PCC_RFCTL9, BIT_PIN_SLP_ALL_CP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//NC
	{REG_PIN_PCC_RFCTL10, BIT_PIN_SLP_ALL_CP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//NC
	{REG_PIN_PCC_RFCTL11, BIT_PIN_SLP_ALL_CP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//NC
	{REG_PIN_PCC_RFCTL12, BIT_PIN_SLP_ALL_CP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//WCN_LTECOEXIST_RX
	{REG_PIN_PCC_RFCTL13, BIT_PIN_SLP_ALL_CP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//WCN_LTECOEXIST_TX
	{REG_PIN_PCC_RFCTL14, BIT_PIN_SLP_AP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(3) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//CODEC_RESET
	{REG_PIN_PCC_RFCTL15, BIT_PIN_SLP_ALL_CP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//COMM_RFCTL_15
	{REG_PIN_PCC_RFCTL16, BIT_PIN_SLP_ALL_CP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//COMM_RFCTL_16
	{REG_PIN_PCC_RFCTL17, BIT_PIN_SLP_ALL_CP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//COMM_RFCTL_17
	{REG_PIN_PCC_RFCTL18, BIT_PIN_SLP_ALL_CP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//COMM_RFCTL_18
	{REG_PIN_PCC_RFCTL19, BIT_PIN_SLP_ALL_CP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//COMM_RFCTL_19
	{REG_PIN_PCC_RFCTL20, BIT_PIN_SLP_ALL_CP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//COMM_RFCTL_20
	{REG_PIN_PCC_RFCTL21, BIT_PIN_SLP_ALL_CP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//COMM_RFCTL_21
	{REG_PIN_PCC_RFCTL22, BIT_PIN_SLP_ALL_CP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//COMM_RFCTL_22
	{REG_PIN_PCC_RFCTL23, BIT_PIN_SLP_ALL_CP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//COMM_RFCTL_23
	{REG_PIN_PCC_RFCTL24, BIT_PIN_SLP_ALL_CP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//COMM_RFCTL_24
	{REG_PIN_PCC_RFCTL25, BIT_PIN_SLP_ALL_CP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//NC
	{REG_PIN_PCC_RFCTL26, BIT_PIN_SLP_ALL_CP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//LVDS_DL_EN_PCC
	{REG_PIN_PCC_RFCTL27, BIT_PIN_SLP_ALL_CP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//LVDS_UL_EN_PCC
	{REG_PIN_PCC_RFFE_SCK0, BIT_PIN_SLP_ALL_CP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_WPD | BIT_PIN_SLP_Z},	//PCC_RFFE_SCK0
	{REG_PIN_PCC_RFFE_SDA0, BIT_PIN_SLP_ALL_CP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_WPD | BIT_PIN_SLP_WPD | BIT_PIN_SLP_Z},	//PCC_RFFE_SDA0
	{REG_PIN_PCC_RFFE_SCK1, BIT_PIN_SLP_ALL_CP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_WPD | BIT_PIN_SLP_Z},	//NC
	{REG_PIN_PCC_RFFE_SDA1, BIT_PIN_SLP_ALL_CP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_WPD | BIT_PIN_SLP_WPD | BIT_PIN_SLP_Z},	//NC
	{REG_PIN_PCC_RFSDA, BIT_PIN_SLP_ALL_CP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_WPD | BIT_PIN_SLP_Z},	//PCC_RFSDA
	{REG_PIN_PCC_RFSCK, BIT_PIN_SLP_ALL_CP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_WPD | BIT_PIN_SLP_Z},	//PCC_RFSCK
	{REG_PIN_PCC_RFSEN, BIT_PIN_SLP_ALL_CP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_WPD | BIT_PIN_SLP_Z},	//PCC_RFSEN
	{REG_PIN_SCC_RFSDA, BIT_PIN_SLP_ALL_CP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_WPD | BIT_PIN_SLP_Z},	//SCC_RFSDA
	{REG_PIN_SCC_RFSCK, BIT_PIN_SLP_ALL_CP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_WPD | BIT_PIN_SLP_Z},	//SCC_RFSCK
	{REG_PIN_SCC_RFSEN, BIT_PIN_SLP_ALL_CP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_WPD | BIT_PIN_SLP_Z},	//SCC_RFSEN
	{REG_PIN_SCC_RF_LVDS_ADC_ON, BIT_PIN_SLP_ALL_CP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//LVDS_RF_ADC_ON_RFB
	{REG_PIN_SCC_RF_LVDS_DAC_ON, BIT_PIN_SLP_ALL_CP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//LVDS_RF_DAC_ON_RFB
	{REG_PIN_SCC_RFCTL0, BIT_PIN_SLP_ALL_CP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//APT_DCDC_BYPASS_EN
	{REG_PIN_SCC_RFCTL1, BIT_PIN_SLP_ALL_CP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//APT_DCDC_FSEL
	{REG_PIN_SCC_RFCTL2, BIT_PIN_SLP_ALL_CP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//APT_DCDC_EN
	{REG_PIN_SCC_RFCTL3, BIT_PIN_SLP_ALL_CP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//NC
	{REG_PIN_SCC_RFCTL4, BIT_PIN_SLP_ALL_CP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//NC
	{REG_PIN_SCC_RFCTL5, BIT_PIN_SLP_ALL_CP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//NC
	{REG_PIN_SCC_RFCTL6, BIT_PIN_SLP_ALL_CP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//NC
	{REG_PIN_SCC_RFCTL7, BIT_PIN_SLP_ALL_CP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//NC          
	{REG_PIN_SCC_RFCTL8, BIT_PIN_SLP_ALL_CP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//NC      
	{REG_PIN_SCC_RFCTL9, BIT_PIN_SLP_ALL_CP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//T2102
	{REG_PIN_SCC_RFCTL10, BIT_PIN_SLP_ALL_CP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//T2103
	{REG_PIN_SCC_RFCTL11, BIT_PIN_SLP_AP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(3) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//CAMERA_FLASH_EN0
	{REG_PIN_SCC_RFCTL12, BIT_PIN_SLP_AP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(3) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//CAMERA_FLASH_EN1
	{REG_PIN_SCC_RFCTL13, BIT_PIN_SLP_AP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(3) | BIT_PIN_WPD | BIT_PIN_SLP_WPD | BIT_PIN_SLP_IE},	//INTR_API            
	{REG_PIN_SCC_RFCTL14, BIT_PIN_SLP_AP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(3) | BIT_PIN_WPD | BIT_PIN_SLP_WPD | BIT_PIN_SLP_IE},	//GPS_GPIO0
	{REG_PIN_SCC_RFCTL15, BIT_PIN_SLP_AP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(3) | BIT_PIN_WPD | BIT_PIN_SLP_WPD | BIT_PIN_SLP_IE},	//GPS_GPIO1
	{REG_PIN_SCC_RFCTL16, BIT_PIN_SLP_ALL_CP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//LVDS_DL_EN_RFB
	{REG_PIN_SCC_RFCTL17, BIT_PIN_SLP_ALL_CP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//LVDS_UL_EN_RFB
	{REG_PIN_SCC_RFFE_SCK0, BIT_PIN_SLP_ALL_CP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_WPD | BIT_PIN_SLP_Z},	//SCC_RFFE_SCK0
	{REG_PIN_SCC_RFFE_SDA0, BIT_PIN_SLP_ALL_CP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_WPD | BIT_PIN_SLP_WPD | BIT_PIN_SLP_Z},	//SCC_RFFE_SDA0
	{REG_PIN_SCC_RFFE_SCK1, BIT_PIN_SLP_ALL_CP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_WPD | BIT_PIN_SLP_Z},	//NC     
	{REG_PIN_SCC_RFFE_SDA1, BIT_PIN_SLP_ALL_CP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_WPD | BIT_PIN_SLP_WPD | BIT_PIN_SLP_Z},	//NC     
	{REG_PIN_U0TXD, BIT_PIN_SLP_AP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//BT_U0RXD
	{REG_PIN_U0RXD, BIT_PIN_SLP_AP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_WPU | BIT_PIN_SLP_WPU | BIT_PIN_SLP_IE},	//BT_U0TXD
	{REG_PIN_U0CTS, BIT_PIN_SLP_AP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_WPU | BIT_PIN_SLP_WPU | BIT_PIN_SLP_IE},	//BT_U0RTS
	{REG_PIN_U0RTS, BIT_PIN_SLP_AP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_WPU | BIT_PIN_SLP_Z},	//BT_U0CTS
	{REG_PIN_U1TXD, BIT_PIN_SLP_AP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//BB_U1TXD
	{REG_PIN_U1RXD, BIT_PIN_SLP_AP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_WPU | BIT_PIN_SLP_WPU | BIT_PIN_SLP_IE},	//BB_U1RXD
	{REG_PIN_U2TXD, BIT_PIN_SLP_AP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//BB_U2TXD
	{REG_PIN_U2RXD, BIT_PIN_SLP_AP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_WPU | BIT_PIN_SLP_WPU | BIT_PIN_SLP_IE},	//BB_U2RXD
	{REG_PIN_U3TXD, BIT_PIN_SLP_AP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//GPS_U0RXD
	{REG_PIN_U3RXD, BIT_PIN_SLP_AP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_WPU | BIT_PIN_SLP_WPU | BIT_PIN_SLP_IE},	//GPS_U0TXD
	{REG_PIN_U3CTS, BIT_PIN_SLP_AP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_WPU | BIT_PIN_SLP_WPU | BIT_PIN_SLP_IE},	//GPS_U0RTS
	{REG_PIN_U3RTS, BIT_PIN_SLP_AP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_WPU | BIT_PIN_SLP_Z},	//GPS_U0CTS
	{REG_PIN_U4TXD, BIT_PIN_SLP_AP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//BB_U4TXD
	{REG_PIN_U4RXD, BIT_PIN_SLP_AP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_WPU | BIT_PIN_SLP_WPU | BIT_PIN_SLP_IE},	//BB_U4RXD
	{REG_PIN_XTL_EN, BIT_PIN_SLP_AP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(3) | BIT_PIN_WPU | BIT_PIN_SLP_WPU | BIT_PIN_SLP_IE},	//M_INT
	{REG_PIN_MTDO_ARM, BIT_PIN_SLP_ALL | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_WPD | BIT_PIN_SLP_Z},	//No Ball in SCH
	{REG_PIN_MTDI_ARM, BIT_PIN_SLP_ALL | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_WPU | BIT_PIN_SLP_WPD | BIT_PIN_SLP_Z},	//No Ball in SCH
	{REG_PIN_MTCK_ARM, BIT_PIN_SLP_ALL | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_WPU | BIT_PIN_SLP_WPD | BIT_PIN_SLP_Z},	//MTCK
	{REG_PIN_MTMS_ARM, BIT_PIN_SLP_ALL | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_WPU | BIT_PIN_SLP_WPD | BIT_PIN_SLP_Z},	//MTMS
	{REG_PIN_MTRST_N_ARM, BIT_PIN_SLP_ALL | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_WPU | BIT_PIN_SLP_WPD | BIT_PIN_SLP_Z},	//No Ball in SCH
	{REG_PIN_LCM_RSTN, BIT_PIN_SLP_AP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//LCM_RSTN
	{REG_PIN_DSI_TE1, BIT_PIN_SLP_AP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_WPD | BIT_PIN_SLP_WPD | BIT_PIN_SLP_IE},	//LCMS_FMARK
	{REG_PIN_DSI_TE0, BIT_PIN_SLP_AP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_WPD | BIT_PIN_SLP_WPD | BIT_PIN_SLP_IE},	//LCM_FMARK
	{REG_PIN_SDA1, BIT_PIN_SLP_AP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(3) | BIT_PIN_WPU | BIT_PIN_SLP_WPU | BIT_PIN_SLP_IE},	//PROX_INT
	{REG_PIN_SCL1, BIT_PIN_SLP_AP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(1) | BIT_PIN_WPD | BIT_PIN_SLP_WPD | BIT_PIN_SLP_IE},	//GPS_COEXIST
	{REG_PIN_EXT_XTL_EN2, BIT_PIN_SLP_AP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(3) | BIT_PIN_WPD | BIT_PIN_SLP_WPD | BIT_PIN_SLP_IE},	//GPS_INT
	{REG_PIN_EXT_XTL_EN3, BIT_PIN_SLP_AP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(3) | BIT_PIN_WPD | BIT_PIN_SLP_WPD | BIT_PIN_SLP_IE},	//WF_WAKE_HOST
	{REG_PIN_KEYOUT0, BIT_PIN_SLP_AP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(3) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_IE},	//BOARD_ID0
	{REG_PIN_KEYOUT1, BIT_PIN_SLP_AP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(3) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_IE},	//BOARD_ID1
	{REG_PIN_KEYOUT2, BIT_PIN_SLP_AP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(3) | BIT_PIN_WPU | BIT_PIN_SLP_WPU | BIT_PIN_SLP_IE},	//M_DRDY
	{REG_PIN_KEYIN0, BIT_PIN_SLP_AP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(1) | BIT_PIN_WPU | BIT_PIN_SLP_WPU | BIT_PIN_SLP_IE},	//KEYINT0
	{REG_PIN_KEYIN1, BIT_PIN_SLP_AP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(1) | BIT_PIN_WPU | BIT_PIN_SLP_WPU | BIT_PIN_SLP_IE},	//KEYINT1
	{REG_PIN_KEYIN2, BIT_PIN_SLP_AP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(3) | BIT_PIN_WPD | BIT_PIN_SLP_WPD | BIT_PIN_SLP_IE},	//INTR_EVENT         
	{REG_PIN_SCL2, BIT_PIN_SLP_AP | BIT_PIN_WPUS | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_WPU | BIT_PIN_SLP_WPU | BIT_PIN_SLP_Z},	//I2C2_SCL                M-Sensor
	{REG_PIN_SDA2, BIT_PIN_SLP_AP | BIT_PIN_WPUS | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_WPU | BIT_PIN_SLP_WPU | BIT_PIN_SLP_Z},	//I2C2_SDA
	{REG_PIN_SCL5, BIT_PIN_SLP_AP | BIT_PIN_WPUS | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_WPU | BIT_PIN_SLP_WPU | BIT_PIN_SLP_Z},	//I2C5_SCL                Audio_codec
	{REG_PIN_SDA5, BIT_PIN_SLP_AP | BIT_PIN_WPUS | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_WPU | BIT_PIN_SLP_WPU | BIT_PIN_SLP_Z},	//I2C5_SDA
	{REG_PIN_CLK_AUX0, BIT_PIN_SLP_AP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//CON_32K_IN            
	{REG_PIN_BONDING_OPT0, BIT_PIN_SLP_AP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_WPD | BIT_PIN_SLP_WPD | BIT_PIN_SLP_Z},	//No Ball In SCH
	{REG_PIN_SPI0_CSN, BIT_PIN_SLP_AP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//CONDEC_SPI_CS
	{REG_PIN_SPI0_DO, BIT_PIN_SLP_AP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//CONDEC_SPI_DI
	{REG_PIN_SPI0_DI, BIT_PIN_SLP_AP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_WPD | BIT_PIN_SLP_WPD | BIT_PIN_SLP_IE},	//CONDEC_SPI_DO
	{REG_PIN_SPI0_CLK, BIT_PIN_SLP_AP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//CONDEC_SPI_CLK
	{REG_PIN_IIS1DI, BIT_PIN_SLP_AP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(3) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//WB_RST
	{REG_PIN_IIS1DO, BIT_PIN_SLP_AP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(3) | BIT_PIN_WPU | BIT_PIN_SLP_WPU | BIT_PIN_SLP_IE},	//G_INT
	{REG_PIN_IIS1CLK, BIT_PIN_SLP_AP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(3) | BIT_PIN_WPD | BIT_PIN_SLP_WPD | BIT_PIN_SLP_IE},	//BT_WAKE_HOST
	{REG_PIN_IIS1LRCK, BIT_PIN_SLP_AP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(3) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//BT_HOST_WAKE       
	{REG_PIN_IIS0DI, BIT_PIN_SLP_ALL | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_WPD | BIT_PIN_SLP_WPD | BIT_PIN_SLP_IE},	//BT_PCM_OUT
	{REG_PIN_IIS0DO, BIT_PIN_SLP_ALL | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_WPD | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//BT_PCM_IN
	{REG_PIN_IIS0CLK, BIT_PIN_SLP_ALL | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_IE},	//BT_PCM_CLK
	{REG_PIN_IIS0LRCK, BIT_PIN_SLP_ALL | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_IE},	//BT_PCM_SYNC
	{REG_PIN_SPI2_CSN, BIT_PIN_SLP_AP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_WPD | BIT_PIN_SLP_Z},	//GPS_SPI_CS         AF(3)?
	{REG_PIN_SPI2_DO, BIT_PIN_SLP_AP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_WPD | BIT_PIN_SLP_Z},	//GPS_SPI_DI
	{REG_PIN_SPI2_DI, BIT_PIN_SLP_AP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_WPD | BIT_PIN_SLP_Z},	//GPS_SPI_DO
	{REG_PIN_SPI2_CLK, BIT_PIN_SLP_AP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_WPD | BIT_PIN_SLP_Z},	//GPS_SPI_CLK
	{REG_PIN_PWMA, BIT_PIN_SLP_AP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//LCD_BL_PWM
	{REG_PIN_IIS3DI, BIT_PIN_SLP_ALL | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_WPD | BIT_PIN_SLP_WPD | BIT_PIN_SLP_IE},	//CODEC_I2S3_DO
	{REG_PIN_IIS3DO, BIT_PIN_SLP_ALL | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_WPD | BIT_PIN_SLP_WPD | BIT_PIN_SLP_IE},	//CODEC_I2S3_DI
	{REG_PIN_IIS3CLK, BIT_PIN_SLP_ALL | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_WPD | BIT_PIN_SLP_WPD | BIT_PIN_SLP_IE},	//CODEC_I2S3_SCLK
	{REG_PIN_IIS3LRCK, BIT_PIN_SLP_ALL | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//CODEC_I2S3_LRCK
	{REG_PIN_WIFI_COEXIST, BIT_PIN_SLP_AP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_WPD | BIT_PIN_SLP_WPD | BIT_PIN_SLP_IE},	//WF_COEXIST
	{REG_PIN_PTEST, BIT_PIN_SLP_ALL | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_WPD | BIT_PIN_SLP_WPD | BIT_PIN_SLP_Z},	//GND
	{REG_PIN_ANA_INT, BIT_PIN_SLP_AP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_WPD | BIT_PIN_SLP_WPD | BIT_PIN_SLP_IE},	//ANA_INT
	{REG_PIN_EXT_RST_B, BIT_PIN_SLP_AP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_WPU | BIT_PIN_SLP_WPU | BIT_PIN_SLP_IE},	//EXT_RST_B
	{REG_PIN_CHIP_SLEEP, BIT_PIN_SLP_AP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//CHIP_SLEEP
	{REG_PIN_DCDC_ARM0_EN, BIT_PIN_SLP_ALL | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//VDDARM0_EN
	{REG_PIN_DCDC_ARM1_EN, BIT_PIN_SLP_ALL | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//VDDARM1_EN            
	{REG_PIN_CLK_32K, BIT_PIN_SLP_ALL | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_IE},	//CLK_32K
{REG_PIN_AUD_SCLK,               BIT_PIN_SLP_ALL|BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(1)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_IE},//CODEC_I2S2_SCLK
{REG_PIN_AUD_SYNC,               BIT_PIN_SLP_ALL|BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(1)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_IE},//CODEC_I2S2_LRCK
{REG_PIN_AUD_ADD0,               BIT_PIN_SLP_ALL|BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(1)|BIT_PIN_WPD|BIT_PIN_SLP_WPD|BIT_PIN_SLP_IE},//CODEC_I2S2_DO
{REG_PIN_AUD_DAD0,               BIT_PIN_SLP_ALL|BIT_PIN_NULL|BITS_PIN_DS(1)|BITS_PIN_AF(1)|BIT_PIN_NUL|BIT_PIN_SLP_NUL|BIT_PIN_SLP_OE},//CODEC_I2S2_DI
	{REG_PIN_AUD_ADD1, BIT_PIN_SLP_ALL | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//T2104
	{REG_PIN_AUD_DAD1, BIT_PIN_SLP_ALL | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//T2105
	{REG_PIN_ADI_D, BIT_PIN_SLP_ALL | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_IE},	//ADI_D
	{REG_PIN_ADI_SYNC, BIT_PIN_SLP_ALL | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//SENSEN_WAKE
	{REG_PIN_ADI_SCLK, BIT_PIN_SLP_ALL | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//ADI_SCLK        
	{REG_PIN_EXTINT0, BIT_PIN_SLP_AP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(3) | BIT_PIN_WPU | BIT_PIN_SLP_WPU | BIT_PIN_SLP_IE},	//CTP_INT       
	{REG_PIN_EXTINT1, BIT_PIN_SLP_AP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(3) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//CTP_RST
	{REG_PIN_SCL3, BIT_PIN_SLP_AP | BIT_PIN_WPUS | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_WPU | BIT_PIN_SLP_Z},	//CTP_SCL
	{REG_PIN_SDA3, BIT_PIN_SLP_AP | BIT_PIN_WPUS | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_WPU | BIT_PIN_SLP_Z},	//CTP_SDA
	{REG_PIN_NFWPN, BIT_PIN_SLP_AP | BIT_PIN_WPUS | BITS_PIN_DS(1) | BITS_PIN_AF(1) | BIT_PIN_WPU | BIT_PIN_SLP_WPD | BIT_PIN_SLP_Z},	//EMMC_D0      GPIO99
	{REG_PIN_NFRB, BIT_PIN_SLP_AP | BIT_PIN_WPUS | BITS_PIN_DS(1) | BITS_PIN_AF(1) | BIT_PIN_WPU | BIT_PIN_SLP_WPD | BIT_PIN_SLP_Z},	//EMMC_CMD     GPIO100
	{REG_PIN_NFCLE, BIT_PIN_SLP_AP | BIT_PIN_WPUS | BITS_PIN_DS(1) | BITS_PIN_AF(1) | BIT_PIN_WPU | BIT_PIN_SLP_WPD | BIT_PIN_SLP_Z},	//EMMC_D6      GPIO101
	{REG_PIN_NFALE, BIT_PIN_SLP_AP | BIT_PIN_WPUS | BITS_PIN_DS(1) | BITS_PIN_AF(1) | BIT_PIN_WPU | BIT_PIN_SLP_WPD | BIT_PIN_SLP_Z},	//EMMC_D7      GPIO102
	{REG_PIN_NFREN, BIT_PIN_SLP_AP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(1) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//EMMC_CLK    GPIO103
	{REG_PIN_NFD4, BIT_PIN_SLP_AP | BIT_PIN_WPUS | BITS_PIN_DS(1) | BITS_PIN_AF(1) | BIT_PIN_WPU | BIT_PIN_SLP_WPD | BIT_PIN_SLP_Z},	//EMMC_D5      GPIO104
	{REG_PIN_NFD5, BIT_PIN_SLP_AP | BIT_PIN_WPUS | BITS_PIN_DS(1) | BITS_PIN_AF(1) | BIT_PIN_WPU | BIT_PIN_SLP_WPD | BIT_PIN_SLP_Z},	//EMMC_D4      GPIO105
	{REG_PIN_NFD6, BIT_PIN_SLP_AP | BIT_PIN_WPUS | BITS_PIN_DS(1) | BITS_PIN_AF(1) | BIT_PIN_WPD | BIT_PIN_SLP_WPD | BIT_PIN_SLP_Z},	//EMMC_RCLK    GPIO106  EMMC_DS for 5.0 Version 
	{REG_PIN_NFD7, BIT_PIN_SLP_AP | BIT_PIN_WPUS | BITS_PIN_DS(1) | BITS_PIN_AF(1) | BIT_PIN_WPU | BIT_PIN_SLP_WPD | BIT_PIN_SLP_Z},	//EMMC_D3      GPIO107
	{REG_PIN_NFD10, BIT_PIN_SLP_AP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(1) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//EMMC_RST    GPIO108
	{REG_PIN_NFD11, BIT_PIN_SLP_AP | BIT_PIN_WPUS | BITS_PIN_DS(1) | BITS_PIN_AF(1) | BIT_PIN_WPU | BIT_PIN_SLP_WPD | BIT_PIN_SLP_Z},	//EMMC_D1      GPIO109
	{REG_PIN_NFD14, BIT_PIN_SLP_AP | BIT_PIN_WPUS | BITS_PIN_DS(1) | BITS_PIN_AF(1) | BIT_PIN_WPU | BIT_PIN_SLP_WPD | BIT_PIN_SLP_Z},	//EMMC_D2      GPIO110
	{REG_PIN_NFCEN0, BIT_PIN_SLP_AP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(3) | BIT_PIN_WPD | BIT_PIN_SLP_WPD | BIT_PIN_SLP_Z},	//Not in SCH
	{REG_PIN_NFWEN, BIT_PIN_SLP_AP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(3) | BIT_PIN_WPD | BIT_PIN_SLP_WPD | BIT_PIN_SLP_Z},	//Not in SCH
	{REG_PIN_NFD0, BIT_PIN_SLP_AP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(3) | BIT_PIN_WPU | BIT_PIN_SLP_WPU | BIT_PIN_SLP_IE},	//TF_DET
	{REG_PIN_NFD1, BIT_PIN_SLP_AP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(3) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//GPS_PDN
	{REG_PIN_NFD2, BIT_PIN_SLP_AP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(3) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//CHG_EN 
	{REG_PIN_NFD3, BIT_PIN_SLP_AP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(3) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//CODEC_PWR_EN 
	{REG_PIN_NFD8, BIT_PIN_SLP_AP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(3) | BIT_PIN_WPD | BIT_PIN_SLP_WPD | BIT_PIN_SLP_Z},	//Not in SCH 
	{REG_PIN_NFD9, BIT_PIN_SLP_AP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(3) | BIT_PIN_WPD | BIT_PIN_SLP_WPD | BIT_PIN_SLP_Z},	//Not in SCH
	{REG_PIN_NFD12, BIT_PIN_SLP_AP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(3) | BIT_PIN_WPD | BIT_PIN_SLP_WPD | BIT_PIN_SLP_Z},	//Not in SCH 
	{REG_PIN_NFD13, BIT_PIN_SLP_AP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(3) | BIT_PIN_WPD | BIT_PIN_SLP_WPD | BIT_PIN_SLP_Z},	//Not in SCH  
	{REG_PIN_NFD15, BIT_PIN_SLP_AP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(3) | BIT_PIN_WPD | BIT_PIN_SLP_WPD | BIT_PIN_SLP_Z},	//Not in SCH            
	{REG_PIN_SD1_CLK,                BIT_PIN_SLP_AP|BIT_PIN_NULL|BITS_PIN_DS(5)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_NUL|BIT_PIN_SLP_OE},//WF_SD1_CLK
	{REG_PIN_SD1_CMD, BIT_PIN_SLP_AP | BIT_PIN_WPUS | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_WPU | BIT_PIN_SLP_WPD | BIT_PIN_SLP_Z},	//WF_SD1_CMD  
	{REG_PIN_SD1_D0, BIT_PIN_SLP_AP | BIT_PIN_WPUS | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_WPU | BIT_PIN_SLP_WPD | BIT_PIN_SLP_Z},	//WF_SD1_D0 
	{REG_PIN_SD1_D1, BIT_PIN_SLP_AP | BIT_PIN_WPUS | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_WPU | BIT_PIN_SLP_WPD | BIT_PIN_SLP_Z},	//WF_SD1_D1  
	{REG_PIN_SD1_D2, BIT_PIN_SLP_AP | BIT_PIN_WPUS | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_WPU | BIT_PIN_SLP_WPD | BIT_PIN_SLP_Z},	//WF_SD1_D2  
	{REG_PIN_SD1_D3, BIT_PIN_SLP_AP | BIT_PIN_WPUS | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_WPU | BIT_PIN_SLP_WPD | BIT_PIN_SLP_Z},	//WF_SD1_D3  
	{REG_PIN_SD0_D3, BIT_PIN_SLP_AP | BIT_PIN_WPUS | BITS_PIN_DS(3) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_WPD | BIT_PIN_SLP_Z},	//TF_SD0_D3
	{REG_PIN_SD0_D2, BIT_PIN_SLP_AP | BIT_PIN_WPUS | BITS_PIN_DS(3) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_WPD | BIT_PIN_SLP_Z},	//TF_SD0_D2
	{REG_PIN_SD0_CMD, BIT_PIN_SLP_AP | BIT_PIN_WPUS | BITS_PIN_DS(3) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_WPD | BIT_PIN_SLP_Z},	//TF_SD0_CMD
	{REG_PIN_SD0_D0, BIT_PIN_SLP_AP | BIT_PIN_WPUS | BITS_PIN_DS(3) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_WPD | BIT_PIN_SLP_Z},	//TF_SD0_D0
	{REG_PIN_SD0_D1, BIT_PIN_SLP_AP | BIT_PIN_WPUS | BITS_PIN_DS(3) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_WPD | BIT_PIN_SLP_Z},	//TF_SD0_D1
	{REG_PIN_SD0_CLK,                BIT_PIN_SLP_AP|BIT_PIN_NULL|BITS_PIN_DS(5)|BITS_PIN_AF(0)|BIT_PIN_NUL|BIT_PIN_SLP_NUL|BIT_PIN_SLP_OE},//TF_SD0_CLK0
	{REG_PIN_CMMCLK, BIT_PIN_SLP_AP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//CAM_CLK0            
	{REG_PIN_CMMCLK1, BIT_PIN_SLP_AP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//CAM_CLK1  
	{REG_PIN_CMRST0, BIT_PIN_SLP_AP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(3) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//CAM_RST0          GPIO
	{REG_PIN_CMRST1, BIT_PIN_SLP_AP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(3) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//CAM_RST1          GPIO
	{REG_PIN_CMPD0, BIT_PIN_SLP_AP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(3) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//CAM_PWDN0         GPIO
	{REG_PIN_CMPD1, BIT_PIN_SLP_AP | BIT_PIN_NULL | BITS_PIN_DS(1) | BITS_PIN_AF(3) | BIT_PIN_NUL | BIT_PIN_SLP_NUL | BIT_PIN_SLP_OE},	//CAM_PWDN1         GPIO                   
	{REG_PIN_SCL0, BIT_PIN_SLP_AP | BIT_PIN_WPUS | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_WPU | BIT_PIN_SLP_WPU | BIT_PIN_SLP_Z},	//I2C0_SCL           
	{REG_PIN_SDA0, BIT_PIN_SLP_AP | BIT_PIN_WPUS | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_WPU | BIT_PIN_SLP_WPU | BIT_PIN_SLP_Z},	//I2C0_SDA           
	{REG_PIN_SDA6, BIT_PIN_SLP_AP | BIT_PIN_WPUS | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_WPU | BIT_PIN_SLP_WPU | BIT_PIN_SLP_Z},	//I2C6_SCL           
	{REG_PIN_SCL6, BIT_PIN_SLP_AP | BIT_PIN_WPUS | BITS_PIN_DS(1) | BITS_PIN_AF(0) | BIT_PIN_WPU | BIT_PIN_SLP_WPU | BIT_PIN_SLP_Z},	//I2C5_SDA           
};

/*here is the adie pinmap such as 2731*/
static pinmap_t adie_pinmap[] = {
	{REG_PIN_ANA_EXT_XTL_EN0, BITS_ANA_PIN_DS(1) | BIT_ANA_PIN_WPD | BIT_ANA_PIN_SLP_IE},	//WB_CLK_REQ
	{REG_PIN_ANA_EXT_XTL_EN1, BITS_ANA_PIN_DS(1) | BIT_ANA_PIN_WPD | BIT_ANA_PIN_SLP_Z},	//NC
	{REG_PIN_ANA_EXT_XTL_EN2, BITS_ANA_PIN_DS(1) | BIT_ANA_PIN_WPD | BIT_ANA_PIN_SLP_IE},	//GPS_CLK_REQ
	{REG_PIN_ANA_EXT_XTL_EN3, BITS_ANA_PIN_DS(1) | BIT_ANA_PIN_WPD | BIT_ANA_PIN_SLP_IE},	//EXT_XTL_EN3
};

int pin_init(void)
{
	int i;
	for (i = 0; i < sizeof(pinmap) / sizeof(pinmap[0]); i++) {
		__raw_writel(pinmap[i].val, CTL_PIN_BASE + pinmap[i].reg);
	}

	for (i = 0; i < sizeof(adie_pinmap) / sizeof(adie_pinmap[0]); i++) {
		sci_adi_set(CTL_ANA_PIN_BASE + adie_pinmap[i].reg, adie_pinmap[i].val);
	}

	return 0;
}