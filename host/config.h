#ifndef _HOST_CONFIG_H_
#define _HOST_CONFIG_H_

#include "../helpers/configPlugin.h"
#include "def.h"
#include "midi.h"

extern "C" void hostScriptCallback(char* key, char* value, const char* filename, std::vector<Var> variables)
{
    if (strcmp(key, "print") == 0) {
        printf(">> LOG: %s\n", value);
    } else if (strcmp(key, "MIDIIN") == 0) {
        loadMidiInput(midiController, value, &midiControllerCallback);
    } else if (strcmp(key, "MIDIOUT") == 0) {
        // NOTE note used for the moments...
        loadMidiOutput(midiOut, value);
    } else if (strcmp(key, "AUDIO_PLUGIN") == 0) {
        AudioPluginHandler::get().loadPlugin(value);
    } else if (strcmp(key, "AUTO_SAVE") == 0) {
        uint32_t msInterval = atoi(value);
        if (msInterval > 0) {
            AudioPluginHandler::get().startAutoSave(msInterval);
        }
    } else {
        AudioPluginHandler::get().config(key, value);
    }
}

#endif