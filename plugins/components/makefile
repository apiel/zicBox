BUILD_DIR ?= build
OBJ_DIR ?= $(BUILD_DIR)/obj

all:
	make PadComponent EncoderComponent Encoder2Component EncoderKnobComponent GranularComponent SequencerComponent\
				  ButtonComponent WaveComponent Wave2Component MonitoringComponent AdsrComponent\

%Component:
	@mkdir -p $(OBJ_DIR)
	g++ -g -c -o $(OBJ_DIR)/$@.o plugin.cpp -fPIC -DPLUGIN_NAME=$@ -DPLUGIN_INCLUDE=\"$@.h\"
	g++ -g -shared -o $(BUILD_DIR)/libzic_$@.so $(OBJ_DIR)/$@.o

clean:
	rm -rf $(BUILD_DIR)
