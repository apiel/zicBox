# MAKEFLAGS += -j$(shell nproc)

include ../../../make_common.mk
include ../../../makeconf/component.mk

BUILD_DIR := ../../../build/$(TARGET_PLATFORM)/libs/components
OBJ_DIR := ../../../build/obj/$(TARGET_PLATFORM)/libs/components

INC=-I../../..

# HTTP=-lcurl
HTTP=-lssl -lcrypto

# track header file to be sure that build is automatically trigger if any dependency changes
TRACK_HEADER_FILES = -MMD -MF $(OBJ_DIR)/$*.d

all:
	+make DrumEnvelopComponent KnobValueComponent GraphEncoderComponent\
				HiddenValueComponent FmAlgoComponent AdsrComponent\
				TextComponent SeqProgressBarComponent ValueComponent\
				RectComponent ClipsComponent KeyboardComponent Keyboard2Component ListComponent\
				WorkspacesComponent HiddenContextComponent KnobActionComponent\
				MacroEnvelopComponent StepEditSampleComponent NoteGridComponent\
				SequencerComponent SampleComponent SequencerCardComponent\
				SequencerValueComponent StringValComponent WorkspaceKnobComponent\
				GitHubComponent GhRepoComponent WifiComponent GraphValueComponent

GitHubComponent:
	@echo "-------- :$@: --------"
	make $(OBJ_DIR)/$@.o EXTRA="$(HTTP)"
	make $(BUILD_DIR)/libzic_$@.so EXTRA="$(HTTP)"

TapeComponent:
	@echo "-------- :$@: --------"
	make $(OBJ_DIR)/$@.o EXTRA="$(shell $(PKG_CONFIG) --cflags --libs sndfile)"
	make $(BUILD_DIR)/libzic_$@.so EXTRA="$(shell $(PKG_CONFIG) --cflags --libs sndfile)"

%Component:
	@echo "-------- :$@: --------"
	make $(OBJ_DIR)/$@.o
	make $(BUILD_DIR)/libzic_$@.so

$(OBJ_DIR)/%.o:
	@mkdir -p $(OBJ_DIR)
	$(CC) -c -o $(OBJ_DIR)/$*.o plugin.cpp $(INC) -fPIC -DPLUGIN_NAME=$* -DPLUGIN_INCLUDE=\"$*.h\" $(TRACK_HEADER_FILES) $(EXTRA)

$(BUILD_DIR)/libzic_%.so: $(OBJ_DIR)/%.o
	@mkdir -p $(BUILD_DIR)
	$(CC) -shared -o $(BUILD_DIR)/libzic_$*.so $(INC) $(OBJ_DIR)/$*.o $(EXTRA)

# Safeguard: include only if .d files exist
-include $(wildcard $(OBJ_DIR)/*.d)

rebuild: clean all

clean:
	rm -rf $(BUILD_DIR)
