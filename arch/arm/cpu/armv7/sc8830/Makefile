obj-y	:= timer.o glb.o reset.o eic.o check_reboot.o misc.o pmu.o  soc.o
ifdef CONFIG_SECURE_BOOT
obj-y += sec_boot.o rsa.o pk1.o sha1_32.o
endif
ifdef CONFIG_SPX30G
obj-y += chip_phy_x30g.o
else
obj-y += chip_phy_v3.o
endif

ifneq ($(strip $(CONFIG_ADIE_SC2723)$(CONFIG_ADIE_SC2723S)),)
obj-y += vibrator_sc2723.o
else
obj-y += vibrator_sc2713.o
endif

