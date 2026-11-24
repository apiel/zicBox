include ../../../make_common.mk

INC=-I../../.. -I..
BUILD_DIR := build/$(TARGET_PLATFORM)
ALSA=`$(PKG_CONFIG) --cflags --libs alsa`
SFML=-lsfml-graphics -lsfml-window -lsfml-system
X11=-lX11 -lXrandr

all: build_zic run

build_zic:
	@mkdir -p $(BUILD_DIR)
	$(CC) -O3 -g -std=c++17 -fms-extensions -fpermissive -o $(BUILD_DIR)/zic zic.cpp $(INC) $(ALSA) $(CFLAGS) $(LDFLAGS) $(SFML) $(X11)

run:
	$(BUILD_DIR)/zic

clean:
	rm -rf build
