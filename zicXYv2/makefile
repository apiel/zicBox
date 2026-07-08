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

all: build_zic run

build_zic:
	@mkdir -p $(BUILD_DIR)
	$(CC) -g -fms-extensions -o $(BUILD_DIR)/zic zic.cpp -fpermissive $(INC) $(ALSA) $(RPI) $(CFLAGS) $(LDFLAGS) $(SMFL) $(SNDFILE)

run:
	$(BUILD_DIR)/zic

PI_TARGET ?= root@zic.local
PI_PASSWORD ?= password
PI_REMOTE_DIR ?= /opt/zicBox

VERSION ?= $(shell latest="$$(gh release list --limit 200 | grep -oE 'zicXY-[0-9]+\.[0-9]+' | cut -d- -f2 | sort -V | tail -n 1)"; if test -n "$$latest"; then major="$${latest%%.*}"; minor="$${latest##*.}"; echo "$$major.$$((minor + 1))"; else echo "2.0"; fi)

showNextVersion:
	@echo $(VERSION)

buildPi:
	$(MAKE) build_zic cc=arm64

pi:
	make buildPi
	- sshpass -p "$(PI_PASSWORD)" ssh "$(PI_TARGET)" "killall zic"
	rsync -avz --progress -e "sshpass -p '$(PI_PASSWORD)' ssh" build/arm64/zic "$(PI_TARGET):$(PI_REMOTE_DIR)/zic"
	rsync -avz --progress -e "sshpass -p '$(PI_PASSWORD)' ssh" ../data/ "$(PI_TARGET):$(PI_REMOTE_DIR)/data"
	sshpass -p "$(PI_PASSWORD)" ssh "$(PI_TARGET)" "chmod +x $(PI_REMOTE_DIR)/zic"
	sshpass -p "$(PI_PASSWORD)" ssh "$(PI_TARGET)" "cd $(PI_REMOTE_DIR) && ./zic"

releaseXY: buildPi
	@echo "Creating versioned GitHub release for zicXY $(VERSION)"
	- rm -rf ../os/zero2w64/overlay/opt/zicBox || true
	- rm -rf ../os/zero2w64/output/target/opt/zicBox || true
	mkdir -p ../os/zero2w64/overlay/opt/zicBox
	cp $(ARM64_BUILD_DIR)/zic ../os/zero2w64/overlay/opt/zicBox/zic
	cp -r ../data ../os/zero2w64/overlay/opt/zicBox/.
	- rm -rf ../os/zero2w64/overlay/opt/zicBox/data/.git || true
	cd ../os/zero2w64 && make
	- rm ../os/zero2w64/output/images/zicXY-OS-rpi-image-$(VERSION).zip || true
	cd ../os/zero2w64/output/images/ && zip zicXY-OS-rpi-image-$(VERSION).zip sdcard.img
	- rm $(ARM64_BUILD_DIR)/zicXY-firmware-ARM64-$(VERSION).zip || true
	cd $(ARM64_BUILD_DIR)/ && zip -r zicXY-firmware-ARM64-$(VERSION).zip zic
	gh release create zicXY-$(VERSION) ../os/zero2w64/output/images/zicXY-OS-rpi-image-$(VERSION).zip $(ARM64_BUILD_DIR)/zicXY-firmware-ARM64-$(VERSION).zip hardware/Gerber_zicPublic_zic_xy_rpi0.zip hardware/zicXY-STL-3d-model.zip --title "zicXY $(VERSION)" --notes "This release contains both zicXY firmware and zicOs image for rpi zero2w."
