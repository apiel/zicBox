#ifndef _MIDI_MAPPING_H_
#define _MIDI_MAPPING_H_

#include "plugins/audio/audioPlugin.h"
#include <vector>

class MidiMapping {
protected:
    AudioPlugin* plugin;
    int valueIndex;
    bool (MidiMapping::*handlePtr)(std::vector<unsigned char>* message);
    uint8_t size;
    uint8_t msg[2];

    bool handleUint8Position1(std::vector<unsigned char>* message)
    {
        plugin->getValue(valueIndex)->set(message->at(1) / 128.0f);
        return true;
    }

    bool handleUint8Position2(std::vector<unsigned char>* message)
    {
        if (msg[1] == message->at(1)) {
            plugin->getValue(valueIndex)->set(message->at(2) / 128.0f);
            return true;
        }
        return false;
    }

    bool handleUint16(std::vector<unsigned char>* message)
    {
        plugin->getValue(valueIndex)->set(((message->at(2) << 7) + message->at(1)) / 16383.0f);
        return true;
    }

public:
    MidiMapping(AudioPlugin* plugin, int valueIndex, uint8_t _size, uint8_t valuePosition, uint8_t _msg0, uint8_t _msg1)
        : plugin(plugin)
        , valueIndex(valueIndex)
        , size(_size)
    {
        msg[0] = _msg0;
        msg[1] = _msg1;
        if (valuePosition == _size) {
            if (size == 2) {
                handlePtr = &MidiMapping::handleUint8Position1;
            } else if (size == 3) {
                handlePtr = &MidiMapping::handleUint8Position2;
            }
        } else {
            handlePtr = &MidiMapping::handleUint16;
        }
    }

    bool handle(std::vector<unsigned char>* message)
    {
        if (message->size() != size) {
            return false;
        }
        if (msg[0] != message->at(0)) {
            return false;
        }
        return (this->*handlePtr)(message);
    }
};

#endif