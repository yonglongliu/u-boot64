#ifndef __SPRD_SPI_H__
#define __SPRD_SPI_H__

/*Globle Variable*/
#define CTL_BASE_SPI0       0x70A00000
#define CTL_BASE_SPI1       0x70B00000
#define CTL_BASE_SPI2       0x70C00000 
#define CTL_BASE_APB        0x71300000
#define CTL_BASE_SPI        CTL_BASE_SPI0

#define SPI0_BASE 		CTL_BASE_SPI
#define SPI_USED_BASE 	SPI0_BASE

#define SPI0_ID        0
#define SPI1_ID        1
#define SPI2_ID        2
#define SPI3_ID        3
#define SPI_USED_ID    SPI0_ID

#define CHN_SPI_INT			9
#define DMA_SPI_TX			0x13

#define SPI_TX_FIFO_DEPTH 		16
#define SPI_RX_FIFO_DEPTH 		16
#define SPI_INT_DIS_ALL		0
#define SPI_RX_FULL_INT_EN		BIT_6
#define SPI_RX_FULL_INT_STS 	BIT_6
#define SPI_RX_FULL_INT_CLR 	BIT_0
#define SPI_RX_FIFO_REAL_EMPTY	BIT_5
#define SPI_S8_MODE_EN			BIT_7
#define SPI_CD_SEL				0x2	//cs1 is sel as cd signal

/*Define the SPI interface mode for LCM*/
#define  SPIMODE_DISABLE           0
#define  SPIMODE_3WIRE_9BIT_SDA    1  // 3 wire 9 bit, cd bit, SDI/SDO share  one IO
#define  SPIMODE_3WIRE_9BIT_SDIO   2  // 3 wire 9 bit, cd bit, SDI, SDO
#define  SPIMODE_4WIRE_8BIT_SDA    3  // 4 wire 8 bit, cd pin, SDI/SDO share one IO
#define  SPIMODE_4WIRE_8BIT_SDIO   4  // 4 wire 8 bit, cd pin, SDI, SDO

/*Define the clk src for SPI mode*/
#define SPICLK_SEL_192M    0
#define SPICLK_SEL_154M    1
#define SPICLK_SEL_96M   2
#define SPICLK_SEL_26M    3

/*SPI CS sel in master mode*/
#define SPI_SEL_CS0 0x0E  //2'B1110
#define SPI_SEL_CS1 0x0D  //2'B1101
#define SPI_SEL_CS2 0x0B  //2'B1011
#define SPI_SEL_CS3 0x07  //2'B0111

/*Structure Definition*/
typedef enum
{
	CASE_INT_TEST = 0,
	CASE_DMA_TEST,
	CASE_LCD_CFG_TEST
}TEST_CASE_ID;

typedef enum
{
	TX_POS_EDGE = 0,
	TX_NEG_EDGE
}TX_EDGE;

typedef enum
{
	RX_POS_EDGE = 0,
	RX_NEG_EDGE
}RX_EDGE;

typedef enum
{
	TX_RX_MSB = 0,
	TX_RX_LSB
}MSB_LSB_SEL;

typedef enum
{
	IDLE_MODE = 0,
	RX_MODE,
	TX_MODE,
	RX_TX_MODE
}TRANCIEVE_MODE;

typedef enum
{
	NO_SWITCH    = 0,
	BYTE_SWITCH  = 1,
	HWORD_SWITCH = 2
}SWT_MODE;

typedef enum
{
	MASTER_MODE = 0,
	SLAVE_MODE = 1
}SPI_OPERATE_MODE_E;

typedef enum
{
	DMA_DISABLE = 0,
	DMA_ENABLE
}DMA_EN;

typedef enum
{
	CS_LOW = 0,
	CS_HIGH
}CS_SIGNAL;

typedef struct _init_param
{
	TX_EDGE tx_edge;
	RX_EDGE rx_edge;
	MSB_LSB_SEL msb_lsb_sel;
	TRANCIEVE_MODE tx_rx_mode;
	SWT_MODE switch_mode;
	SPI_OPERATE_MODE_E op_mode;
	uint32 DMAsrcSize;
	uint32 DMAdesSize;
	uint32 clk_div;
	uint8 data_width;
	uint8 tx_empty_watermark;
	uint8 tx_full_watermark;
	uint8 rx_empty_watermark;
	uint8 rx_full_watermark;
}SPI_INIT_PARM,*SPI_INIT_PARM_P;

/*SPI control register filed definitions */
typedef struct
{
	volatile uint32 data;				// Transmit word or Receive word
	volatile uint32 clkd;				// clock dividor register
	volatile uint32 ctl0;				// control register
	volatile uint32 ctl1;				// Receive Data full threshold/Receive Data full threshold
	volatile uint32 ctl2;				// 2-wire mode reigster
	volatile uint32 ctl3;				// transmit data interval
	volatile uint32 ctl4;				// transmit data interval
	volatile uint32 ctl5;				// transmit data interval
	volatile uint32 ien;				// interrutp enable register
	volatile uint32 iclr;				// interrupt clear register
	volatile uint32 iraw;				// interrupt clear register
	volatile uint32 ists;				// interrupt clear register
	volatile uint32 sts1;				// fifo cnt register, bit[5:0] for RX and [13:8] for TX
	volatile uint32 sts2;				// masked interrupt status register
	volatile uint32 dsp_wait;   		// Used for DSP control
	volatile uint32 sts3;				// tx_empty_threshold and tx_full_threshold
	volatile uint32 ctl6;
	volatile uint32 sts4;
	volatile uint32 fifo_rst;
	volatile uint32 ctl7;               // SPI_RX_HLD_EN : SPI_TX_HLD_EN : SPI_MODE
	volatile uint32 sts5;               // CSN_IN_ERR_SYNC2
	volatile uint32 ctl8;               // SPI_CD_BIT : SPI_TX_DUMY_LEN : SPI_TX_DATA_LEN_H
	volatile uint32 ctl9;               // SPI_TX_DATA_LEN_L
	volatile uint32 ctl10;              // SPI_RX_DATA_LEN_H : SPI_RX_DUMY_LEN
	volatile uint32 ctl11;              // SPI_RX_DATA_LEN_L
	volatile uint32 ctl12;              // SW_TX_REQ : SW_RX_REQ
	volatile uint32 sts6;
	volatile uint32 sts7;
	volatile uint32 sts8;
	volatile uint32 sts9;
} SPI_CTL_REG_T;

/*Function Propertype*/
 void SPI_Enable( uint32 spi_id, uint32 is_en);
 void SPI_Reset( uint32 spi_id, uint32 ms);
 void SPI_ClkSetting(uint32 spi_id, uint32 clk_src, uint32 clk_div);

 void SPI_SetCsLow( uint32 spi_sel_csx , uint32 is_low);
 void SPI_SetCd( uint32 cd);
 void SPI_SetSpiMode(uint32 spi_mode);
 void SPI_SetDatawidth(uint32 datawidth);
 unsigned int SPI_EnableDMA(uint32 spi_index, uint32 is_en);
 void SPI_SetTxLen(uint32 data_len, uint32 dummy_bitlen);
 void SPI_SetRxLen(uint32 data_len, uint32 dummy_bitlen);
 void SPI_TxReq( void );
 void SPI_RxReq( void );
 void SPI_WaitTxFinish();
 void SPI_Init(SPI_INIT_PARM *spi_parm);
 void SPI_WriteData(uint32 data, uint32 data_len, uint32 dummy_bitlen);
 uint32 SPI_ReadData( uint32_t * data,uint32 data_len, uint32 dummy_bitlen );
 unsigned int SPI_ReadID(unsigned char cmd, int frame_size, int read_index, int spi_mode, int sync_pol);
#endif /*_SPRD_SPI_H*/
