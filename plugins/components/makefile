

all:
	make clean_so PadComponent EncoderComponent GranularComponent SequencerComponent\
				  ButtonComponent WaveComponent\
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

PadComponent:
	make compile LIBNAME=PadComponent EXTRA="-DPLUGIN_PROPS=ComponentInterface::Props"

EncoderComponent:
	make compile LIBNAME=EncoderComponent EXTRA="-DPLUGIN_PROPS=ComponentInterface::Props"

GranularComponent:
	make compile LIBNAME=GranularComponent EXTRA="-DPLUGIN_PROPS=ComponentInterface::Props"

WaveComponent:
	make compile LIBNAME=WaveComponent EXTRA="-DPLUGIN_PROPS=ComponentInterface::Props"

SequencerComponent:
	make compile LIBNAME=SequencerComponent EXTRA="-DPLUGIN_PROPS=ComponentInterface::Props"

ButtonComponent:
	make compile LIBNAME=ButtonComponent EXTRA="-DPLUGIN_PROPS=ComponentInterface::Props"
