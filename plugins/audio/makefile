# MAKEFLAGS += -j$(shell nproc)

include ../../make_common.mk
include ../../makeconf/audio.mk

BUILD_DIR := ../../build/$(TARGET_PLATFORM)/libs/audio
OBJ_DIR := ../../build/obj/$(TARGET_PLATFORM)/libs/audio
DEP_DIR := $(OBJ_DIR)/deps  # Directory for dependency files

# For raspberry std::filesystem is not found.
PARAMS := --std=c++17 -lstdc++fs $(RPI)

INC=-I../..

# track header file to be sure that build is automatically trigger if any dependency changes
TRACK_HEADER_FILES = -MMD -MF $(OBJ_DIR)/$*.d

all:
	make EffectGainVolume EffectSampleRateReducer EffectFilter EffectDistortion EffectDelay\
		EffectFilter EffectFilterMultiModeMix EffectFilterMultiModeMoog EffectVolumeMultiFx\
		EffectGrain EffectDistortion2 EffectVolumeDrive EffectVolumeClipping \
		SynthDrum23 SynthKick23 SynthMetalic SynthBass SynthFM2 SynthWavetable\
		SynthSample SynthDrumSample SynthMonoSample\
		Sequencer Tempo AudioSpectrogram\
		Mixer2 Mixer4 Mixer5 Mixer6 Mixer8 Mixer10 Mixer12\
		AudioInputAlsa AudioOutputAlsa AudioOutputAlsa_int16\
		AudioInputPulse AudioOutputPulse\
		SerializeTrack TapeRecording  SampleSequencer EffectFilterMultiMode\
		EffectScatter EffectFilteredMultiFx EffectBandIsolatorFx\
		SynthMulti SynthMultiDrum SynthMultiSample SynthLoop
		

# Pulse was removed from zicOs
# AudioInputPulse AudioOutputPulse 

# SynthSnare SynthHiHat SynthClap SynthFmDrum SynthHybrid SynthGranular SynthFM RamTapeRecording EffectDrum EffectFilterMultiMode2


# rebuild: clean all

rebuild:
	make clean
	make all

# For some weird reason need to paste the path of libpd.so 
# get libpd path `ldconfig -p | grep libpd`
# SynthPd:
# 	make compile LIBNAME=SynthPd EXTRA="$(shell $(PKG_CONFIG) --cflags --libs pd) /lib/x86_64-linux-gnu/libpd.so"
# 
# FIXME: libpd is not available on RPi
SynthPd:
	make compile LIBNAME=SynthPd EXTRA="$(shell $(PKG_CONFIG) --cflags --libs pd) $(shell ldconfig -p | grep libpd.so.0 | tr ' ' '\n' | grep /)"

SynthGranular:
	make compile LIBNAME=SynthGranular EXTRA="$(shell $(PKG_CONFIG) --cflags --libs sndfile)"

SynthLoop:
	make compile LIBNAME=SynthLoop EXTRA="$(shell $(PKG_CONFIG) --cflags --libs sndfile)"

SynthWavetable:
	make compile LIBNAME=SynthWavetable EXTRA="$(shell $(PKG_CONFIG) --cflags --libs sndfile)"

SynthDrum23:
	make compile LIBNAME=SynthDrum23 EXTRA="$(shell $(PKG_CONFIG) --cflags --libs sndfile)"

SynthKick23:
	make compile LIBNAME=SynthKick23 EXTRA="$(shell $(PKG_CONFIG) --cflags --libs sndfile)"

SynthBass:
	make compile LIBNAME=SynthBass EXTRA="$(shell $(PKG_CONFIG) --cflags --libs sndfile)"

SynthHybrid:
	make compile LIBNAME=SynthHybrid EXTRA="$(shell $(PKG_CONFIG) --cflags --libs sndfile)"

SynthSample:
	make compile LIBNAME=SynthSample EXTRA="$(shell $(PKG_CONFIG) --cflags --libs sndfile)"

SynthMultiDrum:
	make compile LIBNAME=SynthMultiDrum EXTRA="$(shell $(PKG_CONFIG) --cflags --libs sndfile)"

SynthMulti:
	make compile LIBNAME=SynthMulti EXTRA="$(shell $(PKG_CONFIG) --cflags --libs sndfile)"

SynthMultiSample:
	make compile LIBNAME=SynthMultiSample EXTRA="$(shell $(PKG_CONFIG) --cflags --libs sndfile)"

SynthDrumSample:
	make compile LIBNAME=SynthDrumSample EXTRA="$(shell $(PKG_CONFIG) --cflags --libs sndfile)"

SynthMonoSample:
	make compile LIBNAME=SynthMonoSample EXTRA="$(shell $(PKG_CONFIG) --cflags --libs sndfile)"

SampleSequencer:
	make compile LIBNAME=SampleSequencer EXTRA="$(shell $(PKG_CONFIG) --cflags --libs sndfile)"

TapeRecording:
	make compile LIBNAME=TapeRecording EXTRA="$(shell $(PKG_CONFIG) --cflags --libs sndfile)"

RamTapeRecording:
	make compile LIBNAME=RamTapeRecording EXTRA="$(shell $(PKG_CONFIG) --cflags --libs sndfile)"

AudioInputPulse:
	make compile LIBNAME=AudioInputPulse EXTRA="$(shell $(PKG_CONFIG) --cflags --libs libpulse-simple)"

AudioOutputPulse:
	make compile LIBNAME=AudioOutputPulse EXTRA="$(shell $(PKG_CONFIG) --cflags --libs libpulse-simple)"

AudioInputAlsa:
	make compile LIBNAME=AudioInputAlsa EXTRA="$(shell $(PKG_CONFIG) --cflags --libs alsa)"

AudioOutputAlsa:
	make compile LIBNAME=AudioOutputAlsa EXTRA="$(shell $(PKG_CONFIG) --cflags --libs alsa)"

AudioOutputAlsa_int16:
	make compile LIBNAME=AudioOutputAlsa_int16 EXTRA="$(shell $(PKG_CONFIG) --cflags --libs alsa)"

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
	$(CC) -c -o $(OBJ_DIR)/$*.o audioPlugin.cpp $(INC) -fPIC -DPLUGIN_NAME=$* -DPLUGIN_INCLUDE=\"$*.h\" $(EXTRA) $(PARAMS) $(TRACK_HEADER_FILES)

$(BUILD_DIR)/libzic_%.so: $(OBJ_DIR)/%.o
	@mkdir -p $(BUILD_DIR)
	$(CC) -shared -o $(BUILD_DIR)/libzic_$*.so $(INC) $(OBJ_DIR)/$*.o $(EXTRA) $(PARAMS)

# Safeguard: include only if .d files exist
-include $(wildcard $(OBJ_DIR)/*.d)

clean:
	rm -rf $(BUILD_DIR)
