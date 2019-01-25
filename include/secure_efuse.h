/*
 * Copyright (C) 2014 Spreadtrum Communications Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#define PROG_OK     0
#define SHA1_ERR   -1
#define AES256_ERR -2
#define SHA256_ERR -3
/***96K***/
#define SPL_IMAGE_SIZE (96*1024)
#define SECTION_SIZE   (512)
/**
SPL_EMMC_SECTOR_NUM=SPL_IMAGE_SIZE / SECTOR_SIZE
**/
#define SPL_EMMC_SECTOR_NUM  (SPL_IMAGE_SIZE/SECTION_SIZE)
#define SAVE_SPL_IMAGE  0x9d000000



#include <common.h>


int secure_efuse_program(void);
int read_master_key(void);

