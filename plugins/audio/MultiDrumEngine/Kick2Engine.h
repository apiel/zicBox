#pragma once

#include "audio/engines/DrumKick2.h"
#include "plugins/audio/MultiEngine.h"

class Kick2Engine : public MultiEngine {
protected:
    DrumKick2 kick2;

    Val& duration = val("DURATION", kick2.duration);
    GraphPointFn ampGraph = [&](float index) { return *kick2.envelopAmp.getMorphShape(index); };
    Val& ampMorph = val("AMP_MORPH", kick2.ampEnv, ampGraph);
    Val& baseFrequency = val("FREQ", kick2.subFreq);
    Val& pitchOffset = val("PITCH", kick2.pitch);
    Val& sweepDepth = val("SWEEP_DEPTH", kick2.sweepDepth);
    Val& sweepSpeed = val("SWEEP_SPEED", kick2.sweepDepth);
    Val& symmetry = val("SYMMETRY", kick2.symmetry);
    Val& clickLevel = val("CLICK", kick2.click);
    Val& airNoise = val("NOISE", kick2.noise);
    Val& driveAmount = val("DRIVE", kick2.drive);
    Val& compressionAmount = val("COMP", kick2.compression);
    Val& toneCutoff = val("TONE", kick2.tone);

public:
    Kick2Engine(AudioPlugin::Props& p, AudioPlugin::Config& c)
        : MultiEngine(p, c, "Kick2")
        , kick2(p.sampleRate)
    {
        initValues();
    }

    void noteOn(uint8_t note, float _velocity, void* = nullptr) override
    {
        kick2.noteOn(note, _velocity);
    }

    void sample(float* buf) override
    {
        buf[track] = kick2.sample();
    }
};