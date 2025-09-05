CC := g++

PKG_CONFIG := pkg-config

ifeq ($(cc),arm)
	MAKEFILE_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
	# BUILDROOT_DIR=$(MAKEFILE_DIR)../zicOs/buildroot/output
	BUILDROOT_DIR=$(MAKEFILE_DIR)../zicOs/buildroot/output2

    # CC := $(BUILDROOT_DIR)/host/bin/aarch64-buildroot-linux-gnu-g++
	CC := $(BUILDROOT_DIR)/host/bin/arm-buildroot-linux-gnueabihf-g++
	TARGET_PLATFORM := arm
	SYSROOT := $(BUILDROOT_DIR)/staging

	CFLAGS += --sysroot=$(SYSROOT) -I$(SYSROOT)/usr/include
	LDFLAGS += --sysroot=$(SYSROOT) -L$(SYSROOT)/usr/lib -L$(SYSROOT)/lib

	PKG_CONFIG := PKG_CONFIG_SYSROOT_DIR=$(SYSROOT) PKG_CONFIG_PATH=$(SYSROOT)/usr/lib/pkgconfig pkg-config
endif

ifeq ($(cc),arm64)
	MAKEFILE_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
	BUILDROOT_DIR=$(MAKEFILE_DIR)../zicOs/zero2w64/output

    CC := $(BUILDROOT_DIR)/host/bin/aarch64-linux-g++
	TARGET_PLATFORM := arm
	SYSROOT := $(BUILDROOT_DIR)/staging

	CFLAGS += --sysroot=$(SYSROOT) -I$(SYSROOT)/usr/include
	LDFLAGS += --sysroot=$(SYSROOT) -L$(SYSROOT)/usr/lib -L$(SYSROOT)/lib

	PKG_CONFIG := PKG_CONFIG_SYSROOT_DIR=$(SYSROOT) PKG_CONFIG_PATH=$(SYSROOT)/usr/lib/pkgconfig pkg-config
endif

# if TARGET_PLATFORM is not set and uname -m is x86_64, then we are on x86
ifeq ($(TARGET_PLATFORM),)
	ifeq ($(shell uname -m),x86_64)
		TARGET_PLATFORM := x86
	else
		TARGET_PLATFORM := arm
	endif
endif

ifeq ($(TARGET_PLATFORM),arm)
    RPI := -DIS_RPI=1
endif