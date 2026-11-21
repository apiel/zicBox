.PHONY: all build_zic run clean test test_v3

all:
	$(MAKE) -C emu all

build_zic:
	$(MAKE) -C emu build_zic

run:
	$(MAKE) -C emu run

test:
	$(MAKE) -C harness

test_v3:
	$(MAKE) -C harness v3

clean:
	$(MAKE) -C emu clean
	$(MAKE) -C harness clean
