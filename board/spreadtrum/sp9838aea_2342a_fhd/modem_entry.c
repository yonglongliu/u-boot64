#include <common.h>


//#include "normal_mode.h"
#include <asm/arch/sprd_reg.h>

#include "cp_boot.h"

#define CP0_ZERO_MAP_ADR    0x50000000
#define CP0_ARM0_EXEC_ADR        0x88400000 

#define CP1_ZERO_MAP_ADR    0x50001000
#define CP1_EXEC_ADR        0x8ab48000


#define PHYS_OFFSET_ADDR			0x80000000
//#define TD_CP_OFFSET_ADDR			0x8000000	/*128*/
//#define TD_CP_SDRAM_SIZE			0x1200000	/*18M*/
//#define WCDMA_CP_OFFSET_ADDR		0x10000000	/*256M*/
//#define WCDMA_CP_SDRAM_SIZE		0x4000000	/*64M*/
#define GGE_CP_OFFSET_ADDR 		0x08000000       /*128M*/
#define GGE_CP_SDRAM_SIZE 		0x01600000 	/*22M*/
#define LTE_CP_OFFSET_ADDR              0x09600000       /*150M*/
#define LTE_CP_SDRAM_SIZE        0x04000000     /*64M*/
#define PMIC_IRAM_ADDR 		0x50800000       /*pmic arm7 iram address remap at AP side*/
#define PMIC_IRAM_SIZE 		0x8000       /*32K*/
#define SIPC_APCP_RESET_ADDR_SIZE	0xC00	/*3K*/
#define SIPC_APCP_RESET_SIZE	0x1000	/*4K*/
//#define SIPC_TD_APCP_START_ADDR		(PHYS_OFFSET_ADDR + TD_CP_OFFSET_ADDR + TD_CP_SDRAM_SIZE - SIPC_APCP_RESET_SIZE)	/*0x897FF000*/
//#define SIPC_WCDMA_APCP_START_ADDR	(PHYS_OFFSET_ADDR + WCDMA_CP_OFFSET_ADDR + WCDMA_CP_SDRAM_SIZE - SIPC_APCP_RESET_SIZE) /*0x93FFF000*/
#define SIPC_GGE_APCP_START_ADDR 	(PHYS_OFFSET_ADDR + GGE_CP_OFFSET_ADDR + GGE_CP_SDRAM_SIZE - SIPC_APCP_RESET_SIZE) 	/*0x895ff000*/
#define SIPC_LTE_APCP_START_ADDR    (0x8AA40000)
#define SIPC_PMIC_SIPC_START_ADDR        (0x5000B000)         /*0x50807400*/



void cp_adr_remap(u32 cp_kernel_exec_addr, u32 cp_zero_map_addr)
{
    u32 data[3] = {0xe59f0000, 0xe12fff10,   cp_kernel_exec_addr};
    memcpy( cp_zero_map_addr, data, sizeof(data));      /* copy cp0 source code */
}

void modem_entry()
{
	/*sipc addr clear*/
	sipc_addr_reset();

#if !defined( CONFIG_KERNEL_BOOT_CP )
	pmic_arm7_boot();

	debugf(" REG_PMU_APB_CP_SOFT_RST=%x,REG_AON_APB_APB_RST1=0x%x\n",  REG_PMU_APB_CP_SOFT_RST,REG_AON_APB_APB_RST1);
	//cp_adr_remap(CP0_ARM0_EXEC_ADR, CP0_ZERO_MAP_ADR);
	//cp0_arm0_boot();

	cp_adr_remap(CP1_EXEC_ADR, CP1_ZERO_MAP_ADR);
	cp1_boot();
#endif
}


void sipc_addr_reset()
{

	/*to be fill in*/
	//memset((void *)SIPC_GGE_APCP_START_ADDR, 0x0, SIPC_APCP_RESET_ADDR_SIZE);
	memset((void *)SIPC_LTE_APCP_START_ADDR, 0x0, SIPC_APCP_RESET_ADDR_SIZE);
	memset((void *)SIPC_PMIC_SIPC_START_ADDR, 0x0, 0x2000);
	return;

}



