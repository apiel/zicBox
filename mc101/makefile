MAKE := make cc=$(cc)

include ../make_common.mk

ifeq ($(TARGET_PLATFORM),x86)
SMFL=-lsfml-graphics -lsfml-window -lsfml-system -DDRAW_SMFL -DDRAW_DESKTOP
endif

INC=-I..

BUILD_DIR := build/$(TARGET_PLATFORM)
OBJ_DIR := build/obj/$(TARGET_PLATFORM)

ALSA=`$(PKG_CONFIG) --cflags --libs alsa`

all: build_mc101 run

build_mc101:
	@mkdir -p $(BUILD_DIR)
	$(CC) -g -fms-extensions -o $(BUILD_DIR)/mc101 mc101.cpp -fpermissive $(INC) $(ALSA) $(RPI) $(CFLAGS) $(LDFLAGS) $(SMFL)

run:
	$(BUILD_DIR)/mc101