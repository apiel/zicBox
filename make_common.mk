CC=g++

ifneq ($(shell uname -m),x86_64)
TARGET_PLATFORM := arm
else
TARGET_PLATFORM := x86
endif