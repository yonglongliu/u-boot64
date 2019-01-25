#include <secureboot/sprdsec_header.h>
#include <secureboot/sprd_rsa.h>
#include <secureboot/sec_string.h>
#include <secureboot/sprdsha.h>
#include <secure_verify.h>
#include <asm/arch/chip_drv_common_io.h>
#include <asm/arch/sci_types.h>
#include <asm/arch/sprd_reg.h>
#include <secureboot/sec_common.h>


#define EFUSE_HASH_STARTID 2
 imgToVerifyInfo img_verify_info = {0};
uint8_t pubkhash[32];

void secboot_verify(void *ptr,void *m,void *n,uint32_t data_len)
{
	uint32_t jflag;
	jflag = data_len;

	switch (jflag){
		case SPRD_FLAG:
			sprd_secure_check((uint8_t*)ptr,(uint8_t*)m);
			break;
		case SANSA_FLAG:
			//sansa_secure_check(m,ptr);
			break;
		default:
			break;
	}
#if 0
#ifdef CONFIG_SPRD_SECBOOT
	sprd_secure_check((uint8_t *)ptr,(uint8_t *)m);
#endif
#ifdef CONFIG_SANSA_SECBOOT
	sansa_secure_check(m,ptr);
#endif
#ifdef CONFIG_NORMAL_SECBOOT
	/*pls add thark progtamme interface*/
#endif
#endif

}

int dl_secure_verify(void *partition_name,void *header,void *code)
{
#ifdef CONFIG_SPRD_SECBOOT
		sprd_dl_verify(partition_name, header, code);
#elif defined CONFIG_SANSA_SECBOOT
	sansa_dl_secure_verify(partition_name,header,code);
#else
	secure_verify(partition_name,header,code);
#endif
}

int secboot_enable_check(void)
{
//#if defined (CONFIG_SECBOOT) 
	uint32_t reg = 0;
	uint32_t bonding = REG32(REG_AON_APB_BOND_OPT0);
	 // uint32_t bonding = (*((volatile uint32_t *)(REG_AON_APB_BOND_OPT0)));
	if(bonding & BIT_2){
		reg = __ddie_efuse_read(EFUSE_HASH_STARTID);
		if ((reg >> 31) & 0x1)
			return 1;
	}
//#endif
	return 0;
}

int secure_efuse_program()
{
	int ret = 0;

	#if defined(CONFIG_SANSA_SECBOOT)
		    printf("###### secure_efuse_program ######\n");
			ret = secure_efuse_program_sansa();
	#endif

	#if defined(CONFIG_SPRD_SECBOOT)
			ret = secure_efuse_program_sprd();
	#endif

	#if defined(CONFIG_SECURE_BOOT)
			ret = secure_efuse_program_native();
	#endif
}

void  secboot_param_set(uint64_t load_buf,imgToVerifyInfo *imginfo)
{
	imginfo->img_addr = load_buf;
	imginfo->img_len = SIMG_BUF;
	imginfo->hash_len = 32;
	imginfo->pubkeyhash = pubkhash;
#ifdef CONFIG_SPRD_SECBOOT
	imginfo->flag = SPRD_FLAG;
#endif
#ifdef CONFIG_SANSA_SECBOOT
	imginfo->flag = SANSA_FLAG;
#endif
}

void secboot_get_pubkhash(uint64_t img_buf,uint64_t imginfo)
{
#if defined (CONFIG_SPRD_SECBOOT)
	sprd_get_hash_key((uint8_t *)img_buf,(uint8_t *)imginfo);//be carefull! the first param in chipram is not same as uboot64
#endif
#if defined (CONFIG_SANSA_SECBOOT)
	sansa_get_hash_key((uint8_t *)img_buf,(uint8_t *)imginfo);
#endif
}
