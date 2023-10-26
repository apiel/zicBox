#ifndef _HOST_CONFIG_H_
#define _HOST_CONFIG_H_

#include "def.h"
#include "../helpers/fs.h"
#include "midi.h"

#define CONFIG_FILE "./config.cfg"

char* trimChar(char* str, char c = '\n')
{
    int len = strlen(str);
    for (int i = 0; i < len; i++) {
        if (str[i] == c) {
            str[i] = '\0';
            break;
        }
    }
    return str;
}

void assignKeyValue(char* key, char* value)
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

void parseConfigLine(char* line)
{
    // ignore comments and empty lines
    if (line[0] == '#' || line[0] == '\n') {
        return;
    }
    // split by '='
    char* key = strtok(line, "=");
    char* value = strtok(NULL, "=");
    if (key == NULL || value == NULL) {
        APP_INFO("Invalid config line: %s\n", line);
        return;
    }
    assignKeyValue(key, trimChar(value));
}

bool loadConfig()
{
    FILE* file = fopen(CONFIG_FILE, "r");
    if (file == NULL) {
        APP_INFO("Failed to load config file: %s\n", CONFIG_FILE);
        return false;
    }

    char line[256];

    while (fgets(line, sizeof(line), file)) {
        parseConfigLine(line);
    }
    fclose(file);

    return true;
}

#endif