RTMIDI=`pkg-config --cflags --libs rtmidi`
# uncomment to enable to load ttf file as font with absolute path to ttf file
# TTF=`pkg-config --cflags --libs freetype2`

# SPI_DEV_MEM=`-lbcm2835 -lbcm_host -DUSE_SPI_DEV_MEM`

include ./make_common.mk

ifeq ($(TARGET_PLATFORM),x86)
SDL2=`sdl2-config --cflags --libs`
endif

BUILD=-Wno-narrowing -ldl $(RTMIDI) 

INC=-I.

# track header file to be sure that build is automatically trigger if any dependency changes
TRACK_HEADER_FILES = -MMD -MF pixel.$(TARGET_PLATFORM).d

pixel: pixelLibs buildPixel runPixel
rebuildPixel: pixelRebuild buildPixel runPixel

sync:
	bash sync.sh
	make pixel

pixelLibs:
	@echo "\n------------------ plugins ------------------\n"
	make -C host
	make -C plugins/audio
	make -C plugins/components/Pixel
	@echo "\nbuild plugins done."

pixelRebuild:
	make -C host rebuild
	make -C plugins/audio rebuild
	make -C plugins/components/Pixel rebuild

buildPixel:
	@echo "\n------------------ build zicPixel ------------------\n"
	make pixel.$(TARGET_PLATFORM)

pixel.$(TARGET_PLATFORM):
	$(CC) -g -fms-extensions -o pixel.$(TARGET_PLATFORM) zicPixel.cpp -ldl $(INC) $(RPI) $(TTF) $(RTMIDI) $(SDL2) $(SPI_DEV_MEM) $(TRACK_HEADER_FILES)

# Safeguard: include only if .d files exist
-include $(wildcard pixel.$(TARGET_PLATFORM).d)

runPixel:
	@echo "\n------------------ run zicPixel ------------------\n"
	./pixel.$(TARGET_PLATFORM)

dev:
	npm run dev

push_wiki:
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
