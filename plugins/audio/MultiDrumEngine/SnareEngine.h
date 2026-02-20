#pragma once

#include "audio/engines/DrumSnare.h"
#include "plugins/audio/MultiEngine.h"

class SnareEngine : public MultiEngine {
protected:
    DrumSnare snare;

    Val& duration = val("DURATION", snare.duration);
    GraphPointFn ampGraph = [&](float index) { return *snare.envelopAmp.getMorphShape(index); };
    Val& ampMorph = val("AMP_MORPH", snare.ampEnv, ampGraph);
    Val& baseFrequency = val("FREQ", snare.baseFrequency);
    Val& bodyDecay = val("BODY_DECAY", snare.bodyDecay);
    Val& snappyLevel = val("SNAPPY", snare.snappyLevel);
    Val& snappyDecay = val("SNAPPY_DECAY", snare.snappyDecay);
    Val& snapTone = val("SNAP_TONE", snare.snapTone);
    Val& pitchDrop = val("PITCH_DROP", snare.pitchDrop);
    Val& impact = val("IMPACT", snare.impact);
    Val& drive = val("DRIVE", snare.drive);
    Val& tightness = val("TIGHTNESS", snare.tightness);
    Val& tone = val("TONE", snare.tone);

public:
    SnareEngine(AudioPlugin::Props& p, AudioPlugin::Config& c)
        : MultiEngine(p, c, "Snare")
        , snare(p.sampleRate)
    {
        initValues();
    }

    void noteOn(uint8_t note, float _velocity, void* = nullptr) override
    {
        snare.noteOn(note, _velocity);
    }

    void sample(float* buf) override
    {
        buf[track] = snare.sample();
    }
};