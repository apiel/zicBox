PIO ?= $(shell which pio 2>/dev/null || echo ~/.platformio/penv/bin/pio)

.PHONY: all build_zic run screenshot clean up esp32

all:
	$(MAKE) -C emu all

build_zic:
	$(MAKE) -C emu build_zic

run:
	$(MAKE) -C emu run

screenshot:
	$(MAKE) -C emu screenshot

clean:
	$(MAKE) -C emu clean

up: esp32

esp32:
	$(PIO) run -d esp32 -t upload
