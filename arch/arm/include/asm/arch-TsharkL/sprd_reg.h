#ifndef _SPRD_REG_DEF_H_
#define _SPRD_REG_DEF_H_

#ifndef BIT
#define BIT(x) (1<<x)
#endif

#include "hardware.h"
#include "chip_drv_common_io.h"

#if defined(CONFIG_SC9838A)
#include "chip_sharklt8/__regs_ap_apb.h"
#include "chip_sharklt8/__regs_ap_ahb_rf.h"
#include "chip_sharklt8/__regs_ap_clk.h"
#include "chip_sharklt8/__regs_mm_ahb_rf.h"
#include "chip_sharklt8/__regs_mm_clk.h"
#include "chip_sharklt8/__regs_gpu_apb_rf.h"
#include "chip_sharklt8/__regs_gpu_clk.h"
#include "chip_sharklt8/__regs_aon_apb_rf.h"
#include "chip_sharklt8/__regs_aon_ckg.h"
#include "chip_sharklt8/__regs_aon_clk.h"
#include "chip_sharklt8/__regs_sc2723_ana_glb.h"
#include "chip_sharklt8/__regs_pmu_apb_rf.h"
#include "chip_sharklt8/__regs_pub_apb.h"
#else
#include "chip_x35l64/__regs_ap_apb.h"
#include "chip_x35l64/__regs_ap_ahb_rf.h"
#include "chip_x35l64/__regs_ap_clk.h"
#include "chip_x35l64/__regs_mm_ahb_rf.h"
#include "chip_x35l64/__regs_mm_clk.h"
#include "chip_x35l64/__regs_gpu_apb_rf.h"
#include "chip_x35l64/__regs_gpu_clk.h"
#include "chip_x35l64/__regs_aon_apb_rf.h"
#include "chip_x35l64/__regs_aon_ckg.h"
#include "chip_x35l64/__regs_aon_clk.h"
#include "chip_x35l64/__regs_sc2723_ana_glb.h"
#include "chip_x35l64/__regs_pmu_apb_rf.h"
#include "chip_x35l64/__regs_pub_apb.h"
//#include "chip_x35l64/__sc2723_regulator_map.h
#endif	/*CONFIG_SC9838A*/
#endif	/*_SPRD_REG_DEF_H_*/
