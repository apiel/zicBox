# WAVESHARE START
WAVESHARE_Config   = ./waveshare/Config
WAVESHARE_EPD      = ./waveshare/LCD
WAVESHARE_FONTS    = ./waveshare/Fonts
WAVESHARE_GUI      = ./waveshare/GUI

WAVESHARE_C= $(shell find $(./waveshare) -type f -name '*.c' -not -path '*/\.*')
# WAVESHARE END

RTMIDI=`pkg-config --cflags --libs rtmidi`
SDL2=`sdl2-config --cflags --libs`
SDL2_ttf=`pkg-config --cflags --libs SDL2_ttf`

ifneq ($(shell uname -m),x86_64)
RPI := -DIS_RPI=1
# WAVESHARE := $(WAVESHARE_C) -DUSE_WAVESHARE=1 -lbcm2835 -lm -DUSE_BCM2835_LIB=1 -I $(WAVESHARE_Config) -I $(WAVESHARE_GUI) -I $(WAVESHARE_EPD)
endif

BUILD=-Wno-narrowing -ldl $(RTMIDI)

main: build run
host: buildHost runHost
all: libs main
allall: libs buildHost soHost main

libs:
	@echo "\n------------------ plugins ------------------\n"
	make -C plugins/audio
	make -C plugins/components
	make -C plugins/controllers
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
	g++ -g -fms-extensions -o zicBox zicBox.cpp $(WAVESHARE) -ldl $(SDL2) $(SDL2_ttf) $(RPI) $(RTMIDI)

run:
	@echo "\n------------------ run zicBox ------------------\n"
	./zicBox

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
