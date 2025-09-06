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
	$(MAKE) $(BUILD_DIR)/pixel

$(BUILD_DIR)/pixel:
	@echo Build using $(CC)
	$(CC) -g -fms-extensions -o $(BUILD_DIR)/pixel zicPixel.cpp -ldl $(INC) $(RPI) $(TTF) $(RTMIDI) $(SDL2) $(SPI_DEV_MEM) $(TRACK_HEADER_FILES)

# Safeguard: include only if .d files exist
-include $(wildcard $(OBJ_DIR)/pixel.d)

watchPixel:
	@echo "\n------------------ watch zicPixel ------------------\n"
	./watch.sh

runPixel:
	@echo "\n------------------ run zicPixel $(TARGET_PLATFORM) ------------------\n"
	$(BUILD_DIR)/pixel

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

release:
	@echo "Creating GitHub release..."
	- rm build/x86.zip
	- rm build/arm.zip
	@echo "Build generic..."
	cd build/x86 && zip -r ../x86.zip .
	cd build/arm && zip -r ../arm.zip .
	cp build/x86.zip build/x86_full.zip
	cp build/arm.zip build/arm_full.zip
	npm run build:pixel
	zip -r build/x86_full.zip data
	zip -r build/arm_full.zip data
	- gh release delete latest -y
	gh release create latest build/x86.zip build/arm.zip build/x86_full.zip build/arm_full.zip --title "Latest firmware release" --notes "This release contains the firmware for x86 and arm, with all the libs. Full version contains as well the data folder, with default configs, samples and weirds presets."

releaseOs:
	@echo "Creating GitHub release of zicOs..."
	- rm build/zicOs.zip
	zip -r build/zicOs.zip ../zicOs/buildroot/output/images/sdcard.img
	- gh release delete zicOs -y
	gh release create zicOs build/zicOs.zip --title "Latest zicOs release" --notes "This release contains the zicOs (64 bit) for cm4, rpi4, rpi3 and rpi zero 2w."

releaseOsRpi3:
	@echo "Creating GitHub release of zicOs for rpi3..."
	npm run build:rpi3
	cd ../zicOs/buildroot && ZICBOX_PATH=/home/alex/Music/zicBox make
	- rm build/zicOsRpi3.zip
	zip -r build/zicOsRpi3.zip ../zicOs/buildroot/output/images/sdcard.img
	- gh release delete zicOsRpi3 -y
	gh release create zicOsRpi3 build/zicOsRpi3.zip --title "zicOs zic Pixel Rpi3A+" --notes "This release contains the zicOs (64 bit) for zic Pixel rpi3A+ version."

releaseOsPixel:
	@echo "Creating GitHub release of zicOs for Zic Pixel on rpi zero..."
	npm run build:pixel
	cd ../zicOs/buildroot && ZICBOX_PATH=/home/alex/Music/zicBox make
	- rm build/zicOsPixel.zip
	zip -r build/zicOsPixel.zip ../zicOs/buildroot/output/images/sdcard.img
	- gh release delete zicOsPixel -y
	gh release create zicOsPixel build/zicOsPixel.zip --title "zicOs zic Pixel Rpi zero" --notes "This release contains the zicOs (64 bit) for zic Pixel (rpi0)."

PI_TARGET ?= root@zic.local
PI_PASSWORD = password
PI_REMOTE_DIR = /opt/zicBox
make pi:
	make pixelLibs cc=arm
	make buildPixel cc=arm
	- sshpass -p "$(PI_PASSWORD)" ssh "$(PI_TARGET)" "killall pixel"
	rsync -avz --progress -e "sshpass -p '$(PI_PASSWORD)' ssh" build/arm/ "$(PI_TARGET):$(PI_REMOTE_DIR)/."
	rsync -avz --progress -e "sshpass -p '$(PI_PASSWORD)' ssh" data/ "$(PI_TARGET):$(PI_REMOTE_DIR)/data"
	sshpass -p "$(PI_PASSWORD)" ssh "$(PI_TARGET)" "chmod +x $(PI_REMOTE_DIR)/pixel"
	sshpass -p "$(PI_PASSWORD)" ssh "$(PI_TARGET)" "cd $(PI_REMOTE_DIR) && ./pixel"

make pi64:
	make pixelLibs cc=arm64
	make buildPixel cc=arm64
	- sshpass -p "$(PI_PASSWORD)" ssh "$(PI_TARGET)" "killall pixel"
	rsync -avz --progress -e "sshpass -p '$(PI_PASSWORD)' ssh" build/arm/ "$(PI_TARGET):$(PI_REMOTE_DIR)/."
	rsync -avz --progress -e "sshpass -p '$(PI_PASSWORD)' ssh" data/ "$(PI_TARGET):$(PI_REMOTE_DIR)/data"
	sshpass -p "$(PI_PASSWORD)" ssh "$(PI_TARGET)" "chmod +x $(PI_REMOTE_DIR)/pixel"
	sshpass -p "$(PI_PASSWORD)" ssh "$(PI_TARGET)" "cd $(PI_REMOTE_DIR) && ./pixel"

# to remove
make piOld:
	make pixelLibs cc=arm
	make buildPixel cc=arm
	- sshpass -p "password" ssh root@zic.local "killall pixel"
	- sshpass -p "password" ssh root@zic.local "rm -rf /opt/zicBox/libs"
	- sshpass -p "password" ssh root@zic.local "rm /opt/zicBox/pixel"
	- sshpass -p "password" ssh root@zic.local "rm /opt/zicBox/data/config.json"
	sshpass -p "password" scp -v -r build/arm/* root@zic.local:/opt/zicBox/.
	sshpass -p "password" scp -v data/config.json root@zic.local:/opt/zicBox/data/config.json
	sshpass -p "password" ssh root@zic.local "chmod +x /opt/zicBox/pixel"
	sshpass -p "password" ssh root@zic.local "cd /opt/zicBox && ./pixel"


splash:
	../zicOs/zero2w64/output/host/bin/aarch64-linux-g++ -g -fms-extensions -o build/arm/splash splash.cpp -fpermissive -I.
	sshpass -p "$(PI_PASSWORD)" scp -v build/arm/splash $(PI_TARGET):$(PI_REMOTE_DIR)/splash

screenOff:
	../zicOs/zero2w64/output/host/bin/aarch64-linux-g++ -g -fms-extensions -o build/arm/screenOff screenOff.cpp -fpermissive -I.
	sshpass -p "$(PI_PASSWORD)" scp -v build/arm/screenOff $(PI_TARGET):$(PI_REMOTE_DIR)/screenOff
