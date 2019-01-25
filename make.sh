if [ "$1" = "" ]
then
	echo "Pleast input command : make tsharkl or make whale or sharklt8"
fi


if [ "$1" = "sp9838aea" ]
then
	export PATH=$PATH:$(pwd)/../prebuilts/gcc/linux-x86/aarch64/aarch64-linux-android-4.9/bin
	export BUILD_DIR=./out
	export make CROSS_COMPILE=aarch64-linux-android-
	export UBOOT_DEBUG_FLAG=-DDEBUG
	make distclean
	make sp9838aea_5mod_config
	make -j4
fi
if [ "$1" = "spwhale_5modeb_volte" ]
then
	export PATH=$PATH:$(pwd)/../prebuilts/gcc/linux-x86/aarch64/aarch64-linux-android-4.9/bin
	export BUILD_DIR=./out
	export make CROSS_COMPILE=aarch64-linux-android-
	export UBOOT_DEBUG_FLAG=-DDEBUG
	make distclean
	make spwhale_5modeb_volte_config
	make -j4
fi

if [ "$1" = "spwhale_5modeb" ]
then
	export PATH=$PATH:$(pwd)/../prebuilts/gcc/linux-x86/aarch64/aarch64-linux-android-4.9/bin
	export BUILD_DIR=./out
	export make CROSS_COMPILE=aarch64-linux-android-
	export UBOOT_DEBUG_FLAG=-DDEBUG
	make distclean
	make spwhale_5modeb_config
	make -j4
fi

if [ "$1" = "sp9830a-5h10_ga1" ]
then
	export PATH=$PATH:$(pwd)/../prebuilts/gcc/linux-x86/arm/arm-eabi-4.8/bin
	export BUILD_DIR=./out
	export make CROSS_COMPILE=arm-eabi-
	export UBOOT_DEBUG_FLAG=-DDEBUG
	make distclean
	make sp9830a-5h10_ga1_config
	make -j4
fi

if [ "$1" = "sp9830a-7h10_ga1" ]
then
	export PATH=$PATH:$(pwd)/../prebuilts/gcc/linux-x86/arm/arm-eabi-4.8/bin
	export BUILD_DIR=./out
	export make CROSS_COMPILE=arm-eabi-
	export UBOOT_DEBUG_FLAG=-DDEBUG
	make distclean
	make sp9830a-7h10_ga1_config
	make -j4
fi
