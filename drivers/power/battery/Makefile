#
# Copyright (C) 2012 Samsung Electronics
# Lukasz Majewski <l.majewski@samsung.com>
#
# SPDX-License-Identifier:	GPL-2.0+
#

obj-$(CONFIG_POWER_BATTERY_TRATS) += bat_trats.o
obj-$(CONFIG_POWER_BATTERY_TRATS2) += bat_trats2.o
ifdef CONFIG_ADIE_SC2713S
obj-y += sprd_battery_2713.o
else
ifdef CONFIG_ADIE_SC2731
obj-y += sprd_battery_2731.o
else
obj-y += sprd_battery.o
endif
endif
obj-$(CONFIG_SPRD_EXT_IC_POWER)	+= sprd_chg_helper.o
obj-$(CONFIG_FAN54015_CHARGE_IC)+= fan54015.o
obj-$(CONFIG_SPRD2701_CHARGE_IC)+= sprd_2701.o

