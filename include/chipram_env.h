#ifndef _CHIPRAM_ENV_H_
#define _CHIPRAM_ENV_H_
#include <asm/arch/chip_releted_def.h>

typedef enum bootloader_mode {
	BOOTLOADER_MODE_UNKNOW = 0x100,
	BOOTLOADER_MODE_DOWNLOAD = 0x102,
	BOOTLOADER_MODE_LOAD =0x104
}boot_mode_t;

#define CHIPRAM_ENV_MAGIC	0x43454e56
typedef struct chipram_env {
	u32 magic; //0x43454e56
	boot_mode_t mode;
	u32 dram_size; //MB
	u64 vol_para_addr;
	u32 keep_charge;
	u32 reserved;
}chipram_env_t;

chipram_env_t* get_chipram_env(void);
boot_mode_t get_boot_role(void);

#endif

