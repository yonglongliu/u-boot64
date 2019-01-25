

#ifndef __SEC_COMMON_H
#define __SEC_COMMON_H

#include<secureboot/sprdsec_header.h>
#include<secureboot/sprd_rsa.h>

typedef struct{
	uint64_t img_addr; //the base address of image to verify
	uint64_t img_len; //length of image
	uint64_t pubkeyhash;//pubkey hash for verifying image
	uint64_t hash_len; //length of pubkeyhash
	uint32_t flag;//sprd or sansa plan
}imgToVerifyInfo;

#define SPRD_FLAG 1
#define SANSA_FLAG 2
extern imgToVerifyInfo img_verify_info;
/*this two marco is diffent among products, pls check it!*/
#if defined (CONFIG_WHALE)
#define IRAM_BEGIN 0x00009e00
#endif
#if defined (CONFIG_SP7730SW) || defined (CONFIG_SP9832A_2H11_VOLTE) || defined (CONFIG_SP9832A_2H12_VOLTE)  || defined (CONFIG_SP9832A_7C10_VOLTE) || defined (CONFIG_SP9832A_7C20_VOLTE)
#define IRAM_BEGIN 0x50000000
#endif

#define UBOOT_START  0x9efffe00
#define VERIFY_ADDR 0x9d000000 //for 32bit TOS verification
#define SIMG_BUF     0x1500000
#define CERT_SIZE    0x1000
#define SYS_HEADER_SIZE 0x200
#define UBOOT_SEC_HASH 0x85000000
#define UBOOT_SEC_PUBKEY UBOOT_SEC_HASH+0x100
#define SEC_SPL_LEN 0xa000 //40k
#define SEC_SIGN 0xC00
typedef struct{
	uint32_t mMagicNum; //0x42544844
	uint32_t mVersion; // 1
	uint8_t mPayloadHash[32]; //sha256 hash val
	uint64_t mImgAddr;  // image loader address
	uint32_t mImgSize;  //image size
	uint8_t reserved[460]; // 460 + 13*4 = 512
}sys_img_header;


void secboot_verify(void *ptr,void *m,void *n,uint32_t data_len);

int dl_secure_verify(void *partition_name,void *header,void *code);

int secboot_enable_check(void);

void secboot_param_set(uint64_t load_buf,imgToVerifyInfo *imginfo);

void secboot_get_pubkhash(uint64_t img_buf,uint64_t imginfo);
int secure_efuse_program();

#endif
