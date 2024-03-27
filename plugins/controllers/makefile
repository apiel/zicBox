BUILD_DIR ?= build
OBJ_DIR ?= $(BUILD_DIR)/obj

ifneq ($(shell uname -m),x86_64)
PIGPIO := -lpigpio -lrt -DPIGPIO=1
endif

all:
	make OscController MidiEncoderController NeotrellisController Mpr121Controller EncoderController

Mpr121Controller:
	make compile LIBNAME=Mpr121Controller EXTRA="$(PIGPIO)"

EncoderController:
	make compile LIBNAME=EncoderController EXTRA="$(PIGPIO)"

OscController:
	make compile LIBNAME=OscController EXTRA="$(shell pkg-config --cflags --libs liblo)"

MidiEncoderController:
	make compile LIBNAME=MidiEncoderController EXTRA="$(shell pkg-config --cflags --libs rtmidi)"

%Controller:
	make compile LIBNAME=$@

compile:
	@mkdir -p $(OBJ_DIR)
	g++ -g -c -o $(OBJ_DIR)/$(LIBNAME).o plugin.cpp -fPIC -DPLUGIN_NAME=$(LIBNAME) -DPLUGIN_INCLUDE=\"$(LIBNAME).h\" $(EXTRA)
	g++ -g -shared -o $(BUILD_DIR)/libzic_$(LIBNAME).so $(OBJ_DIR)/$(LIBNAME).o $(EXTRA)

clean:
	rm -rf $(BUILD_DIR)
