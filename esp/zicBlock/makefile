SDL2=`sdl2-config --cflags --libs`
# SNDFILE=`pkg-config --cflags --libs sndfile`

INC=-I../..

all:
	g++ mainSDL.cpp -o mainSDL $(SDL2) $(INC) $(SNDFILE)
