#pragma once

#include "audio/engines/DrumKickSegment.h"
#include "plugins/audio/MultiEngine.h"

class KickSegmentEngine : public MultiEngine {
protected:
    DrumKickSeg kick;

    // --- Main Parameters ---
    Val& duration = val("DURATION", kick.duration);
    Val& segment0 = val("SEG0", kick.seg0);
    Val& segment20 = val("SEG20", kick.seg20);
    Val& segment40 = val("SEG40", kick.seg40);
    Val& segment60 = val("SEG60", kick.seg60);

    Val& baseFrequency = val("FREQ", kick.baseFrequency);
    Val& pitchRange = val("RANGE", kick.pitchRange);
    Val& punchDrive = val("PUNCH_DRIVE", kick.punchDrive);
    Val& fmDirt = val("FM_DIRT", kick.fmDirt);
    Val& driveAmount = val("DRIVE", kick.driveAmount);
    Val& compressionAmount = val("COMP", kick.compressionAmount);
    Val& waveshapeAmount = val("WAVESHAPE", kick.waveshapeAmount);

public:
    KickSegmentEngine(AudioPlugin::Props& p, AudioPlugin::Config& c)
        : MultiEngine(p, c, "KickSeg")
        , kick(p.sampleRate)
    {
        initValues();
    }

    void noteOn(uint8_t note, float velocity, void* = nullptr) override
    {
        kick.noteOn(note, velocity);
    }

    void sample(float* buf) override
    {
        buf[track] = kick.sample();
    }
};