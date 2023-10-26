all:
	make clean_so EffectGainVolume EffectSampleRateReducer EffectFilter EffectDistortion EffectDelay\
		EffectFilter EffectFilterMultiMode EffectFilterMultiMode2 EffectFilterMultiModeMoog\
		SynthKick23 SynthGranular\
		Sequencer\
		Mixer2 Mixer4 Mixer8\
		AudioInputPulse AudioOutputPulse AudioInputAlsa AudioOutputAlsa\
		clean_o

clean:
	make clean_so clean_o

clean_so:
	rm -f build/*.so

clean_o:
	rm -f *.o

compile:
	g++ -c -o $(LIBNAME).o audioPlugin.cpp -fPIC -DPLUGIN_NAME=$(LIBNAME) -DPLUGIN_INCLUDE=\"$(LIBNAME).h\"
	g++ -shared -o build/libzic_$(LIBNAME).so $(LIBNAME).o $(EXTRA)

EffectGainVolume:
	make compile LIBNAME=EffectGainVolume

EffectSampleRateReducer:
	make compile LIBNAME=EffectSampleRateReducer

EffectDistortion:
	make compile LIBNAME=EffectDistortion

EffectDelay:
	make compile LIBNAME=EffectDelay

EffectFilter:
	make compile LIBNAME=EffectFilter

EffectFilterMultiMode:
	make compile LIBNAME=EffectFilterMultiMode

EffectFilterMultiMode2:
	make compile LIBNAME=EffectFilterMultiMode2

EffectFilterMultiModeMoog:
	make compile LIBNAME=EffectFilterMultiModeMoog

Mixer2:
	make compile LIBNAME=Mixer2

Mixer4:
	make compile LIBNAME=Mixer4

Mixer8:
	make compile LIBNAME=Mixer8

SynthGranular:
	make compile LIBNAME=SynthGranular EXTRA="$(shell pkg-config --cflags --libs sndfile)"

SynthKick23:
	make compile LIBNAME=SynthKick23 EXTRA="$(shell pkg-config --cflags --libs sndfile)"

Sequencer:
	make compile LIBNAME=Sequencer

AudioInputPulse:
	make compile LIBNAME=AudioInputPulse EXTRA="$(shell pkg-config --cflags --libs libpulse-simple)"

AudioOutputPulse:
	make compile LIBNAME=AudioOutputPulse EXTRA="$(shell pkg-config --cflags --libs libpulse-simple)"

AudioInputAlsa:
	make compile LIBNAME=AudioInputAlsa EXTRA="$(shell pkg-config --cflags --libs alsa)"

AudioOutputAlsa:
	make compile LIBNAME=AudioOutputAlsa EXTRA="$(shell pkg-config --cflags --libs alsa)"
