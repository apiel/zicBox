#pragma once

#include <cstdint>
#include <math.h>

const char* MIDI_NOTES_STR[132] = {
    // clang-format off
    "C-1", "C#-1", "D-1", "D#-1", "E-1", "F-1", "F#-1", "G-1", "G#-1", "A-1", "A#-1", "B-1",
    "C0", "C#0", "D0", "D#0", "E0", "F0", "F#0", "G0", "G#0", "A0", "A#0", "B0",
    "C1", "C#1", "D1", "D#1", "E1", "F1", "F#1", "G1", "G#1", "A1", "A#1", "B1",
    "C2", "C#2", "D2", "D#2", "E2", "F2", "F#2", "G2", "G#2", "A2", "A#2", "B2",
    "C3", "C#3", "D3", "D#3", "E3", "F3", "F#3", "G3", "G#3", "A3", "A#3", "B3",
    "C4", "C#4", "D4", "D#4", "E4", "F4", "F#4", "G4", "G#4", "A4", "A#4", "B4",
    "C5", "C#5", "D5", "D#5", "E5", "F5", "F#5", "G5", "G#5", "A5", "A#5", "B5",
    "C6", "C#6", "D6", "D#6", "E6", "F6", "F#6", "G6", "G#6", "A6", "A#6", "B6",
    "C7", "C#7", "D7", "D#7", "E7", "F7", "F#7", "G7", "G#7", "A7", "A#7", "B7",
    "C8", "C#8", "D8", "D#8", "E8", "F8", "F#8", "G8", "G#8", "A8", "A#8", "B8",
    "C9", "C#9", "D9", "D#9", "E9", "F9", "F#9", "G9", "G#9", "A9", "A#9", "B9",
    // clang-format on
};

const uint8_t MIDI_NOTE_C0 = 12;
const uint8_t MIDI_NOTE_C1 = 24;
const uint8_t MIDI_NOTE_C2 = 36;
const uint8_t MIDI_NOTE_C3 = 48;
const uint8_t MIDI_NOTE_C4 = 60;
const uint8_t MIDI_NOTE_C5 = 72;
const uint8_t MIDI_NOTE_C6 = 84;
const uint8_t MIDI_NOTE_C7 = 96;
const uint8_t MIDI_NOTE_C8 = 108;
const uint8_t MIDI_NOTE_C9 = 120;

uint8_t MIDI_NOTE_COUNT = sizeof(MIDI_NOTES_STR) / sizeof(MIDI_NOTES_STR[0]);

const uint8_t MIDI_LAST_NOTE = MIDI_NOTE_COUNT - 1;

bool isBlackKey(int midiNote) {
    // MIDI notes repeat every 12 notes (one octave)
    int noteInOctave = midiNote % 12;
    
    // Black keys in an octave (relative to C=0 in MIDI numbering)
    return noteInOctave == 1 || noteInOctave == 3 || noteInOctave == 6 || 
           noteInOctave == 8 || noteInOctave == 10;
}

float getMidiNoteFrequency(int midiNote, float refFreq = 220.0f, float refNote = 57.0f) {
    return refFreq * powf(2.0f, (midiNote - refNote) / 12.0f);
}