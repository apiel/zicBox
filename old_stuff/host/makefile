include ../make_common.mk

ALSA=`$(PKG_CONFIG) --cflags --libs alsa`

BUILD_DIR := ../build/$(TARGET_PLATFORM)/libs
OBJ_DIR := ../build/obj/$(TARGET_PLATFORM)/libs

INC=-I../.

PARAMS= -Wno-narrowing -ldl $(ALSA) $(INC) $(RPI) $(CFLAGS) $(LDFLAGS)

# track header file to be sure that build is automatically trigger if any dependency changes
TRACK_HEADER_FILES = -MMD -MF $(OBJ_DIR)/libzicHost.d

main:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(OBJ_DIR)
	make $(BUILD_DIR)/libzicHost.so

$(BUILD_DIR)/libzicHost.so:
	$(CC) -g -fPIC -shared -o $(BUILD_DIR)/libzicHost.so zicHost.cpp $(TRACK_HEADER_FILES) $(PARAMS)

# Safeguard: include only if .d files exist
-include $(wildcard $(OBJ_DIR)/libzicHost.d)

standalone: buildStandaloneHost runStandaloneHost

buildStandaloneHost:
	$(CC) -g -o zicHost -Wall zicHost.cpp $(PARAMS)

runStandaloneHost:
	./zicHost
