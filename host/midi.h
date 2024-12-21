#ifndef _MIDI_H_
#define _MIDI_H_

#include "AudioPluginHandler.h"
#include "def.h"

RtMidiIn midiController;
RtMidiOut midiOut;

void midiHandler(std::vector<unsigned char>* message)
{
    if (message->at(0) == 0xf8) {
        AudioPluginHandler::get().clockTick();
    } else if (message->at(0) == 0xfa) {
        AudioPluginHandler::get().start();
    } else if (message->at(0) == 0xfb) {
        AudioPluginHandler::get().pause();
    } else if (message->at(0) == 0xfc) {
        AudioPluginHandler::get().stop();
    } else if (message->at(0) == 0xfe) {
        // ignore active sensing
    } else if (message->at(0) >= 0x90 && message->at(0) < 0xa0) {
        uint8_t channel = message->at(0) - 0x90;
        AudioPluginHandler::get().noteOn(channel, message->at(1), message->at(2) / 127.0);
    } else if (message->at(0) >= 0x80 && message->at(0) < 0x90) {
        uint8_t channel = message->at(0) - 0x80;
        AudioPluginHandler::get().noteOff(channel, message->at(1), message->at(2) / 127.0);
    } else {
        if (AudioPluginHandler::get().midi(message)) {
            return;
        }

        debug("Midi controller message: ");
        unsigned int nBytes = message->size();
        for (unsigned int i = 0; i < nBytes; i++) {
            debug("%02x ", (int)message->at(i));
        }
        debug("\n");
    }
}

void midiControllerCallback(double deltatime, std::vector<unsigned char>* message, void* userData = NULL)
{
    midiHandler(message);
}

int getMidiDevice(RtMidi& midi, const char* portName)
{
    unsigned int portCount = midi.getPortCount();

    for (unsigned int i = 0; i < portCount; i++) {
        if (midi.getPortName(i).find(portName) != std::string::npos) {
            return i;
        }
    }
    return -1;
}

bool loadMidiInput(RtMidiIn& midi, const char* portName, RtMidiIn::RtMidiCallback callback)
{
    int port = getMidiDevice(midi, portName);
    if (port == -1) {
        APP_INFO("Midi input %s not found\n", portName);
        return false;
    }

    midi.openPort(port);
    midi.setCallback(callback);
    midi.ignoreTypes(false, false, false);
    APP_INFO("Midi input loaded: %s\n", midi.getPortName(port).c_str());
    return true;
}

bool loadMidiOutput(RtMidiOut& midi, const char* portName)
{
    int port = getMidiDevice(midi, portName);
    if (port == -1) {
        APP_INFO("Midi output %s not found\n", portName);
        return false;
    }

    midi.openPort(port);
    APP_INFO("Midi output loaded: %s\n", midi.getPortName(port).c_str());
    return true;
}

#endif