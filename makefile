RTMIDI=`pkg-config --cflags --libs rtmidi`
SDL2=`sdl2-config --cflags --libs`
# SDL2=`../test/SDL/build/sdl2-config --cflags --libs`
SDL2_ttf=`pkg-config --cflags --libs SDL2_ttf`

# SPI_DEV_MEM=`-lbcm2835 -lbcm_host -DUSE_SPI_DEV_MEM`

ifneq ($(shell uname -m),x86_64)
RPI := -DIS_RPI=1
else
PIXEL_SDL := $(SDL2)
endif

BUILD=-Wno-narrowing -ldl $(RTMIDI) 

main: build run
pixel: buildPixel runPixel
pixelSSD1306: buildPixelSSD1306 runPixelSSD1306
host: buildHost runHost
all: libs main
allall: libs buildHost soHost main

libs:
	@echo "\n------------------ plugins ------------------\n"
	make -C plugins/audio
	make -C plugins/components/SDL
	make -C plugins/controllers
	make -C plugins/config
	@echo "\nbuild plugins done."

soHost:
	@echo "\n------------------ build host shared library ------------------\n"
	g++ -fPIC -shared -o host/zicHost.so host/zicHost.cpp -fopenmp $(BUILD)
	@echo "\nbuild zicHost.so done."

buildHost:
	@echo "\n------------------ build host ------------------\n"
	g++ -g -o host/zicHost -Wall host/zicHost.cpp -fopenmp $(BUILD)
	@echo "\nbuild host/zicHost done."

runHost:
	@echo "\n------------------ run host ------------------\n"
	./host/zicHost

build:
	@echo "\n------------------ build zicBox ------------------\n"
	g++ -g -fms-extensions -o zicBox zicBox.cpp -ldl $(SDL2) $(SDL2_ttf) $(RPI) $(RTMIDI)

run:
	@echo "\n------------------ run zicBox ------------------\n"
	./zicBox

buildPixel:
	@echo "\n------------------ build zicPixel ------------------\n"
	g++ -g -fms-extensions -o zicPixel zicPixel.cpp -ldl $(RPI) $(RTMIDI) $(PIXEL_SDL) $(SPI_DEV_MEM) $(shell pkg-config --cflags --libs libpulse-simple)

runPixel:
	@echo "\n------------------ run zicPixel ------------------\n"
	./zicPixel

buildPixelSSD1306:
	@echo "\n------------------ build zicPixel SSD1306 ------------------\n"
	g++ -g -fms-extensions -o zicPixelSSD1306 zicPixelSSD1306.cpp -ldl $(RPI) $(RTMIDI) $(PIXEL_SDL) $(shell pkg-config --cflags --libs sndfile) $(shell pkg-config --cflags --libs libpulse-simple)

runPixelSSD1306:
	@echo "\n------------------ run zicPixel ------------------\n"
	./zicPixelSSD1306

gpio:
	@echo "\n------------------ gpio ------------------\n"
	make -C hardware/gpio

push_wiki:
	node doc.js
	cd wiki && git add . && git commit -m "wiki" && git push
	git add wiki && git commit -m "wiki" && git push

push:
	node doc.js
	cd wiki && git add . && git commit -m "wiki" || true && git push
	git add . && git commit -m "$(m)" && git push
