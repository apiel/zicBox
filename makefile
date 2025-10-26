MAKE := make cc=$(cc)

include ./make_common.mk

ifeq ($(TARGET_PLATFORM),x86)
SDL2=`sdl2-config --cflags --libs`
endif

# RTMIDI=`$(PKG_CONFIG) --cflags --libs rtmidi`
# uncomment to enable to load ttf file as font with absolute path to ttf file
# TTF=`$(PKG_CONFIG) --cflags --libs freetype2`

# SPI_DEV_MEM=`-lbcm2835 -lbcm_host -DUSE_SPI_DEV_MEM`

# BUILD=-Wno-narrowing -ldl $(RTMIDI) 

INC=-I.

BUILD_DIR := build/$(TARGET_PLATFORM)
OBJ_DIR := build/obj/$(TARGET_PLATFORM)

# track header file to be sure that build is automatically trigger if any dependency changes
TRACK_HEADER_FILES = -MMD -MP -MF $(OBJ_DIR)/pixel.d

pixel: pixelLibs buildPixel watchPixel

sync:
	bash sync.sh
	$(MAKE) pixel

pixelLibs:
	@echo "\n------------------ plugins ------------------\n"
	$(MAKE) -C host
	$(MAKE) -C plugins/audio customBuild
	$(MAKE) -C plugins/components/Pixel customBuild
	@echo "\nbuild plugins done."

pixelRebuild:
	$(MAKE) -C host rebuild
	$(MAKE) -C plugins/audio rebuild
	$(MAKE) -C plugins/components/Pixel rebuild

buildPixel:
	@echo "\n------------------ build zicPixel ------------------\n"
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(OBJ_DIR)
	$(MAKE) $(BUILD_DIR)/zic

$(BUILD_DIR)/zic:
	@echo Build using $(CC)
	$(CC) -g -fms-extensions -o $(BUILD_DIR)/zic zic.cpp -ldl $(INC) $(RPI) $(TTF) $(RTMIDI) $(SDL2) $(SPI_DEV_MEM) $(TRACK_HEADER_FILES)

# Safeguard: include only if .d files exist
-include $(wildcard $(OBJ_DIR)/zic.d)

watchPixel:
	@echo "\n------------------ watch zicPixel ------------------\n"
	./watch.sh

runPixel:
	@echo "\n------------------ run zicPixel $(TARGET_PLATFORM) ------------------\n"
	WIFI_IFACE=wlp0s20f3 $(BUILD_DIR)/zic

dev:
	npm run dev

push_wiki:BUILD
	node doc.js
	cd wiki && git add . && git commit -m "wiki" && git push
	git add wiki && git commit -m "wiki" && git push

push:
	node doc.js
	cd wiki && git add . && git commit -m "wiki" || true && git push
	git add . && git commit -m "$(m)" && git push

merge:
	git checkout main
	git merge develop
	git push
	git checkout develop

releasePixel:
	@echo "Creating GitHub release of zicOs for Zic Pixel on rpi zero..."
	cd os/zero2w64 && make
	- rm os/zero2w64/output/images/zicOsPixel.zip
	cd os/zero2w64/output/images/ && zip zicOsPixel.zip sdcard.img 
	- gh release delete zicOsPixel -y
	gh release create zicOsPixel os/zero2w64/output/images/zicOsPixel.zip --title "zicOs Pixel Rpi zero2w" --notes "This release contains the zicOs for zic Pixel (rpi zero2w)."
	make releasePixelFirmware

releasePixelFirmware:
	mkdir -p build/pixel_arm64
	cp config.json build/pixel_arm64/config.json
	- rm build/pixel_arm64/zicPixel.zip
	cd build/pixel_arm64/ && zip -r zicPixel.zip *
	rm -rf build/pixel_arm64/config.json
	- gh release delete zicPixel -y
	gh release create zicPixel build/pixel_arm64/zicPixel.zip --title "zicPixel firmware" --notes "This contains the zicPixel firmware, compiled for rpi zero2w."
	- rm build/pixel_arm64/zicPixel.zip

PI_TARGET ?= root@zic.local
PI_PASSWORD = password
PI_REMOTE_DIR = /opt/zicBox

buildPi:
	make pixelLibs cc=pixel_arm64
	make buildPixel cc=pixel_arm64

pi:
	make buildPi
	- sshpass -p "$(PI_PASSWORD)" ssh "$(PI_TARGET)" "killall zic"
	rsync -avz --progress -e "sshpass -p '$(PI_PASSWORD)' ssh" build/pixel_arm64/ "$(PI_TARGET):$(PI_REMOTE_DIR)/."
	rsync -avz --progress -e "sshpass -p '$(PI_PASSWORD)' ssh" data/ "$(PI_TARGET):$(PI_REMOTE_DIR)/data"
	make sendConfig
	sshpass -p "$(PI_PASSWORD)" ssh "$(PI_TARGET)" "chmod +x $(PI_REMOTE_DIR)/zic"
	sshpass -p "$(PI_PASSWORD)" ssh "$(PI_TARGET)" "cd $(PI_REMOTE_DIR) && ./zic"

sendConfig:
	sshpass -p "$(PI_PASSWORD)" scp -v config.json $(PI_TARGET):$(PI_REMOTE_DIR)/config.json

buildSplash:
	$(CC) -g -fms-extensions -o build/pixel_arm64/splash splash.cpp -fpermissive -I.

splash:
	make buildSplash cc=pixel_arm64
	sshpass -p "$(PI_PASSWORD)" scp -v build/pixel_arm64/splash $(PI_TARGET):$(PI_REMOTE_DIR)/splash

gpio:
	$(CC_ZERO2W64) -g -fms-extensions -o build/pixel_arm64/gpio gpio.cpp -fpermissive -I.
	sshpass -p "$(PI_PASSWORD)" scp -v build/pixel_arm64/gpio $(PI_TARGET):$(PI_REMOTE_DIR)/gpio

block:
	make -C esp/zicBlock
	esp/zicBlock/mainSDL