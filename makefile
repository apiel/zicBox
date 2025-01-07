RTMIDI=`pkg-config --cflags --libs rtmidi`
SDL2=`sdl2-config --cflags --libs`
# SDL2=`../test/SDL/build/sdl2-config --cflags --libs`
SDL2_ttf=`pkg-config --cflags --libs SDL2_ttf`

LUA=`pkg-config --cflags --libs lua`

# SPI_DEV_MEM=`-lbcm2835 -lbcm_host -DUSE_SPI_DEV_MEM`

ifneq ($(shell uname -m),x86_64)
RPI := -DIS_RPI=1
BIN_SUFFIX := arm
else
PIXEL_SDL := $(SDL2)
BIN_SUFFIX := x86
endif

BUILD=-Wno-narrowing -ldl $(RTMIDI) 

INC=-I.

pixel: pixelLibs buildPixel runPixel
rebuildPixel: pixelRebuild buildPixel runPixel

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
	g++ -g -fms-extensions -o pixel.$(BIN_SUFFIX) zicPixel.cpp -ldl $(INC) $(RPI) $(RTMIDI) $(PIXEL_SDL) $(SPI_DEV_MEM) $(LUA)

runPixel:
	@echo "\n------------------ run zicPixel ------------------\n"
	./pixel.$(BIN_SUFFIX)

push_wiki:
	node doc.js
	cd wiki && git add . && git commit -m "wiki" && git push
	git add wiki && git commit -m "wiki" && git push

push:
	node doc.js
	cd wiki && git add . && git commit -m "wiki" || true && git push
	git add . && git commit -m "$(m)" && git push
