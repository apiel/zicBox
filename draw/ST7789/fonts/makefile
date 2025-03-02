TTF=`pkg-config --cflags --libs freetype2`

all: compile run

compile:
	g++ ttf2font.cpp -o ttf2font $(TTF)

run:
	./ttf2font Roboto-Thin.ttf