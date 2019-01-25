#include "sprd_cpcmdline.h"
#include "loader_common.h"

#ifdef CONFIG_SIMLOCK
#include "simlock_secretkey.h"
#endif



char *g_prfhwinfo = NULL;
char *g_CPcmdlineBuf = NULL;
#if defined( CONFIG_KERNEL_BOOT_CP )
char CPcmdlineBuf[MAX_CP_CMDLINE_LEN];
#endif

struct rf_board_info {
	unsigned char id;
	unsigned char b32K;
	char* pInfo;
};

static const struct rf_board_info boad_info_arr[] = {
	{0, 0, "SP9832A-2_SR3593_JP_V1.1.1"},
	{8, 0, "SP9832A-2_SR3593_V1.0.0"},
	{9, 0, "SP9832A-2_SR3593_LA_V1.0.0"},
	{10, 0, "SP9832A-2_SR3593_V1.1.0/V1.1.1"},
	{11, 0, "SP9832A-2_SR3593_LA_V1.1.1"},
	{12, 1, "SP9832A-3_V1.0.0(TSX)"}, // repalce 23
	{20, 0, "SP9832A-3_V1.0.0(TCXO)"},
	{22, 0, "SP9832A-3_V1.1.0(TCXO)"},
	{23, 0, "SP9832A-3_V1.0.0(TSX)"}, // is closed, the new id is 12
};

static const char *cmd_arr[] = {
	BOOT_MODE,
	CALIBRATION_MODE,
	LTE_MODE,
	AP_VERSION,
	RF_BOARD_ID,
#ifdef CONFIG_SIMLOCK
	SL_KEY_N,
	SL_KEY_E,
#endif
	RF_HW_INFO,
	K32_LESS,
	CRYSTAL_TYPE,
	AUTO_TEST,
	NULL
};

static unsigned int gpio_state(unsigned int GPIO_NUM){
	int value = 0 ;
	sprd_gpio_request(NULL, GPIO_NUM);
        sprd_gpio_direction_input(NULL, GPIO_NUM);
	value = sprd_gpio_get(NULL,GPIO_NUM);
	return value > 0;
}

static int get_adc_value_board(int channel){
	extern uint16_t sprdbat_auxadc2vbatvol(uint16_t adcvalue);
	int vol = 0;
	uint32_t result;
	uint32_t res;
	//uint32_t channel_vol = sprd_adc_to_vol(adcvalue);
	uint32_t m, n;
	uint32_t bat_numerators, bat_denominators;
	uint32_t adc_channel_numerators, adc_channel_denominators;

	vol = sci_adc_get_value_by_isen(channel,1,20);
	vol = sprdbat_auxadc2vbatvol(vol);
	//res = (u32) sci_adc_ratio(5, 0, 0);
	bat_numerators = 7;//res >> 16;
	bat_denominators = 29;//res & 0xffff;
	//res = (u32) sci_adc_ratio(channel, 1, 0);
	adc_channel_numerators = 1;//res >> 16;
	adc_channel_denominators = 1;//res & 0xffff;

	vol = vol * bat_numerators * (adc_channel_denominators)/
		( bat_denominators * adc_channel_numerators);
	printf("result = %d\n",vol);

#ifdef CONFIG_SP9830I
	if (vol <= 0 || vol >= 1200) {
		printf("vol is out of ranger [0~1200]\n");
		return -1;
	}

	if (vol >0 && vol <= 100)
		vol = 0;
	if (vol > 100 && vol <= 300 )
		vol = 1;
	if (vol > 300 && vol <= 500)
		vol = 2;
	if (vol > 500 && vol <= 700)
		vol = 3;
	if (vol > 700 && vol <= 850)
		vol = 4;
	if (vol > 850 && vol <= 1000)
		vol = 5;
	if (vol > 1000 && vol < 1200)
		vol = 6;
#endif

	return vol;
}


static int Sprd_getHWRFboardID(void){

#ifdef RF_BAND_INFO
	unsigned int gpio_val = 0;
	unsigned int adc_val = 0;
	unsigned char shift_bit = 0;
#ifdef LB_GPIO_NUM
#if defined(CONFIG_SP7731C) || defined(CONFIG_32K_LESS_COMPATIBLE)
	gpio_val |= 0;
	shift_bit++;
	printf("gpio_val0: %d\n", gpio_val);
#else

	gpio_val |= gpio_state(LB_GPIO_NUM) << shift_bit;
	shift_bit++;
	printf("gpio_val0: %d\n", gpio_val);
#endif
#endif

#ifdef MB_GPIO_NUM
	gpio_val |= gpio_state(MB_GPIO_NUM) << shift_bit;
	shift_bit++;
	printf("gpio_val1: %d\n", gpio_val);
#endif

#ifdef HB_GPIO_NUM
	gpio_val |= gpio_state(HB_GPIO_NUM) << shift_bit;
	shift_bit++;
#endif

#ifdef ADC_CHANNEL_FOR_NV
	adc_val = get_adc_value_board(ADC_CHANNEL_FOR_NV);
	if(adc_val >=0){
		adc_val = adc_val << shift_bit;
	}
#endif
	printf("adc_val: %d\n", adc_val);
	return gpio_val + adc_val;
#endif
	return INVALID_BOARD_ID;
}

static char *cpcmdline_get_hw_info(unsigned char id, unsigned char *ptr_b32k)
{
	int i;

	for(i = 0; i < sizeof(boad_info_arr)/ sizeof(struct rf_board_info); i++ )
	{
		if(id == boad_info_arr[i].id) {
			*ptr_b32k = boad_info_arr[i].b32K;
			return boad_info_arr[i].pInfo;
		}
	}

	return NULL;
}

#if defined(CONFIG_SP7731C)
int sprd_get32_less(void){
	unsigned int value_less= 0;
	value_less = gpio_state(LB_GPIO_NUM);
	printf("value_less: %d\n", value_less);
	return value_less;
}
#endif

#if defined(CONFIG_32K_LESS_COMPATIBLE) || defined(CONFIG_32K_SPRD)
int sprd_get32_less(void){
	unsigned int value_less= 0;
#if defined(CONFIG_32K_LESS)
	value_less = 1;
#elif defined(CONFIG_32K)
	value_less = 0;
#else
	value_less = gpio_state(LB_GPIO_NUM);
#endif
	printf("value_less: %d\n", value_less);
	return value_less;
}
#endif


#if defined(CONFIG_TSX)
int sprd_get_crystal(void){
	unsigned int value_tsx = 0;
	value_tsx = gpio_state(TSX_GPIO_NUM);
	printf("value_tsx: %d\n", value_tsx);
	return value_tsx;
}
#endif


static bool is_invalid_cmd(const char *cmd)
{
	int i = 0;
	while(NULL != cmd_arr[i])
	{
		if(0 == strcmp(cmd_arr[i], cmd)){
			return true;
		}
		i++;
	}
	return false;
}

static char *cpcmdline_get_lte_mode(void)
{
	char *ltemode = NULL;

#ifdef CONFIG_SUPPORT_TDLTE
	ltemode = "tcsfb";
#elif defined CONFIG_SUPPORT_WLTE
	ltemode = "fcsfb";
#elif defined CONFIG_SUPPORT_LTE
	ltemode = "lcsfb";
#endif

	return ltemode;
}

static void cmdline_prepare(void)
{
	if(NULL == g_CPcmdlineBuf)
	{
#if defined( CONFIG_KERNEL_BOOT_CP )
		g_CPcmdlineBuf = CPcmdlineBuf;
#else
#ifdef CALIBRATION_FLAG_CP0
		g_CPcmdlineBuf = (char*)CALIBRATION_FLAG_CP0;
#endif
#ifdef CALIBRATION_FLAG_CP1
		g_CPcmdlineBuf = (char*)CALIBRATION_FLAG_CP1;
#endif
#endif
		memset(g_CPcmdlineBuf, 0, MAX_CP_CMDLINE_LEN);
		printf("g_CPcmdlineBuf = 0x%p\n" , g_CPcmdlineBuf);

	}
}


void cmdline_add_cp_cmdline(char *cmd, char* value)
{
	char *p;
	int len;
	//printf("add cmd, cmd = %s, value = %s\n", cmd, value);

	cmdline_prepare();

	if(!is_invalid_cmd(cmd))return;

	len = strlen(g_CPcmdlineBuf);
	p = g_CPcmdlineBuf + len;

	snprintf(p, MAX_CP_CMDLINE_LEN - len, "%s=%s ", cmd, value);
	//printf("cmd = %s\n" , p);
}

void cp_cmdline_fixup(void)
{
	char *value = NULL;
	int boardid = 0;
#if defined(CONFIG_SP7731C) || defined(CONFIG_32K_LESS_COMPATIBLE) ||defined(CONFIG_32K_SPRD)
	int value_less = 0;
#endif
	int value_crystal = 0;
	char buf[10] = {0};
	unsigned char v32kless;

	cmdline_prepare();
	// androd boot mode
	value = getenv("bootmode");
	if (NULL != value)
	{
		cmdline_add_cp_cmdline(BOOT_MODE, value);
	}

	// calibration parameters
	value = get_calibration_parameter();
	if (NULL != value)
	{
		if (NULL != strstr(value, CALIBRATION_MODE))
		{
			// "calibration=%d,%d,146 ", must skipp calibration=
			value += strlen(CALIBRATION_MODE) + 1;
			cmdline_add_cp_cmdline(CALIBRATION_MODE, value);
		} else if (NULL != strstr(value, AUTO_TEST)) {
			// "autotest=1", must skipp autotest=
			value += strlen(AUTO_TEST) + 1;
			cmdline_add_cp_cmdline(AUTO_TEST, value);
		}
	}

	// lte mode
	value = cpcmdline_get_lte_mode();
	if(NULL != value)
	{
		cmdline_add_cp_cmdline(LTE_MODE, value);
	}

	//rf borad id
	boardid = Sprd_getHWRFboardID();
	if (INVALID_BOARD_ID != boardid)
	{
		sprintf(buf,"%d",boardid);
		cmdline_add_cp_cmdline(RF_BOARD_ID, buf);

		// get hw info
		value = cpcmdline_get_hw_info((unsigned char)boardid, &v32kless);
		if (NULL != value)
		{
			g_prfhwinfo = value;
			cmdline_add_cp_cmdline(RF_HW_INFO, value);
#ifndef CONFIG_32K_LESS_COMPATIBLE
			if (v32kless > 0)
			{
				sprintf(buf, "%d", v32kless);
				cmdline_add_cp_cmdline(K32_LESS, buf);
			}
#endif
		}
	}
#if defined(CONFIG_SP7731C)
	//32 less
	value_less=sprd_get32_less();
	sprintf(buf,"%d",value_less);
	cmdline_add_cp_cmdline(K32_LESS, buf);
#endif
#ifdef CONFIG_SIMLOCK
	cmdline_add_cp_cmdline(SL_KEY_N, SL_RSA_N);
	cmdline_add_cp_cmdline(SL_KEY_E, SL_RSA_E);
#endif

#if defined(CONFIG_32K_LESS_COMPATIBLE)
	//32k less
	value_less=sprd_get32_less();
	sprintf(buf,"%d",value_less);
	cmdline_add_cp_cmdline(K32_LESS, buf);
#endif

#if defined(CONFIG_32K_SPRD)
	//32k less
	value_less=sprd_get32_less();
	sprintf(buf,"%d",value_less);
	cmdline_add_cp_cmdline(K32_LESS, buf);
#endif

#if defined(CONFIG_TSX)
	//tsx
	value_crystal = sprd_get_crystal();
	sprintf(buf,"%d",value_crystal);
	cmdline_add_cp_cmdline(CRYSTAL_TYPE, buf);
#endif
	printf("cp cmdline: %s\n", g_CPcmdlineBuf);
}

char *cp_getcmdline(void)
{
	return g_CPcmdlineBuf;
}

char *cp_get_rf_hardware_info(void)
{
	return g_prfhwinfo;
}
