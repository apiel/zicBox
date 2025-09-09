# MAKEFLAGS += -j$(shell nproc)

include ../../make_common.mk

BUILD_DIR := ../../build/$(TARGET_PLATFORM)/libs/controllers
OBJ_DIR := ../../build/obj/$(TARGET_PLATFORM)/libs/controllers

ifneq ($(shell uname -m),x86_64)
PIGPIO := -lpigpio -lrt -DPIGPIO=1
endif

INC=-I../..

all:
	make OscController NeotrellisController Mpr121Controller EncoderController

Mpr121Controller:
	make compile LIBNAME=Mpr121Controller EXTRA="$(PIGPIO)"

EncoderController:
	make compile LIBNAME=EncoderController EXTRA="$(PIGPIO)"

OscController:
	make compile LIBNAME=OscController EXTRA="$(shell $(PKG_CONFIG) --cflags --libs liblo)"

#MidiEncoderController:
#	make compile LIBNAME=MidiEncoderController EXTRA="$(shell $(PKG_CONFIG) --cflags --libs rtmidi)"

%Controller:
	make compile LIBNAME=$@

compile:
	@mkdir -p $(OBJ_DIR)
	$(CC) -g -c -o $(OBJ_DIR)/$(LIBNAME).o plugin.cpp $(INC) -fPIC -DPLUGIN_NAME=$(LIBNAME) -DPLUGIN_INCLUDE=\"$(LIBNAME).h\" $(EXTRA)
	$(CC) -g -shared -o $(BUILD_DIR)/libzic_$(LIBNAME).so $(OBJ_DIR)/$(LIBNAME).o $(INC) $(EXTRA)

clean:
	rm -rf $(BUILD_DIR)
