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

#ifndef _UBOOT_SEC_DRV_H_
#define _UBOOT_SEC_DRV_H_

#include <common.h>

#define TEESMC_32                       0
#define TEESMC_64                       0x40000000
#define TEESMC_FAST_CALL                0x80000000
#define TEESMC_STD_CALL                 0

#define TEESMC_OWNER_MASK               0x3F
#define TEESMC_OWNER_SHIFT              24

#define TEESMC_FUNC_MASK                0xFFFF

#define TEESMC_IS_FAST_CALL(smc_val)    ((smc_val) & TEESMC_FAST_CALL)
#define TEESMC_IS_64(smc_val)           ((smc_val) & TEESMC_64)
#define TEESMC_FUNC_NUM(smc_val)        ((smc_val) & TEESMC_FUNC_MASK)
#define TEESMC_OWNER_NUM(smc_val)       (((smc_val) >> TEESMC_OWNER_SHIFT) & \
                                         TEESMC_OWNER_MASK)

#define TEESMC_CALL_VAL(type, calling_convention, owner, func_num) \
((type) | (calling_convention) | \
(((owner) & TEESMC_OWNER_MASK) << TEESMC_OWNER_SHIFT) |\
((func_num) & TEESMC_FUNC_MASK))

#define TEESMC_OWNER_ARCH 0
#define TEESMC_OWNER_CPU 1
#define TEESMC_OWNER_SIP 2
#define TEESMC_OWNER_OEM 3
#define TEESMC_OWNER_STANDARD 4
#define TEESMC_OWNER_TRUSTED_APP 48
#define TEESMC_OWNER_TRUSTED_OS 50

#define TEESMC_OWNER_TRUSTED_OS_API 63
//
#define TEESMC_FUNCID_SIPCALL_WITH_ARG 0xff02

#define TEESMC64_SIPCALL_WITH_ARG \
TEESMC_CALL_VAL(TEESMC_64, TEESMC_FAST_CALL, TEESMC_OWNER_SIP, \
TEESMC_FUNCID_SIPCALL_WITH_ARG)

#define FUNCTYPE_VERIFY_IMG               0x01
#define FUNCTYPE_PROGRAM_EFUSE            0x02
#define FUNCTYPE_GET_SOCID                0x03
#define FUNCTYPE_UPDATE_VERSION           0x04

struct smc_param64 {
  uint64_t a0;
  uint64_t a1;
  uint64_t a2;
  uint64_t a3;
  uint64_t a4;
  uint64_t a5;
  uint64_t a6;
  uint64_t a7;
};

int uboot_verify_img(uint64_t start_addr, uint64_t lenth);
int uboot_get_socid(uint64_t start_addr, uint64_t lenth);
int uboot_program_efuse(uint64_t start_addr, uint64_t lenth);
int uboot_update_swVersion(uint64_t start_addr, uint64_t lenth);

#define SPRD_AON_APB_APB_EB0 0x402E0000
#define SPRD_REG_EFUSE_CFG0  0x40240010


#endif
