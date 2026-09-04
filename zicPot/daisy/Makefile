# Project Name
TARGET = zicPot_daisy

# Sources
CPP_SOURCES = zic.cpp

# C++ Standard
CPP_STANDARD = -std=gnu++17

# Optimize for size
OPT = -Os

# APP TYPE (BOOT_NONE target for standard ST System DFU bootloader @ 0x08000000)
APP_TYPE = BOOT_NONE

# Library Locations (pointing to zicFilterbank)
ZIC_FILTERBANK_DIR ?= /home/alex/Music/zicFilterbank
DAISYSP_DIR ?= $(ZIC_FILTERBANK_DIR)/DaisySP
LIBDAISY_DIR ?= $(ZIC_FILTERBANK_DIR)/libDaisy

# Include paths for zicBox and zicFilterbank (for fonts and headers)
C_INCLUDES += -I../../ -I.. -I$(ZIC_FILTERBANK_DIR)

# Core location and generic Makefile from libDaisy
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile
