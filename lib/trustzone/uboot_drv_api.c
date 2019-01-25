/*
 * Copyright (c) 2013-2014, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "uboot_sec_drv.h"

int uboot_verify_img(uint64_t start_addr, uint64_t lenth){

  uint32_t ret;
  uint32_t smcid;
  uint32_t funcid;

  //1、准备传入参数
  smcid  = TEESMC64_SIPCALL_WITH_ARG;
  funcid = FUNCTYPE_VERIFY_IMG;

  struct smc_param64 param;
  param.a0 = smcid;
  param.a1 = funcid;
  param.a2 = start_addr;
  param.a3 = lenth;

  //2、调用SMC
  tee_smc_call64(&param);

  //4、获取返回值
  ret = param.a0;
  return ret;
}
static void efuse_power_on(void)
{
  unsigned int i;
  printf("Efuse_power_on\n");
  *(volatile unsigned int *)(SPRD_AON_APB_APB_EB0) |= (1<<13);
  *(volatile unsigned int *)(SPRD_REG_EFUSE_CFG0) = 0xc0000140;
  for(i=0;i<0xFFFF;i++);
  *(volatile unsigned int *)(SPRD_REG_EFUSE_CFG0) = 0xd0000140;
}

static void efuse_power_off(void)
{
  unsigned int i;
  printf("Efuse_power_off\n");
  if((*(volatile unsigned int *)(SPRD_REG_EFUSE_CFG0))&(1<<28))
  {
    *(volatile unsigned int *)(SPRD_REG_EFUSE_CFG0) = 0xc0000140;
	for(i=0;i<0xFFFF;i++);
  }
  *(volatile unsigned int *)(SPRD_REG_EFUSE_CFG0) = 0xa0000140;
  *(volatile unsigned int *)(SPRD_AON_APB_APB_EB0) &= ~(1<<13);
}

//int uboot_program_efuse(uint64_t start_addr, unsigned char* hbk, uint64_t hbkLenth){
  int uboot_program_efuse(uint64_t start_addr, uint64_t lenth){

  uint32_t ret;
  uint32_t smcid;
  uint32_t funcid;
  
  efuse_power_on();
  smcid  = TEESMC64_SIPCALL_WITH_ARG;
  funcid = FUNCTYPE_PROGRAM_EFUSE;
  printf("###### uboot_program_efuse ######\n");
  struct smc_param64 param;
  param.a0 = smcid;
  param.a1 = funcid;
  param.a2 = start_addr;
  param.a3 = lenth;
  printf("a0=%x,a1=%x,a2=%x,a3=%x\n",param.a0,param.a1,param.a2,param.a3);
  tee_smc_call64(&param);

  ret = param.a0;
  efuse_power_off();
  printf("smcid=%x\n",param.a0);
  return ret;
}

int uboot_get_socid(uint64_t start_addr, uint64_t lenth){

  uint32_t ret;
  uint32_t smcid;
  uint32_t funcid;

  smcid  = TEESMC64_SIPCALL_WITH_ARG;
  funcid = FUNCTYPE_GET_SOCID;

  struct smc_param64 param;
  param.a0 = smcid;
  param.a1 = funcid;
  param.a2 = start_addr;
  param.a3 = lenth;

  tee_smc_call64(&param);

  ret = param.a0;
  return ret;
}

int uboot_update_swVersion(uint64_t start_addr, uint64_t lenth){

  uint32_t ret;
  uint32_t smcid;
  uint32_t funcid;

  smcid  = TEESMC64_SIPCALL_WITH_ARG;
  funcid = FUNCTYPE_UPDATE_VERSION;

  struct smc_param64 param;
  param.a0 = smcid;
  param.a1 = funcid;
  param.a2 = start_addr;
  param.a3 = lenth;

  tee_smc_call64(&param);

  ret = param.a0;
  return ret;
}
