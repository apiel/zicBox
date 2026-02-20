#pragma once

#include "audio/engines/DrumKickFM.h"
#include "plugins/audio/MultiEngine.h"

class KickFmEngine : public MultiEngine {
protected:
    DrumKickFM kick;

    // --- Parameter Mapping ---
    Val& duration = val("DURATION", kick.duration);
    GraphPointFn ampGraph = [&](float i) { return *kick.envelopAmp.getMorphShape(i); };
    Val& ampMorph = val("AMP_MORPH", kick.ampEnv, ampGraph);

    Val& baseFreq = val("FREQ", kick.baseFrequency);
    Val& pitchOffset = val("PITCH", kick.pitchOffset);

    Val& fmAmount = val("FM_AMOUNT", kick.fmAmount);
    Val& fmRatio = val("FM_RATIO", kick.fmRatio);
    Val& fmDecay = val("FM_DECAY", kick.fmDecay);
    Val& fmFeedback = val("FM_FEEDBACK", kick.fmFeedback);
    Val& fmCurve = val("FM_CURVE", kick.fmCurve);

    Val& punch = val("PUNCH", kick.punch);
    Val& drive = val("DRIVE", kick.drive);
    Val& tone = val("TONE", kick.tone);

public:
    KickFmEngine(AudioPlugin::Props& p, AudioPlugin::Config& c)
        : MultiEngine(p, c, "KickFM")
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