CC=g++

ifeq ($(shell uname -m),x86_64)
TARGET_PLATFORM := x86
else
RPI := -DIS_RPI=1
TARGET_PLATFORM := arm
endif