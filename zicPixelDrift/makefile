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
	time -p $(CC) -g -fms-extensions -o $(BUILD_DIR)/zic zic.cpp -fpermissive $(INC) $(ALSA) $(RPI) $(CFLAGS) $(LDFLAGS) $(SMFL) $(SNDFILE)

run:
	$(BUILD_DIR)/zic

buildPi:
	$(MAKE) build_zic cc=arm64

PI_TARGET ?= root@zic.local
PI_PASSWORD ?= password
PI_REMOTE_DIR ?= /opt/zicBox

pi:
	make buildPi
	- sshpass -p "$(PI_PASSWORD)" ssh "$(PI_TARGET)" "killall zic"
	rsync -avz --progress -e "sshpass -p '$(PI_PASSWORD)' ssh" build/arm64/zic "$(PI_TARGET):$(PI_REMOTE_DIR)/zic"
	sshpass -p "$(PI_PASSWORD)" ssh "$(PI_TARGET)" "chmod +x $(PI_REMOTE_DIR)/zic"
	sshpass -p "$(PI_PASSWORD)" ssh "$(PI_TARGET)" "cd $(PI_REMOTE_DIR) && ./zic"
