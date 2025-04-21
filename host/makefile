include ../make_common.mk

ALSA=`pkg-config --cflags --libs alsa`

INC=-I../.

PARAMS= -Wno-narrowing -ldl $(ALSA) $(INC) $(RPI) $(CFLAGS) $(LDFLAGS)

# track header file to be sure that build is automatically trigger if any dependency changes
TRACK_HEADER_FILES = -MMD -MF libzicHost.$(TARGET_PLATFORM).d

main:
	make libzicHost.$(TARGET_PLATFORM).so

libzicHost.$(TARGET_PLATFORM).so:
	$(CC) -g -fPIC -shared -o libzicHost.$(TARGET_PLATFORM).so zicHost.cpp $(TRACK_HEADER_FILES) $(PARAMS)

# Safeguard: include only if .d files exist
-include $(wildcard libzicHost.$(TARGET_PLATFORM).d)

rebuild:
	rm libzicHost.$(TARGET_PLATFORM).so
	make libzicHost.$(TARGET_PLATFORM).so

standalone: buildStandaloneHost runStandaloneHost

buildStandaloneHost:
	$(CC) -g -o zicHost -Wall zicHost.cpp $(PARAMS)

runStandaloneHost:
	./zicHost
