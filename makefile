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
TRACK_HEADER_FILES = -MMD -MF $(OBJ_DIR)/pixel.d

pixel: pixelLibs buildPixel runPixel
rebuildPixel: pixelRebuild buildPixel runPixel

sync:
	bash sync.sh
	$(MAKE) pixel

pixelLibs:
	@echo "\n------------------ plugins ------------------\n"
	$(MAKE) -C host
	$(MAKE) -C plugins/audio
	$(MAKE) -C plugins/components/Pixel
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
	cd build/x86 && zip -r ../x86.zip .
	cd build/arm && zip -r ../arm.zip .
	cp build/x86.zip build/x86_full.zip
	cp build/arm.zip build/arm_full.zip
	zip -r build/x86_full.zip data
	zip -r build/arm_full.zip data
	- gh release delete latest -y
	gh release create latest build/x86.zip build/arm.zip build/x86_full.zip build/arm_full.zip --title "Latest firmware release" --notes "This release contains the firmware for x86 and arm, with all the libs. Full version contains as well the data folder, with default configs, samples and weirds presets."

releaseOs:
	@echo "Creating GitHub release of zicOs..."
	- rm build/zicOs_cm4.zip
	zip -r build/zicOs_cm4.zip ../zicOs/buildroot/output/images/sdcard.img
	- gh release delete zicOs -y
	gh release create zicOs build/zicOs_cm4.zip --title "Latest zicOs release" --notes "This release contains the zicOs firmware for cm4."
