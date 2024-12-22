MAKEFLAGS += -j$(shell nproc)

ifneq ($(shell uname -m),x86_64)
SUB_DIR := arm
else
SUB_DIR := x86
endif
BUILD_DIR ?= build/$(SUB_DIR)
OBJ_DIR ?= $(BUILD_DIR)/obj

INC=-I../../..

all:
	make PadComponent EncoderComponent Encoder2Component GranularComponent\
				ButtonComponent WaveComponent Wave2Component MonitoringComponent AdsrComponent\
				SampleComponent GridSequencerComponent RectComponent ToggleComponent PlayComponent\
				TextComponent FmAlgoComponent NavBarComponent ButtonBarComponent\
				EncoderBarComponent GridMiniComponent EncoderBar2Component KeyInfoBarComponent

%Component:
	@mkdir -p $(OBJ_DIR)
	g++ -g -c -o $(OBJ_DIR)/$@.o plugin.cpp $(INC) -fPIC -DPLUGIN_NAME=$@ -DPLUGIN_INCLUDE=\"$@.h\"
	g++ -g -shared -o $(BUILD_DIR)/libzic_$@.so $(INC) $(OBJ_DIR)/$@.o

clean:
	rm -rf $(BUILD_DIR)
