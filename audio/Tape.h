#pragma once

class Tape {
public:
    bool armed = false;
    bool recording = false;

    char file[24] = "tape_0.wav";
};