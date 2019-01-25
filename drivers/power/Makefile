#
# Copyright (c) 2009 Wind River Systems, Inc.
# Tom Rix <Tom.Rix at windriver.com>
#
# SPDX-License-Identifier:	GPL-2.0+
#

obj-$(CONFIG_EXYNOS_TMU)	+= exynos-tmu.o
obj-$(CONFIG_FTPMU010_POWER)	+= ftpmu010.o
obj-$(CONFIG_TPS6586X_POWER)	+= tps6586x.o
obj-$(CONFIG_TWL4030_POWER)	+= twl4030.o
obj-$(CONFIG_TWL6030_POWER)	+= twl6030.o
obj-$(CONFIG_PALMAS_POWER)	+= palmas.o

obj-$(CONFIG_POWER) += power_core.o
obj-$(CONFIG_DIALOG_POWER) += power_dialog.o
obj-$(CONFIG_POWER_FSL) += power_fsl.o
obj-$(CONFIG_POWER_I2C) += power_i2c.o
obj-$(CONFIG_POWER_SPI) += power_spi.o
obj-y += ldo.o

ifneq ($(strip $(CONFIG_ADIE_SC2723)$(CONFIG_ADIE_SC2723S)),)
    obj-y += dcdc_cal_sc2723.o
else

ifdef CONFIG_ADIE_SC2713S
    obj-y += dcdc_cal_sc2713s.o
    ifdef CONFIG_ADIE_SC2713
    obj-y += dcdc_cal.o
endif

else
    ifdef CONFIG_SPX15
    obj-y += dcdc_cal_x15.o
else
    ifdef CONFIG_ADIE_SC2731
    obj-y += regulator_2731.o
    else
    obj-y += dcdc_cal.o
endif
endif
endif # CONFIG_ADIE_SC2713S
endif #CONFIG_ADIE_SC2723
