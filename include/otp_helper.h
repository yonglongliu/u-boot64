#ifndef _OTP_HELP_H_
#define _OTP_HELP_H_

extern u32 __ddie_efuse_read(int blk_index);
extern u32 __adie_efuse_read(int blk_index);
extern u32 __adie_efuse_read_bits(int bit_index, int length);
extern u32 __ddie_arm7_efuse_read(int blk_index);
void __adie_efuse_block_dump(void);
void __ddie_efuse_block_dump(void);
extern int soc_is_sharklc(void);
extern int soc_is_sharkls(void);

enum EFUSE_ID{
	DDIE_EFUSE_ID,
	ARM7_EFUSE_ID,
	ADIE_EFUSE_ID
};
#endif
