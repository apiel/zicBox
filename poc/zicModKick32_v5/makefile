.PHONY: all build_zic run clean test test_v4

all:
	$(MAKE) -C emu all

build_zic:
	$(MAKE) -C emu build_zic

run:
	$(MAKE) -C emu run

test:
	$(MAKE) -C harness

test_v4:
	$(MAKE) -C harness v4

clean:
	$(MAKE) -C emu clean
	$(MAKE) -C harness clean
