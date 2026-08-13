MAKE := make cc=$(cc)

include ../make_common.mk

ifeq ($(TARGET_PLATFORM),x86)
SMFL=-lsfml-graphics -lsfml-window -lsfml-system -DDRAW_SMFL -DDRAW_DESKTOP
endif

INC=-I..

BUILD_DIR := build/$(TARGET_PLATFORM)
OBJ_DIR := build/obj/$(TARGET_PLATFORM)
ARM64_BUILD_DIR := build/arm64

ALSA=`$(PKG_CONFIG) --cflags --libs alsa`
SNDFILE=`$(PKG_CONFIG) --cflags --libs sndfile`

PI_TARGET ?= root@zic.local
PI_PASSWORD ?= password
PI_REMOTE_DIR ?= /opt/zicBox

DISCORD_WEBHOOK?=
-include ../secrets.mk

all: build_zic run

build_zic:
	@mkdir -p $(BUILD_DIR)
	time -p $(CC) -g -fms-extensions -o $(BUILD_DIR)/zic zic.cpp -fpermissive $(INC) $(ALSA) $(RPI) $(CFLAGS) $(LDFLAGS) $(SMFL) $(SNDFILE)

run:
	$(BUILD_DIR)/zic

screenshot:
	ZIC_SCREENSHOT="build/screenshot" $(BUILD_DIR)/zic

VERSION ?= $(shell latest="$$(gh release list --limit 200 | grep -oE 'zicGrid-[0-9]+\.[0-9]+' | cut -d- -f2 | sort -V | tail -n 1)"; if test -n "$$latest"; then major="$${latest%%.*}"; minor="$${latest##*.}"; echo "$$major.$$((minor + 1))"; else echo "1.0"; fi)

showNextVersion:
	@echo $(VERSION)

buildPi:
	$(MAKE) build_zic cc=arm64

buildPi4:
	$(MAKE) build_zic cc=rpi4

pi:
	make buildPi
	- sshpass -p "$(PI_PASSWORD)" ssh "$(PI_TARGET)" "killall zic"
	rsync -avz --progress -e "sshpass -p '$(PI_PASSWORD)' ssh" build/arm64/zic "$(PI_TARGET):$(PI_REMOTE_DIR)/zic"
	rsync -avz --progress -e "sshpass -p '$(PI_PASSWORD)' ssh" ../data/ "$(PI_TARGET):$(PI_REMOTE_DIR)/data"
	sshpass -p "$(PI_PASSWORD)" ssh "$(PI_TARGET)" "chmod +x $(PI_REMOTE_DIR)/zic"
	sshpass -p "$(PI_PASSWORD)" ssh "$(PI_TARGET)" "cd $(PI_REMOTE_DIR) && ./zic"

releaseGrid: buildPi4
	@echo "Creating versioned GitHub release for zicGrid $(VERSION)"
	- rm -rf ../os/rpi4/overlay/opt/zicBox || true
	- rm -rf ../os/rpi4/output/target/opt/zicBox || true
	mkdir -p ../os/rpi4/overlay/opt/zicBox
	cp $(ARM64_BUILD_DIR)/zic ../os/rpi4/overlay/opt/zicBox/zic
	cp -r ../data ../os/rpi4/overlay/opt/zicBox/.
	- rm -rf ../os/rpi4/overlay/opt/zicBox/data/.git || true
	cd ../os/rpi4 && make
	- rm ../os/rpi4/output/images/zicGrid-OS-rpi-image-$(VERSION).zip || true
	cd ../os/rpi4/output/images/ && zip zicGrid-OS-rpi-image-$(VERSION).zip sdcard.img
	- rm $(ARM64_BUILD_DIR)/zicGrid-firmware-ARM64-$(VERSION).zip || true
	cd $(ARM64_BUILD_DIR)/ && zip -r zicGrid-firmware-ARM64-$(VERSION).zip zic
	gh release create zicGrid-$(VERSION) ../os/rpi4/output/images/zicGrid-OS-rpi-image-$(VERSION).zip $(ARM64_BUILD_DIR)/zicGrid-firmware-ARM64-$(VERSION).zip --title "zicGrid $(VERSION)" --notes "This release contains both zicGrid firmware and zicOs image for rpi4."
	-$(MAKE) discordReleaseNotify VERSION=$(VERSION)

release: releaseGrid

discordReleaseNotify:
	@if test -n "$(DISCORD_WEBHOOK)"; then \
		curl -H "Content-Type: application/json" \
		-d '{"embeds":[{"title":"🚀 zicGrid $(VERSION) released","description":"Firmware and OS image are available now.","url":"https://github.com/apiel/zicBox/releases/tag/zicGrid-$(VERSION)"}]}' \
		"$(DISCORD_WEBHOOK)"; \
	else \
		echo "DISCORD_WEBHOOK not configured, skipping Discord notification"; \
	fi


