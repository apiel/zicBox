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
