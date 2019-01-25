
#include "sprd_chg_helper.h"

typedef unsigned char BYTE;

#define CHG_TIME_OUT					(3)
#define IN_DETECT_FAIL				(1)
#define CHG_COLD_FALUT				(1)
#define CHG_HOT_FALUT				(2)
#define CHG_VBAT_FAULT				(1)
#define CHG_WHATCHDOG_FAULT		(1)
#define CHG_SYS_FAULT				(1)

#define INPUT_SRC_CTL	(0)
#define POWER_ON_CTL	(1)
#define CHG_CUR_CTL		(2)
#define TRICK_CHG_CTL	(3)
#define CHG_VOL_CTL		(4)
#define TIMER_CTL		(5)
#define LED_DRV_CTL		(6)
#define LED_CUR_REF_CTL	(7)
#define MISC_CTL			(8)
#define SYS_STATUS_REG	(9)
#define FAULT_REG		(10)

#define VIN_DPM_BIT					(0x78)
#define SW_RESET_BIT					(0x80)
#define TMR_RST_BIT					(0x40)
#define CHG_CUR_BIT					(0xfc)
#define CHG_EN_BIT					(0x30)
#define IN_VOL_LIMIT_BIT				(0x78)
#define IN_CUR_LIMIT_BIT				(0x07)
#define MIN_SYS_VOL_BIT 				(0x0e)
#define PRE_CHG_CUR_BIT				(0xf0)
#define TERMINA_CUR_BIT				(0x0f)
#define CHG_VOL_LIMIT_BIT			(0xfc)
#define PRE_CHG_VOL_BIT				(0x02)
#define RECHG_VOL_BIT				(0x01)
#define CHG_TERMINA_EN_BIT			(0x80)
#define WATCH_DOG_TIMER_BIT			(0x60)
#define CHG_SAFE_TIMER_EN_BIT		(0x10)
#define CHG_SAFE_TIMER_BIT			(0x0c)
#define OTG_CUR_LIMIT_BIT			(0x04)
#define LED_MODE_CTL_BIT				(0x06)
#define FLASH_MODE_CUR_BIT			(0x0f)
#define TORCH_MODE_CUR_BIT			(0xf0)
#define JEITA_ENABLE_BIT				(0x01)
#define VIN_STAT_BIT					(0xc0)
#define CHG_STAT_BIT					(0x30)
//fault reg bit
#define SYS_FAULT_BIT					(0x01)
#define NTC_FAULT_BIT				(0x03)
#define BAT_FAULT_BIT				(0x08)
#define CHG_FAULT_BIT				(0x30)
#define WHATCH_DOG_FAULT_BIT		(0x40)
#define LED_FAULT_BIT				(0x80)

#define VIN_DPM_SHIFT				(0x03)
#define SW_RESET_SHIFT				(0x07)
#define TMR_RST_SHIFT				(0x06)
#define CHG_CUR_SHIFT				(0x02)
#define CHG_EN_SHIFT					(0x04)
#define IN_VOL_LIMIT_SHIFT			(0x03)
#define IN_CUR_LIMIT_SHIFT			(0x00)
#define MIN_SYS_VOL_SHIFT			(0x01)
#define PRE_CHG_CUR_SHIFT			(0x04)
#define TERMINA_CUR_SHIFT			(0x00)
#define CHG_VOL_LIMIT_SHIFT			(0x02)
#define PRE_CHG_VOL_SHIFT			(0x01)
#define RECHG_VOL_SHIFT				(0x00)
#define CHG_TERMINA_EN_SHIFT		(0x07)
#define WATCH_DOG_TIMER_SHIFT		(0x05)
#define CHG_SAFE_TIMER_EN_SHIFT		(0x04)
#define CHG_SAFE_TIMER_SHIFT			(0x02)
#define OTG_CUR_LIMIT_SHIFT			(0x02)
#define LED_MODE_CTL_SHIFT			(0x01)
#define FLASH_MODE_CUR_SHIFT		(0x00)
#define TORCH_MODE_CUR_SHIFT		(0x04)
#define JEITA_ENABLE_SHIFT			(0x00)
#define VIN_STAT_SHIFT				(0x06)
#define CHG_STAT_SHIFT				(0x04)
//fault reg shift
#define SYS_FAULT_SHIFT				(0x00)
#define NTC_FAULT_SHIFT				(0x01)
#define BAT_FAULT_SHIFT				(0x03)
#define CHG_FAULT_SHIFT				(0x04)
#define WHATCH_DOG_FAULT_SHIFT		(0x06)
#define LED_FAULT_SHIFT				(0x07)



#define CHG_CUR_TA_VAL			(0x10)
#define CHG_CUR_USB_VAL			(0x00)
#define CHG_DISABLE_VAL			(0x03)
#define CHG_BAT_VAL				(0x01)
#define CHG_OTG_VAL				(0x02)
#define CHG_OFF_RBF_VAL			(0x03)
#define LED_FLASH_EN_VAL			(0x02)
#define LED_TORCH_EN_VAL		(0x01)


#define I2C_SPEED			(100000)
#ifdef CONFIG_SC9001
#define BUS_NUM		(5)
#else
#define BUS_NUM		(1)
#endif
#define SLAVE_ADDR 		(0x6b)

uint16_t sprd_2701_i2c_init(void)
{
	i2c_set_bus_num(BUS_NUM);
	i2c_init(I2C_SPEED, SLAVE_ADDR);
	return 0;
}
static int sprd_2701_write_reg(int reg, u8 val)
{
	i2c_reg_write(SLAVE_ADDR,reg,val);
   	return 0;
}

int sprd_2701_read_reg(int reg, u8 *value)
{
	int ret;
	ret = i2c_reg_read(SLAVE_ADDR , reg);
	if (ret < 0) {
		printf("%s reg(0x%x), ret(%d)\n", __func__, reg, ret);
		return ret;
	}

	ret &= 0xff;
	*value = ret;
	printf("######sprd_2701readreg reg  = %d value =%d/%x\n",reg, ret, ret);
	return 0;
}

static void sprd_2701_set_value(BYTE reg, BYTE reg_bit,BYTE reg_shift, BYTE val)
{
	BYTE tmp;
	sprd_2701_read_reg(reg, &tmp);
	tmp = tmp & (~reg_bit) |(val << reg_shift);
	printf("sprd_2701_set_value tmp =0x%x,val=0x%x\n",tmp,val);
	sprd_2701_write_reg(reg,tmp);
}

static BYTE sprd_2701_get_value(BYTE reg, BYTE reg_bit, BYTE reg_shift)
{
	BYTE data = 0;
	int ret = 0 ;
	ret = sprd_2701_read_reg(reg, &data);
	ret = (data & reg_bit) >> reg_shift;

	return ret;
}
void sprd_2701_limit_usb_cur()
{
	sprd_2701_set_value(INPUT_SRC_CTL,IN_CUR_LIMIT_BIT ,IN_CUR_LIMIT_SHIFT ,1 );
}
void sprd_2701_usb_start_charging(void)
{
	sprd_2701_set_value(INPUT_SRC_CTL,IN_CUR_LIMIT_BIT ,IN_CUR_LIMIT_SHIFT ,1 );
	sprd_2701_set_value(CHG_CUR_CTL,CHG_CUR_BIT ,CHG_CUR_SHIFT , CHG_CUR_USB_VAL);
}

void sprd_2701_ta_start_charging(void)
{
	sprd_2701_set_value(CHG_CUR_CTL,CHG_CUR_BIT ,CHG_CUR_SHIFT , CHG_CUR_TA_VAL);
}

void sprdchg_2701_start_chg(int type)
{
	printf("sprdchg_2701_start_chg\n type=%d\n",type);
	if(type == ADP_TYPE_CDP || type == ADP_TYPE_DCP){
		printf("charge start with AC\n");
		sprd_2701_ta_start_charging();
	}else{
		printf("charge start with USB\n");
		sprd_2701_usb_start_charging();
	}
}
 void sprdchg_2701_stop_charging(void)
{	
	printf("stop charge\n");
	sprd_2701_set_value(POWER_ON_CTL, CHG_EN_BIT , CHG_EN_SHIFT, CHG_DISABLE_VAL);
}
void  sprd_2701_sw_reset(void)
{
	printf("sprdchg_2701_sw_reset");
	sprd_2701_set_value(POWER_ON_CTL, SW_RESET_BIT, SW_RESET_SHIFT, 1);
}

void sprdchg_2701_init(void)
{
	BYTE data = 0;
	int i = 0;

	printf("2701init\n");
	sprd_2701_i2c_init();
	sprd_2701_sw_reset();
	for(i = 0;  i < 11;  i++){
		sprd_2701_read_reg(i, &data);
		printf("sprd_2701_ReadReg i = %d, data = %x\n",i,data);
	}

	sprd_2701_set_value(TIMER_CTL,WATCH_DOG_TIMER_BIT,WATCH_DOG_TIMER_SHIFT,0); // disable whatchdog 
	//sprd_2701_set_value(MISC_CTL,JEITA_ENABLE_BIT,JEITA_ENABLE_SHIFT,1);//enable jeita
	//sprd_2701_set_value(TIMER_CTL,CHG_TERMINA_EN_BIT,CHG_TERMINA_EN_SHIFT,0); //disable chg termina
	sprd_2701_set_value(TIMER_CTL,CHG_SAFE_TIMER_EN_BIT,CHG_SAFE_TIMER_EN_SHIFT,0); //disable safety timer
}

void sprdchg_2701_reset_timer(void)
{
	printf("2701 reset rimer\n");
	//sprd_2701_set_value(FAN5405_REG_CONTROL0, FAN5405_TMR_RST_OTG,FAN5405_TMR_RST_OTG_SHIFT, RESET32S);
}
struct sprd_ext_ic_operations sprd_extic_op ={
	.ic_init = sprdchg_2701_init,
	.charge_start_ext = sprdchg_2701_start_chg,
	.charge_stop_ext = sprdchg_2701_stop_charging,
	.timer_callback_ext = sprdchg_2701_reset_timer,
};
struct sprd_ext_ic_operations *sprd_get_ext_ic_ops(void){
	return &sprd_extic_op;
}
