BUILD_DIR ?= build
OBJ_DIR ?= $(BUILD_DIR)/obj

all:
	make EffectGainVolume EffectSampleRateReducer EffectFilter EffectDistortion EffectDelay\
		EffectFilter EffectFilterMultiMode EffectFilterMultiMode2 EffectFilterMultiModeMoog\
		SynthKick23 SynthGranular\
		Sequencer\
		Mixer2 Mixer4 Mixer8\
		AudioInputPulse AudioOutputPulse AudioInputAlsa AudioOutputAlsa

SynthGranular:
	make compile LIBNAME=SynthGranular EXTRA="$(shell pkg-config --cflags --libs sndfile)"

SynthKick23:
	make compile LIBNAME=SynthKick23 EXTRA="$(shell pkg-config --cflags --libs sndfile)"

AudioInputPulse:
	make compile LIBNAME=AudioInputPulse EXTRA="$(shell pkg-config --cflags --libs libpulse-simple)"

AudioOutputPulse:
	make compile LIBNAME=AudioOutputPulse EXTRA="$(shell pkg-config --cflags --libs libpulse-simple)"

AudioInputAlsa:
	make compile LIBNAME=AudioInputAlsa EXTRA="$(shell pkg-config --cflags --libs alsa)"

AudioOutputAlsa:
	make compile LIBNAME=AudioOutputAlsa EXTRA="$(shell pkg-config --cflags --libs alsa)"

%:
	make compile LIBNAME=$@

# could do this and prefix all plugin with _ in all
# _%:
# 	make compile LIBNAME=$*

compile:
	@mkdir -p $(OBJ_DIR)
	g++ -c -o $(OBJ_DIR)/$(LIBNAME).o audioPlugin.cpp -fPIC -DPLUGIN_NAME=$(LIBNAME) -DPLUGIN_INCLUDE=\"$(LIBNAME).h\"
	g++ -shared -o build/libzic_$(LIBNAME).so $(OBJ_DIR)/$(LIBNAME).o $(EXTRA)

clean:
	rm -rf $(BUILD_DIR)
