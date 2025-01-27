# MAKEFLAGS += -j$(shell nproc)

ifneq ($(shell uname -m),x86_64)
SUB_DIR := arm
else
SUB_DIR := x86
endif
BUILD_DIR ?= build/$(SUB_DIR)
OBJ_DIR ?= $(BUILD_DIR)/obj
DEP_DIR ?= $(OBJ_DIR)/deps  # Directory for dependency files
# For raspberry std::filesystem is not found.
PARAMS ?= --std=c++17 -lstdc++fs

INC=-I../..

# track header file to be sure that build is automatically trigger if any dependency changes
TRACK_HEADER_FILES = -MMD -MF $(OBJ_DIR)/$*.d

all:
	make EffectGainVolume EffectSampleRateReducer EffectFilter EffectDistortion EffectDelay\
		EffectFilter EffectFilterMultiMode EffectFilterMultiMode2 EffectFilterMultiModeMoog\
		EffectGrain EffectDistortion2 EffectVolumeDrive EffectVolumeClipping \
		SynthKick23 SynthDrum23 SynthGranular SynthBass SynthFM SynthFM2 \
		SynthSample SynthDrumSample SynthMonoSample\
		SynthSnare SynthHiHat SynthClap SynthFmDrum SynthPerc\
		Sequencer Tempo AudioSpectrogram\
		Mixer2 Mixer4 Mixer5 Mixer6 Mixer8 Mixer12\
		AudioInputPulse AudioOutputPulse AudioInputAlsa AudioOutputAlsa\
		SerializeTrack TapeRecording RamTapeRecording EffectDrum SampleSequencer

# rebuild: clean all

rebuild:
	make clean
	make all

# For some weird reason need to paste the path of libpd.so 
# get libpd path `ldconfig -p | grep libpd`
# SynthPd:
# 	make compile LIBNAME=SynthPd EXTRA="$(shell pkg-config --cflags --libs pd) /lib/x86_64-linux-gnu/libpd.so"
# 
# FIXME: libpd is not available on RPi
SynthPd:
	make compile LIBNAME=SynthPd EXTRA="$(shell pkg-config --cflags --libs pd) $(shell ldconfig -p | grep libpd.so.0 | tr ' ' '\n' | grep /)"

SynthGranular:
	make compile LIBNAME=SynthGranular EXTRA="$(shell pkg-config --cflags --libs sndfile)"

SynthKick23:
	make compile LIBNAME=SynthKick23 EXTRA="$(shell pkg-config --cflags --libs sndfile)"

SynthDrum23:
	make compile LIBNAME=SynthDrum23 EXTRA="$(shell pkg-config --cflags --libs sndfile)"

SynthSample:
	make compile LIBNAME=SynthSample EXTRA="$(shell pkg-config --cflags --libs sndfile)"

SynthDrumSample:
	make compile LIBNAME=SynthDrumSample EXTRA="$(shell pkg-config --cflags --libs sndfile)"

SynthMonoSample:
	make compile LIBNAME=SynthMonoSample EXTRA="$(shell pkg-config --cflags --libs sndfile)"

SampleSequencer:
	make compile LIBNAME=SampleSequencer EXTRA="$(shell pkg-config --cflags --libs sndfile)"

TapeRecording:
	make compile LIBNAME=TapeRecording EXTRA="$(shell pkg-config --cflags --libs sndfile)"

RamTapeRecording:
	make compile LIBNAME=RamTapeRecording EXTRA="$(shell pkg-config --cflags --libs sndfile)"

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

# compile:
# 	@mkdir -p $(OBJ_DIR)
# 	g++ -c -o $(OBJ_DIR)/$(LIBNAME).o audioPlugin.cpp $(INC) -fPIC -DPLUGIN_NAME=$(LIBNAME) -DPLUGIN_INCLUDE=\"$(LIBNAME).h\" $(EXTRA) $(PARAMS)
#     g++ -shared -o $(BUILD_DIR)/libzic_$(LIBNAME).so $(INC) $(OBJ_DIR)/$(LIBNAME).o $(EXTRA) $(PARAMS)

compile:
	@echo "-------- :$(LIBNAME): --------"
	make $(OBJ_DIR)/$(LIBNAME).o EXTRA="$(EXTRA)"
	make $(BUILD_DIR)/libzic_$(LIBNAME).so

# add mapping.h and audioPlugin.h as dependency to watch

$(OBJ_DIR)/%.o:
	@mkdir -p $(OBJ_DIR)
	g++ -c -o $(OBJ_DIR)/$*.o audioPlugin.cpp $(INC) -fPIC -DPLUGIN_NAME=$* -DPLUGIN_INCLUDE=\"$*.h\" $(EXTRA) $(PARAMS) $(TRACK_HEADER_FILES)

$(BUILD_DIR)/libzic_%.so: $(OBJ_DIR)/%.o
	@mkdir -p $(BUILD_DIR)
	g++ -shared -o $(BUILD_DIR)/libzic_$*.so $(INC) $(OBJ_DIR)/$*.o $(EXTRA) $(PARAMS)

# Safeguard: include only if .d files exist
-include $(wildcard $(OBJ_DIR)/*.d)

clean:
	rm -rf $(BUILD_DIR)
