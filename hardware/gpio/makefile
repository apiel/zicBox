LIBLO=`pkg-config --cflags --libs liblo`

ifneq ($(shell uname -m),x86_64)
GPIO := -lpigpio -lrt -DPIGPIO=1
endif

all: build run

build:
	g++ -o zicGpio main.cpp $(LIBLO) $(GPIO)

run:
	./zicGpio
