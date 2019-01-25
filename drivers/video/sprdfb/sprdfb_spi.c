/*
 * Copyright (C) 2012 Spreadtrum Communications Inc.
 *
 */
#include "sprdfb_chip_common.h"
#include "sprdfb.h"
#include "sprdfb_panel.h"
#include "../../spi/sprd_spi.h"

#define REG32(x)              (*((volatile uint32 *)(x)))

#define LCM_GPIO_RS	(53)

 SPI_INIT_PARM spi_int_parm[] =
 {
	{
	 TX_POS_EDGE,
	 RX_POS_EDGE,
	 TX_RX_MSB,
	 RX_TX_MODE,
	 NO_SWITCH,
	 MASTER_MODE,
	 0x0,
	 0x0,
	 0x03, //clk_div:(n+1)*2
	 0x0, //data_width.0-32bits per word; n-nbits per word
	 0x0,
	 SPI_TX_FIFO_DEPTH - 1,
	 0x0,
	 SPI_RX_FIFO_DEPTH - 1
	 },  //for spi_lcm test
	//{TX_POS_EDGE,RX_NEG_EDGE,TX_RX_LSB,RX_TX_MODE,NO_SWITCH,SLAVE_MODE,0x0,0x0,0xF0,0x0,0x0,SPI_TX_FIFO_DEPTH - 1,0x0,SPI_RX_FIFO_DEPTH - 1},
 };


 /**---------------------------------------------------------------------------*
 **                 SPI Interface for LCM test case  depend on spi_drv.c      *
 **---------------------------------------------------------------------------*/
//CASE1: 
// --------------------------------------------------------------------------- //
//  Description:   configure the start byte
//	Global resource dependence: 
//  Author:         lichd
//  Note  : LCM test code 
// --------------------------------------------------------------------------- //

 void DISPC_SpiWriteCmd(uint32_t cmd)
{
	SPI_SetDatawidth(8);
	SPI_SetCsLow(0, TRUE);
	SPI_SetCd( 0 );

	// Write a data identical with buswidth
	SPI_WriteData( cmd, 1, 0);

	SPI_SetCsLow(0, FALSE);
}

 void DISPC_SpiWriteData(uint32_t data)
{
	SPI_SetDatawidth(8);

	SPI_SetCsLow(0, TRUE);
	SPI_SetCd( 1 );

	// Write a data identical with buswidth
	SPI_WriteData( data, 1, 0);

	SPI_SetCsLow(0, FALSE);
}

static void SPI_Read( uint32_t* data,uint32_t len)
{
	SPI_SetCsLow(0, TRUE);
	SPI_SetCd( 1 );
	SPI_SetDatawidth(8);
	//Read data 16bits
	SPI_ReadData(data,len, 2);  //unit of buswidth
	SPI_SetCsLow(0, FALSE);
}

void SPI_PinCfg( void )
{
	/*enable access the spi reg*/
	*((volatile uint32 *)(0x4b000008)) |= BIT_1;
	*((volatile uint32 *)(0x4b0000c0)) |= BIT_0;
/*
	//select spi0_2
	CHIP_REG_SET (PIN_LCD_D6_REG, (PIN_FPD_EN | PIN_FUNC_1 | PIN_O_EN)); //SPI0_2_CD
	CHIP_REG_SET (PIN_LCD_RDN_REG, (PIN_FPD_EN | PIN_FUNC_1 | PIN_I_EN)); //SPI0_2_DI
	CHIP_REG_SET (PIN_LCD_WRN_REG, (PIN_FPD_EN | PIN_FUNC_1 | PIN_O_EN)); //SPI0_2_DO
	CHIP_REG_SET (PIN_LCD_CD_REG, (PIN_FPD_EN | PIN_FUNC_1 | PIN_O_EN)); //SPI0_2_CLK
	CHIP_REG_SET (PIN_LCD_CSN0_REG, (PIN_FPD_EN | PIN_FUNC_1 | PIN_O_EN)); //SPI0_2_CS0
*/
#if 0
	TB_REG_OR(GEN0, (1 << 13));

	//select spi0_1
	TB_REG_SET (PIN_LCD_D0_REG, (PIN_DS_2 | PIN_FUNC_1 )); //SPI0_1_DI
	TB_REG_SET (PIN_LCD_D1_REG, (PIN_DS_2 | PIN_FUNC_1 )); //SPI0_1_DO
	TB_REG_SET (PIN_LCD_D2_REG, (PIN_DS_2 | PIN_FUNC_1 )); //SPI0_1_CLK     
	TB_REG_SET (PIN_LCD_D3_REG, (PIN_DS_2 | PIN_FUNC_1 )); //SPI0_1_CS0    
	TB_REG_SET (PIN_LCD_D4_REG, (PIN_DS_2 | PIN_FUNC_1 )); //SPI0_1_CS1       
	TB_REG_SET (PIN_LCD_D5_REG, (PIN_DS_2 | PIN_FUNC_1 )); //SPI0_1_CD 

	// 
	TB_REG_AND(PIN_CTL_REG, ~(BIT_29|BIT_30));
	TB_REG_OR(PIN_CTL_REG, (1<<29));
#endif
}

BOOLEAN sprdfb_spi_init(struct sprdfb_device *dev)
{
#if 1//def CONFIG_SPX15
	/*enable the SPI0 SPI1 and SPI2*/
	REG32(CTL_BASE_APB + 0x1000) |= BIT_5;
	REG32(CTL_BASE_APB + 0x0004) |= BIT_5;
	REG32(CTL_BASE_APB + 0x0004) &= ~(BIT_5);
	REG32(0x21500000 + 0x007C) 	|=  0x03;
#else
	SPI_PinCfg();

	/*reset the spi2*/
	*((volatile uint32 *)(0x4b00004c))|= BIT_31;
	*((volatile uint32 *)(0x4b00004c)) &= ~BIT_31;

#endif
	sprd_gpio_request(NULL,LCM_GPIO_RS);
	sprd_gpio_direction_output(NULL,LCM_GPIO_RS,1);

//	SPI_Enable(SPI_USED_ID, TRUE);
	SPI_Init( spi_int_parm);

//	SPI_ClkSetting( SPI_USED_ID, SPICLK_SEL_78M, 0);
//	SPI_SetDatawidth(9);
//	SPI_SetSpiMode( SPIMODE_3WIRE_9BIT_SDIO );
	return TRUE;
}

BOOLEAN sprdfb_spi_uninit(struct sprdfb_device *dev)
{
	return TRUE;
}

struct ops_spi sprdfb_spi_ops = {
	.spi_send_cmd = DISPC_SpiWriteCmd,
	.spi_send_data = DISPC_SpiWriteData,
	.spi_read = SPI_Read,
};


static int32_t sprdfb_spi_panel_check(struct panel_spec *panel)
{
	if(NULL == panel){
		printf("sprdfb: [%s] fail. (Invalid param)\n", __FUNCTION__);
		return 0;
	}

	if(SPRDFB_PANEL_TYPE_SPI != panel->type){
		printf("sprdfb: [%s] fail. (not spi param)\n", __FUNCTION__);
		return 0;
	}

	FB_PRINT("sprdfb: [%s]\n",__FUNCTION__);

	return 1;
}

static void sprdfb_spi_panel_mount(struct sprdfb_device *dev)
{
	if((NULL == dev) || (NULL == dev->panel)){
		printf("sprdfb: [%s]: Invalid Param\n", __FUNCTION__);
		return;
	}
	dev->panel_if_type = SPRDFB_PANEL_IF_SPI;
	if (dev->panel->info.spi)
	{
		printf("sprdfb:sprdfb_spi_panel_mount sprdfb_spi_ops\n");
		dev->panel->info.spi->ops = &sprdfb_spi_ops;
	}
}

static void sprdfb_spi_panel_init(struct sprdfb_device *dev)
{
	int ret=false;
	printf("sprdfb: [%s]\n",__FUNCTION__);
	ret=sprdfb_spi_init(dev);
	if(!ret)
	{
		printf( "sprdfb: [%s]: bus init fail!\n", __FUNCTION__);
		return ;
	}
}

static void sprdfb_spi_panel_uninit(struct sprdfb_device *dev)
{
	int ret=false;
	printf("sprdfb: [%s]\n",__FUNCTION__);
	ret=sprdfb_spi_uninit(dev);
	if(!ret)
	{
		printf( "sprdfb: [%s]: init fail!\n", __FUNCTION__);
		return ;
	}
}

struct panel_if_ctrl sprdfb_spi_ctrl = {
	.if_name		= "spi",
	.panel_if_check	= sprdfb_spi_panel_check,
	.panel_if_mount	= sprdfb_spi_panel_mount,
	.panel_if_init		= sprdfb_spi_panel_init,
	.panel_if_before_refresh	= NULL,
	.panel_if_after_refresh	= NULL,
};

