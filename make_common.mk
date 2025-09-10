CC := g++

PKG_CONFIG := pkg-config

MAKEFILE_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

#BUILDROOT_ZERO2W64_DIR=$(MAKEFILE_DIR)/../zicOs/zero2w64/output
BUILDROOT_ZERO2W64_DIR=$(MAKEFILE_DIR)/os/zero2w64/output
CC_ZERO2W64 := $(BUILDROOT_ZERO2W64_DIR)/host/bin/aarch64-linux-g++

ifeq ($(cc),pixel_arm64)
	RPI := -DIS_RPI=1
    CC := $(CC_ZERO2W64)
 	TARGET_PLATFORM := pixel_arm64
	SYSROOT := $(BUILDROOT_ZERO2W64_DIR)/staging

	CFLAGS += --sysroot=$(SYSROOT) -I$(SYSROOT)/usr/include
	LDFLAGS += --sysroot=$(SYSROOT) -L$(SYSROOT)/usr/lib -L$(SYSROOT)/lib

	PKG_CONFIG := PKG_CONFIG_SYSROOT_DIR=$(SYSROOT) PKG_CONFIG_PATH=$(SYSROOT)/usr/lib/pkgconfig pkg-config
endif

# if TARGET_PLATFORM is not set and uname -m is x86_64, then we are on x86
# else we are on arm, let's use generic folder `arm` if nothing specified.
ifeq ($(TARGET_PLATFORM),)
	ifeq ($(shell uname -m),x86_64)
		TARGET_PLATFORM := x86
	else
		TARGET_PLATFORM := arm
		RPI := -DIS_RPI=1
	endif
endif
