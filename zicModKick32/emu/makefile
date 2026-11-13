include ../../make_common.mk

INC=-I../.. -I..
BUILD_DIR := build/$(TARGET_PLATFORM)
ALSA=`$(PKG_CONFIG) --cflags --libs alsa`
SFML=-lsfml-graphics -lsfml-window -lsfml-system

all: build_zic run

build_zic:
	@mkdir -p $(BUILD_DIR)
	$(CC) -O3 -g -fms-extensions -fpermissive -o $(BUILD_DIR)/zic zic.cpp $(INC) $(ALSA) $(CFLAGS) $(LDFLAGS) $(SFML)

run:
	$(BUILD_DIR)/zic

clean:
	rm -rf build
