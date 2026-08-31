MAKE := make cc=$(cc)

include ../make_common.mk

ifeq ($(TARGET_PLATFORM),x86)
SMFL=-lsfml-graphics -lsfml-window -lsfml-system -DDRAW_SMFL -DDRAW_DESKTOP
endif

INC=-I.. -I. -I../zic23 -I../old_stuff

BUILD_DIR := build/$(TARGET_PLATFORM)
ARM64_BUILD_DIR := build/arm64

ALSA=`$(PKG_CONFIG) --cflags --libs alsa`
SNDFILE=`$(PKG_CONFIG) --cflags --libs sndfile`

all: build_zic run

build_zic:
	@mkdir -p $(BUILD_DIR)
	$(CC) -g -fms-extensions -o $(BUILD_DIR)/zicPixMini zic.cpp -fpermissive $(INC) $(ALSA) $(RPI) $(CFLAGS) $(LDFLAGS) $(SMFL) $(SNDFILE)

run:
	$(BUILD_DIR)/zicPixMini

screenshot:
	@mkdir -p ../wiki/zicPixMini
	ZIC_SCREENSHOT="../wiki/zicPixMini/screenshot" $(BUILD_DIR)/zicPixMini

buildPi:
	$(MAKE) build_zic cc=arm64
