#pragma once

#include <cstdint>
#include <cstdlib>
#include <cmath>
#include <vector>
#include <algorithm>

class SequenceBrain {
public:
    // Clock parameters
    float bpm = 120.0f;
    float clockSpeedMultiplier = 1.0f; // For spacebar drop acceleration

    // Turing Machine
    uint16_t shiftRegister = 0xACE1; // Initial non-zero seed
    float chaos = 0.0f; // 0.0 to 1.0

    // Quantizer
    float quantize = 0.0f; // 0.0 (raw) to 1.0 (fully quantized acid scales)

    // Performance Drop
    bool spacebarHeld = false;

    // Running states
    double sampleRate = 44100.0;
    double sampleCounter = 0.0;
    uint32_t stepCounter = 0;

    // Trigger outputs for the current step
    bool triggerKick = false;
    bool triggerNoise = false;
    bool triggerAcid = false;
    float currentPitch = 0.0f; // Interpolated pitch value

    // Clock Dividers states
    bool gateDiv2 = false;
    bool gateDiv4 = false;
    bool gateDiv8 = false;
    bool gateDiv16 = false;
    bool gateDiv32 = false;

    SequenceBrain(double sr = 44100.0) : sampleRate(sr) {}

    // Helper to get closest MIDI note in a scale
    float quantizeToScale(float rawMidi, const std::vector<int>& scale) {
        int note = (int)std::round(rawMidi);
        int octave = note / 12;
        int noteInOctave = note % 12;
        if (noteInOctave < 0) noteInOctave += 12;

        int bestNote = scale[0];
        int minDiff = 999;
        for (int scaleNote : scale) {
            int diff = std::abs(scaleNote - noteInOctave);
            if (diff < minDiff) {
                minDiff = diff;
                bestNote = scaleNote;
            }
        }
        return (float)(octave * 12 + bestNote);
    }

    void tick() {
        stepCounter++;

        // Turing Machine Step
        bool lastBit = (shiftRegister & (1 << 15)) != 0;
        
        // Probability of flipping the bit
        float r = (float)rand() / (float)RAND_MAX;
        
        float currentChaos = chaos;
        if (r < currentChaos) {
            lastBit = !lastBit;
        }

        shiftRegister = (shiftRegister << 1) | (lastBit ? 1 : 0);

        // Derive gates from the Turing machine and clock dividers
        gateDiv2  = (stepCounter % 2 == 0);
        gateDiv4  = (stepCounter % 4 == 0);
        gateDiv8  = (stepCounter % 8 == 0);
        gateDiv16 = (stepCounter % 16 == 0);
        gateDiv32 = (stepCounter % 32 == 0);

        // Generate algorithmic triggers based on shift register bits
        // Kick on div4 or div8 depending on shift register bits
        triggerKick = gateDiv4 || ((shiftRegister & 0x01) && gateDiv8);
        
        // Noise (Hi-hat/Snare) triggered on off-beats or specific register bits
        triggerNoise = ((shiftRegister & 0x02) && gateDiv2) || ((shiftRegister & 0x04) && gateDiv8);

        // Acid/Drone note changes on clock div4 or div8
        triggerAcid = gateDiv4 || gateDiv8;

        // Melodic Generation:
        // Extract 8 bits from shift register for pitch
        float rawVal = (float)(shiftRegister & 0xFF) / 255.0f;
        // Map raw value to a MIDI note range: 36 (C2) to 72 (C5)
        float rawMidi = 36.0f + rawVal * 36.0f;

        if (quantize <= 0.05f) {
            // Completely unquantized
            currentPitch = rawMidi;
        } else {
            // Scales configuration
            std::vector<int> chromatic = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
            std::vector<int> minorPentatonic = {0, 3, 5, 7, 10};
            std::vector<int> acidBass = {0, 3, 5, 7, 8, 10}; // Aeolian / Acid friendly

            float qMidi = rawMidi;
            if (quantize < 0.4f) {
                // Chromatic
                qMidi = quantizeToScale(rawMidi, chromatic);
            } else if (quantize < 0.7f) {
                // Minor Pentatonic
                qMidi = quantizeToScale(rawMidi, minorPentatonic);
            } else {
                // Acid Bass
                qMidi = quantizeToScale(rawMidi, acidBass);
            }

            // Smooth interpolation between raw MIDI and quantized MIDI based on quantize knob
            currentPitch = rawMidi + (qMidi - rawMidi) * ((quantize - 0.05f) / 0.95f);
        }
    }

    // Process every audio sample, returns true if a tick occurred
    bool processSample() {
        // Calculate dynamic BPM
        float effectiveBpm = bpm;

        // Samples per step (assuming 16th notes / 4 ticks per beat)
        double samplesPerStep = (60.0 / effectiveBpm) * sampleRate / 4.0;

        sampleCounter += 1.0;
        if (sampleCounter >= samplesPerStep) {
            sampleCounter = 0.0;
            tick();
            return true;
        }
        return false;
    }
};
