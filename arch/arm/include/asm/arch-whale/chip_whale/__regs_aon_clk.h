/* the head file modifier:     g   2014-09-25 15:43:44*/

/*  
* Copyright (C) 2013 Spreadtrum Communications Inc.  
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
*************************************************  
* Automatically generated C header: do not edit *  
*************************************************  
*/  

//#ifndef __SCI_GLB_REGS_H__  
//#error  "Don't include this file directly, Pls include sci_glb_regs.h" 
//#endif 


#ifndef __H_REGS_AON_CLK_CORE_HEADFILE_H__
#define __H_REGS_AON_CLK_CORE_HEADFILE_H__ __FILE__

#define  REGS_AON_CLK_CORE

/* registers definitions for AON_CLK_CORE */
#define REG_AON_CLK_CORE_clk_pwm0_cfg			SCI_ADDR(REGS_AON_CLK_CORE_BASE, 0x48)/*clk_pwm0_cfg*/
#define REG_AON_CLK_CORE_clk_pwm1_cfg			SCI_ADDR(REGS_AON_CLK_CORE_BASE, 0x4c)/*clk_pwm1_cfg*/
#define REG_AON_CLK_CORE_clk_pwm2_cfg			SCI_ADDR(REGS_AON_CLK_CORE_BASE, 0x50)/*clk_pwm2_cfg*/
#define REG_AON_CLK_CORE_clk_pwm3_cfg			SCI_ADDR(REGS_AON_CLK_CORE_BASE, 0x54)/*clk_pwm3_cfg*/

/* bits definitions for register REG_AON_CLK_CORE_clk_pwm0_cfg */
#define BITS_CLK_PWM0_SEL(_X_)		( (_X_) & (BIT(0)|BIT(1)) )

/* bits definitions for register REG_AON_CLK_CORE_clk_pwm1_cfg */
#define BITS_CLK_PWM1_SEL(_X_)		( (_X_) & (BIT(0)|BIT(1)) )

/* bits definitions for register REG_AON_CLK_CORE_clk_pwm2_cfg */
#define BITS_CLK_PWM2_SEL(_X_)		( (_X_) & (BIT(0)|BIT(1)) )

/* bits definitions for register REG_AON_CLK_CORE_clk_pwm3_cfg */
#define BITS_CLK_PWM3_SEL(_X_)		( (_X_) & (BIT(0)|BIT(1)) )

#endif
