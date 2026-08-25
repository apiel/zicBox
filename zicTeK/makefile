MAKE := make

include ../make_common.mk

SFML = -lsfml-graphics -lsfml-window -lsfml-system -DDRAW_SMFL -DDRAW_DESKTOP
INC = -I.. -I.
BUILD_DIR := build/x86

ALSA = `$(PKG_CONFIG) --cflags --libs alsa`

all: build_zic run

build_zic:
	@mkdir -p $(BUILD_DIR)
	$(CC) -g -fms-extensions -o $(BUILD_DIR)/zicTeK zic.cpp -fpermissive $(INC) $(ALSA) $(CFLAGS) $(LDFLAGS) $(SFML) -lpthread

run: build_zic
	$(BUILD_DIR)/zicTeK

clean:
	rm -rf build
