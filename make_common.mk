CC?=g++

ifeq ($(cc),arm)
	MAKEFILE_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
	BUILDROOT_DIR=$(MAKEFILE_DIR)../zicOs/buildroot/output

    CC := $(BUILDROOT_DIR)/host/bin/aarch64-buildroot-linux-gnu-g++
	TARGET_PLATFORM := arm
	SYSROOT = $(BUILDROOT_DIR)/staging

	CFLAGS += --sysroot=$(SYSROOT) -I$(SYSROOT)/usr/include
	LDFLAGS += --sysroot=$(SYSROOT) -L$(SYSROOT)/usr/lib -L$(SYSROOT)/lib
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