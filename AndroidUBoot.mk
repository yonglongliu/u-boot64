ifeq ($(TARGET_ARCH) , arm64)
UBOOT_TOOLCHAIN := aarch64-linux-android-
else
UBOOT_TOOLCHAIN := arm-eabi-
endif

ifeq ($(strip $(CONFIG_64KERNEL_32FRAMEWORK)),true)
UBOOT_TOOLCHAIN := $(FIX_CROSS_COMPILE)
endif

UBOOT_OUT := $(TARGET_OUT_INTERMEDIATES)/u-boot64
UBOOT_CONFIG := $(UBOOT_OUT)/include/config.h
UBOOT_BUILT_BIN := $(UBOOT_OUT)/u-boot.bin
FDL2_BUILT_BIN := $(UBOOT_OUT)/fdl2.bin

ifeq ($(TARGET_BUILD_VARIANT),userdebug)
UBOOT_DEBUG_FLAG := -DDEBUG
endif

export UBOOT_CONFIG_PRODUCT UBOOT_DEBUG_FLAG

.PHONY: $(UBOOT_OUT)
$(UBOOT_OUT):
	@echo "Start U-Boot build board $(UBOOT_DEFCONFIG)"

$(UBOOT_CONFIG): u-boot64/include/configs/$(addsuffix .h,$(UBOOT_DEFCONFIG)) $(UBOOT_OUT)
	@mkdir -p $(UBOOT_OUT)
	$(MAKE) -C u-boot64 CROSS_COMPILE=$(UBOOT_TOOLCHAIN) O=../$(UBOOT_OUT) distclean
	$(MAKE) -C u-boot64 CROSS_COMPILE=$(UBOOT_TOOLCHAIN) O=../$(UBOOT_OUT) $(UBOOT_DEFCONFIG)_config

ifeq ($(strip $(NORMAL_UART_MODE)),true)
	@echo "#define NORMAL_UART_MODE" >> $(UBOOT_CONFIG)
endif

ifeq ($(strip $(CONFIG_SPRD_SECBOOT)),true)
	@echo "#define CONFIG_SPRD_SECBOOT" >> $(UBOOT_OUT)/include/config.h
	@echo "#define CONFIG_SECBOOT" >> $(UBOOT_OUT)/include/config.h
	@echo "CONFIG_SPRD_SECBOOT = y" >> $(UBOOT_OUT)/include/config.mk
endif

ifeq ($(strip $(CONFIG_SANSA_SECBOOT)),true)
	@echo "#define CONFIG_SANSA_SECBOOT" >> $(UBOOT_OUT)/include/config.h
	@echo "#define CONFIG_SECBOOT" >> $(UBOOT_OUT)/include/config.h
	@echo "CONFIG_SANSA_SECBOOT = y" >> $(UBOOT_OUT)/include/config.mk
endif

ifeq ($(strip $(BOARD_TEE_CONFIG)), trusty)
	@echo "#define CONFIG_WITH_SECURE_TOS 1" >> $(UBOOT_CONFIG)
endif

ifeq ($(strip $(BOARD_VMM_CONFIG)), true)
	@echo "CONFIG_WITH_VMM = y" >> $(UBOOT_OUT)/include/config.mk
endif

$(INSTALLED_UBOOT_TARGET) : $(UBOOT_CONFIG)
	$(MAKE) -C u-boot64 CROSS_COMPILE=$(UBOOT_TOOLCHAIN) AUTOBOOT_FLAG=true O=../$(UBOOT_OUT)
	@cp $(UBOOT_BUILT_BIN) $(PRODUCT_OUT)/u-boot_autopoweron.bin
	$(MAKE) -C u-boot64 CROSS_COMPILE=$(UBOOT_TOOLCHAIN) O=../$(UBOOT_OUT) clean
	$(MAKE) -C u-boot64 CROSS_COMPILE=$(UBOOT_TOOLCHAIN) O=../$(UBOOT_OUT)
	@cp $(UBOOT_BUILT_BIN) $(PRODUCT_OUT)
	@cp $(UBOOT_BUILT_BIN) $(FDL2_BUILT_BIN)
	@cp $(FDL2_BUILT_BIN) $(PRODUCT_OUT)
	@echo "Install U-Boot target done"
