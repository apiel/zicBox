RTMIDI=`pkg-config --cflags --libs rtmidi`
SDL2=`sdl2-config --cflags --libs`

# SPI_DEV_MEM=`-lbcm2835 -lbcm_host -DUSE_SPI_DEV_MEM`

ifneq ($(shell uname -m),x86_64)
RPI := -DIS_RPI=1
BIN_PLATFORM := arm
else
PIXEL_SDL := $(SDL2)
BIN_PLATFORM := x86
endif

BUILD=-Wno-narrowing -ldl $(RTMIDI) 

INC=-I.

# track header file to be sure that build is automatically trigger if any dependency changes
TRACK_HEADER_FILES = -MMD -MF pixel.$(BIN_PLATFORM).d

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
	make pixel.$(BIN_PLATFORM)

pixel.$(BIN_PLATFORM):
	g++ -g -fms-extensions -o pixel.$(BIN_PLATFORM) zicPixel.cpp -ldl $(INC) $(RPI) $(RTMIDI) $(PIXEL_SDL) $(SPI_DEV_MEM) $(TRACK_HEADER_FILES)

# Safeguard: include only if .d files exist
-include $(wildcard pixel.$(BIN_PLATFORM).d)

runPixel:
	@echo "\n------------------ run zicPixel ------------------\n"
	./pixel.$(BIN_PLATFORM)

dev:
	rm -rf config/
	npm run dev

push_wiki:
	node doc.js
	cd wiki && git add . && git commit -m "wiki" && git push
	git add wiki && git commit -m "wiki" && git push

push:
	node doc.js
	cd wiki && git add . && git commit -m "wiki" || true && git push
	git add . && git commit -m "$(m)" && git push
