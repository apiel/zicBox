include ../../../make_common.mk

INC=-I../../.. -I..
BUILD_DIR := build/$(TARGET_PLATFORM)

all: test_dsp

test_dsp:
	@mkdir -p $(BUILD_DIR)
	$(CC) -O3 -g -std=c++17 -o $(BUILD_DIR)/test_dsp test_dsp.cpp $(INC) $(CFLAGS) $(LDFLAGS)
	$(BUILD_DIR)/test_dsp

clean:
	rm -rf build
