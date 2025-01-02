RTMIDI=`pkg-config --cflags --libs rtmidi`

LUA=`pkg-config --cflags --libs lua`

ifneq ($(shell uname -m),x86_64)
RPI := -DIS_RPI=1
BIN_PLATFORM := arm
else
BIN_PLATFORM := x86
endif

BUILD=-Wno-narrowing -ldl $(RTMIDI) 

INC=-I../.

# track header file to be sure that build is automatically trigger if any dependency changes
TRACK_HEADER_FILES = -MMD -MF libzicHost.$(BIN_PLATFORM).d

main:
	make libzicHost.$(BIN_PLATFORM).so

libzicHost.$(BIN_PLATFORM).so:
	g++ -fPIC -shared -o libzicHost.$(BIN_PLATFORM).so zicHost.cpp -fopenmp $(BUILD) $(INC) $(LUA) $(TRACK_HEADER_FILES)

# Safeguard: include only if .d files exist
-include $(wildcard libzicHost.$(BIN_PLATFORM).d)

rebuild:
	rm libzicHost.$(BIN_PLATFORM).so
	make libzicHost.$(BIN_PLATFORM).so

standalone: buildStandaloneHost runStandaloneHost

buildStandaloneHost:
	g++ -g -o zicHost -Wall zicHost.cpp -fopenmp $(BUILD) $(INC) $(LUA)

runStandaloneHost:
	./zicHost
