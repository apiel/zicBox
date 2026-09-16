include ../make_common.mk

SFML := -lsfml-graphics -lsfml-audio -lsfml-window -lsfml-system -DDRAW_SMFL -DDRAW_DESKTOP
INC := -I.. -I.

BUILD_DIR := build/desktop

.PHONY: all desktop run screenshot clean

all: desktop run

desktop:
	@mkdir -p $(BUILD_DIR)
	$(CC) -O3 -g -std=c++17 -o $(BUILD_DIR)/zicModMolecular main.cpp $(INC) $(SFML) $(CFLAGS) $(LDFLAGS)

run: desktop
	$(BUILD_DIR)/zicModMolecular

screenshot: desktop
	ZIC_SCREENSHOT="screenshot" $(BUILD_DIR)/zicModMolecular

clean:
	rm -rf build
