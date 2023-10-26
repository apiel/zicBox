RTMIDI=`pkg-config --cflags --libs rtmidi`
SDL2=`sdl2-config --cflags --libs`
SDL2_ttf=`pkg-config --cflags --libs SDL2_ttf`

ifneq ($(shell uname -m),x86_64)
RPI := -DIS_RPI=1
endif

BUILD=-Wno-narrowing -ldl $(RTMIDI)

main: build run
host: buildHost runHost
all: libs buildHost soHost build run

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
	g++ -g -fms-extensions -o zicBox zicBox.cpp -ldl $(SDL2) $(SDL2_ttf) $(RPI) $(RTMIDI)

run:
	@echo "\n------------------ run zicBox ------------------\n"
	./zicBox
