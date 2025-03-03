TTF=`pkg-config --cflags --libs freetype2`

all: compile run

compile:
	g++ ttf2font.cpp -o ttf2font $(TTF)

run:
	./ttf2font Roboto-Thin.ttf 8
	./ttf2font Roboto-Thin.ttf 12
	./ttf2font Roboto-Thin.ttf 16
	./ttf2font Roboto-Thin.ttf 24
	./ttf2font /usr/share/fonts/truetype/dejavu/DejaVuSans.ttf 8
	./ttf2font /usr/share/fonts/truetype/dejavu/DejaVuSans.ttf 12
	./ttf2font /usr/share/fonts/truetype/dejavu/DejaVuSans.ttf 16
	./ttf2font /usr/share/fonts/truetype/dejavu/DejaVuSans.ttf 24
	./ttf2font /usr/share/fonts/truetype/freefont/FreeSans.ttf 8
	./ttf2font /usr/share/fonts/truetype/freefont/FreeSans.ttf 12
	./ttf2font /usr/share/fonts/truetype/freefont/FreeSans.ttf 16
	./ttf2font /usr/share/fonts/truetype/freefont/FreeSans.ttf 24