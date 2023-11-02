#ifndef _HOST_CONFIG_H_
#define _HOST_CONFIG_H_

#include "def.h"
#include "midi.h"
#include "../dustscript/dustscript.h"

void hostScriptCallback(char* key, char* value, const char * filename, uint8_t indentation)
{
    if (strcmp(key, "MIDIIN") == 0) {
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
    } else if (strcmp(key, "MIDI_NOTE_CHANNEL") == 0) {
        midiNoteChannel = atoi(value);
        APP_INFO("Midi note channel set: %d\n", midiNoteChannel);
    } else if (strcmp(key, "DEBUG") == 0) {
        if (strcmp(value, "true") == 0) {
            enableDebug();
        }
    } else {
        AudioPluginHandler::get().config(key, value);
    }
}

void loadHostConfig()
{
    DustScript::load("./config.cfg", hostScriptCallback);
}

#endif