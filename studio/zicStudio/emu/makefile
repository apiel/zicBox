include ../../../make_common.mk

INC=-I../../.. -I..
BUILD_DIR := build/$(TARGET_PLATFORM)
ALSA=`$(PKG_CONFIG) --cflags --libs alsa`
SFML=-lsfml-graphics -lsfml-window -lsfml-system

SCREENSHOT_DIR := build/screenshots

all: build_zic run

build_zic:
	@mkdir -p $(BUILD_DIR)
	$(CC) -O2 -g -std=c++17 -fms-extensions -fpermissive -w -o $(BUILD_DIR)/zic zic.cpp $(INC) $(ALSA) $(CFLAGS) $(LDFLAGS) $(SFML)

run:
	$(BUILD_DIR)/zic

# Render the three engines plus the narrow layout without needing sound, then
# check the captures are real images rather than empty frames.
screenshots: build_zic
	@mkdir -p $(SCREENSHOT_DIR)
	@for t in 0 1 2; do \
		echo "rendering track $$t"; \
		ZIC_SILENT=1 ZIC_SCREENSHOT=$(CURDIR)/$(SCREENSHOT_DIR)/track$$t.png ZIC_DEMO=1 ZIC_TRACK=$$t $(BUILD_DIR)/zic || exit 1; \
	done
	@echo "rendering narrow layout"
	@ZIC_SILENT=1 ZIC_SCREENSHOT=$(CURDIR)/$(SCREENSHOT_DIR)/narrow.png ZIC_DEMO=1 ZIC_NARROW=1 ZIC_TRACK=1 $(BUILD_DIR)/zic || exit 1
	@for f in $(SCREENSHOT_DIR)/*.png; do \
		size=$$(stat -c%s "$$f"); \
		test $$size -gt 20000 || { echo "screenshot too small: $$f ($$size bytes)"; exit 1; }; \
		echo "ok $$f ($$size bytes)"; \
	done

clean:
	rm -rf build