MAKE := make cc=$(cc)

include ../make_common.mk

ifeq ($(TARGET_PLATFORM),x86)
SMFL=-lsfml-graphics -lsfml-window -lsfml-system -DDRAW_SMFL -DDRAW_DESKTOP
endif

INC=-I..

BUILD_DIR := build/$(TARGET_PLATFORM)
OBJ_DIR := build/obj/$(TARGET_PLATFORM)

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

VERSION ?= $(shell latest="$$(gh release list --limit 200 2>/dev/null | awk '{print $$1}' | sed -n 's/^zicXY-\([0-9]\+\.[0-9]\+\)$$/\1/p' | sort -V | tail -n 1)"; if test -n "$$latest"; then major="$${latest%%.*}"; minor="$${latest##*.}"; echo "$$major.$$((minor + 1))"; else echo "2.0"; fi)

showNextVersion:
	@echo $(VERSION)

buildPi:
	$(MAKE) build_zic cc=arm64
	cp $(BUILD_DIR)/studio $(BUILD_DIR)/zic

pi:
	make buildPi
	- sshpass -p "$(PI_PASSWORD)" ssh "$(PI_TARGET)" "killall zic"
	rsync -avz --progress -e "sshpass -p '$(PI_PASSWORD)' ssh" build/arm64/zic "$(PI_TARGET):$(PI_REMOTE_DIR)/zic"
	rsync -avz --progress -e "sshpass -p '$(PI_PASSWORD)' ssh" ../data/ "$(PI_TARGET):$(PI_REMOTE_DIR)/data"
	sshpass -p "$(PI_PASSWORD)" ssh "$(PI_TARGET)" "chmod +x $(PI_REMOTE_DIR)/zic"
	sshpass -p "$(PI_PASSWORD)" ssh "$(PI_TARGET)" "cd $(PI_REMOTE_DIR) && ./zic"

releaseXY: buildPi
	@echo "Creating versioned GitHub release for zicXY $(VERSION)"
	npm run build:xy
	cd ../os/zero2w64 && make
	- rm ../os/zero2w64/output/images/zicOsXY-$(VERSION).zip
	cd ../os/zero2w64/output/images/ && zip zicOsXY-$(VERSION).zip sdcard.img
	gh release create zicOsXY-$(VERSION) ../os/zero2w64/output/images/zicOsXY-$(VERSION).zip --title "zicOs XY $(VERSION) Rpi zero2w" --notes "This release contains the zicOs image for zicXY $(VERSION) (rpi zero2w)."
	$(MAKE) releaseXYFirmware VERSION=$(VERSION)

releaseXYFirmware: buildPi
	mkdir -p $(BUILD_DIR)
	cp ../config.json $(BUILD_DIR)/config.json
	- rm $(BUILD_DIR)/zicXY-$(VERSION).zip
	cd $(BUILD_DIR)/ && zip -r zicXY-$(VERSION).zip *
	rm -rf $(BUILD_DIR)/config.json
	gh release create zicXY-$(VERSION) $(BUILD_DIR)/zicXY-$(VERSION).zip --title "zicXY firmware $(VERSION)" --notes "This contains the zicXY firmware $(VERSION), compiled for rpi zero2w."

releaseXYv2: releaseXY

releaseXYv2Firmware: releaseXYFirmware