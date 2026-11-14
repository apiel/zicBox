# Offline harness: make            -> Kick32v4
#                  make v3         -> same tests on Kick32v3 (reference)
#                  make wav        -> also writes wavs into build/wav
INC=-I../..
CXXFLAGS=-O2 -std=c++17 -fms-extensions -fpermissive -w

all: build/harness
	./build/harness

build/harness: harness.cpp ../../audio/engines/Kick32v4.h
	@mkdir -p build
	g++ $(CXXFLAGS) -o $@ harness.cpp $(INC)

v3: harness.cpp
	@mkdir -p build
	g++ $(CXXFLAGS) -DENGINE_HEADER='"audio/engines/Kick32v3.h"' -DENGINE=Kick32v3 -o build/harness_v3 harness.cpp $(INC)
	./build/harness_v3

wav: build/harness
	@mkdir -p build/wav
	./build/harness build/wav

clean:
	rm -rf build
