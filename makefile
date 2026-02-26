MAKE := make cc=$(cc)

include ./make_common.mk

ifeq ($(TARGET_PLATFORM),x86)
# SDL2=`sdl2-config --cflags --libs` -DDRAW_SDL2 -DDRAW_DESKTOP
SMFL=-lsfml-graphics -lsfml-window -lsfml-system -DDRAW_SMFL -DDRAW_DESKTOP
endif

# uncomment to enable to load ttf file as font with absolute path to ttf file
# TTF=`$(PKG_CONFIG) --cflags --libs freetype2`

INC=-I.

BUILD_DIR := build/$(TARGET_PLATFORM)
OBJ_DIR := build/obj/$(TARGET_PLATFORM)

# track header file to be sure that build is automatically trigger if any dependency changes
TRACK_HEADER_FILES = -MMD -MP -MF $(OBJ_DIR)/pixel.d

all: zicLibs buildZic watchZic

sync:
	bash sync.sh
	$(MAKE) pixel

zicLibs:
	@echo "\n------------------ plugins ------------------\n"
	$(MAKE) -C host
	$(MAKE) -C plugins/audio customBuild
	$(MAKE) -C plugins/components/Pixel customBuild
	@echo "\nbuild plugins done."

buildZic:
	@echo "\n------------------ build zic ------------------\n"
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(OBJ_DIR)
	$(MAKE) $(BUILD_DIR)/zic

$(BUILD_DIR)/zic:
	@echo Build using $(CC)
	$(CC) -g -fms-extensions -o $(BUILD_DIR)/zic zic.cpp -ldl $(INC) $(RPI) $(TTF) $(RTMIDI) $(SDL2) $(SMFL) $(SPI_DEV_MEM) $(TRACK_HEADER_FILES)

# Safeguard: include only if .d files exist
-include $(wildcard $(OBJ_DIR)/zic.d)

watchZic:
	@echo "\n------------------ watch zic ------------------\n"
	./watch.sh

runZic:
	@echo "\n------------------ run zic $(TARGET_PLATFORM) ------------------\n"
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
	npm run build:pixel
	cd os/zero2w64 && make
	- rm os/zero2w64/output/images/zicOsPixel.zip
	cd os/zero2w64/output/images/ && zip zicOsPixel.zip sdcard.img 
	- gh release delete zicOsPixel -y
	gh release create zicOsPixel os/zero2w64/output/images/zicOsPixel.zip --title "zicOs Pixel Rpi zero2w" --notes "This release contains the zicOs for zic Pixel (rpi zero2w)."
	make releasePixelFirmware

releasePixelFirmware:
	mkdir -p build/arm64
	cp config.json build/arm64/config.json
	- rm build/arm64/zicPixel.zip
	cd build/arm64/ && zip -r zicPixel.zip *
	rm -rf build/arm64/config.json
	- gh release delete zicPixel -y
	gh release create zicPixel build/arm64/zicPixel.zip --title "zicPixel firmware" --notes "This contains the zicPixel firmware, compiled for rpi zero2w."
	- rm build/arm64/zicPixel.zip

releaseXY:
	@echo "Creating GitHub release of zicOs for Zic Pixel on rpi zero..."
	npm run build:xy
	cd os/zero2w64 && make
	- rm os/zero2w64/output/images/zicOsXY.zip
	cd os/zero2w64/output/images/ && zip zicOsXY.zip sdcard.img 
	- gh release delete zicOsXY -y
	gh release create zicOsXY os/zero2w64/output/images/zicOsXY.zip --title "zicOs XY Rpi zero2w" --notes "This release contains the zicOs for zic XY (rpi zero2w)."
	make releaseXyFirmware

releaseXyFirmware:
	mkdir -p build/arm64
	cp config.json build/arm64/config.json
	- rm build/arm64/zicXY.zip
	cd build/arm64/ && zip -r zicXY.zip *
	rm -rf build/arm64/config.json
	- gh release delete zicXY -y
	gh release create zicXY build/arm64/zicXY.zip --title "zicXY firmware" --notes "This contains the zicXY firmware, compiled for rpi zero2w."
	- rm build/arm64/zicXY.zip

PI_TARGET ?= root@zic.local
PI_PASSWORD = password
PI_REMOTE_DIR = /opt/zicBox

buildPi:
	make zicLibs cc=arm64
	make buildZic cc=arm64

pi:
	make buildPi
	- sshpass -p "$(PI_PASSWORD)" ssh "$(PI_TARGET)" "killall zic"
	rsync -avz --progress -e "sshpass -p '$(PI_PASSWORD)' ssh" build/arm64/ "$(PI_TARGET):$(PI_REMOTE_DIR)/."
	rsync -avz --progress -e "sshpass -p '$(PI_PASSWORD)' ssh" data/ "$(PI_TARGET):$(PI_REMOTE_DIR)/data"
	make sendConfig
	sshpass -p "$(PI_PASSWORD)" ssh "$(PI_TARGET)" "chmod +x $(PI_REMOTE_DIR)/zic"
	sshpass -p "$(PI_PASSWORD)" ssh "$(PI_TARGET)" "cd $(PI_REMOTE_DIR) && ./zic"

sendConfig:
	sshpass -p "$(PI_PASSWORD)" scp -v config.json $(PI_TARGET):$(PI_REMOTE_DIR)/config.json

buildSplash:
	$(CC) -g -fms-extensions -o build/arm64/splash splash.cpp -fpermissive -I.

splash:
	make buildSplash cc=arm64
	sshpass -p "$(PI_PASSWORD)" scp -v build/arm64/splash $(PI_TARGET):$(PI_REMOTE_DIR)/splash

gpio:
	$(CC_ZERO2W64) -g -fms-extensions -o build/arm64/gpio gpio.cpp -fpermissive -I.
	sshpass -p "$(PI_PASSWORD)" scp -v build/arm64/gpio $(PI_TARGET):$(PI_REMOTE_DIR)/gpio

block:
	make -C esp/zicBlock
	esp/zicBlock/mainSDL

stm32emu:
	cd mcu/stm32 && g++ emu.cpp -o emu -lsfml-graphics -lsfml-window -lsfml-system -lasound -lpthread -I../../. -I./Core/Inc/.
	cd mcu/stm32 && ./emu

stm32up:
	cd mcu/stm32 && make
	sudo dfu-util -a 0 -D mcu/stm32/build/rack.bin -s 0x08000000:leave