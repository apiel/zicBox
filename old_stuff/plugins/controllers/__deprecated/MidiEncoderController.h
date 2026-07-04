/** Description:
This code defines the blueprint for a component known as the `MidiEncoderController`. Its core purpose is to connect to and interpret signals from physical MIDI devices, particularly rotary encoders (the continuous turning knobs often found on hardware controllers).

Think of this component as a specialized translator.

It uses a dedicated library to establish communication with any standard MIDI input device plugged into the computer. Upon starting, the controller can list all available devices and then attempts to load a specific device chosen by the user's configuration.

When a physical knob is turned, the MIDI device sends a digital message. The `MidiEncoderController` intercepts this message, instantly determines two things: which knob was turned, and whether it was turned clockwise or counter-clockwise.

The system maintains a list of virtual encoders, allowing the user to map specific physical MIDI signals to specific functions within the software. This setup ensures that regardless of the hardware used, the software receives a standardized, simple directional command (up or down). This directional output is then forwarded to the rest of the application for tasks like controlling volume, changing settings, or navigating menus.

sha: 29666e21f9adbc69aab0e657ca77bf7a510220e5a4febd96dc6a98c2e220e3f4 
*/
#ifndef _MIDI_ENCODER_CONTROLLER_H_
#define _MIDI_ENCODER_CONTROLLER_H_

#include <RtMidi.h>
#include <stdio.h>
#include <string.h>

#include "controllerInterface.h"

void midiHandler(double timeStamp, std::vector<unsigned char>* message, void* userData);

// For the moment everything is hardcoded, might want to make this more configurable...

class MidiEncoderController : public ControllerInterface {
protected:
    RtMidiIn midi;

    void list()
    {
        unsigned int portCount = midi.getPortCount();
        printf("List midi input devices:\n");
        for (unsigned int i = 0; i < portCount; i++) {
            printf("  - DEVICE=%s\n", midi.getPortName(i).c_str());
        }
    }

    void load(std::string portName)
    {
        if (portName == "") {
            printf("Midi input cannot be empty\n");
            return;
        }

        printf("Search for midi input: %s\n", portName.c_str());

        unsigned int portCount = midi.getPortCount();
        for (unsigned int i = 0; i < portCount; i++) {
            if (midi.getPortName(i).find(portName) != std::string::npos) {
                if (midi.isPortOpen()) {
                    midi.closePort();
                }
                midi.openPort(i);
                printf("Midi input loaded: %s\n", midi.getPortName(i).c_str());
                return;
            }
        }

        printf("Midi input %s not found\n", portName.c_str());
    }

public:
    struct Encoder {
        uint8_t value = 0;
        uint8_t encoderId = -1;
    } encoders[127];

    MidiEncoderController(Props& props, uint16_t id)
        : ControllerInterface(props, id)
    {
        midi.setCallback(midiHandler, this);
        midi.ignoreTypes(false, false, false);

        encoders[0].encoderId = 0;
        encoders[1].encoderId = 1;
        encoders[2].encoderId = 2;
        encoders[3].encoderId = 3;
        encoders[4].encoderId = 4;
        encoders[5].encoderId = 5;
        encoders[6].encoderId = 6;
        encoders[7].encoderId = 7;

        list();
    }

    bool config(char* key, char* value)
    {
        if (strcmp(key, "DEVICE") == 0) {
            load(value);
            return true;
        } else if (strcmp(key, "ENCODER_TARGET") == 0) {
            char* cc = strtok(value, " ");
            char* encoderId = strtok(NULL, " ");
            printf("ENCODER_TARGET: %s %s\n", cc, encoderId);
            encoders[atoi(cc)].encoderId = atoi(encoderId);
            return true;
        }
        return false;
    }
};

void midiHandler(double timeStamp, std::vector<unsigned char>* message, void* userData)
{
    MidiEncoderController* plugin = (MidiEncoderController*)userData;

    int8_t direction = 1;
    if (message->at(2) < plugin->encoders[message->at(1)].value || message->at(2) == 0) {
        direction = -1;
    }
    plugin->encoders[message->at(1)].value = message->at(2);

    // printf("Midi encoder message: %d %d\n", plugin->encoders[message->at(1)].encoderId, direction);
    plugin->encoder(plugin->encoders[message->at(1)].encoderId, direction, timeStamp);
}

#endif