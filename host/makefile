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

main:
	g++ -fPIC -shared -o zicHost.$(BIN_PLATFORM).so zicHost.cpp -fopenmp $(BUILD) $(INC) $(LUA)

standalone: buildStandaloneHost runStandaloneHost

buildStandaloneHost:
	g++ -g -o zicHost -Wall zicHost.cpp -fopenmp $(BUILD) $(INC) $(LUA)

runStandaloneHost:
	./zicHost
