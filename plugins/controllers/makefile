

all:
	make clean_so OscController MidiEncoderController\
		clean_o

clean:
	make clean_so clean_o

clean_so:
	rm -f build/*.so

clean_o:
	rm -f *.o

compile:
	g++ -g -c -o $(LIBNAME).o plugin.cpp -fPIC -DPLUGIN_NAME=$(LIBNAME) -DPLUGIN_INCLUDE=\"$(LIBNAME).h\" $(EXTRA)
	g++ -g -shared -o build/libzic_$(LIBNAME).so $(LIBNAME).o $(EXTRA)

OscController:
	make compile LIBNAME=OscController EXTRA="$(shell pkg-config --cflags --libs liblo)"

MidiEncoderController:
	make compile LIBNAME=MidiEncoderController EXTRA="$(shell pkg-config --cflags --libs rtmidi)"
