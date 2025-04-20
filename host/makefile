CC=g++
ALSA=`pkg-config --cflags --libs alsa`

ifneq ($(shell uname -m),x86_64)
RPI := -DIS_RPI=1
TARGET_PLATFORM := arm
else
TARGET_PLATFORM := x86
endif

BUILD=-Wno-narrowing -ldl $(ALSA)

INC=-I../.

# track header file to be sure that build is automatically trigger if any dependency changes
TRACK_HEADER_FILES = -MMD -MF libzicHost.$(TARGET_PLATFORM).d

main:
	make libzicHost.$(TARGET_PLATFORM).so

libzicHost.$(TARGET_PLATFORM).so:
	$(CC) -g -fPIC -shared -o libzicHost.$(TARGET_PLATFORM).so zicHost.cpp -fopenmp $(BUILD) $(INC) $(TRACK_HEADER_FILES) $(RPI)

# Safeguard: include only if .d files exist
-include $(wildcard libzicHost.$(TARGET_PLATFORM).d)

rebuild:
	rm libzicHost.$(TARGET_PLATFORM).so
	make libzicHost.$(TARGET_PLATFORM).so

standalone: buildStandaloneHost runStandaloneHost

buildStandaloneHost:
	$(CC) -g -o zicHost -Wall zicHost.cpp -fopenmp $(BUILD) $(INC) $(RPI)

runStandaloneHost:
	./zicHost
