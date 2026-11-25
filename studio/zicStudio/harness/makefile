include ../../../make_common.mk

INC=-I../../..
CXXFLAGS=-O2 -std=c++17 -fms-extensions -fpermissive -w
BUILD_DIR := build/$(TARGET_PLATFORM)

all: test_dsp

test_dsp:
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CXXFLAGS) -o $(BUILD_DIR)/test_dsp test_dsp.cpp $(INC)
	$(BUILD_DIR)/test_dsp

clean:
	rm -rf build