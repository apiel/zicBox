CC?=g++

BUILDROOT_DIR=../zicOs/buildroot/output


ifeq ($(cc),arm)
    CC := /host/bin/arm-buildroot-linux-gnueabihf-g++
	TARGET_PLATFORM := arm
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