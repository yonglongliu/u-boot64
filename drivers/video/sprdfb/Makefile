#
# (C) Copyright 2000-2007
# Haibing.Yang haibing.yang@spreadtrum.com
#
# See file CREDITS for list of people who contributed to this
# project.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

# obj-$(CONFIG_LCD) += sprdfb_backlight.o

## basic fb driver
obj-$(CONFIG_LCD) += sprdfb_main.o sprdfb_panel.o
obj-$(CONFIG_LCD) += sprdfb_rgb.o sprdfb_mipi.o sprdfb_dsi.o
ifeq ($(CONFIG_FB_SWDISPC), y)
	obj-$(CONFIG_LCD) += sprdfb_swdispc.o
else
	obj-$(CONFIG_LCD) += sprdfb_dispc.o
endif
## old panel interface
ifeq ($(CONFIG_LCD), y)
obj-$(CONFIG_I2C) += sprdfb_mcu.o sprdfb_i2c.o
obj-$(CONFIG_SPI) += sprdfb_spi.o
endif

obj-$(CONFIG_LCD) += sprdfb_chip_common.o
## related to specific chip
ifeq ($(CONFIG_LCD), y)
obj-$(CONFIG_SC9836A) += sprdfb_chip_9630.o
obj-$(CONFIG_SC9838A) += sprdfb_chip_9630.o
obj-$(CONFIG_SC9630) += sprdfb_chip_9630.o
obj-$(CONFIG_SC8830) += sprdfb_chip_8830.o
obj-$(CONFIG_SC9001) += sprdfb_chip_9001.o
endif

## only related to FPGA
obj-$(CONFIG_FPGA)+= sprdfb_gpio2spi.o
obj-$(CONFIG_FPGA)+= sprdfb_usc28c_init.o

