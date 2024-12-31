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
	+make TextGridComponent DrumEnvelopComponent Encoder3Component GraphEncoderComponent\
				StepEditComponent HiddenEncoderComponent FmAlgoComponent AdsrComponent\
				TextComponent SeqBarComponent SeqProgressBarComponent SeqSynthBarComponent ValueComponent\
				SpectrogramComponent RectComponent ClipsComponent

# %Component:
# 	@mkdir -p $(OBJ_DIR)
# 	g++ -g -c -o $(OBJ_DIR)/$@.o plugin.cpp $(INC) -fPIC -DPLUGIN_NAME=$@ -DPLUGIN_INCLUDE=\"$@.h\"
# 	g++ -g -shared -o $(BUILD_DIR)/libzic_$@.so $(INC) $(OBJ_DIR)/$@.o

%Component:
	make $(OBJ_DIR)/$@.o
	make $(BUILD_DIR)/libzic_$@.so

$(OBJ_DIR)/%.o: %.h ../component.h
	@mkdir -p $(OBJ_DIR)
	g++ -c -o $(OBJ_DIR)/$*.o plugin.cpp $(INC) -fPIC -DPLUGIN_NAME=$* -DPLUGIN_INCLUDE=\"$*.h\"

$(BUILD_DIR)/libzic_%.so: $(OBJ_DIR)/%.o
	@mkdir -p $(BUILD_DIR)
	g++ -shared -o $(BUILD_DIR)/libzic_$*.so $(INC) $(OBJ_DIR)/$*.o

rebuild: clean all

clean:
	rm -rf $(BUILD_DIR)
