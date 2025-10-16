
all:
	mkdir -p build
	cd build && cmake .. && cmake --build .
	cp build/zic.uf2 /media/$(USER)/RPI-RP2
	sleep 1
	make debug

debug:
	sudo cat /dev/ttyACM0