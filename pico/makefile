
all:
	mkdir -p build
	cd build && cmake .. && cmake --build .
	cp build/zic.uf2 /media/$(USER)/RPI-RP2
	make debug

debug:
	sudo cat /dev/ttyACM0