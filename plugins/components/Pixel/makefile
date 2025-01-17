MAKEFLAGS += -j$(shell nproc)

ifneq ($(shell uname -m),x86_64)
SUB_DIR := arm
else
SUB_DIR := x86
endif
BUILD_DIR ?= build/$(SUB_DIR)
OBJ_DIR ?= $(BUILD_DIR)/obj

INC=-I../../..

# track header file to be sure that build is automatically trigger if any dependency changes
TRACK_HEADER_FILES = -MMD -MF $(OBJ_DIR)/$*.d

all:
	+make TextGridComponent DrumEnvelopComponent KnobValueComponent GraphEncoderComponent\
				StepEditComponent HiddenValueComponent FmAlgoComponent AdsrComponent\
				TextComponent SeqBarComponent SeqProgressBarComponent SeqSynthBarComponent ValueComponent\
				SpectrogramComponent RectComponent ClipsComponent KeyboardComponent ListComponent\
				WorkspacesComponent SampleComponent TapeComponent StepEdit2Component\
				MacroEnvelopComponent

# %Component:
# 	@mkdir -p $(OBJ_DIR)
# 	g++ -g -c -o $(OBJ_DIR)/$@.o plugin.cpp $(INC) -fPIC -DPLUGIN_NAME=$@ -DPLUGIN_INCLUDE=\"$@.h\"
# 	g++ -g -shared -o $(BUILD_DIR)/libzic_$@.so $(INC) $(OBJ_DIR)/$@.o

%Component:
	@echo "-------- :$@: --------"
	make $(OBJ_DIR)/$@.o
	make $(BUILD_DIR)/libzic_$@.so

$(OBJ_DIR)/%.o:
	@mkdir -p $(OBJ_DIR)
	g++ -c -o $(OBJ_DIR)/$*.o plugin.cpp $(INC) -fPIC -DPLUGIN_NAME=$* -DPLUGIN_INCLUDE=\"$*.h\" $(TRACK_HEADER_FILES)

$(BUILD_DIR)/libzic_%.so: $(OBJ_DIR)/%.o
	@mkdir -p $(BUILD_DIR)
	g++ -shared -o $(BUILD_DIR)/libzic_$*.so $(INC) $(OBJ_DIR)/$*.o

# Safeguard: include only if .d files exist
-include $(wildcard $(OBJ_DIR)/*.d)

rebuild: clean all

clean:
	rm -rf $(BUILD_DIR)
