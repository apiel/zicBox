RTMIDI=`pkg-config --cflags --libs rtmidi`
SDL2=`sdl2-config --cflags --libs`
SDL2_ttf=`pkg-config --cflags --libs SDL2_ttf`

# LVGL

INC = -I./ -I./lvgl/
CFLAGS = -O0 -g

SRC_DIR				:= ./
# WORKING_DIR			:= ./build
BUILD_DIR			:= obj

COMPILE				= gcc $(CFLAGS) $(INC)

SRCS 	:= $(shell find ./ -type f -name '*.c' -not -path '*/\.*')
OBJECTS := $(patsubst ./%,./obj/%,$(SRCS:.c=.o))

# LVGL END

ifneq ($(shell uname -m),x86_64)
RPI := -DIS_RPI=1
endif

BUILD=-Wno-narrowing -ldl $(RTMIDI)

main: build run
host: buildHost runHost
all: libs main
allall: libs buildHost soHost main

obj/%.o: .//%.c
	@echo 'Building project file: $<'
	@mkdir -p $(dir $@)
	@$(COMPILE) -c -o "$@" "$<"

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

build: $(OBJECTS)
	@echo "\n------------------ build zicBox ------------------\n"
	g++ -g -fms-extensions -o zicBox zicBox.cpp -ldl $(SDL2) $(SDL2_ttf) $(RPI) $(RTMIDI) $(OBJECTS) ${LDLIBS}

# g++ -g -fms-extensions -o zicBox zicBox.cpp -ldl $(SDL2) $(SDL2_ttf) $(RPI) $(RTMIDI)

run:
	@echo "\n------------------ run zicBox ------------------\n"
	./zicBox

gpio:
	@echo "\n------------------ gpio ------------------\n"
	make -C hardware/gpio
