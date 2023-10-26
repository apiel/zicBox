RTMIDI=`pkg-config --cflags --libs rtmidi`

BUILD=-Wno-narrowing -ldl $(RTMIDI)

host: buildHost runHost
all: libs buildHost soHost

libs:
	@echo "\n------------------ plugins ------------------\n"
	make -C plugins/audio
	make -C plugins/components
	make -C plugins/controllers
	@echo "\nbuild plugins done."

soHost:
	@echo "\n------------------ build_so ------------------\n"
	g++ -fPIC -shared -o host/zicHost.so host/zicHost.cpp -fopenmp $(BUILD)
	@echo "\nbuild zicHost.so done."

buildHost:
	@echo "\n------------------ build ------------------\n"
	g++ -g -o zicHost -Wall host/zicHost.cpp -fopenmp $(BUILD)
	@echo "\nbuild host/zicHost done."

runHost:
	@echo "\n------------------ run ------------------\n"
	./host/zicHost
