#ifndef _HOST_CONFIG_H_
#define _HOST_CONFIG_H_

#include "../dustscript/dustscript.h"
#include "../helpers/getFullpath.h"
#include "def.h"
#include "midi.h"

void hostScriptCallback(char* key, char* value, const char* filename, uint8_t indentation)
{
    if (strcmp(key, "print") == 0) {
        printf(">> LOG: %s\n", value);
    } else if (strcmp(key, "INCLUDE") == 0) {
        char fullpath[512];
        getFullpath(value, filename, fullpath);
        DustScript::load(fullpath, hostScriptCallback);
    } else if (strcmp(key, "MIDIIN") == 0) {
        loadMidiInput(midiController, value, &midiControllerCallback);
    } else if (strcmp(key, "MIDIOUT") == 0) {
        // NOTE note used for the moments...
        loadMidiOutput(midiOut, value);
    } else if (strcmp(key, "AUDIO_OUTPUT") == 0) {
        strcpy(audioOutputName, value);
        APP_INFO("Audio output set: %s\n", audioOutputName);
    } else if (strcmp(key, "AUDIO_INPUT") == 0) {
        strcpy(audioInputName, value);
        APP_INFO("Audio input set: %s\n", audioInputName);
    } else if (strcmp(key, "AUDIO_PLUGIN") == 0) {
        AudioPluginHandler::get().loadPlugin(value);
    } else if (strcmp(key, "GAIN_OUTPUT") == 0) {
        // float gain = AudioPluginHandler::get().gainVolume.setGain(atof(value)).gain;
        // APP_INFO("Gain output set: %f\n", gain);
    } else if (strcmp(key, "DEBUG") == 0) {
        if (strcmp(value, "true") == 0) {
            enableDebug();
        }
    } else {
        AudioPluginHandler::get().config(key, value);
    }
}

void loadHostConfig(const char* filename)
{
    DustScript::load(filename, hostScriptCallback, { .variables = { { "IS_RPI",
#ifdef IS_RPI
                                                         "true"
#else
                                                         "false"
#endif
                                                     } } });
}

#endif