#include <asm/arch/sci_types.h>
#include <asm/arch/os_api.h>
//#include <asm/arch/regs_global.h>
#include "sprd_spi.h"

/**---------------------------------------------------------------------------*
 **                         Globle Variable                                  *
 **---------------------------------------------------------------------------*/

//endian switch mode
typedef enum
{
	ENDIAN_SWITCH_NONE    = 0x00,    //  2'b00: 0xABCD => 0xABCD
	ENDIAN_SWITCH_ALL     =0x01,     //  2'b01: 0xABCD => 0xDCBA
	ENDIAN_SWITCH_MODE0   =0x02 ,    //  2'b01: 0xABCD => 0xBADC
	ENDIAN_SWITCH_MODE1   = 0x03 ,   //  2'b01: 0xABCD => 0xCDAB
	ENDIAN_SWITCH_MAX
} DMA_ENDIANSWITCH_E ;  //should be added to dma_hal_new.h
//data width
typedef enum DMA_DATAWIDTH
{
	DMA_DATAWIDTH_BYTE = 0,
	DMA_DATAWIDTH_HALFWORD,
	DMA_DATAWIDTH_WORD,
	DMA_DATAWIDTH_MAX
}  DMA_DATAWIDTH_E;
//request mode
typedef enum DMA_CHN_REQMODE
{
	DMA_CHN_REQMODE_NORMAL = 0,
	DMA_CHN_REQMODE_TRASACTION,
	DMA_CHN_REQMODE_LIST,
	DMA_CHN_REQMODE_INFINITE,
	DMA_CHN_REQMODE_MAX
} DMA_CHN_REQMODE_E;

typedef enum LCM_DMA_RETURN_E
{
	LCM_ERR_ID    = 0,
	LCM_ERR_BUSWIDTH =1,   //only support:8-bit,16-bit 80_sys_bus
	LCM_ERR_SRCADDR =2,    //not HalfWORD_align
	LCM_ERR_ENDIAN
} LCM_DMA_RETURN_E;       //SHOULD define in lcd.h

#define GEN0_SPI2_EN 1
#if 0
void SPI_Enable( uint32 spi_id, uint32 is_en)
{
	if(is_en)
	{
		switch(spi_id)
		{
			case 0:
				*(volatile uint32 *)GR_GEN0 |= ( 1 << GEN0_SPI0_EN);
				break;
			case 1:
				*(volatile uint32 *)GR_GEN0 |= ( 1 << GEN0_SPI1_EN);
				break;
			case 2:
				*(volatile uint32 *)GR_GEN0 |= ( 1 << GEN0_SPI2_EN);
				break;
			default:
				break;
		}
	}
	else
	{

	}

}
void SPI_Reset( uint32 spi_id, uint32 ms)
{
	uint32 i = 0;

	if(spi_id == 0)
	{
		*(volatile uint32 *)GR_SOFT_RST |= (1 << 14);
		for (i=0; i<0x100; i++);
		*(volatile uint32 *)GR_SOFT_RST &=~(1 << 14);
	}
	else if(spi_id == 1)
	{
		*(volatile uint32 *)GR_SOFT_RST |= (1 << 15);
		for (i=0; i<0x100; i++);
		*(volatile uint32 *)GR_SOFT_RST &=~(1 << 15);
	}
	else if(spi_id == 2)
	{
		*(volatile uint32 *)GR_SOFT_RST |= (1 << 31);
		for (i=0; i<0x100; i++);
		*(volatile uint32 *)GR_SOFT_RST &=~(1 << 31);
	}
}

static void SPI_PinConfig(void)
{
}

// The dividend is clk_spiX_div[1:0] + 1
void SPI_ClkSetting(uint32 spi_id, uint32 clk_src, uint32 clk_div)
{
	//clk_spi0_sel: [3:2]---->2'b:00-78M 01-26M,01-104M,11-48M,
	//clk_spi0_div: [5:4]---->div,  clk/(div+1)

	if(spi_id == 0)
	{
		*(volatile uint32 *) GR_CLK_DLY |= (clk_src&3)<<26;
		*(volatile uint32 *) GR_GEN2   |= (clk_div&7)<<21;
	} else if(spi_id == 1){
		*(volatile uint32 *) GR_CLK_DLY |= (clk_src&3)<<30;
		*(volatile uint32 *) GR_GEN2   |= (clk_div&7)<<11;
	} else if(spi_id == 2){
		*(volatile uint32 *) GR_GEN3 |= (clk_src&3)<<3;
		*(volatile uint32 *) GR_GEN3   |= (clk_div&7)<<5;
	}
}
#endif
#define SPI_SEL_CS_SHIFT 8
#define SPI_SEL_CS_MASK (0x0F<<SPI_SEL_CS_SHIFT)
void SPI_SetCsLow( uint32 spi_sel_csx , uint32 is_low)
{
	volatile SPI_CTL_REG_T *spi_ctr_ptr = (volatile SPI_CTL_REG_T*)(SPI_USED_BASE);
	uint32 temp;

	if(is_low)     {
		//spi_ctl0[11:8]:cs3<->cs0 chip select, 0-selected;1-none
		spi_ctr_ptr->ctl0 &= ~(SPI_SEL_CS_MASK);
		spi_ctr_ptr->ctl0 &= ~((1<<spi_sel_csx)<<SPI_SEL_CS_SHIFT);
	}
	else
	{
		//spi_ctl0[11:8]:cs3<->cs0 chip select, 0-selected;1-none
		spi_ctr_ptr->ctl0 |= ((1<<spi_sel_csx)<<SPI_SEL_CS_SHIFT);
	}
}

#define SPI_CD_MASK  BIT_15
extern int sprd_gpio_request(struct gpio_chip *chip, unsigned offset);
extern int sprd_gpio_direction_output(struct gpio_chip *chip, unsigned offset, int value);
extern void sprd_gpio_set(struct gpio_chip *chip, unsigned offset, int value);

void SPI_SetCd( uint32 cd)
{
	volatile SPI_CTL_REG_T *spi_ctr_ptr = (volatile SPI_CTL_REG_T*)(SPI_USED_BASE);
	//0-command;1-data
	sprd_gpio_set(NULL,53,cd);
#if 0
	if(cd == 0)
		spi_ctr_ptr->ctl8 &= ~(SPI_CD_MASK);
	else
		spi_ctr_ptr->ctl8 |= (SPI_CD_MASK);
#endif
}

// USE spi interface to write cmd/data to the lcm
// pay attention to the data_format
typedef enum data_width
{
	DATA_WIDTH_7bit =7,
	DATA_WIDTH_8bit =8,
	DATA_WIDTH_9bit =9,
	DATA_WIDTH_10bit=10,
	DATA_WIDTH_11bit=11,
	DATA_WIDTH_12bit=12,
}  SPI_DATA_WIDTH;

// Set spi work mode for LCM with spi interface
#define SPI_MODE_SHIFT    3 //[5:3]
#define SPI_MODE_MASK     (0x07<<SPI_MODE_SHIFT)
void SPI_SetSpiMode(uint32 spi_mode)
{
	volatile SPI_CTL_REG_T *spi_ctr_ptr = (volatile SPI_CTL_REG_T *)(SPI_USED_BASE);
	uint32 temp = spi_ctr_ptr->ctl7;

	temp &= ~SPI_MODE_MASK;
	temp |= (spi_mode<<SPI_MODE_SHIFT);

	spi_ctr_ptr->ctl7 = temp;
}

// Transmit data bit number:spi_ctl0[6:2]
void  SPI_SetDatawidth(uint32 datawidth)
{
	volatile SPI_CTL_REG_T *spi_ctr_ptr = (volatile SPI_CTL_REG_T *)(SPI_USED_BASE);
	uint32 temp = spi_ctr_ptr->ctl0;

	if( 32 == datawidth )
	{
		spi_ctr_ptr->ctl0 &= ~0x7C;  //  [6:2]
		return;
	}

	temp &= ~0x0000007C;  //mask
	temp |= (datawidth<<2);

	spi_ctr_ptr->ctl0 = temp;
}

#define TX_MAX_LEN_MASK     0xFFFFF
#define TX_DUMY_LEN_MASK    0x3F    //[09:04]
#define TX_DATA_LEN_H_MASK  0x0F    //[03:00]
#define TX_DATA_LEN_L_MASK  0xFFFF  //[15:00]
/*****************************************************************************/
//  Description:  Set rxt data length with dummy_len
//    Note       :  the unit is identical with datawidth you set
/*****************************************************************************/
void SPI_SetTxLen(uint32 data_len, uint32 dummy_bitlen)
{
	volatile SPI_CTL_REG_T *spi_ctr_ptr = (volatile SPI_CTL_REG_T *)(SPI_USED_BASE);
	uint32 ctl8 = spi_ctr_ptr->ctl8;
	uint32 ctl9 = spi_ctr_ptr->ctl9;

	data_len &= TX_MAX_LEN_MASK;

	ctl8 &= ~((TX_DUMY_LEN_MASK<<4) | TX_DATA_LEN_H_MASK);
	ctl9 &= ~( TX_DATA_LEN_L_MASK );

	// set dummy_bitlen in bit[9:4] and data_len[19:16] in bit[3:0]
	spi_ctr_ptr->ctl8 = (ctl8 | (dummy_bitlen<<4) | (data_len>>16));
	// set data_len[15:00]
	spi_ctr_ptr->ctl9 = (ctl9 | (data_len&0xFFFF));
}


#define RX_MAX_LEN_MASK     		0xFFFFFF
#define RX_DUMY_LEN_MASK    	0x3F    //[09:04]
#define RX_DATA_LEN_H_MASK  	0x0F    //[03:00]
#define RX_DATA_LEN_L_MASK  	0xFFFF  //[15:00]
/*****************************************************************************/
//  Description:  Set rxt data length with dummy_len
//    Note       :  the unit is identical with datawidth you set
/*****************************************************************************/
void SPI_SetRxLen(uint32 data_len, uint32 dummy_bitlen)
{
	volatile SPI_CTL_REG_T *spi_ctr_ptr = (volatile SPI_CTL_REG_T *)(SPI_USED_BASE);
	uint32 ctl10 = spi_ctr_ptr->ctl10;
	uint32 ctl11 = spi_ctr_ptr->ctl11;

	data_len &= RX_MAX_LEN_MASK;

	ctl10 &= ~((RX_DUMY_LEN_MASK<<4) | RX_DATA_LEN_H_MASK);
	ctl11 &= ~( RX_DATA_LEN_L_MASK );

	// set dummy_bitlen in bit[9:4] and data_len[19:16] in bit[3:0]
	spi_ctr_ptr->ctl10 = (ctl10 | (dummy_bitlen<<4) | (data_len>>16));
	// set data_len[15:00]
	spi_ctr_ptr->ctl11 = (ctl11 | (data_len&0xFFFF));

	/* in SPIMODE_4WIRE_8BIT_SDIO
	the tx len has to set 0 to generate 8 clk, or generate 9clk */
	if(0x18==(spi_ctr_ptr->ctl7&(0x7 << 3))){
		spi_ctr_ptr->ctl8 &= 0xfff0;
		spi_ctr_ptr->ctl9 &= 0x0;
		}
}

// Request txt trans before send data
#define SW_RX_REQ_MASK BIT_0
#define SW_TX_REQ_MASK BIT_1
void SPI_TxReq( void )
{
	volatile SPI_CTL_REG_T *spi_ctr_ptr = (volatile SPI_CTL_REG_T *)(SPI_USED_BASE);

	spi_ctr_ptr->ctl12 |= SW_TX_REQ_MASK;
}

void SPI_RxReq( void )
{
	volatile SPI_CTL_REG_T *spi_ctr_ptr = (volatile SPI_CTL_REG_T *)(SPI_USED_BASE);

	spi_ctr_ptr->ctl12 |= SW_RX_REQ_MASK;
}

/*****************************************************************************/
//  Description:      To enable or disable DMA depending on parameter(is_enable)
//    Note:
/*****************************************************************************/
unsigned int SPI_EnableDMA(uint32 spi_index, uint32 is_enable)
{
	volatile SPI_CTL_REG_T* spi_ctl;

	spi_ctl = (volatile SPI_CTL_REG_T *)(SPI_USED_BASE+0x3000*spi_index);

	if (is_enable)
	{
		spi_ctl->ctl2 |= BIT_6;
	}
	else
	{
		spi_ctl->ctl2 &= ~BIT_6;
	}

	spi_ctl->ctl7 |= (BIT_7);

	return SCI_TRUE;
}

#define SPI_DMA_TIME_OUT        0x80000
#define BURST_SIZE              16
#define BURST_SIZE_MARK         0xF
#define LENGTH_4_DIVIDE         4

void SPI_Init(SPI_INIT_PARM *spi_parm)
{
	volatile SPI_CTL_REG_T *spi_ctr_ptr = (volatile SPI_CTL_REG_T *)(SPI_USED_BASE);
	uint32 temp;
	uint32 ctl0, ctl1, ctl2, ctl3;

	/*default clk is 500k 192M /(0xc0 * 2)*/
	spi_ctr_ptr->clkd =spi_parm->clk_div;

	temp  = 0;
	temp |= (spi_parm->tx_edge << 1)    |
		(spi_parm->rx_edge << 0)    |
		(0x1 << 13) |
		(spi_parm->msb_lsb_sel<< 7) ;
	spi_ctr_ptr->ctl0 = temp;

	spi_ctr_ptr->ctl1 |= BIT_12 | BIT_13;     // set rx/tx mode

	/*rx fifo full watermark is 16*/
	spi_ctr_ptr->ctl3 = 0x10;

	/*set SPIMODE_4WIRE_8BIT_SDA mode*/
	spi_ctr_ptr->ctl7 &= ~(0x7 << 3);
	spi_ctr_ptr->ctl7 |= SPIMODE_4WIRE_8BIT_SDA << 3;

#if 0
	// set water mark of reveive FIFO
	spi_ctr_ptr->ctl3  = (ctl3 & ~0xFFFF) |
		((BURST_SIZE >>2) <<8) |
		(BURST_SIZE >>2);
#endif
}

void SPI_unInit(SPI_INIT_PARM *spi_parm)
{
	volatile SPI_CTL_REG_T *spi_ctr_ptr = NULL;
}

void SPI_WaitTxFinish()
{
	volatile SPI_CTL_REG_T *spi_ctr_ptr = (volatile SPI_CTL_REG_T *)(SPI_USED_BASE);

	while( !(spi_ctr_ptr->iraw)&BIT_8 ) // IS tx finish
	{
	}
	spi_ctr_ptr->iclr |= BIT_8;

	// Wait for tx real empty
	while( !((spi_ctr_ptr->sts2)&BIT_7) )
	{
	}

	// Wait for spi bus idle
	while((spi_ctr_ptr->sts2)&BIT_8)
	{
	}
}

void SPI_WriteData(uint32 data, uint32 data_len, uint32 dummy_bitlen)
{
	uint32 command;
	volatile SPI_CTL_REG_T *spi_ctr_ptr = (volatile SPI_CTL_REG_T *)(SPI_USED_BASE);

	// The unit of data_len is identical with buswidth
	SPI_SetTxLen(data_len, dummy_bitlen);
	SPI_TxReq( );

	spi_ctr_ptr->data = data;

	SPI_WaitTxFinish();
}

uint32 SPI_ReadData( uint32_t * data,uint32 data_len, uint32 dummy_bitlen )
{
	int32_t i = 0 ;
	uint32 read_data=0, rxt_cnt=0;
	volatile SPI_CTL_REG_T *spi_ctr_ptr = (volatile SPI_CTL_REG_T *)(SPI_USED_BASE);

	// The unit of data_len is identical with buswidth
	SPI_SetRxLen(data_len, dummy_bitlen);
	SPI_RxReq( );

	//Wait for spi receive finish
	while( !((spi_ctr_ptr->iraw)&BIT_9) )
	{
		//wait rxt fifo full
		if((spi_ctr_ptr->iraw)&BIT_6)
		{
			rxt_cnt = (spi_ctr_ptr->ctl3)&0x1F;
			while(rxt_cnt--)
			{
				read_data = spi_ctr_ptr->data;
			}
		}
	}

	// Wait for spi bus idle
	while((spi_ctr_ptr->sts2)&BIT_8)
	{
	}

	for(i = 0 ; i<data_len ; i ++)
		data[i] = spi_ctr_ptr->data;
	return 0;
}

unsigned int SPI_ReadID(unsigned char cmd, int frame_size, int read_index, int spi_mode, int sync_pol)
{
	int i;
	unsigned int spi_data;
	volatile SPI_CTL_REG_T *spi_ctr_ptr = (volatile SPI_CTL_REG_T *)(SPI_USED_BASE);

	/*reset the spi_2*/
	*((volatile uint32 *)(CTL_BASE_APB + 0x4)) = 0x80;
	for (i = 0x0; i < 0x100; i++);
	*((volatile uint32 *)(CTL_BASE_APB + 0x4)) = 0x00;
	spi_ctr_ptr->ctl0 = 0x0;
	spi_ctr_ptr->ctl1 = 0x0;
	spi_ctr_ptr->ctl2 = 0x0;
	spi_ctr_ptr->ctl3 = 0x0;
	spi_ctr_ptr->ctl4 = 0x0;
	spi_ctr_ptr->ctl5 = 0x0;
	spi_ctr_ptr->ctl6 = 0x0;
	spi_ctr_ptr->ctl7 = 0x0;
	spi_ctr_ptr->ctl8 = 0x0;
	spi_ctr_ptr->ctl9 = 0x0;
	spi_ctr_ptr->ctl10 = 0x0;
	spi_ctr_ptr->ctl11 = 0x0;
	spi_ctr_ptr->ctl12 = 0x0;

	/*default spi clk = 26Mhz/2(0x41 + 1) = 200k*/
	spi_ctr_ptr->clkd = 0x41;

	if (sync_pol)
	{
		spi_ctr_ptr->ctl0 |= 0x1 << 15;
	}
	/*set cs to high*/
	spi_ctr_ptr->ctl0 |= 0xf << 8;
	/*set frame_size*/
	spi_ctr_ptr->ctl0 |= frame_size << 2;

	switch (spi_mode) {
	case 0:
		spi_ctr_ptr->ctl0 |= 0x1 << 1;
		break;
	case 1:
		spi_ctr_ptr->ctl0 |= 0x1 << 0;
		break;
	case 2:
		spi_ctr_ptr->ctl0 |= (0x1 << 13) | (0x1 << 1);
		break;
	case 3:
		spi_ctr_ptr->ctl0 |= (0x1 << 13) | (0x1 << 0);
		break;
	default:
		return 0xff;
	}

	/*enable 3 wire mode*/
	spi_ctr_ptr->ctl1 |= (0x1 << 5);
	/*read write bit*/
	spi_ctr_ptr->ctl1 |= (frame_size - 1) & 0x1f;
	/*read and write mode*/
	spi_ctr_ptr->ctl1 |= 0x3 << 12;
	/*read data start bit*/
	spi_ctr_ptr->ctl2 |= read_index & 0x1f;

	/*set cs low*/
	spi_ctr_ptr->ctl0 &= ~(0xf << 8);

	spi_data = cmd << (read_index + 1);
	spi_ctr_ptr->data = spi_data;

	/*wait for send complete*/
	while (!(spi_ctr_ptr->sts2 & (0x1 << 7)));
	while (spi_ctr_ptr->sts2 & (0x1 << 8));

	/*set cs high*/
	spi_ctr_ptr->ctl0 |= 0xf << 8;

	spi_data = spi_ctr_ptr->data & 0xff;

	printf("read lcd id is %x\n", spi_data);

	/*reset the spi_2*/
	*((volatile uint32 *)(CTL_BASE_APB + 0x4)) = 0x80;
	for (i = 0x0; i < 0x100; i++)
		;
	*((volatile uint32 *)(CTL_BASE_APB + 0x4)) = 0x00;

	return spi_data;
}
